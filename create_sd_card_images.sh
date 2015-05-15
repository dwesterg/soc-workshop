#!/bin/sh

PROGRAM_NAME="$(basename ${0})"
THE_SD_FAT_TAR_GZ=
DEV_BOARD_TYPE=

###############################################################################
#
# Parse the command line arguments
#
###############################################################################
for i in "$@"
do
case $i in
	-h|--help)
		echo ""
		echo "USAGE: ${PROGRAM_NAME} \\"
		echo "	--sd_fat=<path-to-sd_fat.*.tar.gz> \\"
		echo "	--board=<dev-board-type>"
		echo ""
		echo "Valid board types are:"
		echo "	ALTERA_AV_SOC"
		echo "	ALTERA_CV_SOC"
		echo "	ARROW_SOCKIT"
		echo "	CRITICALLINK_MITYSOM_DEVKIT"
		echo "	DE0_NANO_SOC"
		echo "	MACNICA_HELIO_14"
		echo ""
		echo "Must be run as root user, or 'sudo'."
		echo ""
		exit 1
	;;
	--sd_fat=*)
		[ -z "${THE_SD_FAT_TAR_GZ}" ] || {
			echo ""
			echo "ERROR: multiple --sd_fat arguments on command line"
			echo ""
			exit 1
		}
		THE_SD_FAT_TAR_GZ="${i#*=}"
		shift
	;;
	--board=ALTERA_AV_SOC)
		[ -z "${DEV_BOARD_TYPE}" ] || {
			echo ""
			echo "ERROR: multiple --board arguments on command line"
			echo ""
			exit 1
		}
		DEV_BOARD_TYPE="${i#*=}"
		shift
	;;
	--board=ALTERA_CV_SOC)
		[ -z "${DEV_BOARD_TYPE}" ] || {
			echo ""
			echo "ERROR: multiple --board arguments on command line"
			echo ""
			exit 1
		}
		DEV_BOARD_TYPE="${i#*=}"
		shift
	;;
	--board=ARROW_SOCKIT)
		[ -z "${DEV_BOARD_TYPE}" ] || {
			echo ""
			echo "ERROR: multiple --board arguments on command line"
			echo ""
			exit 1
		}
		DEV_BOARD_TYPE="${i#*=}"
		shift
	;;
	--board=CRITICALLINK_MITYSOM_DEVKIT)
		[ -z "${DEV_BOARD_TYPE}" ] || {
			echo ""
			echo "ERROR: multiple --board arguments on command line"
			echo ""
			exit 1
		}
		DEV_BOARD_TYPE="${i#*=}"
		shift
	;;
	--board=DE0_NANO_SOC)
		[ -z "${DEV_BOARD_TYPE}" ] || {
			echo ""
			echo "ERROR: multiple --board arguments on command line"
			echo ""
			exit 1
		}
		DEV_BOARD_TYPE="${i#*=}"
		shift
	;;
	--board=MACNICA_HELIO_14)
		[ -z "${DEV_BOARD_TYPE}" ] || {
			echo ""
			echo "ERROR: multiple --board arguments on command line"
			echo ""
			exit 1
		}
		DEV_BOARD_TYPE="${i#*=}"
		shift
	;;
	*)
		# unknown option
	;;
esac
done

# verify root user is running script
[ "${EUID}" -ne 0 ] && {
		echo ""
		echo "ERROR: script must be run as root, use 'sudo'"
		echo ""
		exit 1
}

###############################################################################
#
# Change these variables to apply to your environment
#
###############################################################################
MY_SD_CARD_IMAGE_1M_BLOCKS="512"

###############################################################################
#
# These are variables used by the script
#
###############################################################################
MY_SD_FAT_MNT="$(mktemp --tmpdir=. --directory TMP_SD_FAT_MNT.XXXX)"
MY_TMP_TAR="$(mktemp --tmpdir=. --directory TMP_TAR.XXXX)"

###############################################################################
#
# Verify that all the required input directories and files exist
#
###############################################################################

