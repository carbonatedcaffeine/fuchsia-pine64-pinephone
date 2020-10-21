#!/usr/bin/env bash

# Copyright 2018 The Fuchsia Authors
#
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT

set -eo pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ZIRCON_DIR="${DIR}/../../../../.."
SCRIPTS_DIR="${ZIRCON_DIR}/scripts"

BOARD=mt8167s_ref
ZIRCON_BUILD_DIR=
BOOT_IMG=
ZIRCON_BOOTIMAGE=

while getopts "B:o:z:" FLAG; do
    case $FLAG in
        B) ZIRCON_BUILD_DIR="${OPTARG}";;
        o) BOOT_IMG="${OPTARG}";;
        z) ZIRCON_BOOTIMAGE="${OPTARG}";;
        \?)
            echo unrecognized option
            exit 1
            ;;
    esac
done

if [[ -z "${ZIRCON_BUILD_DIR}" ]]; then
    echo must specify a Zircon build directory
    HELP
fi

ROOT_BUILD_DIR="${ZIRCON_BUILD_DIR%%.zircon}"

# boot shim for our board
BOOT_SHIM="${ROOT_BUILD_DIR}/${BOARD}-boot-shim.bin"

# zircon ZBI image with prepended boot shim
SHIMMED_ZIRCON_BOOTIMAGE="${ZIRCON_BOOTIMAGE}.shim"

cat "${BOOT_SHIM}" "${ZIRCON_BOOTIMAGE}" > "${SHIMMED_ZIRCON_BOOTIMAGE}"

mkimage -f auto -A arm64 -O linux -T kernel -C none -a 0x40080000 -e 0x40080000 \
    -d "${SHIMMED_ZIRCON_BOOTIMAGE}" \
    -b "${ZIRCON_DIR}/kernel/target/arm64/dtb/dummy-device-tree.dtb" "${BOOT_IMG}"
