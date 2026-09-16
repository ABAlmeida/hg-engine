#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_dir="$(cd "${script_dir}/.." && pwd)"
log_dir="${repo_dir}/.scratch"
log_file="${log_dir}/olivine-gdb.txt"
command_file="${script_dir}/debug_olivine_gdb.gdb"

mkdir -p "${log_dir}"

if command -v gdb-multiarch >/dev/null 2>&1; then
    gdb_bin="$(command -v gdb-multiarch)"
elif [[ -x /ucrt64/bin/gdb-multiarch.exe ]]; then
    gdb_bin=/ucrt64/bin/gdb-multiarch.exe
elif command -v gdb >/dev/null 2>&1; then
    gdb_bin="$(command -v gdb)"
else
    echo "Could not find gdb-multiarch or gdb in this MSYS environment." >&2
    exit 1
fi

cat <<EOF
Connecting to melonDS ARM9 GDB stub on 127.0.0.1:3333.

Before continuing, melonDS must have:
  - its ARM9 GDB stub enabled on port 3333
  - JIT disabled
  - the game already running

When the game hangs:
  1. Press Ctrl-C in this terminal.
  2. At the (gdb) prompt, enter: hang-snapshot
  3. Enter: continue
  4. Repeat the snapshot two more times if possible.
  5. Enter: quit

The complete session is logged to:
  ${log_file}
EOF

cd "${repo_dir}"
exec "${gdb_bin}" --quiet --command="${command_file}"
