#!/bin/sh

target_dir="build"

echo "Building netronix frame unit tests ..."


[ -d "$target_dir" ] && {
    echo "Removing old build directory ..."
    rm -rf "$target_dir"
}

mkdir "$target_dir"
cd "$target_dir"

echo "Building unit test at: $(pwd) ..."

cmake ..
make -j