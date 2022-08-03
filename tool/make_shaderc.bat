
cd ../free/shaderc

mkdir build64

cd build64

rem "C:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 14 2015" -A Win32 -DSHADERC_ENABLE_SHARED_CRT:INT=1  -DSHADERC_SKIP_TESTS=ON -DSHADERC_SKIP_INSTALL=ON "../" 
"%~dp0/cmake/bin/cmake.exe" -G "Visual Studio 17 2022" -DSHADERC_ENABLE_SHARED_CRT:INT=1  -DSHADERC_SKIP_TESTS=ON -DSHADERC_SKIP_INSTALL=ON "../" 

"%~dp0/cmake/bin/cmake.exe" --build . --config Debug

pause
