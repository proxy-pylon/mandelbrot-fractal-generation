#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <cmath>
#include <string>
#include <iomanip>
#include <chrono>
#include <mpi.h>

// Color mapping function
struct RGB {
    unsigned char r, g, b;
};

RGB get_color(int iterations, int max_iterations) {
    if (iterations == max_iterations) {
        return {0, 0, 0};  // Black for points in the set
    }
    
    // Smooth coloring
    double t = static_cast<double>(iterations) / max_iterations;
    
    // Create a nice color gradient
    RGB color;
    color.r = static_cast<unsigned char>(9 * (1 - t) * t * t * t * 255);
    color.g = static_cast<unsigned char>(15 * (1 - t) * (1 - t) * t * t * 255);
    color.b = static_cast<unsigned char>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
    
    return color;
}

// Mandelbrot iteration count
int mandelbrot_iterations(double cx, double cy, int max_iter) {
    double zx = 0.0, zy = 0.0;
    int iter = 0;
    
    while (zx * zx + zy * zy <= 4.0 && iter < max_iter) {
        double temp = zx * zx - zy * zy + cx;
        zy = 2.0 * zx * zy + cy;
        zx = temp;
        iter++;
    }
    
    return iter;
}

// Write PPM file
void write_ppm(const std::string& filename, int width, int height, 
               const std::vector<RGB>& pixels) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    
    out << "P3\n" << width << " " << height << "\n255\n";
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const RGB& pixel = pixels[y * width + x];
            out << static_cast<int>(pixel.r) << ' '
                << static_cast<int>(pixel.g) << ' '
                << static_cast<int>(pixel.b) << ' ';
        }
        out << '\n';
    }
}

// Generate one frame
void generate_frame(int frame_num, int n_max, double x_center, double y_center,
                   int nx, int ny, double pixel_size, std::vector<RGB>& pixels) {
    // Calculate bounds
    double x_min = x_center - (nx / 2.0) * pixel_size;
    double y_min = y_center - (ny / 2.0) * pixel_size;
    
    // Compute each pixel
    for (int py = 0; py < ny; ++py) {
        for (int px = 0; px < nx; ++px) {
            double cx = x_min + px * pixel_size;
            double cy = y_min + py * pixel_size;
            
            int iterations = mandelbrot_iterations(cx, cy, n_max);
            pixels[py * nx + px] = get_color(iterations, n_max);
        }
    }
}

