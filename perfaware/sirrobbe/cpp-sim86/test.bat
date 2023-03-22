@echo off

echo running all listings

rem building the decoder
call build.bat

rem running all listings

echo.
echo listing-0037
call nasm ..\..\part1\listing_0037_single_register_mov.asm
copy ..\..\part1\listing_0037_single_register_mov listing_0037_single_register_mov
call .\build\sim86_clang_debug.exe listing_0037_single_register_mov
call .\build\sim86_clang_debug.exe listing_0037_single_register_mov > listing_0037_disassembly.asm
call nasm listing_0037_disassembly.asm
call fc listing_0037_single_register_mov listing_0037_disassembly

echo.
echo listing-0038
call nasm ..\..\part1\listing_0038_many_register_mov.asm
copy ..\..\part1\listing_0038_many_register_mov listing_0038_many_register_mov
call .\build\sim86_clang_debug.exe listing_0038_many_register_mov
call .\build\sim86_clang_debug.exe listing_0038_many_register_mov > listing_0038_disassembly.asm
call nasm listing_0038_disassembly.asm
call fc listing_0038_many_register_mov listing_0038_disassembly


rem echo listing-0039
rem nasm listing-0039.asm
rem decoder.exe listing-0039 > listing-0039-disassembly.asm
rem nasm listing-0039-disassembly.asm
rem fc listing-0039 listing-0039-disassembly

rem echo listing-0040
rem nasm listing-0040.asm
rem decoder.exe listing-0040 > listing-0040-disassembly.asm
rem nasm listing-0040-disassembly.asm
rem fc listing-0040 listing-0040-disassembly

rem echo listing-0041
rem nasm listing-0041.asm
rem decoder.exe listing-0041 > listing-0041-disassembly.asm
rem nasm listing-0041-disassembly.asm
rem fc listing-0041 listing-0041-disassembly