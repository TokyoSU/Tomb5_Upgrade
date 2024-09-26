@echo off
@title Compiling vcpkg libraries
@color 0

vcpkg install angelscript[core,addons]:x64-windows
vcpkg install sdl2:x64-windows
vcpkg install sdl2-image[core,libjpeg-turbo,libwebp,tiff]:x64-windows
vcpkg install sdl2-mixer-ext[core,ffmpeg,libflac,libvorbis,mpg123,wavpack,libgme]:x64-windows
vcpkg install sdl2-net:x64-windows
vcpkg install sdl2-ttf:x64-windows
vcpkg install dxsdk-d3dx:x64-windows
vcpkg install assimp:x64-windows
vcpkg install tinyfiledialogs:x64-windows
vcpkg install spdlog:x64-windows
vcpkg install freetype:x64-windows
vcpkg install zlib:x64-windows

vcpkg install angelscript[core,addons]:x86-windows
vcpkg install sdl2:x86-windows
vcpkg install sdl2-image[core,libjpeg-turbo,libwebp,tiff]:x86-windows
vcpkg install sdl2-mixer-ext[core,ffmpeg,libflac,libvorbis,mpg123,wavpack,libgme]:x86-windows
vcpkg install sdl2-net:x86-windows
vcpkg install sdl2-ttf:x86-windows
vcpkg install dxsdk-d3dx:x86-windows
vcpkg install assimp:x86-windows
vcpkg install tinyfiledialogs:x86-windows
vcpkg install spdlog:x86-windows
vcpkg install freetype:x86-windows
vcpkg install zlib:x86-windows

pause