#!/bin/sh

#
# validate input arguments
#
[ $# -eq 1 ] || {
	
	echo ""
	echo "USAGE: $0 <file_name>"
	echo ""
	echo "This script calculates the crc32 for <file_name> and stores it in file"
	echo "<file_name>.crc32"
	echo ""
	exit 1
}

[ -e ${1} ] || {

	echo ""
	echo "ERROR: unable to locate file \"${1}\" for crc calculation."
	echo ""
	exit 1
}

#
# create a temporary file for our TCL script that we feed to quartus_sh
#
TEMP_SCRIPT=$( mktemp sc_script.XXXXXX )

[ -e ${TEMP_SCRIPT} ] || {

	echo ""
	echo "ERROR: unable to create temporary file ${TEMP_SCRIPT}"
	echo ""
	exit 1
}

#
# create a TCL script that we feed to quartus_sh
#
cat > $TEMP_SCRIPT <<END_OF_SCRIPT

package require crc32

variable file_chan [ open "${1}.crc32" [ list WRONLY CREAT ] ]
fconfigure \$file_chan -translation binary

puts \$file_chan [ ::crc::crc32 -format "%X" -file ${1} ]

close \$file_chan
unset file_chan

exit 0

END_OF_SCRIPT

#
# feed our TCL script to quartus_sh
#
quartus_sh --script=${TEMP_SCRIPT}

[ $? -eq 0 ] || {
	rm -f ${TEMP_SCRIPT}
	echo ""
	echo "CRC32 failed..."
	echo ""

	exit 1
}

rm -f ${TEMP_SCRIPT}
echo ""
echo "CRC32 succeeded..."
echo ""
exit 0
