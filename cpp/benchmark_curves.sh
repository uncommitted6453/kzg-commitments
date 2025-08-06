#!/bin/bash

# KZG Commitments Curve Benchmarking Script
# This script benchmarks the performance of KZG commitments across different elliptic curves

set -e  # Exit on any error

# Define curve types
CURVES=("bn158" "bn254" "bls12381")

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to run benchmark for a specific curve
benchmark_curve() {
    local curve=$1
    
    print_status "Starting benchmark for curve: $curve"
    
    # Step 1: Clean the build
    print_status "Cleaning previous build..."
    make clean > /dev/null 2>&1
    
    # Step 2: Build the specific curve library
    print_status "Building KZG library for $curve..."
    make lib/kzg-$curve.a > /dev/null 2>&1
    
    # Step 3: Build benchmark linking against the specific curve
    print_status "Building benchmark for $curve..."
    make benchmark/benchmark-$curve > /dev/null 2>&1
    
    # Step 4: Run the benchmark
    print_status "Running benchmark for $curve..."
    echo ""
    echo "Results for $curve curve:"
    
    # Create benchmark_results directory if it doesn't exist
    mkdir -p benchmark_results
    
    # Set result file name
    result_file="benchmark_results/${curve}.txt"
    
    # Run the benchmark and capture output
    if ./benchmark/benchmark-$curve | tee "$result_file"; then
        echo ""
        print_success "Benchmark completed successfully for $curve"
        print_status "Results saved to: $result_file"
    else
        echo ""
        print_error "Benchmark failed for $curve"
        return 1
    fi
    
    echo ""
    echo "============================================="
    echo ""
}

# Main execution
main() {
    echo "kzg-commitments benchmarking"
    echo "========================================="
    echo "Benchmarking curves: ${CURVES[*]}"
    echo ""
    
    # Check if we're in the correct directory
    if [[ ! -f "Makefile" ]]; then
        print_error "Makefile not found. Please run this script from the cpp directory."
        exit 1
    fi
    
    # Run benchmarks for each curve
    for curve in "${CURVES[@]}"; do
        benchmark_curve "$curve"
    done
    
    print_success "All benchmarks completed!"
    
    # Show summary of saved files
    echo ""
    echo "Saved results:"
    for curve in "${CURVES[@]}"; do
        result_file="benchmark_results/${curve}.txt"
        if [[ -f "$result_file" ]]; then
            echo "- $curve: $result_file"
        fi
    done
}

# Handle command line arguments
if [[ $# -eq 1 ]]; then
    # Single curve benchmark
    curve=$1
    if [[ " ${CURVES[*]} " =~ " $curve " ]]; then
        benchmark_curve "$curve"
    else
        print_error "Invalid curve: $curve"
        print_status "Available curves: ${CURVES[*]}"
        exit 1
    fi
else
    # Run all benchmarks
    main
fi
