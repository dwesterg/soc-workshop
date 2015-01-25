#!/bin/sh

# verify the /dev/validator_sign device node exists
[ -c /dev/validator_sign ] || {
	echo "/dev/validator_sign does not exist" >&2
	exit 1
}

# open /dev/validator_sign
exec 3<> /dev/validator_sign

# push 32 byte message from stdin into /dev/validator_sign device
dd bs=32 count=1 2> /dev/null >&3

# extract 96 byte signed message from /dev/validator_sign device
dd bs=96 count=1 2> /dev/null <&3

# close /dev/validator_sign
exec 3<> /dev/null

