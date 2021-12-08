set "SDL2=H:\apps\code\libcurl\vcpkg\packages\sdl2_x86-windows"
set "SDL2_Image=H:\apps\code\libcurl\vcpkg\packages\sdl2-image_x86-windows"
if [%1]==[debug] build debug
if [%1]==[release] build release