cd vendor_git

cd glfw
cmake -DCMAKE_TOOLCHAIN_FILE=CMake/x86_64-w64-mingw32.cmake .
cmake .
make -j12
cp src/libglfw3.a ../../compiled_libs/windows

cd ..

cd glew
make clean
make extensions
make SYSTEM=linux-mingw64 glew.lib -j12 2>> /dev/null
cp lib/libglew32.a ../../compiled_libs/windows