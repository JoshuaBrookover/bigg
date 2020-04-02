@echo off
mkdir build
cd build
cmake ../../.. -DBIGG_ASSET_GEN=ON
cmake --build . --parallel --target bigg_assets
