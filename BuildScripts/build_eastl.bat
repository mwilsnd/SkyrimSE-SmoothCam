@echo off
cd %2
set build_folder=%1
mkdir %build_folder%
pushd %build_folder%
call cmake ../ %3 -DEASTL_BUILD_TESTS:BOOL=OFF -DEASTL_BUILD_BENCHMARK:BOOL=OFF