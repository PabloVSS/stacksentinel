#!/bin/bash

set -e

echo "== Building StackSentinel =="
gcc -fno-omit-frame-pointer -g src/*.c -o build/bin/sentinel

echo "== Building examples =="
gcc -fno-omit-frame-pointer -g examples/vulnerable.c -o build/bin/vulnerable
gcc -fno-omit-frame-pointer -g examples/safe.c -o build/bin/safe

echo ""
echo "===== SAFE EXECUTION ====="
echo "Type a normal input (<=16 chars)"
./build/bin/safe

echo ""
echo "===== STACK TRACE (SAFE CONTEXT) ====="
./build/bin/sentinel

echo ""
echo "===== VULNERABLE EXECUTION ====="
echo "Try a long input (e.g., 100+ chars)"
./build/bin/vulnerable

echo ""
echo "===== STACK TRACE (AFTER VULNERABLE CALL) ====="
./build/bin/sentinel

echo ""
echo "Demo finished."