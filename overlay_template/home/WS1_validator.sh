#!/bin/sh


# verify the executable validator_sign.sh exists
[ -x "/examples/validator/validator_sign.sh" ] || {
	echo "executable '${VALIDATOR_SIGN}' does not exist" >&2
	exit 1
}

#verify that the validator_module is not loaded
if [ -e "/dev/validator_sign" ] 
then
	echo "validator module already loaded"
else
	insmod /lib/modules/3.10.31-ltsi/extra/validator_module.ko

fi

#verify that the utilities tee, sha256sum, sed and perl are available
type tee > /dev/null 2>&1 || {
	echo "tee utility is not available" >&2
	exit 1
}

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

#verify that the lab application is available
[ -x fpga_fft ] || {
	echo "fpga_fft application is not available" >&2
	echo "is everything in the /home directory?" >&2
	exit 1
}


./fpga_fft 0 128 > validate_file.txt

# hash the input file
THE_HASH=$(sha256sum validate_file.txt | sed -e "s/\([0-9a-fA-F]\{64\}\).*/\1/")
[ "${#THE_HASH}" -eq "64" ] || {
	echo "failed to compute input file hash" >&2
	exit 1
}

# sign the hash
echo -n ${THE_HASH} | perl -pe 'chomp;$_=pack("H*",$_)' | /examples/validator/validator_sign.sh > WS1_validator.sign

rm validate_file.txt
