#!/usr/bin/env python3
"""
Parse scaling test results and generate plots for speedup and efficiency.
"""

import re
import matplotlib.pyplot as plt
import numpy as np

def parse_output_file(filename):
    """Parse the SLURM output file to extract timing data."""
    
    with open(filename, 'r') as f:
        content = f.read()
    
    # Pattern to match: "Running with X MPI processes" followed by timing info
    pattern = r'Running with (\d+) MPI processes.*?Total time: ([\d.]+) seconds'
    matches = re.findall(pattern, content, re.DOTALL)
    
    results = {}
    for num_procs, time_str in matches:
        num_procs = int(num_procs)
        time = float(time_str)
        
        if num_procs not in results:
            results[num_procs] = []
        results[num_procs].append(time)
    
    return results

def calculate_statistics(results):
    """Calculate mean and standard deviation for each process count."""
    
    stats = {}
    for num_procs, times in sorted(results.items()):
        times_array = np.array(times)
        stats[num_procs] = {
            'mean': np.mean(times_array),
            'std': np.std(times_array),
            'min': np.min(times_array),
            'max': np.max(times_array),
            'runs': len(times_array)
        }
    
    return stats

def print_statistics(stats):
    """Print statistics in a nice table format."""
    
    print("\nScaling Results")
    print("=" * 80)
    print(f"{'Processes':<12} {'Mean Time (s)':<15} {'Std Dev':<12} {'Speedup':<12} {'Efficiency':<12}")
    print("-" * 80)
    
    # Get baseline (single process) time
    baseline_time = stats[1]['mean']
    
    for num_procs in sorted(stats.keys()):
        mean_time = stats[num_procs]['mean']
        std_dev = stats[num_procs]['std']
        speedup = baseline_time / mean_time
        efficiency = speedup / num_procs * 100
        
        print(f"{num_procs:<12} {mean_time:<15.2f} {std_dev:<12.4f} {speedup:<12.2f} {efficiency:<12.1f}%")

def plot_results(stats, output_prefix='scaling'):
    """Generate speedup and efficiency plots."""
    
    process_counts = sorted(stats.keys())
    mean_times = [stats[p]['mean'] for p in process_counts]
    std_times = [stats[p]['std'] for p in process_counts]
    
    baseline_time = stats[1]['mean']
    speedups = [baseline_time / t for t in mean_times]
    efficiencies = [s / p * 100 for s, p in zip(speedups, process_counts)]
    
    # Ideal speedup line
    ideal_speedup = process_counts
    
    # Create figure with two subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Speedup plot
    ax1.plot(process_counts, speedups, 'bo-', label='Actual Speedup', linewidth=2, markersize=8)
    ax1.plot(process_counts, ideal_speedup, 'r--', label='Ideal Speedup', linewidth=2)
    ax1.set_xlabel('Number of MPI Processes', fontsize=12)
    ax1.set_ylabel('Speedup', fontsize=12)
    ax1.set_title('Parallel Speedup', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend(fontsize=10)
    ax1.set_xscale('log', base=2)
    ax1.set_yscale('log', base=2)
    
    # Efficiency plot
    ax2.plot(process_counts, efficiencies, 'go-', linewidth=2, markersize=8)
    ax2.axhline(y=100, color='r', linestyle='--', label='Ideal Efficiency (100%)')
    ax2.set_xlabel('Number of MPI Processes', fontsize=12)
    ax2.set_ylabel('Parallel Efficiency (%)', fontsize=12)
    ax2.set_title('Parallel Efficiency', fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=10)
    ax2.set_xscale('log', base=2)
    ax2.set_ylim([0, 110])
    
    plt.tight_layout()
    plt.savefig(f'{output_prefix}_plots.png', dpi=300, bbox_inches='tight')
    print(f"\nPlot saved as '{output_prefix}_plots.png'")
    plt.show()

if __name__ == '__main__':
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_results.py <slurm_output_file>")
        print("Example: python3 analyze_results.py scaling_12345.out")
        sys.exit(1)
    
    filename = sys.argv[1]
    
    print(f"Parsing results from: {filename}")
    results = parse_output_file(filename)
    
    if not results:
        print("No timing data found in the file!")
        sys.exit(1)
    
    stats = calculate_statistics(results)
    print_statistics(stats)
    
    plot_results(stats)
