import subprocess
import os
import sys
from typing import Tuple

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    RESET = '\033[0m'


CA53_DIR = r"..\CA53"  
MAKE_CMD = r"iarbuild.exe .\CA53.ewp -make Debug -parallel 4"
KEYWORD = "No change; not overwriting"
MAX_ATTEMPTS = 10


SCRIPT_DIR = os.path.abspath(os.path.dirname(__file__))
IMAGES_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "..", "Images"))
os.makedirs(IMAGES_DIR, exist_ok=True)

def print_header(text: str):
    print(f"\n{Colors.CYAN}{'='*60}{Colors.RESET}")
    print(f"{Colors.BOLD}{Colors.CYAN}{text.center(60)}{Colors.RESET}")
    print(f"{Colors.CYAN}{'='*60}{Colors.RESET}")

def print_step(text: str):
    print(f"{Colors.BLUE}[*] {text}{Colors.RESET}")

def print_success(text: str):
    print(f"{Colors.GREEN}[✓] {text}{Colors.RESET}")

def print_error(text: str):
    print(f"{Colors.RED}[✗] {text}{Colors.RESET}")

def print_warning(text: str):
    print(f"{Colors.YELLOW}[!] {text}{Colors.RESET}")

def run_command(cmd: str, cwd: str) -> Tuple[bool, str]:
    result = subprocess.run(cmd, shell=True, cwd=cwd,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    output = result.stdout + result.stderr
    return (result.returncode == 0, output)

def copy_ca53_files(project_dir: str) -> bool:
    """Copy CA53 bin and map to Images"""
    import shutil
    import glob

    bin_name = "CA53.bin"
    map_name = "CA53.map"

    exe_dir = os.path.join(project_dir, "Debug", "Exe")
    bin_path = os.path.join(exe_dir, bin_name)

    list_dir = os.path.join(project_dir, "Debug", "List")
    map_path = None
    if os.path.exists(list_dir):
        map_files = glob.glob(os.path.join(list_dir, "*.map"))
        if map_files:
            map_path = map_files[0]

    success = True
    if os.path.exists(bin_path):
        shutil.copy2(bin_path, os.path.join(IMAGES_DIR, bin_name))
        print_success(f"Copied {bin_name} to Images")
    else:
        print_error(f"{bin_name} not found in {exe_dir}")
        success = False

    if map_path:
        shutil.copy2(map_path, os.path.join(IMAGES_DIR, map_name))
        print_success(f"Copied {os.path.basename(map_path)} to Images as {map_name}")
    else:
        print_warning(f"No .map file found in {list_dir}")

    return success

def check_and_loop_make(initial_output: str) -> Tuple[bool, str]:
    original_dir = os.getcwd()
    try:
        os.chdir(CA53_DIR)

        print_header("Checking CA53 build output and looping make if needed")

        # 检查第一次 build 输出
        if KEYWORD in initial_output:
            print_success(f"Found '{KEYWORD}' in initial build output")
            copy_ca53_files(CA53_DIR)
            return True, "Initial build succeeded and keyword found"

        # 循环执行 make 直到找到关键词
        attempt = 1
        while attempt <= MAX_ATTEMPTS:
            print_step(f"[Attempt {attempt}/{MAX_ATTEMPTS}] Executing make command: {MAKE_CMD}")
            make_ok, make_output = run_command(MAKE_CMD, CA53_DIR)
            print(make_output)

            if not make_ok:
                print_error(f"Make attempt {attempt} failed")
                attempt += 1
                continue

            if KEYWORD in make_output:
                print_success(f"Found '{KEYWORD}' at attempt {attempt}")
                copy_ca53_files(CA53_DIR)
                return True, f"Keyword found after {attempt} make attempts"

            print_warning(f"Keyword not found, retrying...")
            attempt += 1

        return False, f"Max attempts ({MAX_ATTEMPTS}) reached without finding keyword"

    except Exception as e:
        return False, f"Error: {e}"
    finally:
        os.chdir(original_dir)

if __name__ == "__main__":
    initial_output = sys.stdin.read()
    if not initial_output:
        print_error("No initial build output received on stdin")
        sys.exit(1)

    success, message = check_and_loop_make(initial_output)
    print_header("RESULT")
    if success:
        print_success(message)
    else:
        print_error(message)
        sys.exit(1)