#!/bin/sh


ELF_INPUT_FILE="./application/firmware.elf"
FLASH_OUTPUT_FILE="./temp/firmware.flash"
BINARY_FLASH_FILE="./temp/firmware.bin"
HEADER_FILE="./temp/header_file.txt"
HEADER_FILE_BIN="./temp/header_file.bin"
PACKAGED_APP_BIN="./temp/packaged_app.bin"
PACKAGED_APP_HEX="./temp/packaged_app.hex"
BOOT_LOADER_SREC="./boot_loader_cfi.srec"

elf2flash --base=0x0000 --end=0x1000000 --input=${ELF_INPUT_FILE} --output=${FLASH_OUTPUT_FILE} --boot=${BOOT_LOADER_SREC}

nios2-elf-objcopy -I srec -O binary ${FLASH_OUTPUT_FILE} ${BINARY_FLASH_FILE}

./calc_crc32.sh ${BINARY_FLASH_FILE}

echo -n "AA55A55A" > ${HEADER_FILE}
printf "%08X" $( stat --format=%s ${BINARY_FLASH_FILE} ) >> ${HEADER_FILE}
cat < ${BINARY_FLASH_FILE}.crc32 | xargs -I{} printf "%08X" 0x{} >> ${HEADER_FILE}

cat ${HEADER_FILE} | perl -e 'print pack("H*", <>)' > ${HEADER_FILE_BIN}

./calc_crc32.sh ${HEADER_FILE_BIN}
cat < ${HEADER_FILE_BIN}.crc32 | xargs -I{} printf "%08X" 0x{} >> ${HEADER_FILE}

cat ${HEADER_FILE} | perl -e 'print pack("H*", <>)' > ${HEADER_FILE_BIN}

cat ${HEADER_FILE_BIN} ${BINARY_FLASH_FILE} > ${PACKAGED_APP_BIN}

nios2-elf-objcopy -I binary -O ihex ${PACKAGED_APP_BIN} ${PACKAGED_APP_HEX}
