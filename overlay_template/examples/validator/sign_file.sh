#!/bin/sh

# verify input argument count
[ "${#}" -eq "2" ] || {
	echo "incorrect input argument count" >&2
	echo "USAGE: ${0} <validator_sign.sh path> <input file path>" >&2
	exit 1
}

# verify the executable validator_sign.sh exists
[ -x "${1}" ] || {
	echo "executable '${1}' does not exist" >&2
	exit 1
}

# verify the input file exists
[ -f "${2}" ] || {
	echo "file '${2}' does not exist" >&2
	exit 1
}

#verify that the utilities sha256sum, sed and perl are available
type sha256sum > /dev/null 2>&1 || {
	echo "sha256sum utility is not available" >&2
	exit 1
}

type sed > /dev/null 2>&1 || {
	echo "sed utility is not available" >&2
	exit 1
}

type perl > /dev/null 2>&1 || {
	echo "perl utility is not available" >&2
	exit 1
}

# hash the input file
THE_HASH=$(sha256sum ${2} | sed -e "s/\([0-9a-fA-F]\{64\}\).*/\1/")
[ "${#THE_HASH}" -eq "64" ] || {
	echo "failed to compute input file hash" >&2
	exit 1
}

# sign the hash
echo -n ${THE_HASH} | perl -pe 'chomp;$_=pack("H*",$_)' | "${1}" > "${2}.sign"

