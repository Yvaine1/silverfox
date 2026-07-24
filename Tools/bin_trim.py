#!/usr/bin/env python3
import argparse
import os
import sys
import time


def parse_int(val):
    """
    Support decimal (1234) or hexadecimal (0x1234)
    """
    try:
        return int(val, 0)
    except ValueError:
        raise argparse.ArgumentTypeError(
            f"Invalid number format: {val}. Use decimal or hexadecimal (0x...)"
        )

def wait_for_file_size(file_path, required_size, max_wait=30, check_interval=1):
    """
    Wait until the file reaches the required size (or larger), with max wait limit
    :param file_path: Path to the target file
    :param required_size: Minimum required file size (bytes)
    :param max_wait: Maximum wait time in seconds (default: 30s)
    :param check_interval: Check interval in seconds (default: 1s)
    :return: None (will block until condition is met or timeout)
    :raise TimeoutError: If max wait time is exceeded
    """
    file_full_path = os.path.abspath(file_path)  # 标准化绝对路径，防止相对路径歧义
    file_name_only = os.path.basename(file_path) # 单独提取【纯文件名】（含后缀，如 test.bin）
    file_dir = os.path.dirname(file_full_path)   # 提取文件所在目录（可选打印）
    
    print(f"\n=== Waiting for file size ===")
    print(f"Target file PATH  : {file_full_path}")  # 打印【完整文件路径】
    print(f"Target file NAME  : {file_name_only}")  # 打印【具体文件名】（你核心需要的）
    print(f"Required size     : {required_size} bytes (0x{required_size:X})")
    print(f"Max wait time     : {max_wait}s")
    print(f"Check interval    : {check_interval}s")
    print("-----------------------------")

    start_time = time.time()
    elapsed_time = 0

    while True:
        # Calculate elapsed time
        elapsed_time = time.time() - start_time
        
        # Check if max wait time is exceeded
        if elapsed_time >= max_wait:
            current_size = os.path.getsize(file_path) if os.path.isfile(file_path) else 0
            raise TimeoutError(
                f"Timeout after {max_wait} seconds! "
                f"File size ({current_size} bytes) did not reach required size ({required_size} bytes)"
            )

        # Check if file exists first
        if not os.path.isfile(file_path):
            current_size = 0
            print(f"Elapsed: {elapsed_time:.1f}s | File not found yet (current: {current_size} bytes)")
        else:
            # Get current file size
            current_size = os.path.getsize(file_path)
            print(f"Elapsed: {elapsed_time:.1f}s | Current size: {current_size} bytes (0x{current_size:X})")
        
        # Check if size requirement is met
        if current_size >= required_size:
            print(f"\nFile size requirement met! (current: {current_size} bytes ≥ required: {required_size} bytes)")
            # Delay 3 seconds as requested before proceeding
            print(f"Delaying 3 seconds before processing...")
            time.sleep(3)
            break
        
        # Wait and check again
        time.sleep(check_interval)

def remove_data(infile, outfile, offset, length):
    # Calculate required minimum file size (offset + length)
    required_size = offset + length
    
    # Step 1: Wait for file to reach required size (max 30s)
    try:
        wait_for_file_size(infile, required_size)
    except TimeoutError as e:
        raise e  # Propagate timeout error to main
    
    # Step 2: Read file after size requirement is met
    with open(infile, 'rb') as f:
        data = f.read()

    filesize = len(data)
    if offset < 0 or length < 0:
        raise ValueError("offset and length must not be negative")

    if offset + length > filesize:
        raise ValueError(
            f"offset + length exceeds file size: "
            f"{hex(offset + length)} > {hex(filesize)}"
        )

    # Step 3: Remove specified range
    new_data = data[:offset] + data[offset + length:]

    # Step 4: Write output file
    with open(outfile, 'wb') as f:
        f.write(new_data)

    # Step 5: Print summary
    print("\n=== Binary cut completed ===")
    print(f"Input file       : {infile}")
    print(f"Output file      : {outfile}")
    print(f"Remove offset    : {hex(offset)} ({offset} bytes)")
    print(f"Remove length    : {hex(length)} ({length} bytes)")
    print(f"Original size    : {filesize} bytes (0x{filesize:X})")
    print(f"New file size    : {len(new_data)} bytes (0x{len(new_data):X})")
    print(f"Reduction        : {length} bytes (0x{length:X})")

def main():
    parser = argparse.ArgumentParser(
        prog="cut_bin.py",
        description=(
            "Remove a specified range (offset and length) from a binary (.bin) file.\n"
            "Automatically waits for the input file to reach the required size before processing.\n"
            "Max wait time: 30 seconds (timeout if not met).\n\n"
            "Typical use cases:\n"
            "  - Firmware trimming\n"
            "  - Removing reserved areas / headers / padding\n"
            "  - Binary post-processing for production\n"
            "  - Processing files that are still being generated/written"
        ),
        epilog=(
            "Examples:\n"
            "  python3 cut_bin.py -i input.bin -o output.bin --offset 0x1000 --length 0x200\n"
            "  python3 cut_bin.py -i fw.bin -o fw_new.bin --offset 4096 --length 512\n"
        ),
        formatter_class=argparse.RawTextHelpFormatter
    )

    parser.add_argument(
        "-i", "--input",
        required=True,
        metavar="FILE",
        help="Input binary file path (will wait if file is missing or too small)"
    )

    parser.add_argument(
        "-o", "--output",
        required=True,
        metavar="FILE",
        help="Output binary file path (will be created if it does not exist)"
    )

    parser.add_argument(
        "--offset",
        required=True,
        type=parse_int,
        metavar="OFFSET",
        help="Start offset to remove (decimal or hex with 0x prefix)"
    )

    parser.add_argument(
        "--length",
        required=True,
        type=parse_int,
        metavar="LENGTH",
        help="Length to remove (decimal or hex with 0x prefix)"
    )

    parser.add_argument(
        "--check-interval",
        type=int,
        default=1,
        metavar="SECONDS",
        help="Interval (seconds) to check file size (default: 1s)"
    )

    args = parser.parse_args()

    # Basic input validation
    if not args.check_interval > 0:
        print(f"Error: check-interval must be positive (got {args.check_interval})")
        sys.exit(1)

    try:
        remove_data(
            args.input,
            args.output,
            args.offset,
            args.length
        )
    except TimeoutError as e:
        print(f"\nCritical Error: {e}")
        sys.exit(1)  # Use different exit code for timeout
    except KeyboardInterrupt:
        print("\n\nOperation cancelled by user (Ctrl+C)")
        sys.exit(1)
    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
