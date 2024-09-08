@echo off
@title Compiling vcpkg libraries
@color 0
vcpkg remove sdl2:x64-windows
vcpkg remove sdl2-image:x64-windows
vcpkg remove sdl2-mixer-ext:x64-windows
vcpkg remove sdl2-net:x64-windows
vcpkg remove sdl2-ttf:x64-windows
vcpkg remove dxsdk-d3dx:x64-windows
vcpkg remove assimp:x64-windows
vcpkg remove tinyfiledialogs:x64-windows
vcpkg remove spdlog:x64-windows
vcpkg remove freetype:x64-windows
pause