#!/usr/bin/env bash
set -euo pipefail

# Wrapper for running QEMU in development environments that may not have it
# pre-installed. If QEMU is already available in PATH, we exec it directly.
# Otherwise (and if running as root), we install the minimal package set from
# the distribution repositories and try again.
#
# Environment knobs:
#   QEMU_BIN               Override the QEMU binary name (default: qemu-system-i386).
#   OSMOSIS_QEMU_AUTO_INSTALL
#                          Set to 0 to skip auto-install and just fail when QEMU
#                          is missing.

QEMU_BIN="${QEMU_BIN:-qemu-system-i386}"
AUTO_INSTALL="${OSMOSIS_QEMU_AUTO_INSTALL:-1}"
PACKAGE_NAME="qemu-system-x86"

maybe_install_qemu() {
    if [ "$AUTO_INSTALL" = "0" ]; then
        return 1
    fi

    if [ "$(id -u)" -ne 0 ]; then
        echo "QEMU ($QEMU_BIN) not found and auto-install skipped (not running as root)." >&2
        echo "Install ${PACKAGE_NAME} or set QEMU_BIN to an existing binary." >&2
        return 1
    fi

    echo "QEMU ($QEMU_BIN) not found; installing ${PACKAGE_NAME} via apt-get..." >&2
    export DEBIAN_FRONTEND=noninteractive
    apt-get update -y
    apt-get install -y --no-install-recommends "${PACKAGE_NAME}"
}

if ! command -v "${QEMU_BIN}" >/dev/null 2>&1; then
    maybe_install_qemu || {
        echo "QEMU ($QEMU_BIN) is not available. Install it or point QEMU_BIN at a custom build." >&2
        exit 1
    }
fi

exec "${QEMU_BIN}" "$@"
