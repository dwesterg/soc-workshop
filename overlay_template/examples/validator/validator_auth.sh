#!/bin/sh

# verify the /dev/validator_auth device node exists
[ -c /dev/validator_auth ] || {
	echo "/dev/validator_auth does not exist" >&2
	exit 1
}

# open /dev/validator_auth
exec 3<> /dev/validator_auth

# push 96 byte signed message from stdin into /dev/validator_auth device
dd bs=96 count=1 2> /dev/null >&3

# extract 4 byte result from /dev/validator_auth device
dd bs=4 count=1 2> /dev/null <&3 | hexdump -e '1/4 "%08X" "\n"' | sed -e "s/00000000/SUCCESS/" -e "s/FFFFFFFF/FAILURE/"

# close /dev/validator_auth
exec 3<> /dev/null

