#!/bin/sh

# verify root user is running script
[ "${EUID}" -ne 0 ] && {
		echo ""
		echo "ERROR: script must be run as root, use 'sudo'"
		echo ""
		exit 1
}

[ -f "${1}" ] || {
	echo ""
	echo "ERROR: file not found"
	echo "'${1}'"
	echo ""
	exit 1	
}

./create_sd_card_images.sh --compress --sd_fat=$1 --board=ALTERA_AV_SOC || {
	echo "ERROR"
	exit 1	
}
./create_sd_card_images.sh --compress --sd_fat=$1 --board=ALTERA_CV_SOC || {
	echo "ERROR"
	exit 1	
}
./create_sd_card_images.sh --compress --sd_fat=$1 --board=ARROW_SOCKIT || {
	echo "ERROR"
	exit 1	
}
 ./create_sd_card_images.sh --compress --sd_fat=$1 --board=CRITICALLINK_MITYSOM_DEVKIT || {
 	echo "ERROR"
 	exit 1	
 }
./create_sd_card_images.sh --compress --sd_fat=$1 --board=DE0_NANO_SOC || {
	echo "ERROR"
	exit 1	
}
# ./create_sd_card_images.sh --compress --sd_fat=$1 --board=MACNICA_HELIO_14 || {
# 	echo "ERROR"
# 	exit 1	
# }

