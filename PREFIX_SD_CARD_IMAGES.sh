#!/bin/sh

[ $# -eq 1 ] || {
	echo ""
	echo "USAGE: ${0} <prefix>"
	echo "This script renames all files in the CWD named 'sd_card.*.image.gz' to"
	echo "'<prefix>.sd_card.*.image.gz'"
	echo ""
	exit 1
}

for NEXT in $(find . -name "sd_card.*.image.gz")
do
	FILE_NAME=$(echo ${NEXT} | sed -e "s/\.\///")
	mv "${FILE_NAME}" "${1}.${FILE_NAME}"
	echo "renamed '${1}.${FILE_NAME}'"
done

