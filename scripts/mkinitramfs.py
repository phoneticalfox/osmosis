#!/usr/bin/env python3
import os
import struct
import sys
from pathlib import Path


def write_entry(out, name, data):
    name_bytes = name.encode("utf-8")
    if len(name_bytes) >= 64:
        raise SystemExit(f"initramfs entry name too long: {name}")
    header = name_bytes.ljust(64, b"\0") + struct.pack("<I", len(data))
    out.write(header)
    out.write(data)
    if len(data) % 4:
        out.write(b"\0" * (4 - len(data) % 4))


def main():
    if len(sys.argv) < 4:
        raise SystemExit("usage: mkinitramfs.py <out> <hello_user.elf> <hello.txt>")

    out_path = Path(sys.argv[1])
    files = [
        ("bin/hello_user", Path(sys.argv[2])),
        ("init/hello.txt", Path(sys.argv[3])),
    ]

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("wb") as out:
        for name, path in files:
            data = path.read_bytes()
            write_entry(out, name, data)
        out.write(b"\0" * 64)
        out.write(struct.pack("<I", 0))


if __name__ == "__main__":
    main()
