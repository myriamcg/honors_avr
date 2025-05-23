#!/bin/bash

# Set the working directory

# Copy current directory contents to /runner
# (In a script, you'll want to make sure the necessary files are already in /runner,
#  or you can use cp if copying from a specific source)
# Uncomment the following line if you need to copy files from a specific directory
# cp -r /path/to/source/. .

# Make scripts executable

# Run make in the src directory

# Set environment variables
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_SKIP_CPUFREQ=1
export CC=/runner/src/afl-cc
export CXX=/runner/src/afl-c++
export AFL_CC_COMPILER=LLVM
export AFL_LLVM_DICT2FILE=/runner/dict.txt
export AFL_NO_UI=1

# Optionally, print the environment variables to confirm they are set
echo "Environment variables set:"
echo "AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=$AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES"
echo "AFL_SKIP_CPUFREQ=$AFL_SKIP_CPUFREQ"
echo "CC=$CC"
echo "CXX=$CXX"
echo "AFL_CC_COMPILER=$AFL_CC_COMPILER"
echo "AFL_LLVM_DICT2FILE=$AFL_LLVM_DICT2FILE"
echo "AFL_NO_UI=$AFL_NO_UI"

