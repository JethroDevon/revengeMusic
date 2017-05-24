mkdir build
cd build
cmake ..
msbuild revengeMusic.sln /p:Configuration=Debug
msbuild revengeMusic.sln /p:Configuration=Release
cd ..
