#!/bin/sh

# verify the files we need exist
[ -f "validation_archive.tar.gz" ] || {
	echo ""
	echo "ERROR: file not found 'validation_archive.tar.gz'"
	echo ""
	exit 1
}

[ -f "validation_archive.tar.gz.sign" ] || {
	echo ""
	echo "ERROR: file not found 'validation_archive.tar.gz.sign'"
	echo ""
	exit 1
}

# authenticate the validation archive
echo ""
echo "--------------------------------------------------------------------------------"
echo "  Authenticate validation archive"
echo "--------------------------------------------------------------------------------"
/examples/validator/auth_file.sh \
	/examples/validator/validator_auth.sh \
	validation_archive.tar.gz \
	validation_archive.tar.gz.sign || {
		echo ""
		echo "ERROR: 'validation_archive.tar.gz' authentication failed"
		echo ""
		exit 1
	}

# decompress the validation archive
gunzip validation_archive.tar.gz || {
	echo ""
	echo "ERROR: while decompressing 'validation_archive.tar.gz'"
	echo ""
	exit 1
}

# extract the validation archive
tar xf validation_archive.tar || {
	echo ""
	echo "ERROR: while extracting 'validation_archive.tar'"
	echo ""
	exit 1
}

# change directory into the validation archive
cd validation_archive || {
	echo ""
	echo "ERROR: while changing directory into 'validation_archive'"
	echo ""
	exit 1
}

# verify the files we need exist
[ -f "ws1_lab_validator" ] || {
	echo ""
	echo "ERROR: file not found 'ws1_lab_validator'"
	echo ""
	exit 1
}

[ -f "validation_signature.bin" ] || {
	echo ""
	echo "ERROR: file not found 'validation_signature.bin'"
	echo ""
	exit 1
}

# create binary sha256sum hash of ws1_lab_validator
sha256sum ws1_lab_validator | dd bs=64 count=1 2> /dev/null | perl -pe 'chomp;$_=pack("H*",$_)'> hash_salt.bin || {
	echo ""
	echo "ERROR: while hashing 'ws1_lab_validator'"
	echo ""
	exit 1
}

# split validation_signature.bin into salt and sign
dd if=validation_signature.bin of=hash_salt.bin bs=32 count=1 seek=1 conv=notrunc 2> /dev/null || {
	echo ""
	echo "ERROR: while extracting salt from 'validation_signature.bin'"
	echo ""
	exit 1
}

dd if=validation_signature.bin of=sign.bin bs=32 count=3 skip=1 2> /dev/null || {
	echo ""
	echo "ERROR: while extracting signature from 'validation_signature.bin'"
	echo ""
	exit 1
}

# authenticate hash salt
echo ""
echo "--------------------------------------------------------------------------------"
echo "  Authenticate validation signature"
echo "--------------------------------------------------------------------------------"
/examples/validator/auth_file.sh \
	/examples/validator/validator_auth.sh \
	hash_salt.bin \
	sign.bin || {
		echo ""
		echo "ERROR: 'hash_salt.bin' authentication failed"
		echo ""
		exit 1
	}

# dump strings for and apps
echo ""
echo "--------------------------------------------------------------------------------"
echo "  fpga_fft strings"
echo "--------------------------------------------------------------------------------"
[ -f "fpga_fft" ] || {
	echo ""
	echo "ERROR: file not found 'fpga_fft'"
	echo ""
	exit 1
}
[ "ELF" == "$(dd if=fpga_fft bs=1 skip=1 count=3 2> /dev/null)" ] || {
	echo ""
	echo "ERROR: 'fpga_fft' does not appear to be an ELF file"
	echo ""
	exit 1
}
strings fpga_fft | grep "FILE=" || {
	echo ""
	echo "ERROR: dumping strings from 'fpga_fft'"
	echo ""
	exit 1
}
strings fpga_fft | grep "DATE=" || {
	echo ""
	echo "ERROR: dumping strings from 'fpga_fft'"
	echo ""
	exit 1
}
strings fpga_fft | grep "TIME=" || {
	echo ""
	echo "ERROR: dumping strings from 'fpga_fft'"
	echo ""
	exit 1
}

echo ""
echo "--------------------------------------------------------------------------------"
echo "  fft.bin strings"
echo "--------------------------------------------------------------------------------"
[ -f "fft.bin" ] || {
	echo ""
	echo "ERROR: file not found 'fft.bin'"
	echo ""
	exit 1
}
strings fft.bin | grep "FILE=" || {
	echo ""
	echo "ERROR: dumping strings from 'fft.bin'"
	echo ""
	exit 1
}
strings fft.bin | grep "DATE=" || {
	echo ""
	echo "ERROR: dumping strings from 'fft.bin'"
	echo ""
	exit 1
}
strings fft.bin | grep "TIME=" || {
	echo ""
	echo "ERROR: dumping strings from 'fft.bin'"
	echo ""
	exit 1
}

echo ""
echo "--------------------------------------------------------------------------------"
echo "  ws1_lab_validator strings"
echo "--------------------------------------------------------------------------------"
[ -f "ws1_lab_validator" ] || {
	echo ""
	echo "ERROR: file not found 'ws1_lab_validator'"
	echo ""
	exit 1
}
[ "ELF" == "$(dd if=ws1_lab_validator bs=1 skip=1 count=3 2> /dev/null)" ] || {
	echo ""
	echo "ERROR: 'ws1_lab_validator' does not appear to be an ELF file"
	echo ""
	exit 1
}
strings ws1_lab_validator | grep "FILE=" || {
	echo ""
	echo "ERROR: dumping strings from 'ws1_lab_validator'"
	echo ""
	exit 1
}
strings ws1_lab_validator | grep "DATE=" || {
	echo ""
	echo "ERROR: dumping strings from 'ws1_lab_validator'"
	echo ""
	exit 1
}
strings ws1_lab_validator | grep "TIME=" || {
	echo ""
	echo "ERROR: dumping strings from 'ws1_lab_validator'"
	echo ""
	exit 1
}

# exit successfully
echo ""
echo "--------------------------------------------------------------------------------"
echo "  Validation successful.  Verify strings."
echo "--------------------------------------------------------------------------------"
echo ""

exit 0

