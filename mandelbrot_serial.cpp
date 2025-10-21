#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <cmath>
#include <string>
#include <iomanip>
#include <chrono>

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
                   int nx, int ny, double pixel_size, bool store_images) {
    std::vector<RGB> pixels(nx * ny);
    
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
    
    // Save image if requested
    if (store_images) {
        std::ostringstream filename;
        filename << "frame_" << std::setw(4) << std::setfill('0') << frame_num << ".ppm";
        write_ppm(filename.str(), nx, ny, pixels);
        std::cout << "Generated " << filename.str() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 10) {
        std::cerr << "Usage: " << argv[0] 
                  << " n_max x_center y_center N_x N_y init_pixel_size zoom_per_second n_frames store_images\n";
        std::cerr << "Example: " << argv[0] 
                  << " 1000 -0.5 0.0 1920 1080 0.001 1.5 600 1\n";
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
    
    std::cout << "Mandelbrot Set Generator (Serial)\n";
    std::cout << "==================================\n";
    std::cout << "Max iterations: " << n_max << "\n";
    std::cout << "Center: (" << x_center << ", " << y_center << ")\n";
    std::cout << "Resolution: " << nx << " x " << ny << "\n";
    std::cout << "Initial pixel size: " << init_pixel_size << "\n";
    std::cout << "Zoom per second: " << zoom_per_second << "\n";
    std::cout << "Number of frames: " << n_frames << "\n";
    std::cout << "Store images: " << (store_images ? "yes" : "no") << "\n\n";
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Generate frames with progressive zoom
    double zoom_per_frame = std::pow(zoom_per_second, 1.0 / 60.0);
    
    for (int frame = 1; frame <= n_frames; ++frame) {
        double current_pixel_size = init_pixel_size / std::pow(zoom_per_frame, frame - 1);
        
        generate_frame(frame, n_max, x_center, y_center, nx, ny, 
                      current_pixel_size, store_images);
        
        if (frame % 10 == 0) {
            std::cout << "Progress: " << frame << "/" << n_frames << " frames\n";
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end_time - start_time).count();
    
    std::cout << "\nTotal time: " << elapsed << " seconds\n";
    std::cout << "Time per frame: " << elapsed / n_frames << " seconds\n";
    
    return 0;
}
