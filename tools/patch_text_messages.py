#!/usr/bin/env python3
"""Apply sparse, source-controlled replacements to extracted message banks."""

import argparse
import json
import subprocess
import tempfile
from pathlib import Path


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--msgenc", required=True, type=Path)
    parser.add_argument("--charmap", required=True, type=Path)
    parser.add_argument("--msgdata-dir", required=True, type=Path)
    parser.add_argument("patches", nargs="+", type=Path)
    return parser.parse_args()


def run_msgenc(msgenc, *args):
    result = subprocess.run(
        [str(msgenc), *map(str, args)],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip()
        raise RuntimeError(f"msgenc failed: {detail}")


def read_decoded_messages(path):
    text = path.read_bytes().decode("utf-8")
    # msgenc explicitly writes CRLF while its Windows text stream also
    # translates LF to CRLF, producing CR-CR-LF between records. Do not use
    # splitlines(): it treats each CR as a separate boundary and shifts every
    # message index after the first record.
    for separator in ("\r\r\n", "\r\n", "\n"):
        if separator in text:
            messages = text.split(separator)
            if messages[-1] == "":
                messages.pop()
            return messages
    return [text]


def main():
    args = parse_args()
    patch_sets = []
    for patch_path in args.patches:
        with patch_path.open(encoding="utf-8") as patch_file:
            patch_sets.append((patch_path, json.load(patch_file)))

    with tempfile.TemporaryDirectory(dir=args.msgdata_dir) as temp_dir_name:
        temp_dir = Path(temp_dir_name)
        for patch_path, banks in patch_sets:
            for bank_name, replacements in banks.items():
                # narchive names message members with a three-digit index
                # (7_000 through 7_999), while patch files use ordinary
                # decimal bank IDs for readability.
                bank_path = args.msgdata_dir / f"7_{int(bank_name):03d}"
                decoded_path = temp_dir / f"{bank_name}.txt"
                if not bank_path.is_file():
                    raise FileNotFoundError(f"missing message bank: {bank_path}")

                run_msgenc(args.msgenc, "-d", "-c", args.charmap, bank_path, decoded_path)
                messages = read_decoded_messages(decoded_path)

                for index_text, replacement in replacements.items():
                    index = int(index_text)
                    if index >= len(messages):
                        raise IndexError(
                            f"{patch_path}: bank {bank_name} has no message {index}"
                        )
                    expected = replacement["expected"]
                    if messages[index] == replacement["text"]:
                        continue
                    if expected not in messages[index]:
                        raise ValueError(
                            f"{patch_path}: bank {bank_name}, message {index} no longer "
                            f"contains the expected source text"
                        )
                    messages[index] = replacement["text"]

                decoded_path.write_text(
                    "\r\n".join(messages) + "\r\n",
                    encoding="utf-8",
                    newline="",
                )
                run_msgenc(args.msgenc, "-e", "-c", args.charmap, decoded_path, bank_path)


if __name__ == "__main__":
    main()
