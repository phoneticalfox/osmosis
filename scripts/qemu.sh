#!/usr/bin/env bash
set -euo pipefail

# Small, side-effect-free QEMU launcher. Dependency installation belongs to the
# developer or CI environment; running the kernel must never mutate the host.
# Set QEMU_BIN to use a non-default binary or an explicit path.

QEMU_BIN="${QEMU_BIN:-qemu-system-i386}"

if ! command -v "${QEMU_BIN}" >/dev/null 2>&1; then
    echo "QEMU ($QEMU_BIN) is not available." >&2
    echo "Install qemu-system-x86 or set QEMU_BIN to an existing executable." >&2
    exit 127
fi

exec "${QEMU_BIN}" "$@"
