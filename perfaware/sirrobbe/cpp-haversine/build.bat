@echo off
IF NOT EXIST build mkdir build
pushd build

echo msvc debug
call cl -nologo -Zi -FC -EHsc -std:c++20 ..\main.cpp -Fehaversine_msvc_debug.exe

echo clang debug
call clang -g -fuse-ld=lld -std=c++20 ..\main.cpp -o haversine_clang_debug.exe

echo msvc release
call cl -O2 -nologo -Zi -EHsc -FC -std:c++20 ..\main.cpp -Fehaversine_msvc_release.exe

echo clang release
call clang -O3 -g -fuse-ld=lld -std=c++20 ..\main.cpp -o haversine_clang_release.exe
popd
