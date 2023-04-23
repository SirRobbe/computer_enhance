import os
import glob
import subprocess
from colorama import init, Fore, Style

def run_listing(name: str, listing_path: str, mode: str):

    listing_name = listing_path.split('\\')[-1]
    disassembly_name = f"{listing_name}-disassembly"

    print(f"{Fore.CYAN}{name}{Style.RESET_ALL}")
    subprocess.call(f"nasm {listing_path}.asm")
    subprocess.call(f".\\build\\sim86_clang_debug.exe {listing_path} {mode}")

    if mode == "--print":

        with open(f"{disassembly_name}.asm", "w") as output:
            subprocess.call(f".\\build\\sim86_clang_debug.exe {listing_path} {mode}",
                            stdout=output,
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

    run_listing("listing-0037", "..\\..\\part1\\listing_0037_single_register_mov", "--print")
    run_listing("listing-0038", "..\\..\\part1\\listing_0038_many_register_mov", "--print")
    run_listing("listing-0039", "..\\..\\part1\\listing_0039_more_movs", "--print")
    run_listing("listing-0040", "..\\..\\part1\\listing_0040_challenge_movs", "--print")
    run_listing("listing-0041", "..\\..\\part1\\listing_0041_add_sub_cmp_jnz", "--print")
    run_listing("listing-0043", "..\\..\\part1\\listing_0043_immediate_movs", "--sim")
    run_listing("listing-0044", "..\\..\\part1\\listing_0044_register_movs", "--sim")
    run_listing("listing-0046", "..\\..\\part1\\listing_0046_add_sub_cmp", "--sim")
    run_listing("listing-0048", "..\\..\\part1\\listing_0048_ip_register", "--sim")
    run_listing("listing-0049", "..\\..\\part1\\listing_0049_conditional_jumps", "--sim")
