#!/usr/bin/env bash
set -e # exit if any cmd returns non zero

ROOT_DIR="$(dirname "$0")"
BUILD_DIR="$ROOT_DIR/../build/test/bin"
BIN="$BUILD_DIR/all_tests"

usage() {
    echo "Usage:"
    echo "  $0                      # run all tests"
    echo "  $0 <group>              # run tests matching test group"
    echo "  $0 <group> <test case>  # run tests matching test case"
    echo ""
    echo "Examples:"
    echo "  $0                              # run all tests"
    echo "  $0 FAT32                        # run all cases in FAT32 group"
    echo "  $0 FAT32 OpenRootPrintAndClose  # runs single OpenRootPrintAndClose test case"
    exit 1
}

if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    usage
    exit 0
fi

make -C "$ROOT_DIR/.." test
echo "Finished building tests"

if [ ! -f "$BIN" ]; then
  echo "Error: Test binary not found at $BIN.. Building"
#   make -C "$ROOT_DIR/.." test
fi
if [ $# -eq 0 ]; then
    # Run all
    "$BIN"
fi
if [ $# -eq 1 ]; then
    # Group selected
    "$BIN" --gtest_filter="$1.*"
fi
if [ $# -eq 2 ]; then
    # Group and Test Case selected
    "$BIN" --gtest_filter="$1.$2"
fi