// Master process (rank 0)
void master_process(int n_max, double x_center, double y_center, int nx, int ny,
                   double init_pixel_size, double zoom_per_frame, int n_frames,
                   bool store_images, int num_workers) {
    
    std::cout << "Master: Distributing " << n_frames << " frames to " 
              << num_workers << " workers\n";
    
    int next_frame = 1;
    int completed_frames = 0;
    
    // Send initial work to all workers
    for (int worker = 1; worker <= num_workers && next_frame <= n_frames; ++worker) {
        MPI_Send(&next_frame, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
        next_frame++;
    }
    
    // Dynamic work distribution
    while (completed_frames < n_frames) {
        MPI_Status status;
        int frame_done;
        
        // Receive completion notification
        MPI_Recv(&frame_done, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
        int worker = status.MPI_SOURCE;
        completed_frames++;
        
        // Receive pixel data if storing images
        if (store_images) {
            std::vector<RGB> pixels(nx * ny);
            MPI_Recv(pixels.data(), nx * ny * 3, MPI_UNSIGNED_CHAR, 
                    worker, 2, MPI_COMM_WORLD, &status);
            
            // Save the image
            std::ostringstream filename;
            filename << "frame_" << std::setw(4) << std::setfill('0') << frame_done << ".ppm";
            write_ppm(filename.str(), nx, ny, pixels);
        }
        
        if (completed_frames % 10 == 0 || completed_frames == n_frames) {
            std::cout << "Progress: " << completed_frames << "/" << n_frames << " frames\n";
        }
        
        // Send next frame or termination signal
        if (next_frame <= n_frames) {
            MPI_Send(&next_frame, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
            next_frame++;
        } else {
            int terminate = -1;
            MPI_Send(&terminate, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
        }
    }
}

// Worker process
void worker_process(int n_max, double x_center, double y_center, int nx, int ny,
                   double init_pixel_size, double zoom_per_frame, bool store_images) {
    
    std::vector<RGB> pixels(nx * ny);
    
    while (true) {
        int frame_num;
        MPI_Status status;
        
        // Receive frame assignment
        MPI_Recv(&frame_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        
        // Check for termination signal
        if (frame_num == -1) break;
        
        // Calculate pixel size for this frame
        double current_pixel_size = init_pixel_size / std::pow(zoom_per_frame, frame_num - 1);
        
        // Generate the frame
        generate_frame(frame_num, n_max, x_center, y_center, nx, ny, 
                      current_pixel_size, pixels);
        
        // Send completion notification
        MPI_Send(&frame_num, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        
        // Send pixel data if storing images
        if (store_images) {
            MPI_Send(pixels.data(), nx * ny * 3, MPI_UNSIGNED_CHAR, 0, 2, MPI_COMM_WORLD);
        }
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 10) {
        if (rank == 0) {
            std::cerr << "Usage: " << argv[0] 
                      << " n_max x_center y_center N_x N_y init_pixel_size zoom_per_second n_frames store_images\n";
            std::cerr << "Example: mpirun -np 4 " << argv[0] 
                      << " 1000 -0.5 0.0 1920 1080 0.001 1.5 600 1\n";
        }
        MPI_Finalize();
        return 1;
    }
    
    int n_max = std::stoi(argv[1]);
    double x_center = std::stod(argv[2]);
    double y_center = std::stod(argv[3]);
    int nx = std::stoi(argv[4]);
    int ny = std::stoi(argv[5]);
    double init_pixel_size = std::stod(argv[6]);
    double zoom_per_second = std::stod(argv[7]);
    int n_frames = std::stoi(argv[8]);
    bool store_images = (std::stoi(argv[9]) != 0);
    
    double zoom_per_frame = std::pow(zoom_per_second, 1.0 / 60.0);
    
    if (rank == 0) {
        std::cout << "Mandelbrot Set Generator (MPI Parallel)\n";
        std::cout << "========================================\n";
        std::cout << "MPI processes: " << size << "\n";
        std::cout << "Max iterations: " << n_max << "\n";
        std::cout << "Center: (" << x_center << ", " << y_center << ")\n";
        std::cout << "Resolution: " << nx << " x " << ny << "\n";
        std::cout << "Initial pixel size: " << init_pixel_size << "\n";
        std::cout << "Zoom per second: " << zoom_per_second << "\n";
        std::cout << "Number of frames: " << n_frames << "\n";
        std::cout << "Store images: " << (store_images ? "yes" : "no") << "\n\n";
    }
    
    double start_time = MPI_Wtime();
    
    if (size == 1) {
        // Serial execution with single process
        std::vector<RGB> pixels(nx * ny);
        for (int frame = 1; frame <= n_frames; ++frame) {
            double current_pixel_size = init_pixel_size / std::pow(zoom_per_frame, frame - 1);
            generate_frame(frame, n_max, x_center, y_center, nx, ny, current_pixel_size, pixels);
            
            if (store_images) {
                std::ostringstream filename;
                filename << "frame_" << std::setw(4) << std::setfill('0') << frame << ".ppm";
                write_ppm(filename.str(), nx, ny, pixels);
            }
            
            if (frame % 10 == 0) {
                std::cout << "Progress: " << frame << "/" << n_frames << " frames\n";
            }
        }
    } else {
        // Parallel execution
        if (rank == 0) {
            master_process(n_max, x_center, y_center, nx, ny, init_pixel_size, 
                          zoom_per_frame, n_frames, store_images, size - 1);
        } else {
            worker_process(n_max, x_center, y_center, nx, ny, init_pixel_size, 
                          zoom_per_frame, store_images);
        }
    }
    
    double end_time = MPI_Wtime();
    
    if (rank == 0) {
        double elapsed = end_time - start_time;
        std::cout << "\nTotal time: " << elapsed << " seconds\n";
        std::cout << "Time per frame: " << elapsed / n_frames << " seconds\n";
    }
    
    MPI_Finalize();
    return 0;
}
