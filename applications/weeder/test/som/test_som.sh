#!/bin/bash

BIN_DIR=../../../../build/bin/

echo "*** Running the Python version of the algorithm ***"
time python3 test_som.py

echo 
echo "*** Running the C++ version of the algorithm ***"
time $BIN_DIR/r_regression_test_som -d

echo 
echo "*** Comparing the outputs ***"
$BIN_DIR/r_regression_test_som_compare test_som.out dump.out

echo 
echo "*** Cleaning up ***"
rm -f test_som.out dump.out

