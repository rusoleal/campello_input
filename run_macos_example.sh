#!/bin/sh
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/examples/macos/build"

cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR/examples/macos"
cmake --build "$BUILD_DIR"

open "$BUILD_DIR/campello_input_macos_example.app"
