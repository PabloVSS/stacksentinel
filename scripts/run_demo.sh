#!/bin/bash

set -e

echo "== Building StackSentinel =="
make

echo ""
echo "===== SAFE EXECUTION ====="
./build/bin/safe

echo ""
echo "===== STACK TRACE ====="
./build/bin/sentinel

echo ""
echo "===== VULNERABLE EXECUTION ====="
echo "Try long input (100+ chars)"
./build/bin/vulnerable

echo ""
echo "===== STACK TRACE AGAIN ====="
./build/bin/sentinel

echo ""
echo "Demo finished."