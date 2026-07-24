#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
EMMC Image Packager Tool (MERGED VERSION: Build + Pack)
Simple tool to build module images with Git versioning + IAR Project Build
Usage: python mkimg.py [--list] [--all] [--build] [--boot] [--img] [-p module_type]
"""

import os
import struct
import zlib
import argparse
import sys
import subprocess
import re
import tarfile
import shutil
from datetime import datetime
from typing import Dict, List, Optional, Tuple

# ======================== 原build.py 完整代码段 START ========================
# ANSI color codes for better output visibility
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    RESET = '\033[0m'

# Project configurations from build.py
BUILD_PROJECTS: Dict[str, Dict] = {
    "FSBL": {
        "dir": r"..\FSBL", 
        "cmd": r"iarbuild.exe .\FSBL.ewp -build Debug",
        "clean_cmd": r"iarbuild.exe .\FSBL.ewp -clean Debug"
    },
    "CA53_BOOT": {
        "dir": r"..\CA53_BOOT", 
        "cmd": r"iarbuild.exe .\CA53_BOOT.ewp -build Debug",
        "clean_cmd": r"iarbuild.exe .\CA53_BOOT.ewp -clean Debug"
    },
    "CR5_0": {
        "dir": r"..\CR5_0", 
        "cmd": r"iarbuild.exe .\CR5_0.ewp -build Debug",
        "clean_cmd": r"iarbuild.exe .\CR5_0.ewp -clean Debug"
    },
    "CR5_1": {
        "dir": r"..\CR5_1", 
        "cmd": r"iarbuild.exe .\CR5_1.ewp -build Debug",
        "clean_cmd": r"iarbuild.exe .\CR5_1.ewp -clean Debug"
    }
}

# Special CA53 dual-build commands
CA53_COMMANDS = [
    r"iarbuild.exe .\CA53.ewp -build Debug -parallel 4"
]
CA53_CLEAN_COMMANDS = [
    r"iarbuild.exe .\CA53.ewp -clean Debug"
]

# Build Tool Path Config
BUILD_TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_IMAGES_DIR = os.path.normpath(os.path.join(BUILD_TOOLS_DIR, "..", "Images"))

# Build 模块映射关系 (mkimg的模块名 -> build的工程名)
MODULE_TO_BUILD_MAP = {
    "BOOT": ["FSBL", "CA53_BOOT"],
    "BOOT_PT": ["FSBL", "CA53_BOOT"],
    "CA53": ["CA53"],
    "CA53_PT": ["CA53"],
    "CR50": ["CR5_0"],
    "CR50_PT": ["CR5_0"],
    "CR51": ["CR5_1"],
    "CR51_PT": ["CR5_1"],
    "BIT28": [],  # BIT28是bit流文件，无需编译
    "BIT28_PT": []
}


BOOTGEN_SCRIPT = os.path.normpath(os.path.join(BUILD_TOOLS_DIR, "003_LNX_Auto_bootgen-fmzq.bat")) 

def print_header(text: str):
    print(f"\n{Colors.CYAN}{'='*60}{Colors.RESET}")
    print(f"{Colors.BOLD}{Colors.CYAN}{text.center(60)}{Colors.RESET}")
    print(f"{Colors.CYAN}{'='*60}{Colors.RESET}")

def print_step(text: str):
    print(f"\n{Colors.BLUE}[*] {text}{Colors.RESET}")

def print_success(text: str):
    print(f"{Colors.GREEN}[✓] {text}{Colors.RESET}")

def print_error(text: str):
    print(f"{Colors.RED}[✗] {text}{Colors.RESET}")

def print_warning(text: str):
    print(f"{Colors.YELLOW}[!] {text}{Colors.RESET}")

def print_status(module: str, success: bool, message: str = ""):
    status = f"{Colors.GREEN}PASS{Colors.RESET}" if success else f"{Colors.RED}FAIL{Colors.RESET}"
    module_str = f"{Colors.BOLD}{module:12}{Colors.RESET}"
    if message:
        print(f"  {module_str} {status} - {message}")
    else:
        print(f"  {module_str} {status}")

def clean_images() -> Tuple[bool, str]:
    """
    清空Images目录下的所有文件/子文件夹，保留目录本身 + 保留 .gitignore 文件
    """
    try:
        if not os.path.exists(BUILD_IMAGES_DIR):
            os.makedirs(BUILD_IMAGES_DIR)
            return True, f"Images目录不存在,已创建: {BUILD_IMAGES_DIR}"
        
        for filename in os.listdir(BUILD_IMAGES_DIR):
            if filename == ".gitignore":
                continue
                
            file_path = os.path.join(BUILD_IMAGES_DIR, filename)
            try:
                if os.path.isfile(file_path) or os.path.islink(file_path):
                    os.unlink(file_path)  # 删除文件/软链接
                elif os.path.isdir(file_path):
                    shutil.rmtree(file_path) # 删除子目录及内部所有内容
            except Exception as e:
                return False, f"删除文件失败 {file_path}: {str(e)}"
        
        print_success(f"成功清空 Images 目录所有文件: {BUILD_IMAGES_DIR}")
        return True, "Images目录清理完成"
    
    except Exception as e:
        return False, f"清理Images目录异常: {str(e)}"

def package_images_tar_gz(tag="V1.0.0") -> Tuple[bool, str]:
    """
    打包Images目录下 CA53/CR50/CR51相关文件 + 所有txt文件 为 tar.gz压缩包
    :param tag: 版本号标签，如 V1.0.0,最终生成 {tag}.tar.gz
    :return: 成功/失败状态 + 提示信息
    """
    import tarfile
    import glob
    tar_filename = f"{tag}.tar.gz"
    tar_filepath = os.path.join(BUILD_IMAGES_DIR, tar_filename)
    file_list = []
    
    try:
        # 1. 匹配规则：Images目录下 含CA53/CR50/CR51的所有文件
        key_word_patterns = [
            os.path.join(BUILD_IMAGES_DIR, "CA53*"),
            os.path.join(BUILD_IMAGES_DIR, "CR5*"),
        ]
        for pattern in key_word_patterns:
            file_list.extend(glob.glob(pattern))
            
        # 2. 匹配规则：Images目录下 所有 .info 后缀文件
        txt_pattern = os.path.join(BUILD_IMAGES_DIR, "*.info")
        file_list.extend(glob.glob(txt_pattern))

        # 去重+过滤无效文件
        file_list = list(set(file_list))
        file_list = [f for f in file_list if os.path.isfile(f) and os.path.exists(f)]
        
        if not file_list:
            return True, "未匹配到CA53/CR50/CR51相关文件及txt文件，跳过打包"

        # 3. 生成tar.gz压缩包 (gz高压缩，保留源文件)
        with tarfile.open(tar_filepath, "w:gz") as tar:
            for file_path in file_list:
                # 只把文件名加入压缩包，不包含上级目录，解压后纯净
                arcname = os.path.basename(file_path)
                tar.add(file_path, arcname=arcname)

        return True, f"成功打包tar.gz: {tar_filename} | 共打包 {len(file_list)} 个文件"
    
    except Exception as e:
        return False, f"打包tar.gz失败: {str(e)}"

def generate_boot_bin() -> Tuple[bool, str]:
    """
    调用003_LNX_Auto_bootgen脚本生成BOOT.bin
    """
    original_dir = os.getcwd()
    try:
        # 切换到Images目录执行（确保生成的BOOT.bin在正确路径）
        os.chdir(BUILD_IMAGES_DIR)
        
        # 检查bootgen脚本是否存在
        if not os.path.exists(BOOTGEN_SCRIPT):
            return False, f"Bootgen脚本不存在: {BOOTGEN_SCRIPT}"
        
        print_step(f"执行Bootgen脚本生成BOOT.bin: {BOOTGEN_SCRIPT}")
        # 执行bootgen脚本
        result = subprocess.run(
            BOOTGEN_SCRIPT, 
            shell=True, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE, 
            text=True
        )
        
        # 打印脚本输出
        if result.stdout:
            print(f"Bootgen输出:\n{result.stdout}")
        if result.stderr:
            print_warning(f"Bootgen警告:\n{result.stderr}")
        
        # 检查返回码和BOOT.bin是否生成
        if result.returncode != 0:
            return False, f"Bootgen执行失败，返回码: {result.returncode}"
        
        boot_bin_path = os.path.join(BUILD_IMAGES_DIR, "BOOT.bin")
        if not os.path.exists(boot_bin_path):
            return False, f"Bootgen执行成功，但未生成BOOT.bin: {boot_bin_path}"
        
        print_success(f"BOOT.bin生成成功: {boot_bin_path} (大小: {os.path.getsize(boot_bin_path)} 字节)")
        return True, "BOOT.bin生成完成"
    
    except Exception as e:
        return False, f"生成BOOT.bin异常: {str(e)}"
    finally:
        os.chdir(original_dir)


def copy_files_after_build(project_name: str, project_dir: str) -> bool:
    """
    Perform file copy operations after successful build
    """
    original_dir = os.getcwd()
    
    print_header(f"Executing post-build file copy - {project_name}")
    
    try:
        os.chdir(project_dir)
        current_dir = os.getcwd()
        print_step(f"Current directory: {current_dir}")
        print_step(f"Target Images directory: {IMAGES_DIR}")
        
        # Define target filenames based on project
        if project_name == "CA53":
            bin_name = "CA53.bin"
            map_name = "CA53.map"
        elif project_name == "CR5_0":
            bin_name = "CR50.bin"
            map_name = "CR5_0.map"
        elif project_name == "CR5_1":
            bin_name = "CR51.bin"
            map_name = "CR5_1.map"
        else:
            print_error(f"Copy not supported for {project_name}")
            return False
        
        all_success = True
        copied_files = []
        
        # 1. Copy .bin file
        print_step(f"Searching for {bin_name}")
        exe_dir = os.path.join(current_dir, "Debug", "Exe")
        bin_path = os.path.join(exe_dir, bin_name)
        
        if os.path.exists(bin_path):
            target_file = os.path.join(IMAGES_DIR, bin_name)
            print_success(f"Found source file: {bin_name}")
            print_step(f"Copying to: {target_file}")
            
            try:
                shutil.copy2(bin_path, target_file)
                print_success(f"Successfully copied {bin_name}")
                copied_files.append(bin_name)
            except Exception as e:
                print_error(f"Copy failed: {e}")
                all_success = False
        else:
            print_error(f"{bin_name} not found")
            print_step(f"Search path: {bin_path}")
            if os.path.exists(exe_dir):
                print_step(f"Files in Debug/Exe directory:")
                for file in os.listdir(exe_dir):
                    print(f"    - {file}")
            else:
                print_error(f"Directory does not exist: {exe_dir}")
            all_success = False
        
        # 2. Copy .map file
        print_step(f"Searching for {project_name}.map")
        list_dir = os.path.join(current_dir, "Debug", "List")
        if os.path.exists(list_dir):
            possible_map_patterns = [
                map_name,
                f"{project_name}.map",
                "*.map",
            ]
            
            source_file = None
            for pattern in possible_map_patterns:
                if "*" in pattern:
                    search_pattern = os.path.join(list_dir, pattern)
                    files = glob.glob(search_pattern)
                    if files:
                        source_file = files[0]
                        break
                else:
                    file_path = os.path.join(list_dir, pattern)
                    if os.path.exists(file_path):
                        source_file = file_path
                        break
            
            if source_file:
                target_file = os.path.join(IMAGES_DIR, map_name)
                print_success(f"Found source file: {os.path.basename(source_file)}")
                print_step(f"Copying to: {target_file}")
                
                try:
                    shutil.copy2(source_file, target_file)
                    print_success(f"Successfully copied {os.path.basename(source_file)} -> {map_name}")
                    copied_files.append(map_name)
                except Exception as e:
                    print_error(f"Copy failed: {e}")
                    all_success = False
            else:
                print_warning(f"No .map file found in {list_dir}")
                if os.path.exists(list_dir):
                    print_step(f"Files in Debug/List directory:")
                    for file in os.listdir(list_dir):
                        print(f"    - {file}")
        else:
            print_error(f"Directory does not exist: {list_dir}")
            all_success = False
        
        # Show summary
        print_header("Copy Summary")
        if copied_files:
            print_success("Successfully copied files to Images:")
            for file in copied_files:
                print(f"    - {os.path.join(IMAGES_DIR, file)}")
        
        print_step(f"Images directory contents:")
        for file in sorted(os.listdir(IMAGES_DIR)):
            print(f"    - {file}")
        
        return all_success
            
    except Exception as e:
        print_error(f"Error during file copy: {e}")
        return False
    finally:
        os.chdir(original_dir)

def clean_project(project_name: str) -> Tuple[bool, str]:
    original_dir = os.getcwd()
    if project_name == "CA53":
        return clean_ca53()
    if project_name not in BUILD_PROJECTS:
        return False, f"Project '{project_name}' not found"
    project = BUILD_PROJECTS[project_name]
    try:
        os.chdir(project["dir"])
        result = subprocess.run(project["clean_cmd"], shell=True)
        if result.returncode == 0:
            return True, f"Clean successful"
        else:
            return False, f"Clean failed, return code: {result.returncode}"
    except Exception as e:
        return False, f"Error: {e}"
    finally:
        os.chdir(original_dir)

def clean_ca53() -> Tuple[bool, str]:
    original_dir = os.getcwd()
    ca53_dir = r"..\CA53"
    try:
        os.chdir(ca53_dir)
        result = subprocess.run(CA53_CLEAN_COMMANDS[0], shell=True)
        if result.returncode == 0:
            return True, "Clean completed successfully"
        else:
            return False, f"Clean failed, return code: {result.returncode}"
    except Exception as e:
        return False, f"Error: {e}"
    finally:
        os.chdir(original_dir)

def build_ca53(clean_first: bool = True) -> Tuple[bool, str]:
    original_dir = os.getcwd()
    ca53_dir = r"..\CA53"
        # Auto-clean before building
    if clean_first:
        clean_success, clean_msg = clean_ca53()
        if not clean_success:
            print_warning(f"Clean had issues: {clean_msg}")
    
    print_header("Building CA53 (two-step build)")
    
    try:
        os.chdir(ca53_dir)
        print_step(f"Directory: {os.getcwd()}")
        
        # Execute first CA53 build command
        print_step(f"[Step 1/2] {CA53_COMMANDS[0]}")

        result1 = subprocess.run(
            CA53_COMMANDS[0],
            shell=True,
            cwd=r"..\CA53",
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        build_output = result1.stdout + result1.stderr
        print(build_output)

        if result1.returncode != 0:
            return False, f"-build failed, return code: {result1.returncode}"

        loop_script = os.path.join(os.path.dirname(__file__), "ca53_loop_make.py")
        loop_result = subprocess.run(
            [sys.executable, loop_script],
            input=build_output,
            text=True
        )

        if loop_result.returncode == 0:
            return True, "CA53 build and loop make completed successfully"
        else:
            return False, "CA53 loop make failed"

    except Exception as e:
        return False, f"Error: {e}"
    finally:
        os.chdir(original_dir)

def build_project(project_name: str, clean_first: bool = True) -> Tuple[bool, str]:
    """Build a specific project with auto-clean before build"""
    original_dir = os.getcwd()
    
    if project_name == "CA53":
        return build_ca53(clean_first)
    
    if project_name not in BUILD_PROJECTS:
        return False, f"Project '{project_name}' not found"
    
    project = BUILD_PROJECTS[project_name]
    
    # Auto-clean before building
    if clean_first:
        clean_success, clean_msg = clean_project(project_name)
        if not clean_success:
            print_warning(f"Clean had issues: {clean_msg}")
    
    print_header(f"Building {project_name}")
    
    try:
        os.chdir(project["dir"])
        print_step(f"Directory: {os.getcwd()}")
        print_step(f"Command: {project['cmd']}")
        
        result = subprocess.run(project["cmd"], shell=True)
        
        if result.returncode == 0:
            msg = f"Build successful"
            
            # Auto-copy files to Images
            if project_name in ["CA53", "CR5_0", "CR5_1"]:
                copy_success = copy_files_after_build(project_name, project["dir"])
                if not copy_success:
                    msg = f"Build successful but file copy failed"
                    return False, msg
            return True, msg
        else:
            if project_name in ["CR5_0", "CR5_1"]:
                print_warning(f"{project_name} Build failed, executing bin_trim.py...")
                exe_dir = os.path.join(os.getcwd(), "Debug", "Exe")
                
                if project_name == "CR5_0":
                    infile = os.path.join(exe_dir, "CR5_0.bin")
                    outfile = os.path.join(exe_dir, "CR50.bin")
                    length = "0x8ff0000"
                else:  # CR5_1
                    infile = os.path.join(exe_dir, "CR5_1.bin")
                    outfile = os.path.join(exe_dir, "CR51.bin")
                    length = "0x10ff0000"
                
                bin_trim_path = os.path.join(BUILD_TOOLS_DIR, "bin_trim.py")
                trim_cmd = (
                    f'python "{bin_trim_path}" -i "{infile}" -o "{outfile}" '
                    f'--offset 0x10000 --length {length}'
                )
                print_step(f"Executing: {trim_cmd}")
                trim_result = subprocess.run(trim_cmd, shell=True)
                
                if trim_result.returncode == 0:
                    print_success(f"bin_trim.py executed successfully")
                    if project_name in ["CR5_0", "CR5_1"]:
                        copy_success = copy_files_after_build(project_name, project["dir"])
                        if not copy_success:
                            print_error(f"Build successful but file copy failed")
                    print_success(f"Copied {os.path.basename(outfile)} to Images")
                    return True, f"Build successful"
                else:
                    print_error(f"bin_trim.py failed, return code: {trim_result.returncode}")
            
            return False, f"Build failed, return code: {result.returncode}"
            
    except Exception as e:
        return False, f"Error: {e}"
    finally:
        os.chdir(original_dir)

def build_by_module_type(module_type: str) -> bool:
    """根据mkimg的模块类型，自动编译对应的工程"""
    if module_type not in MODULE_TO_BUILD_MAP:
        print_warning(f"No build project for module {module_type}, skip build")
        return True
    build_projects = MODULE_TO_BUILD_MAP[module_type]
    if module_type in ["BIT28", "BIT28_PT"]:
        print_header(f"EXTRACT BITSTREAM - {module_type}")
        
        # Define file paths
        source_file = r"..\FM_ZQ_bsp\bit\zu28dr.bit.tar.gz"
        target_dir = r"."
        os.makedirs(target_dir, exist_ok=True)

        # Extract the file
        try:
            with tarfile.open(source_file, 'r:gz') as tar:
                tar.extractall(target_dir)
            print_success(f"Successfully extracted {source_file} to {target_dir} directory")
            return True
        except Exception as e:
            print_warning(f"Extraction failed: {e}")
            return False
    if not build_projects:
        return True
    print_header(f"AUTO BUILD - {module_type} DEPENDENCIES")
    all_success = True
    for proj in build_projects:
        success, msg = build_project(proj, clean_first=True)
        print_status(proj, success, msg)
        if not success:
            all_success = False

    if module_type in ["BOOT", "BOOT_PT"] and all_success:
        bootgen_success, bootgen_msg = generate_boot_bin()
        print_status("BOOTGEN", bootgen_success, bootgen_msg)
        if not bootgen_success:
            all_success = False

    return all_success

def build_all_for_pack() -> bool:
    """mkimg --all 对应的全量编译 (不含BOOT)"""
    print_header(f"AUTO BUILD ALL (EXCLUDE BOOT) - FOR PACKAGE")
    build_list = ["CR5_0", "CR5_1", "CA53"]
    all_success = True
    for proj in build_list:
        success, msg = build_project(proj, clean_first=True)
        print_status(proj, success, msg)
        if not success:
            all_success = False
    return all_success

# ======================== 原build.py 完整代码段 END ========================

# ======================== 原mkimg.py 完整代码段 START ========================
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
IMAGES_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, '..', 'Images'))
os.chdir(IMAGES_DIR)
print(f"{Colors.BLUE}[*] Working directory set to: {IMAGES_DIR}{Colors.RESET}")


class EMMCImagePackager:
    def __init__(self):
        self.MODULE_TYPES = {
            'BOOT': b'BOOT    ',
            'BOOT_PT': b'BOOT_PT ',
            'CA53': b'CA53    ',
            'CA53_PT': b'CA53_PT ',
            'CR50': b'CR50    ',
            'CR50_PT': b'CR50_PT ',
            'CR51': b'CR51    ',
            'CR51_PT': b'CR51_PT ',
            'BIT28': b'BIT28   ',
            'BIT28_PT': b'BIT28_PT'
        }
        
        self.GIT_TAG_SUFFIXES = {
            'BOOT': 'boot',
            'BOOT_PT': 'boot',
            'CA53': 'ca53',
            'CA53_PT': 'ca53',
            'CR50': 'cr50',
            'CR50_PT': 'cr50',
            'CR51': 'cr51',
            'CR51_PT': 'cr51',
            'BIT28': 'bit28',
            'BIT28_PT': 'bit28',
            'FSBL': 'fsbl',
            'CABOOT': 'caboot',
        }
        
        self.MODULE_CONFIGS = {
            'BOOT': {
                'format': 'BOOT',
                'header_format': self._get_boot_header_format(),
                'files': {
                    'fsbl': 'fsbl.out',
                    'boot': 'BOOT.bin',  # 依赖bootgen生成的文件
                    'caboot': 'CA53_BOOT.out'
                },
                'structure': 'triple'
            },
            'BOOT_PT': {
                'format': 'BOOT',
                'header_format': self._get_boot_header_format(),
                'files': {
                    'fsbl': 'fsbl.out',
                    'boot': 'BOOT.bin',
                    'caboot': 'CA53_BOOT.out'
                },
                'structure': 'triple'
            },
            'CA53': {
                'format': 'STANDARD',
                'header_format': self._get_standard_header_format(),
                'files': {'map': 'CA53.map', 'bin': 'CA53.bin'},
                'structure': 'triple'
            },
            'CA53_PT': {
                'format': 'STANDARD',
                'header_format': self._get_standard_header_format(),
                'files': {'map': 'CA53.map', 'bin': 'CA53.bin'},
                'structure': 'triple'
            },
            'CR50': {
                'format': 'STANDARD',
                'header_format': self._get_standard_header_format(),
                'files': {'map': 'CR5_0.map', 'bin': 'CR50.bin'},
                'structure': 'triple'
            },
            'CR50_PT': {
                'format': 'STANDARD',
                'header_format': self._get_standard_header_format(),
                'files': {'map': 'CR5_0.map', 'bin': 'CR50.bin'},
                'structure': 'triple'
            },
            'CR51': {
                'format': 'STANDARD',
                'header_format': self._get_standard_header_format(),
                'files': {'map': 'CR5_1.map', 'bin': 'CR51.bin'},
                'structure': 'triple'
            },
            'CR51_PT': {
                'format': 'STANDARD',
                'header_format': self._get_standard_header_format(),
                'files': {'map': 'CR5_1.map', 'bin': 'CR51.bin'},
                'structure': 'triple'
            },
            'BIT28': {
                'format': 'BIT28',
                'header_format': self._get_standard_header_format(),
                'files': {'map': '', 'bin': 'zu28dr.bit'},
                'structure': 'double'
            },
            'BIT28_PT': {
                'format': 'BIT28',
                'header_format': self._get_standard_header_format(),
                'files': {'map': '', 'bin': 'zu28dr.bit'},
                'structure': 'double'
            }
        }
        
        self.SIGNATURE = b'DGMODIMG'
        self.MODULE_INFO_SIZE = 1024
        self.BLOCK_SIZE = 512
        
        self.IMG_INFO_SIZE = 4096
        self.IMG_HEADER_FORMAT = '<'
        self.IMG_HEADER_FORMAT += '4s'
        self.IMG_HEADER_FORMAT += '4s'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += '32s'
        self.IMG_HEADER_FORMAT += '12s'
        self.IMG_HEADER_FORMAT += '12s'
        self.IMG_HEADER_FORMAT += '8s'
        self.IMG_HEADER_FORMAT += '8s'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += '8s'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += '8s'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += '8s'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += '8s'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'
        self.IMG_HEADER_FORMAT += 'I'

        self.IMG_HEADER_SIZE = struct.calcsize(self.IMG_HEADER_FORMAT)
        
        self.STATE = b'\xFF\xFF\xFF\xFF'
        self.IMG_SIGNATURE = b'DGKJ'
        self.BOARD_TYPE = b'silverfox'.ljust(32, b'\x00')
        self.IMAGE_VERSION = b'V1.0.0'.ljust(8, b' ')
        
        self.MODULE_FILES = {
            'BOOT': 'MOD_BOOT',
            'A53': 'MOD_CA53',
            'CR50': 'MOD_CR50',
            'CR51': 'MOD_CR51',
            '28dr': 'MOD_BIT28',
            'UE': 'ue.conf'
        }
# boot
    def _get_boot_header_format(self):  
        return '<8s8sII12s12s8s8sI8sI8sII'
    
    def _get_standard_header_format(self):
        return '<8s8sII12s12s8sI8sII'
    
    def _get_latest_git_tag(self, tag_suffix):
        try:
            if tag_suffix == 'bit28':
                return self._get_latest_bit28_tag()
            
            patterns = [
                f'V*[0-9]*.[0-9]*.[0-9]*_{tag_suffix}',
                f'V*[0-9]*.[0-9]*.[0-9]*-{tag_suffix}',
            ]
            
            all_tags = []
            for pattern in patterns:
                try:
                    cmd = ['git', 'tag', '--list', pattern, '--sort=-v:refname']
                    result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=3)
                    tags = [t.strip() for t in result.stdout.strip().split('\n') if t.strip()]
                    
                    for tag in tags:
                        if self._validate_tag_suffix(tag, tag_suffix):
                            version = self._extract_version_from_tag(tag)
                            if version:
                                all_tags.append((tag, version))
                except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
                    continue
            
            if not all_tags:
                return None, None
            
            all_tags.sort(key=lambda x: self._version_to_tuple(x[1]), reverse=True)
            return all_tags[0]
            
        except Exception as e:
            print_warning(f"Warning: Error getting Git tags for '{tag_suffix}': {e}")
            return None, None
    
    def _get_latest_bit28_tag(self):
        try:
            patterns = [
                f'V[0-9][0-9][0-9][0-9][0-9][0-9][A-Fa-f0-9][A-Fa-f0-9]_bit28',
                f'V[0-9][0-9][0-9][0-9][0-9][0-9][A-Fa-f0-9][A-Fa-f0-9]-bit28',
            ]
            
            all_tags = []
            for pattern in patterns:
                try:
                    cmd = ['git', 'tag', '--list', pattern, '--sort=-refname']
                    result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=3)
                    tags = [t.strip() for t in result.stdout.strip().split('\n') if t.strip()]
                    
                    for tag in tags:
                        if self._validate_bit28_tag(tag):
                            version = self._extract_bit28_version(tag)
                            if version:
                                all_tags.append((tag, version))
                except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
                    continue
            
            if not all_tags:
                return None, None
            
            all_tags.sort(key=lambda x: self._bit28_date_to_number(x[1]), reverse=True)
            if all_tags:
                print_step(f"  Found BIT28 tags: {[t[0] for t in all_tags]}")
                print_step(f"  Using latest tag: {all_tags[0][0]} -> {all_tags[0][1]}")
            
            return all_tags[0] if all_tags else (None, None)
            
        except Exception as e:
            print_warning(f"Warning: Error fetching BIT28 Git tags: {e}")
            return None, None
    
    def _get_latest_image_tag(self):
        try:
            cmd = ['git', 'tag', '--list', 'V*[0-9]*.[0-9]*.[0-9]*', '--sort=-v:refname']
            result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=3)
            tags = [t.strip() for t in result.stdout.strip().split('\n') if t.strip()]
            
            all_tags = []
            for tag in tags:
                tag_lower = tag.lower()
                if re.match(r'^v[0-9]+\.[0-9]+\.[0-9]+$', tag_lower):
                    version = self._extract_version_from_tag(tag)
                    if version:
                        all_tags.append((tag, version))
            
            if not all_tags:
                return None, None
            
            all_tags.sort(key=lambda x: self._version_to_tuple(x[1]), reverse=True)
            return all_tags[0]
            
        except Exception as e:
            print_warning(f"Warning: Error getting image Git tags: {e}")
            return None, None
    
    def _validate_bit28_tag(self, tag):
        tag_lower = tag.lower()
        pattern = r'^v[A-Fa-f0-9]{8}[_-]bit28$'
        return bool(re.match(pattern, tag_lower))
    
    def _extract_bit28_version(self, tag):
        tag_upper = tag.upper()
        patterns = [
            r'^V([A-Fa-f0-9]{8})_BIT28$',
            r'^V([A-Fa-f0-9]{8})-BIT28$',
        ]
        
        for pattern in patterns:
            match = re.match(pattern, tag_upper)
            if match:
                return match.group(1)
        
        return None
    
    def _bit28_date_to_number(self, version_str):
        if not version_str:
            return 0
        try:
            return int(version_str,16)
        except ValueError:
            return 0
    
    def _validate_tag_suffix(self, tag, required_suffix):
        tag_lower = tag.lower()
        required_lower = required_suffix.lower()
        
        if required_suffix == 'bit28':
            return self._validate_bit28_tag(tag)
        
        patterns = [f'_{required_lower}$', f'-{required_lower}$']
        
        for pattern in patterns:
            if re.search(pattern, tag_lower):
                version_part = tag_lower.rsplit(f'_{required_lower}', 1)[0]
                if '-' in required_lower:
                    version_part = tag_lower.rsplit(f'-{required_lower}', 1)[0]
                
                if re.match(r'^v[0-9]+\.[0-9]+\.[0-9]+$', version_part):
                    return True
        return False
    
    def _extract_version_from_tag(self, tag):
        tag_upper = tag.upper()
        version_pattern = r'^([Vv][0-9]+\.[0-9]+\.[0-9]+)'
        match = re.match(version_pattern, tag_upper)
        return match.group(1).upper() if match else None
    
    def _version_to_tuple(self, version_str):
        if not version_str:
            return (0, 0, 0)
        
        if version_str.isdigit() and len(version_str) == 8:
            try:
                year = int(version_str[0:2])
                month = int(version_str[2:4])
                day = int(version_str[4:6])
                hour = int(version_str[6:8])
                return (year, month, day, hour, 0)
            except ValueError:
                return (0, 0, 0, 0, 0)
        
        if version_str.upper().startswith('V'):
            version_str = version_str[1:]
        
        parts = version_str.split('.')
        version_tuple = []
        for part in parts[:3]:
            try:
                version_tuple.append(int(part))
            except ValueError:
                version_tuple.append(0)
        
        while len(version_tuple) < 3:
            version_tuple.append(0)
        
        return tuple(version_tuple)
    
    def _format_version_bytes(self, version_str, module_type=None):
        if isinstance(version_str, bytes):
            version_str = version_str.decode('ascii', errors='ignore')
        
        if module_type and (module_type.startswith('BIT28')):
            if not version_str.isdigit() and version_str.upper().startswith('V'):
                version_str = version_str[1:]
        else:
            if not version_str.upper().startswith('V'):
                version_str = 'V' + version_str
        
        version_bytes = version_str.encode('ascii', errors='ignore')
        if len(version_bytes) > 8:
            version_bytes = version_bytes[:8]
        else:
            version_bytes = version_bytes.ljust(8, b' ')
        
        return version_bytes
    
    def get_module_version(self, module_type):
        suffix = self.GIT_TAG_SUFFIXES.get(module_type)
        if not suffix:
            raise ValueError(f"No Git tag suffix defined for: {module_type}")
        
        if module_type.startswith('BIT28'):
            print_step(f"  Looking for BIT28 tags (format: V25120500_bit28)...")
        
        tag, version = self._get_latest_git_tag(suffix)
        if not tag or not version:
            raise RuntimeError(
                f"Error: No Git tag found for module '{module_type}' with suffix '{suffix}'\n"
                f"Please create tag: git tag V1.0.0_{suffix} && git push origin V1.0.0_{suffix}"
            )
        
        print_success(f"  Using Git tag: {tag} -> {version}")
        return self._format_version_bytes(version, module_type)
    
    def get_fsbl_version(self):
        tag, version = self._get_latest_git_tag('fsbl')
        if not tag or not version:
            raise RuntimeError(
                "Error: No Git tag found for FSBL with suffix 'fsbl'\n"
                "Please create tag: git tag V1.0.0_fsbl && git push origin V1.0.0_fsbl"
            )
        
        print_success(f"  FSBL version: {version}")
        return self._format_version_bytes(version)
    
    def get_caboot_version(self):
        tag, version = self._get_latest_git_tag('caboot')
        if not tag or not version:
            raise RuntimeError(
                "Error: No Git tag found for CABOOT with suffix 'caboot'\n"
                "Please create tag: git tag V1.0.0_caboot && git push origin V1.0.0_caboot"
            )
        
        print_success(f"  CABOOT version: {version}")
        return self._format_version_bytes(version)
    
    def get_bootbin_version(self, module_type):
        return self.get_module_version(module_type)
    
    def get_bin_version(self, module_type):
        return self.get_module_version(module_type)
    
    def get_image_version(self):
        tag, version = self._get_latest_image_tag()
        if not tag or not version:
            print_warning(f"  No image Git tag found, using default: {self.IMAGE_VERSION.decode('ascii').strip()}")
            return self.IMAGE_VERSION
        print_success(f"  Using image tag: {tag} -> {version}")
        return self._format_version_bytes(version)

    def align_to_512(self, data: bytes) -> bytes:
        data_len = len(data)
        pad_len = (512 - (data_len % 512)) % 512
        aligned_data = data + b'\x00' * pad_len
        return aligned_data

    def calculate_crc32(self, data):
        return zlib.crc32(data) & 0xFFFFFFFF
    
    def get_current_datetime(self):
        now = datetime.now()
        build_date = now.strftime("%b %d %Y").encode('ascii').ljust(12, b'\x00')
        build_time = now.strftime("%H:%M:%S").encode('ascii').ljust(12, b'\x00')
        return build_date, build_time
    
    def get_img_datetime(self):
        now = datetime.now()
        build_date = now.strftime("%b %d %Y").encode('ascii').ljust(12, b'\x00')
        build_time = now.strftime("%H:%M:%S").encode('ascii').ljust(12, b'\x00')
        return build_date, build_time
    
    def align_to_block(self, size, alignment=None):
        if alignment is None:
            alignment = self.BLOCK_SIZE
        return (size + alignment - 1) // alignment * alignment
    
    def read_binary_file(self, file_path):
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"File not found: {file_path}")
        with open(file_path, 'rb') as f:
            return f.read()
    
    def is_boot_module(self, module_type):
        return module_type.startswith('BOOT')

    def extract_module_version(self, module_data):
        if len(module_data) < 1024:
            return b'UNKNOWN'
        
        module_info = module_data[:1024]
        
        try:
            standard_format = '<8s8sII12s12s8sI8sI'
            standard_size = struct.calcsize(standard_format)
            
            if len(module_info) >= standard_size:
                fields = struct.unpack(standard_format, module_info[:standard_size])
                signature, mod_type, crc, hdr_size, date, time, mod_ver, map_sz, bin_ver, bin_sz = fields
                
                if signature == b'DGMODIMG':
                    return mod_ver.rstrip(b' ')
                
                boot_format = '<8s8sII12s12s8s8sI8sI8sI'
                boot_size = struct.calcsize(boot_format)
                
                if len(module_info) >= boot_size:
                    fields = struct.unpack(boot_format, module_info[:boot_size])
                    signature, mod_type, crc, hdr_size, date, time, mod_ver, fsbl_ver, fsbl_sz, caboot_ver, caboot_sz, boot_ver, boot_sz = fields
                    
                    if signature == b'DGMODIMG':
                        return mod_ver.rstrip(b' ')
            
        except struct.error:
            pass
        
        return b'UNKNOWN'.ljust(8, b' ')
    
    def get_module_version_from_file(self, module_name, file_path):
        if not os.path.exists(file_path):
            return None
        
        try:
            with open(file_path, 'rb') as f:
                data = f.read()
            
            version_bytes = self.extract_module_version(data)
            version_str = version_bytes.decode('ascii', errors='ignore').rstrip()
            
            if version_str != 'UNKNOWN':
                return version_str
            
            mapping = {
                'BOOT': 'BOOT',
                'A53': 'CA53',
                'CR50': 'CR50',
                'CR51': 'CR51',
                '28dr': 'BIT28',
                'UE': 'UE'
            }
            
            if module_name in mapping:
                suffix = self.GIT_TAG_SUFFIXES.get(mapping[module_name])
                if suffix:
                    tag, version = self._get_latest_git_tag(suffix)
                    if tag and version:
                        if module_name == '28dr' and version.startswith('V'):
                            return version[1:]
                        return version
            
            return None
            
        except Exception:
            return None
    
    def get_module_config(self, module_type):
        config = self.MODULE_CONFIGS.get(module_type)
        if not config:
            raise ValueError(f"Unsupported module type: {module_type}")
        return config
    
    def get_module_files(self, module_type):
        config = self.get_module_config(module_type)
        files = []
        
        for key, default_path in config['files'].items():
            if default_path:
                files.append((key, default_path))
        
        return files

    def create_module_info(self, file_data, module_type):
        config = self.get_module_config(module_type)
        header_format = config['header_format']
        header_size = struct.calcsize(header_format)
        
        module_type_bytes = self.MODULE_TYPES[module_type]
        build_date, build_time = self.get_current_datetime()
        module_version = self.get_module_version(module_type)
        
        if config['format'] == 'BOOT':
            # 1. fsbl.out + caboot.out + BOOT.bin
            fsbl_data = file_data['fsbl']
            boot_data = file_data['boot']
            caboot_data = file_data['caboot']
            
            fsbl_size = len(fsbl_data)
            boot_size = len(boot_data)
            caboot_size = len(caboot_data)

            # boot_data_aligned = self.align_to_512(boot_data)
            caboot_data_aligned = self.align_to_512(caboot_data)
            # boot_crc_data = boot_data + caboot_data
            boot_crc_data =  boot_data + caboot_data_aligned
            boot_crc = self.calculate_crc32(boot_crc_data)
            
            bootbin_version = self.get_bootbin_version(module_type)
            fsbl_version = self.get_fsbl_version()
            caboot_version = self.get_caboot_version()
            
            temp_header = struct.pack(header_format,
                self.SIGNATURE,          # char signature[8]
                module_type_bytes,       # char type[8]
                0,                       # u32 module_header_crc
                header_size,             # u32 module_header_size
                build_date,              # char build_date[12]
                build_time,              # char build_time[12]
                module_version,          # char module_version[8]
                fsbl_version,            # char fsbl_version[8]
                fsbl_size,               # u32 fsbl_size
                caboot_version,          # char caboot_version[8]
                caboot_size,             # u32 caboot_size
                bootbin_version,         # char bootbin_version[8]
                boot_size,               # u32 bootbin_size
                boot_crc                # u32 boot_crc
            )
            # 计算模块头CRC：跳过前24字节(signature+type+crc+size)
            crc_data = temp_header[20:-4]  # 核心修改：20开始，-4结束（排除最后4字节的boot_crc）
            header_crc = self.calculate_crc32(crc_data)
            
            header = struct.pack(header_format,
                self.SIGNATURE,
                module_type_bytes,
                header_crc, 
                header_size,
                build_date,
                build_time,
                module_version,
                fsbl_version,
                fsbl_size,
                caboot_version,
                caboot_size,
                bootbin_version,
                boot_size,
                boot_crc  
            )
        else:
            map_data = file_data.get('map', b'')  # 普通模块有map，FPGA无map（空字节）
            bin_data = file_data['bin']
            
            map_size = len(map_data)
            bin_size = len(bin_data)
            
            # 2. 计算新增的module_crc：分场景
            if config['format'] == 'BIT28':
                # FPGA/BIT28：仅计算bit文件的CRC32
                
                bin_data_aligned = self.align_to_512(bin_data)
                module_crc_data = bin_data_aligned
            else:
                # 普通模块（CA53/CR50/CR51）：计算map + bin 拼接后的CRC32
                bin_data_aligned = self.align_to_512(bin_data)
                map_data_aligned = self.align_to_512(map_data)
                module_crc_data = map_data_aligned + bin_data_aligned
            module_crc = self.calculate_crc32(module_crc_data)
            
            bin_version = self.get_bin_version(module_type)
            
            temp_header = struct.pack(header_format,
                self.SIGNATURE,          # char signature[8]
                module_type_bytes,       # char type[8]
                0,                       # u32 module_header_crc 
                header_size,             # u32 module_header_size
                build_date,              # char build_date[12]
                build_time,              # char build_time[12]
                module_version,          # char module_version[8]
                map_size,                # u32 map_size（FPGA为0）
                bin_version,             # char bin_version[8]
                bin_size,                # u32 bin_size
                module_crc              # u32 module_crc
            )
            crc_data = temp_header[20:-4]
            header_crc = self.calculate_crc32(crc_data)
            
            header = struct.pack(header_format,
                self.SIGNATURE,
                module_type_bytes,
                header_crc,  
                header_size,
                build_date,
                build_time,
                module_version,
                map_size,
                bin_version,
                bin_size,
                module_crc          
            )
        
        # 生成1024字节的模块信息（头128字节 + 补0到1024）
        module_info = bytearray(header)
        remaining = self.MODULE_INFO_SIZE - len(module_info)
        if remaining > 0:
            module_info.extend(b'\x00' * remaining)
        
        if len(module_info) != self.MODULE_INFO_SIZE:
            raise ValueError("Module info size error: Must be 1024 bytes")
        
        return bytes(module_info), header_size

    def combine_modules(self):
        try:
            print_header("COMBINING MODULES INTO SINGLE IMAGE")
            print_step(f"Output file: image")
            
            print_step("Fetching image version from Git...")
            try:
                image_version_bytes = self.get_image_version()
                image_version_str = image_version_bytes.decode('ascii').strip()
                print_success(f"  Image version: {image_version_str}")
            except Exception as e:
                print_warning(f"Warning: Error getting image version: {e}")
                image_version_bytes = self.IMAGE_VERSION
                image_version_str = image_version_bytes.decode('ascii').strip()
                print_step(f"  Using default image version: {image_version_str}")
            
            ue_src  = r"..\FM_ZQ_bsp\lib\ue.conf"
            ue_dst  = r"."
            if os.path.exists(ue_src):
                shutil.copy2(ue_src, ue_dst)
                print_success(f"Copied {ue_src} -> {ue_dst}")
            else:
                print_warning(f"{ue_src} not found, skipping UE module")

            print_step("\nChecking module files...")
            module_files = {}
            
            for module_name, file_name in self.MODULE_FILES.items():
                if os.path.exists(file_name):
                    try:
                        data = self.read_binary_file(file_name)
                        version = self.get_module_version_from_file(module_name, file_name)
                        
                        if version:
                            version_bytes = version.encode('ascii', errors='ignore').ljust(8, b' ')
                        else:
                            version_bytes = b'UNKNOWN'.ljust(8, b' ')
                        
                        module_files[module_name] = {
                            'data': data,
                            'size': len(data),
                            'aligned_size': self.align_to_block(len(data)),
                            'version': version_bytes
                        }
                        
                        if version:
                            print_success(f"  {module_name}: {file_name} ({len(data)} bytes) [Version: {version}]")
                        else:
                            print_step(f"  {module_name}: {file_name} ({len(data)} bytes) [Version: UNKNOWN]")
                    except Exception as e:
                        print_error(f"  {module_name}: Error reading {file_name}: {e}")
                        module_files[module_name] = {
                            'data': b'',
                            'size': 0,
                            'aligned_size': 0,
                            'version': b'UNKNOWN'.ljust(8, b' ')
                        }
                else:
                    module_files[module_name] = {
                        'data': b'',
                        'size': 0,
                        'aligned_size': 0,
                        'version': b'UNKNOWN'.ljust(8, b' ')
                    }
                    print_warning(f"  {module_name}: File not found: {file_name}")
            
            current_offset = self.IMG_INFO_SIZE
            module_offsets = {}
            for module_name in ['BOOT', 'A53', 'CR50', 'CR51', '28dr', 'UE']:
                module_offsets[module_name] = {
                    'offset': current_offset,
                    'size': module_files[module_name]['size'],
                    'aligned_size': module_files[module_name]['aligned_size']
                }
                current_offset += module_files[module_name]['aligned_size']
            
            print_step("\nCreating IMG_INFO (4KB)...")
            build_date, build_time = self.get_img_datetime()
            
            try:
                temp_header = struct.pack(
                    self.IMG_HEADER_FORMAT,
                    self.STATE,
                    self.IMG_SIGNATURE,
                    0,
                    0,
                    self.IMG_HEADER_SIZE,
                    self.BOARD_TYPE,
                    build_date,
                    build_time,
                    image_version_bytes,
                    module_files['BOOT']['version'],
                    module_offsets['BOOT']['offset'],
                    module_offsets['BOOT']['size'],
                    module_files['A53']['version'],
                    module_offsets['A53']['offset'],
                    module_offsets['A53']['size'],
                    module_files['CR50']['version'],
                    module_offsets['CR50']['offset'],
                    module_offsets['CR50']['size'],
                    module_files['CR51']['version'],
                    module_offsets['CR51']['offset'],
                    module_offsets['CR51']['size'],
                    module_files['28dr']['version'],
                    module_offsets['28dr']['offset'],
                    module_offsets['28dr']['size'],
                    module_offsets['UE']['offset'],
                    module_offsets['UE']['size']
                )
                
                crc_data = temp_header[12:]
                header_crc = self.calculate_crc32(crc_data)
                
                img_info = struct.pack(
                    self.IMG_HEADER_FORMAT,
                    self.STATE,
                    self.IMG_SIGNATURE,
                    header_crc,
                    0,
                    self.IMG_HEADER_SIZE,
                    self.BOARD_TYPE,
                    build_date,
                    build_time,
                    image_version_bytes,
                    module_files['BOOT']['version'],
                    module_offsets['BOOT']['offset'],
                    module_offsets['BOOT']['size'],
                    module_files['A53']['version'],
                    module_offsets['A53']['offset'],
                    module_offsets['A53']['size'],
                    module_files['CR50']['version'],
                    module_offsets['CR50']['offset'],
                    module_offsets['CR50']['size'],
                    module_files['CR51']['version'],
                    module_offsets['CR51']['offset'],
                    module_offsets['CR51']['size'],
                    module_files['28dr']['version'],
                    module_offsets['28dr']['offset'],
                    module_offsets['28dr']['size'],
                    module_offsets['UE']['offset'],
                    module_offsets['UE']['size']
                )
                
                img_info_full = bytearray(img_info)
                remaining = self.IMG_INFO_SIZE - len(img_info_full)
                if remaining > 0:
                    img_info_full.extend(b'\x00' * remaining)
                
            except struct.error as e:
                raise ValueError(f"struct.pack error: {e}. Check that all string fields are bytes objects.")
            
            print_success(f"IMG_INFO size: {len(img_info_full)} bytes (fixed 4KB)")
            print_success(f"Header CRC: 0x{header_crc:08X}")
            
            total_size = current_offset
            print_step(f"Total image size: {total_size} bytes")
            
            image_data = bytearray()
            image_data.extend(img_info_full)
            
            for module_name in ['BOOT', 'A53', 'CR50', 'CR51', '28dr', 'UE']:
                data = module_files[module_name]['data']
                if data:
                    image_data.extend(data)
                
                aligned_padding = module_files[module_name]['aligned_size'] - len(data)
                if aligned_padding > 0:
                    image_data.extend(b'\x00' * aligned_padding)
            
            if os.path.exists('image'):
                os.remove('image')
            
            print_step(f"\nWriting image file...")
            with open('image', 'wb') as f:
                f.write(image_data)
            
            img_info_text = self.print_image_info(img_info_full, module_offsets, module_files)
            with open('img_info.info', 'w', encoding='utf-8') as f_txt:
                f_txt.write(img_info_text)
            print_success("Version info saved to: img_info.info (text format)")

            print_success(f"\nCombined image created successfully: image")
            return True,image_version_str
            
        except Exception as e:
            print_error(f"\nCombine image failed: {e}")
            return False

    def create_image(self, module_type, output_file=None):
        try:
            if output_file is None:
                output_file = f'MOD_{module_type}'
            
            print_header(f"Building module: {module_type}")
            print_step(f"Output file: {output_file}")
            
            config = self.get_module_config(module_type)
            required_files = self.get_module_files(module_type)
            
            file_data = {}
            missing_files = []
            
            for file_key, default_path in required_files:
                if not os.path.exists(default_path):
                    missing_files.append(f"{file_key.upper()}: {default_path}")
                else:
                    file_data[file_key] = self.read_binary_file(default_path)
                    print_success(f"  {file_key.upper()} file: {default_path} ({len(file_data[file_key])} bytes)")
            
            if missing_files:
                raise FileNotFoundError(f"Missing files:\n" + "\n".join(missing_files))
            
            print_step("\nFetching versions from Git...")
            try:
                module_version = self.get_module_version(module_type)
                module_version_str = module_version.decode('ascii').strip()
                
                if self.is_boot_module(module_type):
                    fsbl_version = self.get_fsbl_version()
                    caboot_version = self.get_caboot_version()
                    bootbin_version = self.get_bootbin_version(module_type)
                    print_success(f"  Module version: {module_version_str}")
                    print_success(f"  FSBL version: {fsbl_version.decode('ascii').strip()}")
                    print_success(f"  CABOOT version: {caboot_version.decode('ascii').strip()}")
                    print_success(f"  BOOT.bin version: {bootbin_version.decode('ascii').strip()}")
                else:
                    bin_version = self.get_bin_version(module_type)
                    print_success(f"  Module version: {module_version_str}")
                    print_success(f"  BIN version: {bin_version.decode('ascii').strip()}")
            except RuntimeError as e:
                raise e
            
            print_step("\nCreating module info...")
            module_info, header_size = self.create_module_info(file_data, module_type)
            print_success(f"  Module info: 1024 bytes (1KB)")
            print_success(f"  Header size: {header_size} bytes")
            
            image_data = bytearray(module_info)
            
            if config['format'] == 'BOOT':
                boot_data = file_data['boot']
                caboot_data = file_data['caboot']
                caboot_aligned = self.align_to_block(len(caboot_data))
                
                image_data.extend(boot_data)
                image_data.extend(caboot_data)
                if caboot_aligned > len(caboot_data):
                    image_data.extend(b'\x00' * (caboot_aligned - len(caboot_data)))
                
                print_step(f"\nData layout (BOOT):")
                print_step(f"  Module info: 0x00000000 - 0x000003FF")
                print_step(f"  BOOT data:   0x00000400 - 0x{0x400 + len(boot_data) - 1:08X}")
                print_step(f"  CABOOT data: 0x{0x400 + len(boot_data):08X} - 0x{0x400 + len(boot_data) + len(caboot_data) - 1:08X}")
                print_step(f"  CABOOT aligned: {caboot_aligned} bytes")
                
            elif config['structure'] == 'double':
                bin_data = file_data['bin']
                bin_aligned = self.align_to_block(len(bin_data))
                
                image_data.extend(bin_data)
                if bin_aligned > len(bin_data):
                    image_data.extend(b'\x00' * (bin_aligned - len(bin_data)))
                
                print_step(f"\nData layout (BIT28):")
                print_step(f"  Module info: 0x00000000 - 0x000003FF")
                print_step(f"  BIT28 data:  0x00000400 - 0x{0x400 + len(bin_data) - 1:08X}")
                print_step(f"  BIT28 aligned: {bin_aligned} bytes")
                
            else:
                map_data = file_data.get('map', b'')
                bin_data = file_data['bin']
                
                map_aligned = self.align_to_block(len(map_data))
                bin_aligned = self.align_to_block(len(bin_data))
                
                image_data.extend(map_data)
                if map_aligned > len(map_data):
                    image_data.extend(b'\x00' * (map_aligned - len(map_data)))
                image_data.extend(bin_data)
                if bin_aligned > len(bin_data):
                    image_data.extend(b'\x00' * (bin_aligned - len(bin_data)))
                
                print_step(f"\nData layout (Standard):")
                print_step(f"  Module info: 0x00000000 - 0x000003FF")
                if map_data:
                    print_step(f"  MAP data:    0x00000400 - 0x{0x400 + len(map_data) - 1:08X}")
                print_step(f"  BIN data:    0x{0x400 + map_aligned:08X} - 0x{0x400 + map_aligned + len(bin_data) - 1:08X}")
            
            print_step(f"\nWriting {len(image_data)} bytes to {output_file}...")
            with open(output_file, 'wb') as f:
                f.write(image_data)
            
            if os.path.getsize(output_file) != len(image_data):
                raise ValueError("File size mismatch")
            
            self.print_image_details(module_info, file_data, module_type, header_size)
            print_success(f"\nImage created successfully: {output_file}")
            return True
            
        except Exception as e:
            print_error(f"\nImage creation failed: {e}")
            return False

    def print_image_details(self, module_info, file_data, module_type, header_size):
        config = self.get_module_config(module_type)
        # 截取真实模块头数据，排除后续补0部分
        module_header = module_info[:header_size]
        
        if config['format'] == 'BOOT':
            # 解包：严格匹配BOOT模块打包顺序（14个字段，boot_crc在最后）
            # sig[8s], typ[8s], header_crc[I], hdr_sz[I], date[12s], time[12s],
            # mod_ver[8s], fsbl_ver[8s], fsbl_sz[I], caboot_ver[8s], caboot_sz[I],
            # boot_ver[8s], boot_sz[I], boot_crc[I]
            fields = struct.unpack(config['header_format'], module_header)
            sig, typ, header_crc, hdr_sz, date, time, mod_ver, fsbl_ver, fsbl_sz, caboot_ver, caboot_sz, boot_ver, boot_sz, boot_crc = fields
            
            print("\n" + "=" * 60)
            print("IMAGE DETAILS [BOOT MODULE]")
            print("=" * 60)
            print(f"{'Field':<20} {'Value'}")
            print("-" * 60)
            print(f"{'Signature':<20} {sig.decode('ascii').strip()}")
            print(f"{'Type':<20} {typ.decode('ascii').strip()}")
            print(f"{'Module Version':<20} {mod_ver.decode('ascii').strip()}")
            # 正确打印模块头CRC和BOOT数据CRC（boot_crc在最后一位）
            print(f"{'Module Header CRC':<20} 0x{header_crc:08X}")
            print(f"{'BOOT Data CRC (2in1)':<20} 0x{boot_crc:08X}")
            print(f"{'  CRC Calc Range':<20} BOOT.bin + caboot.out")
            print(f"{'Module Header Size':<20} {hdr_sz} bytes (FIXED)")
            print(f"{'Build Date':<20} {date.decode('ascii').strip('\\x00')}")
            print(f"{'Build Time':<20} {time.decode('ascii').strip('\\x00')}")
            print(f"{'FSBL Version':<20} {fsbl_ver.decode('ascii').strip()}")
            print(f"{'FSBL Size':<20} {fsbl_sz} bytes")
            print(f"{'CABOOT Version':<20} {caboot_ver.decode('ascii').strip()}")
            print(f"{'CABOOT Size':<20} {caboot_sz} bytes")
            print(f"{'BOOT Version':<20} {boot_ver.decode('ascii').strip()}")
            print(f"{'BOOT Size':<20} {boot_sz} bytes")
            # 计算总镜像大小，保持原有逻辑
            total_data_size = len(file_data['boot']) + self.align_to_block(len(file_data['caboot']))
            print(f"{'Total Image Size':<20} {self.MODULE_INFO_SIZE + total_data_size} bytes")
        else:
            # 解包：严格匹配标准模块打包顺序（11个字段，module_crc在最后）
            # sig[8s], typ[8s], header_crc[I], hdr_sz[I], date[12s], time[12s],
            # mod_ver[8s], map_sz[I], bin_ver[8s], bin_sz[I], module_crc[I]
            fields = struct.unpack(config['header_format'], module_header)
            sig, typ, header_crc, hdr_sz, date, time, mod_ver, map_sz, bin_ver, bin_sz, module_crc = fields
            
            print("\n" + "=" * 60)
            if config['format'] == 'BIT28':
                print("IMAGE DETAILS [FPGA/BIT28 MODULE]")
            else:
                print("IMAGE DETAILS [STANDARD MODULE]")
            print("=" * 60)
            print(f"{'Field':<20} {'Value'}")
            print("-" * 60)
            print(f"{'Signature':<20} {sig.decode('ascii').strip()}")
            print(f"{'Type':<20} {typ.decode('ascii').strip()}")
            print(f"{'Module Version':<20} {mod_ver.decode('ascii').strip()}")
            # 正确打印模块头CRC和数据CRC（module_crc在最后一位）
            print(f"{'Module Header CRC':<20} 0x{header_crc:08X}")
            print(f"{'Module Header Size':<20} {hdr_sz} bytes (FIXED)")
            print(f"{'Build Date':<20} {date.decode('ascii').strip('\\x00')}")
            print(f"{'Build Time':<20} {time.decode('ascii').strip('\\x00')}")
            if config['format'] == 'BIT28':
                print(f"{'BIT28 Version':<20} {bin_ver.decode('ascii').strip()}")
                print(f"{'BIT28 Size':<20} {bin_sz} bytes")
                print(f"{'BIT28 Data CRC':<20} 0x{module_crc:08X}")
                print(f"{'  CRC Calc Range':<20} zu28dr.bit (single file)")
            else:
                print(f"{'MAP Size':<20} {map_sz} bytes")
                print(f"{'BIN Version':<20} {bin_ver.decode('ascii').strip()}")
                print(f"{'BIN Size':<20} {bin_sz} bytes")
                print(f"{'Module Data CRC':<20} 0x{module_crc:08X}")
                print(f"{'  CRC Calc Range':<20} xx.map + xx.bin")
            # 计算总镜像大小，保持原有逻辑
            total_data_size = self.align_to_block(len(file_data.get('map', b''))) + self.align_to_block(len(file_data['bin']))
            print(f"{'Total Image Size':<20} {self.MODULE_INFO_SIZE + total_data_size} bytes")
            print("=" * 60)

    def print_image_info(self, img_info, module_offsets, module_files):
        (state, signature, header_crc, header_offset, header_size,
        board_type, build_date, build_time, image_version,
        boot_version, boot_offset, boot_size,
        a53_version, a53_offset, a53_size,
        cr50_version, cr50_offset, cr50_size,
        cr51_version, cr51_offset, cr51_size,
        dr28_version, dr28_offset, dr28_size,
        ue_offset, ue_size) = struct.unpack(self.IMG_HEADER_FORMAT, img_info[:self.IMG_HEADER_SIZE])

        signature_str = signature.decode('ascii', errors='ignore').rstrip()
        board_type_str = board_type.decode('ascii', errors='ignore').rstrip('\x00')
        build_date_str = build_date.decode('ascii', errors='ignore').rstrip()
        build_time_str = build_time.decode('ascii', errors='ignore').rstrip()
        image_version_str = image_version.decode('ascii', errors='ignore').rstrip()
        boot_version_str = boot_version.decode('ascii', errors='ignore').rstrip()
        a53_version_str = a53_version.decode('ascii', errors='ignore').rstrip()
        cr50_version_str = cr50_version.decode('ascii', errors='ignore').rstrip()
        cr51_version_str = cr51_version.decode('ascii', errors='ignore').rstrip()
        dr28_version_str = dr28_version.decode('ascii', errors='ignore').rstrip()

        text_content = []
        def echo(line):
            print(line)
            text_content.append(line)

        echo("=" * 60)
        echo("COMBINED IMAGE INFORMATION")
        echo("=" * 60)
        echo(f"{'Field':<20} {'Value'}")
        echo("-" * 60)
        echo(f"{'Signature':<20} {signature_str}")
        echo(f"{'Header CRC':<20} 0x{header_crc:08X}")
        echo(f"{'Board type':<20} {board_type_str}")
        echo(f"{'Build date':<20} {build_date_str}")
        echo(f"{'Build time':<20} {build_time_str}")
        echo(f"{'Image version':<20} {image_version_str}")
        echo("-" * 60)

        modules = [
            ('BOOT', boot_version_str, boot_offset, boot_size, module_files['BOOT']['size']),
            ('A53', a53_version_str, a53_offset, a53_size, module_files['A53']['size']),
            ('CR50', cr50_version_str, cr50_offset, cr50_size, module_files['CR50']['size']),
            ('CR51', cr51_version_str, cr51_offset, cr51_size, module_files['CR51']['size']),
            ('28dr', dr28_version_str, dr28_offset, dr28_size, module_files['28dr']['size']),
            ('UE', 'N/A', ue_offset, ue_size, module_files['UE']['size'])
        ]
        for name, version, offset, size, actual in modules:
            if actual > 0:
                echo(f"{name} version:     {version}")
                echo(f"{name} offset:      0x{offset:08X}")
                echo(f"{name} size:        {size} bytes")
            else:
                echo(f"{name}:             Not present")

        echo("-" * 60)
        echo(f"{'IMG_INFO size':<20} 4 KB")
        total_size = self.IMG_INFO_SIZE
        for module_name in ['BOOT', 'A53', 'CR50', 'CR51', '28dr', 'UE']:
            total_size += module_files[module_name]['aligned_size']
        echo(f"{'Total size':<20} {total_size} bytes")
        echo("=" * 60)

        return '\n'.join(text_content)

    def list_modules(self):
        print("SUPPORTED MODULE TYPES")
        print("=" * 65)
        print(f"{'Module':<10} {'Tag Suffix':<12} {'Required Files':<35} {'Tag Format'}")
        print("=" * 65)
        
        for module_type in ['CA53', 'CA53_PT', 'CR50', 'CR50_PT', 'CR51', 'CR51_PT', 'BOOT', 'BOOT_PT', 'BIT28', 'BIT28_PT']:
            config = self.MODULE_CONFIGS.get(module_type, {})
            suffix = self.GIT_TAG_SUFFIXES.get(module_type, '')
            
            files = []
            for key, path in config.get('files', {}).items():
                if path:
                    files.append(f"{key.upper()}={path}")
            
            file_str = ', '.join(files)
            
            if module_type.startswith('BIT28'):
                tag_format = "V25120500_bit28"
            else:
                tag_format = f"V1.0.0_{suffix}"
            
            print(f"{module_type:<10} {suffix:<12} {file_str:<35} {tag_format}")
            
        print("\nGIT TAGS REQUIRED:")
        print("  Module tags:     V1.0.0_ca53, V1.0.0_cr50, V1.0.0_boot, etc.")
        print("  BIT28 tags:      V25120500_bit28 (date format: YYMMDDHH)")
        print("  FSBL tag:        V1.0.0_fsbl  (required for BOOT modules)")
        print("  CABOOT tag:      V1.0.0_caboot  (required for BOOT modules)")
        print("  IMAGE tag:       V1.0.0, V1.1.0, V2.0.0 (no suffix, for combined image)")
        print("\nNotes:")
        print("  - PT and non-PT versions share the same Git tag suffix")
        print("  - BIT28 uses date-based tags: V25120500_bit28 = 2025-12-05 00")
        print("  - BIT28 version in image: 25120500 (without 'V' prefix)")
        print("  - Files are expected to be in the current directory")
        print("=" * 65)
    
    def build_all_modules(self):
        print_header("BUILDING ALL MODULES (EXCLUDE BOOT)")
        print_step(f"Date: {datetime.now().strftime('%b %d %Y')}")
        print_step(f"Time: {datetime.now().strftime('%H:%M:%S')}")
        print("=" * 60)
        
        # Define file paths
        source_file = r"..\FM_ZQ_bsp\bit\zu28dr.bit.tar.gz"
        target_dir = r"."
        os.makedirs(target_dir, exist_ok=True)

        # Extract the file
        try:
            with tarfile.open(source_file, 'r:gz') as tar:
                tar.extractall(target_dir)
            print_success(f"Successfully extracted {source_file} to {target_dir} directory")
        except Exception as e:
            print_warning(f"Extraction failed: {e}")

        success_count = 0
        failed_modules = []
        # 剔除BOOT模块，只打包CA53/CR50/CR51/BIT28
        all_modules = ['CA53', 'CR50', 'CR51', 'BIT28']
        
        for module_type in all_modules:
            print(f"\n" + "=" * 55)
            print(f"Building: {module_type}")
            print("=" * 55)
            
            try:
                if self.create_image(module_type):
                    success_count += 1
                else:
                    failed_modules.append(module_type)
            except Exception as e:
                print_error(f"Error building {module_type}: {e}")
                failed_modules.append(module_type)
        
        print(f"\n" + "=" * 60)
        print("BUILD SUMMARY")
        print(f"Successful: {success_count}/{len(all_modules)}")
        if failed_modules:
            print_error("Failed modules:", ', '.join(failed_modules))
        print("=" * 60)
        
        return success_count > 0

# ======================== 原mkimg.py 完整代码段 END ========================

# ======================== 整合后的主函数 START (核心修改区域) ========================
def main():
    epilog="""Examples:
        python mkimg.py --all           # One-click execution: Full compilation + package all modules → merge to generate final image
        python mkimg.py --clean         # Clear all files in the Images directory (keep the directory)
        python mkimg.py --build         # Auto compile and package: CA53/CR50/CR51/BIT28 (EXCLUDE BOOT)
        python mkimg.py --img           # Merge all MOD_XXX to generate final image file
        python mkimg.py --boot          # Auto compile FSBL/CA53_BOOT + package MOD_BOOT
        python mkimg.py --list          # View list of supported modules
        python mkimg.py -p CA53         # Auto compile CA53 + package MOD_CA53
        """

    parser = argparse.ArgumentParser(
        description='EMMC Image Packager Tool (MERGED: Build + Pack)',
        epilog=epilog,
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument('module', nargs='?', help='Module type to build')
    parser.add_argument('--list', action='store_true', help='List supported modules')
    parser.add_argument('--all', action='store_true', help='One-click execution: Full compilation + package all modules + merge to generate final image')
    parser.add_argument('--build', action='store_true', help='Build all modules (EXCLUDE BOOT) + auto compile')
    parser.add_argument('--boot', action='store_true', help='Compile FSBL/CA53_BOOT project + package to generate MOD_BOOT')
    parser.add_argument('--img', action='store_true', help='Combine all modules into image file')
    parser.add_argument('--clean', action='store_true', help='Clear all files in the Images directory')
    parser.add_argument('-p', '--project', help='Module type to build (same as module arg)')
    
    args = parser.parse_args()

    target_module = args.project if args.project else args.module

    packager = EMMCImagePackager()
    
    if args.clean:
        print_header("=== 执行清理操作: 清空 Images 目录所有文件 ===")
        clean_ok, clean_msg = clean_images()
        if clean_ok:
            print_success(clean_msg)
            return 0
        else:
            print_error(clean_msg)
            return 1

    if args.list:
        packager.list_modules()
        return 0


    if args.all:
        print_header("=== 一键全流程: 全量编译 → 打包所有模块 → 合并生成最终镜像 ===")
        # 第一步：全量编译CA53/CR50/CR51
        build_ok = build_all_for_pack()
        if not build_ok:
            print_error("全量编译失败，终止全流程")
            return 1
        # 第二步：打包所有模块生成MOD_XXX
        pack_ok = packager.build_all_modules()
        if not pack_ok:
            print_error("模块打包失败，终止全流程")
            return 1
        
        build_ok = build_by_module_type("BOOT")
        if not build_ok:
            print_error(f"BOOT 依赖编译失败，终止打包流程")
            return 1
        # 编译成功后执行打包
        success = packager.create_image("BOOT")
        # 第三步：合并所有MOD生成最终image文件
        img_ok,image_version = packager.combine_modules()
        if img_ok:
            print_header("=== 开始打包CA53/CR50/CR51相关文件+imginfo文件为tar.gz ===")
            tar_ok, tar_msg = package_images_tar_gz(tag=image_version)
            if tar_ok:
                print_success(tar_msg)
            print_success("\n 全流程执行完成:镜像已生成: image")
        return 0 if img_ok else 1

    if args.build:
        # 核心逻辑1: --build 先编译CA53/CR50/CR51，再打包(剔除BOOT)
        print_header("=== AUTO BUILD + PACK ALL (EXCLUDE BOOT) ===")
        build_ok = build_all_for_pack()
        if not build_ok:
            print_error("全量编译失败，终止打包流程")
            return 1
        pack_ok = packager.build_all_modules()
        return 0 if pack_ok else 1

    if args.img:
        success,image_version = packager.combine_modules()
        if success:
            print_header("=== 开始打包CA53/CR50/CR51相关文件为tar.gz ===")
            tar_ok, tar_msg = package_images_tar_gz(tag=image_version)
            if tar_ok:
                print_success(tar_msg)
            else:
                print_error(tar_msg)
        return 0 if success else 1


    if args.boot:
        print_header(f"=== AUTO BUILD BOOT DEPENDENCIES (FSBL + CA53_BOOT) ===")
        build_ok = build_by_module_type("BOOT")
        if not build_ok:
            print_error(f"BOOT 依赖编译失败，终止打包流程")
            return 1
        # 编译成功后执行打包
        success = packager.create_image("BOOT")
        return 0 if success else 1

    if target_module:
        if target_module not in packager.MODULE_TYPES:
            print_error(f"Error: Unsupported module type '{target_module}'")
            print("\nSupported modules:")
            for mod in sorted(packager.MODULE_TYPES.keys()):
                print(f"  {mod}")
            return 1
        # 核心逻辑2: 指定模块 -p XXX 先编译对应工程，再打包
        print_header(f"=== AUTO BUILD {target_module} DEPENDENCIES ===")
        build_ok = build_by_module_type(target_module)
        if not build_ok:
            print_error(f"{target_module} 依赖编译失败，终止打包流程")
            return 1
        # 编译成功后执行打包
        success = packager.create_image(target_module)
        return 0 if success else 1

    # 无参数时打印帮助
    print("EMMC Image Packager Tool (MERGED VERSION: Build + Pack)")
    print("=" * 62)
    print("One-stop Tool for IAR Compilation + Image Packaging")
    print("\nCore Usage:")
    print("  python mkimg.py --list        View supported modules")
    print("  python mkimg.py --all         One-click full process (Recommended): Compile + Package + Merge to generate final image")
    print("  python mkimg.py --build       Auto compile + Package all modules")
    print("  python mkimg.py --boot        Compile BOOT project + Package MOD_BOOT")
    print("  python mkimg.py -p CA53       Compile CA53 + Package MOD_CA53")
    print("  python mkimg.py -p CA5_PT     Compile CA53 + Package MOD_CA53")
    print("  python mkimg.py --img         Merge all MODs to generate final image")
    print("=" * 62)
    return 0

if __name__ == '__main__':
    sys.exit(main())