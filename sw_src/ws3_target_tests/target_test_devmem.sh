[ "$(./demo_devmem -a | md5sum)" == "527bb73ec24ad037c40e943158a77229  -" ] && {
	echo "Validated the initial RAM contents."
} || {
	echo ""
	echo "ERROR: failed to validate the initial RAM contents, this may be expected."
	echo ""
}

[ "$(./demo_devmem -o | md5sum)" == "240fac46892f9f075200bdca43e4ce1a  -" ] && {
	echo "Validated the initial ROM contents."
} || {
	echo ""
	echo "ERROR: failed to validate the initial ROM contents."
	echo ""
}

./demo_devmem -d && {
	echo "Ran demo_devmem DMA."
} || {
	echo ""
	echo "ERROR: failed to run demo_devmem DMA."
	echo ""
}

[ "$(./demo_devmem -a | md5sum)" == "240fac46892f9f075200bdca43e4ce1a  -" ] && {
	echo "Validated the DMA RAM contents."
} || {
	echo ""
	echo "ERROR: failed to validate the DMA RAM contents."
	echo ""
}

cat /dev/zero | ./demo_devmem -f && {
	echo "Copied /dev/zero to demo_devmem RAM fill."
} || {
	echo ""
	echo "ERROR: failed to copy /dev/zero to demo_devmem RAM fill."
	echo ""
}

[ "$(./demo_devmem -a | md5sum)" == "0f343b0931126a20f133d67c2b018a3b  -" ] && {
	echo "Validated the ZERO'ed RAM contents."
} || {
	echo ""
	echo "ERROR: failed to validate the ZERO'ed RAM contents."
	echo ""
}

./demo_devmem -t | grep \
	-e "difference between snapshots.*0x00000041" \
	-e "difference between snapshots.*0x00000040" \
	-e "difference between snapshots.*0x0000003F" > /dev/null && {
	echo "Validated the demo_devmem timer operation."
} || {
	echo ""
	echo "ERROR: failed to validate the demo_devmem timer operation."
	echo ""
}

