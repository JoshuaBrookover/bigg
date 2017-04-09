@echo off
mkdir build
cd build
cmake ../../..
cmake --build . --target bigg_assets