[ -f "${THE_SD_FAT_TAR_GZ:?must specify --sd_fat argument, see --help for details}" ] || {
	echo "ERROR: could not locate file '${THE_SD_FAT_TAR_GZ}'"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

[ -n "${DEV_BOARD_TYPE}" ] || {
	echo "ERROR: valid --board argument required, see --help for details"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

[ -d "${MY_SD_FAT_MNT}" ] || {
	echo "ERROR: could not locate temp directory '${MY_SD_FAT_MNT}'"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

[ -d "${MY_TMP_TAR}" ] || {
	echo "ERROR: could not locate temp directory '${MY_TMP_TAR}'"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

###############################################################################
#
# Verify that none of our working files or directories already exist
#
###############################################################################

###############################################################################
#
# Build our output file name
#
###############################################################################
MY_SD_CARD_IMAGE="${THE_SD_FAT_TAR_GZ#*.}"
MY_SD_CARD_IMAGE="${MY_SD_CARD_IMAGE%%.*}"
MY_SD_CARD_IMAGE="sd_card.${DEV_BOARD_TYPE}.${MY_SD_CARD_IMAGE}.image"

###############################################################################
#
# Echo what we're doing from here on...
#
###############################################################################
echo "Creating SD card image file filled with 0xFF pattern."
dd if=/dev/zero bs=1M count=${MY_SD_CARD_IMAGE_1M_BLOCKS} 2> /dev/null | tr '\000' '\377' > ${MY_SD_CARD_IMAGE} || { 
	echo "ERROR"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Attaching SD card image file to loop device."
MY_LOOP_DEV=$(losetup --show -f ${MY_SD_CARD_IMAGE}) || { 
	echo "ERROR"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Creating partition table in MBR of SD card image file."
fdisk ${MY_LOOP_DEV} <<EOF > /dev/null 2>&1
n
p
3

+1M
n
p
1

+256M
n
p
2


t
1
0b
t
2
83
t
3
a2
w
EOF

echo "Detaching SD card image file from loop device."
losetup -d ${MY_LOOP_DEV} || { 
	echo "ERROR"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Attaching SD card image file to loop device with partition scan."
MY_LOOP_DEV=$(losetup --show -f --partscan ${MY_SD_CARD_IMAGE}) || { 
	echo "ERROR"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Verify loop partition 1 exists."
[ -b "${MY_LOOP_DEV}p1" ] || {
	echo "ERROR"
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Verify loop partition 2 exists."
[ -b "${MY_LOOP_DEV}p2" ] || {
	echo "ERROR"
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Verify loop partition 3 exists."
[ -b "${MY_LOOP_DEV}p3" ] || {
	echo "ERROR"
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Initializing FAT volume in partition 1 of SD card image file."
mkfs -t vfat -F 32 ${MY_LOOP_DEV}p1 > /dev/null || { 
	echo "ERROR"
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Mounting FAT partition of SD card image file."
mount ${MY_LOOP_DEV}p1 ${MY_SD_FAT_MNT} || { 
	echo "ERROR"
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Extracting sd_fat.tar.gz."
tar -C ${MY_TMP_TAR} -xf ${THE_SD_FAT_TAR_GZ} || { 
	echo "ERROR"
	umount ${MY_SD_FAT_MNT}
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Copying into FAT partition."
cp -R ${MY_TMP_TAR}/* ${MY_SD_FAT_MNT} || { 
	echo "ERROR"
	umount ${MY_SD_FAT_MNT}
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Copying preloader image into partition 3 of SD card image file."
[ -f "${MY_SD_FAT_MNT}/${DEV_BOARD_TYPE}/preloader-mkpimage.bin" ] || {
	echo "ERROR: could not locate preloader image file"
	umount ${MY_SD_FAT_MNT}
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}
dd if="${MY_SD_FAT_MNT}/${DEV_BOARD_TYPE}/preloader-mkpimage.bin" of=${MY_LOOP_DEV}p3 2> /dev/null || { 
	echo "ERROR"
	umount ${MY_SD_FAT_MNT}
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Unmount the FAT partition."
umount ${MY_SD_FAT_MNT} || { 
	echo "ERROR"
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Initializing ext3 volume in partition 2 of SD card image file."
mkfs -t ext3 ${MY_LOOP_DEV}p2 > /dev/null 2>&1 || { 
	echo "ERROR"
	losetup -d ${MY_LOOP_DEV}
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Detaching SD card image file from loop device."
losetup -d ${MY_LOOP_DEV} || { 
	echo "ERROR"
	rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR}
	exit 1
}

echo "Removing temporary working directories."
rm -Rf ${MY_SD_FAT_MNT} ${MY_TMP_TAR} || { 
	echo "ERROR"
	exit 1
}

echo "SD card image created."
echo "'${MY_SD_CARD_IMAGE}' is now ready to be copied onto an SD card."

