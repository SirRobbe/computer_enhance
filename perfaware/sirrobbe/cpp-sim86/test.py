import os
import glob
import subprocess
from colorama import init, Fore, Style

def run_listing(name: str, listing_path: str):

    listing_name = listing_path.split('\\')[-1]
    disassembly_name = f"{listing_name}-disassembly"

    print(f"{Fore.CYAN}{name}{Style.RESET_ALL}")
    subprocess.call(f"nasm {listing_path}.asm")
    subprocess.call(f".\\build\\sim86_clang_debug.exe {listing_path}")

    with open(f"{disassembly_name}.asm", "w") as output:
        subprocess.call(f".\\build\\sim86_clang_debug.exe {listing_path}", stdout=output,
                        shell=True)

    subprocess.call(f"nasm {disassembly_name}.asm")

    print(f"{Fore.MAGENTA}")
    subprocess.call(f"fc {listing_path} {disassembly_name}")
    print(f"{Style.RESET_ALL}")


if __name__ == "__main__":

    init()

    print(f"{Fore.GREEN}deleting all old listing files{Style.RESET_ALL}")
    for file in glob.glob("listing*"):
        os.remove(file)

    print(f"{Fore.GREEN}building the decoder{Style.RESET_ALL}")
    subprocess.call("build.bat")

    print("")
    print(f"{Fore.GREEN}running all listings{Style.RESET_ALL}\n")

    run_listing("listing-0037", "..\\..\\part1\\listing_0037_single_register_mov")
    run_listing("listing-0038", "..\\..\\part1\\listing_0038_many_register_mov")
    run_listing("listing-0039", "..\\..\\part1\\listing_0039_more_movs")
