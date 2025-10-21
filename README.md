[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/CI7ybTzY)

# Mandelbrot Set with MPI - PHYS 421 Assignment 4

**Author:** Adilet Akimshe  
**Course:** PHYS 421 Parallel Computing  
**Date:** October 2025

This project implements both serial and parallel versions of a Mandelbrot set generator using MPI (Message Passing Interface). The parallel version uses a dynamic master-worker distribution strategy for optimal load balancing.

---

## Table of Contents

1. [Requirements](#requirements)
2. [Building the Project](#building-the-project)
3. [Running the Programs](#running-the-programs)
4. [Assignment Parts](#assignment-parts)
5. [Performance Testing](#performance-testing)
6. [Troubleshooting](#troubleshooting)

---

## Requirements

### Software Dependencies

- **C++ Compiler**: GCC 13.2+ or compatible
- **MPI Implementation**: OpenMPI 4.1+ or MPICH 3.0+
- **ffmpeg**: For video generation (optional but recommended)
- **ImageMagick**: For image format conversion (optional)
- **Python 3**: For data analysis (optional)

### On Shabyt Cluster

```bash
module load OpenMPI/5.0.7-GCC-14.2.0
```

### On Ubuntu/Debian

```bash
sudo apt install build-essential libopenmpi-dev openmpi-bin ffmpeg imagemagick
```

---

## Building the Project

### Clean Build

```bash
make clean
make
```

This produces two executables:
- `mandelbrot_serial` - Serial implementation
- `mandelbrot_mpi` - MPI parallel implementation

### Manual Compilation (if Makefile fails)

```bash
# Serial version
g++ -O3 -std=c++11 -Wall -o mandelbrot_serial mandelbrot_serial.cpp -lm

# MPI version
mpicxx -O3 -std=c++11 -Wall -o mandelbrot_mpi mandelbrot_mpi.cpp -lm
```

### Verify Executables

```bash
ls -lh mandelbrot_serial mandelbrot_mpi
file mandelbrot_mpi  # Should show "ELF 64-bit LSB executable"
```

---

## Running the Programs

### Command Syntax

Both programs use the same command-line interface:

```bash
./mandelbrot_serial <n_max> <x_center> <y_center> <N_x> <N_y> <init_pixel_size> <zoom_per_second> <n_frames> <store_images>
```

**Parameters:**
- `n_max` - Maximum iterations (e.g., 1000)
- `x_center` - X-coordinate of center (e.g., -0.5)
- `y_center` - Y-coordinate of center (e.g., 0.0)
- `N_x` - Image width in pixels (e.g., 1920)
- `N_y` - Image height in pixels (e.g., 1080)
- `init_pixel_size` - Initial pixel size (e.g., 0.001)
- `zoom_per_second` - Magnification per second (e.g., 1.5)
- `n_frames` - Number of frames to generate (e.g., 600)
- `store_images` - Save to disk: 1=yes, 0=no

### Quick Test

```bash
# Serial test (generates 5 frames, 800x600)
./mandelbrot_serial 100 -0.5 0 800 600 0.002 1.0 5 1

# MPI test with 4 processes
mpirun -np 4 ./mandelbrot_mpi 100 -0.5 0 800 600 0.002 1.0 10 1
```

---

## Assignment Parts

### Part A: High-Resolution Static Image

Generate a single 3000×2500 image of the full Mandelbrot set:

```bash
./mandelbrot_serial 1000 -0.5 0 3000 2500 0.001 1.0 1 1
```

**Output:** `frame_0001.ppm` (approximately 22 MB)

**Convert to PNG:**
```bash
convert frame_0001.ppm mandelbrot_full_set.png
```

**Expected time:** 1-3 minutes on a modern laptop

---

### Part C: Video Generation (600 Frames)

#### Step 1: Generate Frames

```bash
mpirun -np 8 ./mandelbrot_mpi \
    1000 \
    -0.7436438870371587 \
    0.1318259042053120 \
    1920 \
    1080 \
    0.001 \
    1.5 \
    600 \
    1
```

**Parameters explained:**
- Resolution: 1920×1080 (Full HD)
- Center: Interesting region in the Mandelbrot set
- Zoom: 1.5× magnification per second (50% per second)
- Frames: 600 frames = 10 seconds at 60 fps

**Expected time:** 20-45 minutes with 8 processes

**Output:** Files `frame_0001.ppm` through `frame_0600.ppm`

#### Step 2: Verify Frames

```bash
# Check number of frames
ls frame_*.ppm | wc -l
# Should output: 600

# Check file sizes
ls -lh frame_0001.ppm frame_0600.ppm
```

#### Step 3: Create Video

```bash
ffmpeg -framerate 60 -i frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p -crf 18 video.mp4
```

**Alternative quality settings:**

```bash
# Smaller file size (lower quality)
ffmpeg -framerate 60 -i frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p -crf 23 video.mp4

# Maximum quality (larger file)
ffmpeg -framerate 60 -i frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p -crf 15 -preset slow video.mp4
```

#### Step 4: Verify Video

```bash
# Check video properties
ffprobe video.mp4

# Play video
ffplay video.mp4
# or: vlc video.mp4
# or: mpv video.mp4
```

**Expected output:**
- Duration: 10 seconds
- Resolution: 1920×1080
- Frame rate: 60 fps
- File size: 5-20 MB (depending on CRF setting)

#### Optional: Create Preview GIF

```bash
ffmpeg -i video.mp4 -vf "fps=30,scale=320:-1:flags=lanczos" -loop 0 preview.gif
```

---

### Part D: Performance Analysis on Shabyt

#### Step 1: Upload Files to Shabyt

```bash
# From local machine
scp -r . your_username@shabyt.nu.edu.kz:~/mandelbrot_project/
```

#### Step 2: Connect and Setup

```bash
ssh your_username@shabyt.nu.edu.kz
cd mandelbrot_project

# Load modules
module load OpenMPI/5.0.7-GCC-14.2.0

# Compile
make clean
make

# Set permissions
chmod +x mandelbrot_mpi mandelbrot_serial
chmod +x run_scaling.sh
```

#### Step 3: Edit Batch Script

Edit `run_scaling.sh` to update your email:

```bash
nano run_scaling.sh
# Change: #SBATCH --mail-user=your.name@nu.edu.kz
```

Adjust parameters if needed:
```bash
N_MAX=2000      # Increase for longer runtime
NX=2400         # Image width
NY=2000         # Image height
N_FRAMES=120    # Number of frames
```

#### Step 4: Submit Job

```bash
sbatch run_scaling.sh
```

#### Step 5: Monitor Progress

```bash
# Check job status
squeue -u $USER

# Watch output in real-time
tail -f scaling_*.out

# Or check periodically
watch -n 30 'squeue -u $USER'
```

#### Step 6: Download Results

When job completes:

```bash
# From local machine
scp your_username@shabyt.nu.edu.kz:~/mandelbrot_project/scaling_*.out ./
```

#### Step 7: Analyze Results

```bash
# On local machine
python3 analyze_results.py scaling_XXXXX.out
```

**Output:**
- Timing table printed to terminal
- `scaling_plots.png` - Speedup and efficiency graphs

---

## Performance Testing

### Expected Results

Based on testing with parameters:
- n_max = 2000
- Resolution = 2400×2000
- Frames = 120

| Processes | Time (s) | Speedup | Efficiency (%) |
|-----------|----------|---------|----------------|
| 1         | 1863     | 1.00    | 100.0          |
| 2         | 1843     | 1.01    | 50.5           |
| 4         | 609      | 3.06    | 76.4           |
| 8         | 274      | 6.81    | 85.1           |
| 16        | 128      | 14.61   | 91.3           |
| 32        | 66       | 28.04   | 87.6           |

**Key observations:**
- Excellent scaling from 4 to 32 processes
- 2-process case shows overhead due to master-worker architecture
- Parallel efficiency remains above 85% even at 32 processes

---

## Troubleshooting

### Compilation Issues

**Error: `mpicxx: command not found`**
```bash
# Load MPI module
module load OpenMPI

# Or install locally
sudo apt install libopenmpi-dev openmpi-bin
```

**Error: Permission denied**
```bash
chmod +x mandelbrot_mpi mandelbrot_serial
```

### Runtime Issues

**Error: `prterun was unable to launch`**

Recompile on the compute node:
```bash
# In your batch script, add:
make clean
make
chmod +x mandelbrot_mpi
```

**Frames taking too long**

Reduce parameters:
```bash
# Use lower resolution or fewer frames
mpirun -np 4 ./mandelbrot_mpi 500 -0.5 0 1280 720 0.001 1.5 300 1
```

**Out of memory**

Reduce resolution:
```bash
mpirun -np 4 ./mandelbrot_mpi 1000 -0.5 0 1280 720 0.001 1.5 600 1
```

### Video Creation Issues

**Error: `ffmpeg: command not found`**
```bash
sudo apt install ffmpeg
```

**Video plays too fast/slow**

Adjust framerate:
```bash
# Slower (30 fps)
ffmpeg -framerate 30 -i frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p -crf 18 video.mp4

# Faster (120 fps)
ffmpeg -framerate 120 -i frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p -crf 18 video.mp4
```

**Video file too large**

Increase CRF (lower quality):
```bash
ffmpeg -framerate 60 -i frame_%04d.ppm -c:v libx264 -pix_fmt yuv420p -crf 28 video.mp4
```

---

## Project Structure

```
mandelbrot_project/
├── mandelbrot_serial.cpp     # Serial implementation
├── mandelbrot_mpi.cpp        # MPI parallel implementation
├── Makefile                  # Build configuration
├── README.md                 # This file
├── run_scaling.sh            # SLURM batch script for Shabyt
├── analyze_results.py        # Python analysis script
├── report_a4_akimshe.pdf     # Assignment report
└── video.mp4                 # Generated video (Part C)
```

---

## Algorithm Details

### Mandelbrot Iteration

For each pixel at position (x, y):
1. Convert to complex number: c = x + iy
2. Initialize: z₀ = 0
3. Iterate: z_{n+1} = z_n² + c
4. Count iterations until |z| > 2 or reach n_max
5. Map iteration count to color

### Parallelization Strategy

**Dynamic Master-Worker Pattern:**
- Master (rank 0): Distributes frame numbers, collects results, writes files
- Workers (rank > 0): Compute assigned frames, return results
- Load balancing: Workers request new frames upon completion
- Communication: MPI_Send/MPI_Recv for frame distribution

**Why frame-level parallelism?**
- Different frames have different costs (zoom level)
- Dynamic distribution handles load imbalance
- Minimal communication overhead
- Simple implementation

---

## Color Scheme

The implementation uses smooth color gradients:

```
t = iterations / n_max
R = 255 × 9(1-t)t³
G = 255 × 15(1-t)²t²
B = 255 × 8.5(1-t)³t
```

Points in the set (iterations = n_max) are colored black (0, 0, 0).

---

## Additional Resources

- **Assignment description:** Check course materials
- **MPI documentation:** https://www.open-mpi.org/doc/
- **ffmpeg documentation:** https://ffmpeg.org/documentation.html
- **Mandelbrot set (Wikipedia):** https://en.wikipedia.org/wiki/Mandelbrot_set

---

## Contact

For questions or issues related to this assignment, contact:
- **Instructor:** Prof. Sergiy Bubin
- **Course:** PHYS 421 Parallel Computing
- **Institution:** Nazarbayev University

---

## Acknowledgments

- PPM writer code snippets provided in course materials
- Assignment template by Prof. Sergiy Bubin and Prof. Bekdaulet Shukirgaliyev
- Claude 3.5 Sonnet for code development assistance
- Shabyt HPC cluster support team

---

## License

This code is submitted as part of PHYS 421 coursework at Nazarbayev University.
Academic integrity policies apply.