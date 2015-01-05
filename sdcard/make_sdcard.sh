#!/bin/bash
PRELOADER=$1
SD_FAT_TAR=$2

dd if=/dev/zero of=./sdcard.img bs=1G count=1
LO_DEV=$(losetup --show -f sdcard.img)
echo Loopback device is ${LO_DEV}
read -p "Press [Enter] key to continue or CTRL-C to quit..."

sfdisk --force ${LO_DEV} < sdcard_parts

partprobe ${LO_DEV}

dd if=${PRELOADER} of=${LO_DEV}p3 bs=1 seek=0
mkfs -t vfat ${LO_DEV}p1
mkdir tmp
mount ${LO_DEV}p1 tmp
tar -xzvf ${SD_FAT_TAR} -C tmp
umount tmp
rmdir tmp
losetup -d ${LO_DEV}

