@echo off
mkdir build
cd build
cmake ../../.. -DBIGG_ASSET_GEN=ON
cmake --build . --target bigg_assets
