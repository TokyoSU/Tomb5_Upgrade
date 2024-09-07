@echo off
@title Compiling vcpkg libraries
@color 0
vcpkg remove glm:x64-windows
vcpkg remove glfw3:x64-windows
vcpkg remove glad[core,loader,extensions,gl-api-latest]:x64-windows
vcpkg remove assimp:x64-windows
vcpkg remove tinyfiledialogs:x64-windows
vcpkg remove spdlog:x64-windows
vcpkg remove freetype:x64-windows
pause