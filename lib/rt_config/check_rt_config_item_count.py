#!/usr/bin/env python3
"""Validate the linked RT_CONFIG_ITEM iterable-section count."""

import argparse
import re
import subprocess
import sys


NM_LINE_RE = re.compile(
    r"^(?P<addr>[0-9A-Fa-f]+)(?:\s+(?P<size>[0-9A-Fa-f]+))?\s+"
    r"(?P<type>\S+)\s+(?P<name>\S+)$"
)


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", required=True, help="Path to the final Zephyr ELF")
    parser.add_argument("--nm", required=True, help="Path to the target nm executable")
    parser.add_argument(
        "--max-items",
        required=True,
        type=int,
        help="Configured CONFIG_RT_MAX_ITEMS value",
    )
    parser.add_argument(
        "--section",
        default="rt_config_item",
        help="Iterable section struct name",
    )
    return parser.parse_args()


def run_nm(nm_path, elf_path):
    result = subprocess.run(
        [nm_path, "-a", "-S", elf_path],
        check=False,
        capture_output=True,
        text=True,
    )

    if result.returncode != 0:
        sys.stderr.write(result.stdout)
        sys.stderr.write(result.stderr)
        raise RuntimeError(f"{nm_path} failed for {elf_path}")

    return result.stdout.splitlines()


def parse_symbols(lines):
    symbols = []

    for line in lines:
        match = NM_LINE_RE.match(line.strip())
        if not match:
            continue

        size_text = match.group("size")
        symbols.append(
            {
                "addr": int(match.group("addr"), 16),
                "size": int(size_text, 16) if size_text else None,
                "name": match.group("name"),
            }
        )

    return symbols


def find_symbol(symbols, name):
    for symbol in symbols:
        if symbol["name"] == name:
            return symbol

    return None


def count_section_items(symbols, section):
    start_name = f"_{section}_list_start"
    end_name = f"_{section}_list_end"
    start_symbol = find_symbol(symbols, start_name)
    end_symbol = find_symbol(symbols, end_name)

    if start_symbol is None or end_symbol is None:
        raise RuntimeError(
            f"Could not find {start_name}/{end_name}; rt_config iterable section is missing"
        )

    start = start_symbol["addr"]
    end = end_symbol["addr"]

    if end < start:
        raise RuntimeError(
            f"Invalid {section} bounds: start=0x{start:x}, end=0x{end:x}"
        )

    item_addresses = {
        symbol["addr"]
        for symbol in symbols
        if symbol["size"]
        and symbol["size"] > 0
        and start <= symbol["addr"] < end
        and symbol["name"] not in {start_name, end_name}
    }

    return len(item_addresses), end - start


def main():
    args = parse_args()

    try:
        symbols = parse_symbols(run_nm(args.nm, args.elf))
        count, byte_count = count_section_items(symbols, args.section)
    except Exception as exc:
        print(f"rt_config item-count validation failed: {exc}", file=sys.stderr)
        return 2

    print(
        f"rt_config: linked {count} item(s), "
        f"CONFIG_RT_MAX_ITEMS={args.max_items}, section_bytes={byte_count}"
    )

    if count > args.max_items:
        print(
            f"rt_config item count {count} exceeds CONFIG_RT_MAX_ITEMS={args.max_items}",
            file=sys.stderr,
        )
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
