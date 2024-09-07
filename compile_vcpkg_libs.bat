@echo off
@title Compiling vcpkg libraries
@color 0
vcpkg install glm:x64-windows
vcpkg install glfw3:x64-windows
vcpkg install glad[core,loader,extensions,gl-api-latest]:x64-windows
vcpkg install assimp:x64-windows
vcpkg install tinyfiledialogs:x64-windows
vcpkg install spdlog:x64-windows
vcpkg install freetype:x64-windows
pause