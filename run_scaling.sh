#!/bin/bash

# ---------------------------------------------------
# Directives section that requests specific resources
# ---------------------------------------------------

#SBATCH --job-name=mandelbrot_scaling
#SBATCH --time=0-5:00:00                 # Time requested: 2 hours should be enough
#SBATCH --mem=4G                         # Memory requested: 4 GB
#SBATCH --ntasks=64                      # Maximum number of MPI tasks
#SBATCH --partition=CPU                  # Specify partition name (CPU not compute)
#SBATCH --output=scaling_%j.out          # Standard output file
#SBATCH --error=scaling_%j.err           # Error messages file
#SBATCH --mail-type=END,FAIL             # Email when job ends or fails
#SBATCH --mail-user=adilet.akimshe@nu.edu.kz  # Your email address

# ---------------------------------------------------
# Your code section
# ---------------------------------------------------

# Load required modules
module load OpenMPI

# Print module information for debugging
echo "Loaded modules:"
module list
echo ""

# Print MPI information
echo "MPI information:"
which mpirun
mpirun --version
echo ""

# Ensure we're in the right directory
echo "Current directory: $(pwd)"
echo "Files in directory:"
ls -lh
echo ""

# Compile the program (in case it wasn't compiled or permissions were lost)
echo "Compiling program..."
make clean
make

if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed!"
    exit 1
fi

# Make executable explicitly
chmod +x mandelbrot_mpi mandelbrot_serial

# Verify executable
echo ""
echo "Checking executable:"
ls -lh mandelbrot_mpi
file mandelbrot_mpi
echo ""

# Test parameters - adjust these to get ~30-60 min runtime on 1 core
# These values should give approximately 40-50 minutes on 1 core
N_MAX=2000
NX=2400
NY=2000
N_FRAMES=120

echo "=========================================="
echo "Mandelbrot Scaling Test"
echo "=========================================="
echo "Parameters:"
echo "  n_max = $N_MAX"
echo "  Resolution = ${NX}x${NY}"
echo "  Frames = $N_FRAMES"
echo "  Center: -0.7436438870371587, 0.1318259042053120"
echo ""
echo "Starting scaling tests..."
echo ""

# Array of process counts to test
PROCESS_COUNTS=(1 2 4 8 16 32 64)

# Run tests for each process count
for np in "${PROCESS_COUNTS[@]}"; do
    echo "=========================================="
    echo "Running with $np MPI processes"
    echo "=========================================="
    echo ""
    
    # Run the test 3 times for reproducibility
    for run in 1 2 3; do
        echo "Run $run/3 with $np processes:"
        
        mpirun -np $np ./mandelbrot_mpi \
            $N_MAX \
            -0.7436438870371587 \
            0.1318259042053120 \
            $NX \
            $NY \
            0.001 \
            1.5 \
            $N_FRAMES \
            0
        
        echo ""
    done
    
    echo ""
done

echo "=========================================="
echo "All scaling tests completed!"
echo "=========================================="
echo ""
echo "Results summary:"
echo "Process counts tested: ${PROCESS_COUNTS[@]}"
echo "Runs per configuration: 3"
echo ""
echo "Check the output file for detailed timing results."
