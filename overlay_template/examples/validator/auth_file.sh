#!/bin/sh

# verify input argument count
[ "${#}" -eq "3" ] || {
	echo "incorrect input argument count" >&2
	echo "USAGE: ${0} <validator_auth.sh path> <input file path> <sign file path>" >&2
	exit 1
}

# verify the executable validator_auth.sh exists
[ -x "${1}" ] || {
	echo "executable '${1}' does not exist" >&2
	exit 1
}

# verify the input file exists
[ -f "${2}" ] || {
	echo "file '${2}' does not exist" >&2
	exit 1
}

# verify the sign file exists
[ -f "${3}" ] || {
	echo "file '${3}' does not exist" >&2
	exit 1
}

#verify that the utilities sha256sum, sed perl and wc are available
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

type wc > /dev/null 2>&1 || {
	echo "wc utility is not available" >&2
	exit 1
}

# size the signature file
SIGN_SIZE=$(wc -c "${3}" | sed -e "s/\([0-9]*\).*/\1/")
[ "${SIGN_SIZE}" -eq "96" ] || {
	echo "ERROR: signature file is not 96 bytes." >&2
	exit 1
}

# authenticate the signature file
AUTH_RESULT=$(cat "${3}" | "${1}")
[ "${AUTH_RESULT}" == "SUCCESS" ] || {
	echo "ERROR: signature file does not authenticate." >&2
	exit 1
}

# hash the input file
THE_HASH=$(sha256sum ${2} | sed -e "s/\([0-9a-fA-F]\{64\}\).*/\1/")
[ "${#THE_HASH}" -eq "64" ] || {
	echo "failed to compute input file hash" >&2
	exit 1
}

# convert the sign file from binary to hex
# VERSION[2] : DESIGN HASH[10] : UNIQUE ID[8] : RANDOM SALT[12] : MESSAGE_IN[32] : HMAC_OUT[32]
VERSION=$(cat "${3}" | perl -pe '$_=unpack("H*",$_)' | sed -e "s/\(.\{4\}\)\(.\{20\}\)\(.\{16\}\)\(.\{24\}\)\(.\{64\}\)\(.\{64\}\)/\1/")
DESIGN_HASH=$(cat "${3}" | perl -pe '$_=unpack("H*",$_)' | sed -e "s/\(.\{4\}\)\(.\{20\}\)\(.\{16\}\)\(.\{24\}\)\(.\{64\}\)\(.\{64\}\)/\2/")
UNIQUE_ID=$(cat "${3}" | perl -pe '$_=unpack("H*",$_)' | sed -e "s/\(.\{4\}\)\(.\{20\}\)\(.\{16\}\)\(.\{24\}\)\(.\{64\}\)\(.\{64\}\)/\3/")
RANDOM_SALT=$(cat "${3}" | perl -pe '$_=unpack("H*",$_)' | sed -e "s/\(.\{4\}\)\(.\{20\}\)\(.\{16\}\)\(.\{24\}\)\(.\{64\}\)\(.\{64\}\)/\4/")
MESSAGE_IN=$(cat "${3}" | perl -pe '$_=unpack("H*",$_)' | sed -e "s/\(.\{4\}\)\(.\{20\}\)\(.\{16\}\)\(.\{24\}\)\(.\{64\}\)\(.\{64\}\)/\5/")
HMAC_OUT=$(cat "${3}" | perl -pe '$_=unpack("H*",$_)' | sed -e "s/\(.\{4\}\)\(.\{20\}\)\(.\{16\}\)\(.\{24\}\)\(.\{64\}\)\(.\{64\}\)/\6/")

# compare the file hash to the signed hash
[ "${THE_HASH}" == "${MESSAGE_IN}" ] || {
	echo "ERROR: computed file hash does not equal signed hash" >&2
	echo "FILE HASH = ${THE_HASH}" >&2
	echo "SIGN HASH = ${MESSAGE_IN}" >&2
	exit 1
}

# output success
echo "SUCCESSFUL AUTHENTICATION"
echo "    VERSION : ${VERSION}"
echo "DESIGN HASH : ${DESIGN_HASH}"
echo "    CHIP ID : ${UNIQUE_ID}"
echo "      NONCE : ${RANDOM_SALT}"
echo "       HASH : ${MESSAGE_IN}"
echo "       HMAC : ${HMAC_OUT}"

