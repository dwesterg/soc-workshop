#!/bin/sh

hexdump -v -e '1/1 "0x%02X\n"' post_data_0.bin > post_data_0.hex
hexdump -v -e '1/1 "0x%02X\n"' post_data_1.bin > post_data_1.hex
hexdump -v -e '1/1 "0x%02X\n"' post_data_2.bin > post_data_2.hex
hexdump -v -e '1/1 "0x%02X\n"' post_result_0.bin > post_result_0.hex
hexdump -v -e '1/1 "0x%02X\n"' post_result_1.bin > post_result_1.hex
hexdump -v -e '1/1 "0x%02X\n"' post_result_2.bin > post_result_2.hex
hexdump -v -e '1/1 "0x%02X\n"' pre_data_0.bin > pre_data_0.hex
hexdump -v -e '1/1 "0x%02X\n"' pre_data_1.bin > pre_data_1.hex
hexdump -v -e '1/1 "0x%02X\n"' pre_data_2.bin > pre_data_2.hex
hexdump -v -e '1/1 "0x%02X\n"' pre_result_0.bin > pre_result_0.hex
hexdump -v -e '1/1 "0x%02X\n"' pre_result_1.bin > pre_result_1.hex
hexdump -v -e '1/1 "0x%02X\n"' pre_result_2.bin > pre_result_2.hex

md5sum *.bin
md5sum *.hex

SGDMA_TO_FFT_CSR_BASE=0x80000
SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE=0x90000
SGDMA_FROM_FFT_CSR_BASE=0xA0000
SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE=0xB0000
DATA_BASE=0xC0000
RESULT_BASE=0xC8000
FFT_CSR=0xD0000
DMA_DATA_BASE=0x40000
DMA_RESULT_BASE=0x48000
SAMPLE_SIZE=128

# set the FFT sample size
NEXT_WORD=$((FFT_CSR))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 ${SAMPLE_SIZE}

#-------------------------------------------------------------------------------
# fill the FFT input data buffer
echo "fill FFT input buffer"
NEXT_BYTE=$((DATA_BASE))
cat pre_data_0.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 ${NEXT}
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

# clear the FFT result buffer
echo "clear FFT result buffer"
NEXT_BYTE=$((RESULT_BASE))
cat pre_result_0.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 ${NEXT}
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

# start the DMA to FFT
NEXT_WORD=$((SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 ${DMA_DATA_BASE}
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 $(expr ${SAMPLE_SIZE} \* 4)
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0x80000300

# start the DMA from FFT
NEXT_WORD=$((SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 ${DMA_RESULT_BASE}
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 $(expr ${SAMPLE_SIZE} \* 8)
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0x80001000

# check for DMA complete
NEXT_WORD=$((SGDMA_TO_FFT_CSR_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
STATUS_TO=$(devmem ${ADDR} 32)
NEXT_WORD=$((SGDMA_FROM_FFT_CSR_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
STATUS_FROM=$(devmem ${ADDR} 32)

STATUS_TO=$((STATUS_TO & 1))
STATUS_FROM=$((STATUS_FROM & 1))

[ ${STATUS_TO} -eq 1 ] && {
	echo "DMA to FFT is still running"
	exit 1
}

[ ${STATUS_FROM} -eq 1 ] && {
	echo "DMA from FFT is still running"
	exit 1
}

# read the DMA result buffer
echo "read FFT result buffer"
NEXT_BYTE=$((RESULT_BASE))
cat post_result_0.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 >> post_result_0.hex.out
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

#-------------------------------------------------------------------------------
# fill the FFT input data buffer
echo "fill FFT input buffer"
NEXT_BYTE=$((DATA_BASE))
cat pre_data_1.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 ${NEXT}
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

# clear the FFT result buffer
echo "clear FFT result buffer"
NEXT_BYTE=$((RESULT_BASE))
cat pre_result_1.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 ${NEXT}
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

# start the DMA to FFT
NEXT_WORD=$((SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 ${DMA_DATA_BASE}
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 $(expr ${SAMPLE_SIZE} \* 4)
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0x80000300

# start the DMA from FFT
NEXT_WORD=$((SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 ${DMA_RESULT_BASE}
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 $(expr ${SAMPLE_SIZE} \* 8)
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0x80001000

# check for DMA complete
NEXT_WORD=$((SGDMA_TO_FFT_CSR_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
STATUS_TO=$(devmem ${ADDR} 32)
NEXT_WORD=$((SGDMA_FROM_FFT_CSR_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
STATUS_FROM=$(devmem ${ADDR} 32)

STATUS_TO=$((STATUS_TO & 1))
STATUS_FROM=$((STATUS_FROM & 1))

[ ${STATUS_TO} -eq 1 ] && {
	echo "DMA to FFT is still running"
	exit 1
}

[ ${STATUS_FROM} -eq 1 ] && {
	echo "DMA from FFT is still running"
	exit 1
}

# read the DMA result buffer
echo "read FFT result buffer"
NEXT_BYTE=$((RESULT_BASE))
cat post_result_1.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 >> post_result_1.hex.out
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

#-------------------------------------------------------------------------------
# fill the FFT input data buffer
echo "fill FFT input buffer"
NEXT_BYTE=$((DATA_BASE))
cat pre_data_2.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 ${NEXT}
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

# clear the FFT result buffer
echo "clear FFT result buffer"
NEXT_BYTE=$((RESULT_BASE))
cat pre_result_2.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 ${NEXT}
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

# start the DMA to FFT
NEXT_WORD=$((SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 ${DMA_DATA_BASE}
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 $(expr ${SAMPLE_SIZE} \* 4)
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0x80000300

# start the DMA from FFT
NEXT_WORD=$((SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 ${DMA_RESULT_BASE}
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 $(expr ${SAMPLE_SIZE} \* 8)
NEXT_WORD=$((NEXT_WORD + 4))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
devmem ${ADDR} 32 0x80001000

# check for DMA complete
NEXT_WORD=$((SGDMA_TO_FFT_CSR_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
STATUS_TO=$(devmem ${ADDR} 32)
NEXT_WORD=$((SGDMA_FROM_FFT_CSR_BASE))
ADDR=$(printf "0xFF2%x" ${NEXT_WORD})
STATUS_FROM=$(devmem ${ADDR} 32)

STATUS_TO=$((STATUS_TO & 1))
STATUS_FROM=$((STATUS_FROM & 1))

[ ${STATUS_TO} -eq 1 ] && {
	echo "DMA to FFT is still running"
	exit 1
}

[ ${STATUS_FROM} -eq 1 ] && {
	echo "DMA from FFT is still running"
	exit 1
}

# read the DMA result buffer
echo "read FFT result buffer"
NEXT_BYTE=$((RESULT_BASE))
cat post_result_2.hex | while read NEXT
do
	ADDR=$(printf "0xFF2%x" ${NEXT_BYTE})
	devmem ${ADDR} 8 >> post_result_2.hex.out
	NEXT_BYTE=$(($NEXT_BYTE + 1))
done

md5sum *.hex.out

rm *.hex *.hex.out

echo "FFT target test complete..."

