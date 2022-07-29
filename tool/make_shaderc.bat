
cd ../free/shaderc

mkdir build

cd build

"C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 14 2015" -A Win32 -DSHADERC_ENABLE_SHARED_CRT:INT=1  -DSHADERC_SKIP_TESTS=ON -DSHADERC_SKIP_INSTALL=ON "../" 

"C:\Program Files\CMake\bin\cmake.exe" --build . --config Debug
