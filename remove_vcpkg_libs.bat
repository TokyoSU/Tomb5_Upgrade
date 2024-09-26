@echo off
@title Compiling vcpkg libraries
@color 0

vcpkg remove angelscript:x64-windows
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
vcpkg remove zlib:x64-windows

vcpkg remove angelscript:x86-windows
vcpkg remove sdl2:x86-windows
vcpkg remove sdl2-image:x86-windows
vcpkg remove sdl2-mixer-ext:x86-windows
vcpkg remove sdl2-net:x86-windows
vcpkg remove sdl2-ttf:x86-windows
vcpkg remove dxsdk-d3dx:x86-windows
vcpkg remove assimp:x86-windows
vcpkg remove tinyfiledialogs:x86-windows
vcpkg remove spdlog:x86-windows
vcpkg remove freetype:x86-windows
vcpkg remove zlib:x86-windows

pause