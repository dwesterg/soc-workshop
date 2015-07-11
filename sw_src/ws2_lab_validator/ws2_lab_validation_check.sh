#!/bin/sh

# verify the files we need exist
[ -f "ws2_validation_archive.tar.gz" ] || {
	echo ""
	echo "ERROR: file not found 'ws2_validation_archive.tar.gz'"
	echo ""
	echo 1
}

[ -f "ws2_validation_archive.tar.gz.sign" ] || {
	echo ""
	echo "ERROR: file not found 'ws2_validation_archive.tar.gz.sign'"
	echo ""
	echo 1
}

# authenticate the validation archive
echo ""
echo "--------------------------------------------------------------------------------"
echo "  Authenticate validation archive"
echo "--------------------------------------------------------------------------------"
/examples/validator/auth_file.sh \
	/examples/validator/validator_auth.sh \
	ws2_validation_archive.tar.gz \
	ws2_validation_archive.tar.gz.sign || {
		echo ""
		echo "ERROR: 'ws2_validation_archive.tar.gz' authentication failed"
		echo ""
		echo 1
	}

# decompress the validation archive
gunzip ws2_validation_archive.tar.gz || {
	echo ""
	echo "ERROR: while decompressing 'ws2_validation_archive.tar.gz'"
	echo ""
	echo 1
}

# extract the validation archive
tar xf ws2_validation_archive.tar || {
	echo ""
	echo "ERROR: while extracting 'ws2_validation_archive.tar'"
	echo ""
	echo 1
}

# change directory into the validation archive
cd ws2_validation_archive || {
	echo ""
	echo "ERROR: while changing directory into 'ws2_validation_archive'"
	echo ""
	echo 1
}

# verify the files we need exist
[ -f "ws2_lab_validator" ] || {
	echo ""
	echo "ERROR: file not found 'ws2_lab_validator'"
	echo ""
	echo 1
}

[ -f "validation_signature.bin" ] || {
	echo ""
	echo "ERROR: file not found 'validation_signature.bin'"
	echo ""
	echo 1
}

# create binary sha256sum hash of ws2_lab_validator
sha256sum ws2_lab_validator | dd bs=64 count=1 2> /dev/null | perl -pe 'chomp;$_=pack("H*",$_)'> hash_salt.bin || {
	echo ""
	echo "ERROR: while hashing 'ws2_lab_validator'"
	echo ""
	echo 1
}

# split validation_signature.bin into salt and sign
dd if=validation_signature.bin of=hash_salt.bin bs=32 count=1 seek=1 conv=notrunc 2> /dev/null || {
	echo ""
	echo "ERROR: while extracting salt from 'validation_signature.bin'"
	echo ""
	echo 1
}

dd if=validation_signature.bin of=sign.bin bs=32 count=3 skip=1 2> /dev/null || {
	echo ""
	echo "ERROR: while extracting signature from 'validation_signature.bin'"
	echo ""
	echo 1
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
		echo 1
	}

# dump strings for and apps
echo ""
echo "--------------------------------------------------------------------------------"
echo "  ws2_validation_info.txt"
echo "--------------------------------------------------------------------------------"
[ -f "ws2_validation_info.txt" ] || {
	echo ""
	echo "ERROR: file not found 'ws2_validation_info.txt'"
	echo ""
	echo 1
}

echo "Kernel Build Information."
echo "Verify correct username and appropriate timestamp."

grep "Built by:" ws2_validation_info.txt || {
	echo ""
	echo "ERROR: dumping username from ws2_validation_info.txt"
	echo ""
	echo 1
}
grep "Built on host:" ws2_validation_info.txt || {
	echo ""
	echo "ERROR: dumping hostname from ws2_validation_info.txt"
	echo ""
	echo 1
}
grep "Built on date:" ws2_validation_info.txt || {
	echo ""
	echo "ERROR: dumping date from ws2_validation_info.txt"
	echo ""
	echo 1
}


echo ""
echo "--------------------------------------------------------------------------------"
echo "  ws2_lab_validator strings"
echo "--------------------------------------------------------------------------------"
[ -f "ws2_lab_validator" ] || {
	echo ""
	echo "ERROR: file not found 'ws2_lab_validator'"
	echo ""
	echo 1
}
[ "ELF" == "$(dd if=ws2_lab_validator bs=1 skip=1 count=3 2> /dev/null)" ] || {
	echo ""
	echo "ERROR: 'ws2_lab_validator' does not appear to be an ELF file"
	echo ""
	echo 1
}
strings ws2_lab_validator | grep "FILE=" || {
	echo ""
	echo "ERROR: dumping strings from 'ws2_lab_validator'"
	echo ""
	echo 1
}
strings ws2_lab_validator | grep "DATE=" || {
	echo ""
	echo "ERROR: dumping strings from 'ws2_lab_validator'"
	echo ""
	echo 1
}
strings ws2_lab_validator | grep "TIME=" || {
	echo ""
	echo "ERROR: dumping strings from 'ws2_lab_validator'"
	echo ""
	echo 1
}

# echo successfully
echo ""
echo "--------------------------------------------------------------------------------"
echo "  Validation successful.  Verify strings."
echo "--------------------------------------------------------------------------------"
echo ""

echo 0

