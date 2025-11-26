#!/usr/bin/env bash
set -e # exit if any cmd returns non zero

ROOT_DIR="$(dirname "$0")/.."

usage() {
    echo "Usage:"
    echo "  $0 -l {partition}       # load kmodule & mount partition of disk "
    echo "  $0 -e                   # unmount disk and remove kmodule"
    exit 1
}

if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    usage
    exit 0
fi

if [ $# -eq 2 ]; then
    if [[ $1 == "-l" ]]; then
        # Load kmod and mount disk
        echo "Loading nbd"
        sudo modprobe nbd max_part=8
        echo "Connecting disk"
        sudo qemu-nbd --connect=/dev/nbd0 -f raw $ROOT_DIR/disk.img
        echo "Listing partitions"
        lsblk /dev/nbd0
        sudo mkdir -p /mnt/qemu_mount
        echo "Mounting partition $2"
        sudo mount /dev/nbd0p$2 /mnt/qemu_mount
        echo "Mounted partition $2 of disk.img to /mnt/qemu_mount"
    fi
fi

if [ $# -eq 1 ]; then
    if [ $1 == "-e" ]; then
        # Unmount disk and unload kmod
        sudo umount /mnt/qemu_mount
        echo "Unmounted disk"
        sudo qemu-nbd --disconnect /dev/nbd0
        sudo rmmod -f nbd
        echo "Removed module"
    fi
fi


# sudo qemu-nbd --connect=/dev/nbd0 disk.img
# lsblk /dev/nbd0
# sudo mkdir -p /mnt/qemu_mount
# sudo mount /dev/nbd0p1 /mnt/qemu_mount
# sudo umount /mnt/qemu_mount
# sudo qemu-nbd --disconnect /dev/nbd0
# # You can also use `sudo rmmod nbd` to remove the module if no longer needed.
