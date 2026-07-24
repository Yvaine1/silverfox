#!/usr/bin/env python3
#!/usr/bin/env python3
import sys
import re
import os
import hashlib
from elftools.elf.elffile import ELFFile

if len(sys.argv) != 6:
    print("Usage:")
    print("parse_iar_map.py <elf> <map> <elf_out.txt> <map_out.txt> <symbol_table.c>")
    sys.exit(1)

elf_path = sys.argv[1]
map_path = sys.argv[2]
elf_out = sys.argv[3]
map_out = sys.argv[4]
out_c = sys.argv[5]

# ==========================================================
# 1. 解析 ELF 
# ==========================================================

all_elf_syms = []
valid_func_ranges = [] 

with open(elf_path, "rb") as f:
    elf = ELFFile(f)

    symtab = elf.get_section_by_name(".symtab")
    if not symtab:
        print("No .symtab found")
        sys.exit(1)

    for sym in symtab.iter_symbols():
        if sym["st_info"]["type"] != "STT_FUNC":
            continue

        addr = sym["st_value"]
        size = sym["st_size"]
        name = sym.name

        all_elf_syms.append((addr, size, name))

valid_funcs = []
zero_size_syms = []
for addr, size, name in all_elf_syms:
    if size > 0:
        valid_funcs.append((addr, size, name))
        valid_func_ranges.append((addr, addr + size))
    else:
        zero_size_syms.append((addr, size, name))


filtered_zero_syms = []
for z_addr, z_size, z_name in zero_size_syms:

    is_inside_valid_range = False
    
    for (range_start, range_end) in valid_func_ranges:
        if range_start <= z_addr < range_end:
            is_inside_valid_range = True
            break  
    
    if not is_inside_valid_range:
        filtered_zero_syms.append((z_addr, z_size, z_name))


tmp_symbols = valid_funcs + filtered_zero_syms

tmp_symbols.sort(key=lambda x: x[0])

# ==========================================================
# 2. 合并 ?_NN
# ==========================================================

merged_symbols = []

for addr, size, name in tmp_symbols:

    if not merged_symbols:
        merged_symbols.append([addr, size, name])
        continue

    prev_addr, prev_size, prev_name = merged_symbols[-1]
    prev_end = prev_addr + prev_size

    if name.startswith("?_") and size > 0:
        if addr == prev_end:
            merged_symbols[-1][1] += size
            continue

    merged_symbols.append([addr, size, name])

# ==========================================================
# 3. 合并相同地址
# ==========================================================

elf_symbols = []

for addr, size, name in merged_symbols:

    if not elf_symbols:
        elf_symbols.append([addr, size, name])
        continue

    prev_addr, prev_size, prev_name = elf_symbols[-1]

    if addr == prev_addr:

        elf_symbols[-1][1] = max(prev_size, size)

        names = set(prev_name.split("/"))
        names.add(name)
        elf_symbols[-1][2] = "/".join(sorted(names))

    else:
        elf_symbols.append([addr, size, name])

# ==========================================================
# 4. 解析 MAP
# ==========================================================

map_symbols = []

with open(map_path, "r", encoding="utf-8", errors="ignore") as f:
    lines = f.readlines()

i = 0
while i < len(lines):

    line = lines[i]

    # ----------------------------
    # 情况1：函数名单独一行
    # ----------------------------
    if line.strip() and "0x" not in line:

        name = line.strip()

        if i + 1 < len(lines):

            addr_match = re.search(
                r"0x([0-9A-Fa-f']+)\s+0x([0-9A-Fa-f']+)\s+Code\s+\S+\s+(\S+)",
                lines[i + 1]
            )

            if addr_match:

                addr = int(addr_match.group(1).replace("'", ""), 16)
                size = int(addr_match.group(2).replace("'", ""), 16)
                obj  = addr_match.group(3)

                map_symbols.append((addr, size, name, obj))

                i += 2
                continue

            addr_match = re.search(
                r"0x([0-9A-Fa-f']+)\s+Code\s+\S+\s+(\S+)",
                lines[i + 1]
            )

            if addr_match:

                addr = int(addr_match.group(1).replace("'", ""), 16)
                size = 0
                obj  = addr_match.group(2)

                map_symbols.append((addr, size, name, obj))

                i += 2
                continue


    # ----------------------------
    # 情况2：单行 (有size)
    # ----------------------------
    m = re.match(
        r"\s*(\S.*?)\s+0x([0-9A-Fa-f']+)\s+0x([0-9A-Fa-f']+)\s+Code\s+\S+\s+(\S+)",
        line
    )

    if m:

        name = m.group(1).strip()
        addr = int(m.group(2).replace("'", ""), 16)
        size = int(m.group(3).replace("'", ""), 16)
        obj  = m.group(4)

        map_symbols.append((addr, size, name, obj))

        i += 1
        continue

    # ----------------------------
    # 情况3：单行 (没有size)
    # ----------------------------
    m = re.match(
        r"\s*(\S.*?)\s+0x([0-9A-Fa-f']+)\s+Code\s+\S+\s+(\S+)",
        line
    )

    if m:

        name = m.group(1).strip()
        addr = int(m.group(2).replace("'", ""), 16)
        size = 0
        obj  = m.group(3)

        map_symbols.append((addr, size, name, obj))

    i += 1

# 地址排序
map_symbols.sort(key=lambda x: x[0])
# 写 MAP 输出
with open(map_out, "w", encoding="utf-8") as f:
    for addr, size, name, obj in map_symbols:
        f.write(f"0x{addr:016X} {size} {name} {obj}\n")

print("MAP symbols:", len(map_symbols))

# ==========================================================
# 5. MAP地址字典
# ==========================================================

map_dict = {}

for addr, size, name, obj in map_symbols:
    if addr not in map_dict:
        map_dict[addr] = (name, obj)

# ==========================================================
# 6. ELF符号补充 objfile + C++反查
# ==========================================================

final_syms = []

for addr, size, name in elf_symbols:

    obj = ""

    if addr in map_dict:

        map_name, map_obj = map_dict[addr]

        obj = map_obj

        if name.startswith("_Z"):
            name = map_name

    final_syms.append((addr, size, name, obj))

with open(elf_out, "w", encoding="utf-8") as f:
    for addr, size, name, obj in final_syms:
        f.write(f"0x{addr:016X} {size} {name} {obj}\n")

print("ELF symbols:", len(final_syms))

# ============================================================
# 7. 生成 C 符号表
# ============================================================

c = []
c.append("/* Auto-generated: ELF + MAP merged symbol table */")
c.append("#include \"symbol_table.h\"")
c.append("")
c.append("const symbol_t g_symbol_table[] = {")

for addr, size, name, obj in final_syms:

    safe = name.replace('"', '\\"')
    obj_safe = obj.replace('"', '\\"')

    c.append(
        f'    {{ (uintptr_t)0x{addr:016X}ULL, 0x{size:X}, "{obj_safe}", "{safe}" }},'
    )

c.append("};")
c.append("")
c.append(
    "const int g_symbol_count = sizeof(g_symbol_table)/sizeof(g_symbol_table[0]);"
)

new = "\n".join(c) + "\n"

# ============================================================
# 8. 内容不变则不写
# ============================================================

if os.path.exists(out_c):

    old = open(out_c, "r", encoding="utf-8", errors="ignore").read()

    if hashlib.sha256(old.encode()).hexdigest() == hashlib.sha256(new.encode()).hexdigest():

        print("No change; not overwriting", out_c)
        sys.exit(0)

with open(out_c, "w", newline="\n", encoding="utf-8") as f:
    f.write(new)

print("Wrote", out_c)