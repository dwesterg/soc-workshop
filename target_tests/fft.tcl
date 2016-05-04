package provide adc_toolkit 0.1
package require Tcl 8.5

set m [lindex [ get_service_paths master ] 0]
puts "'$m'"
open_service master $m


#set a bunch of defines so we can easily change them.
set SGDMA_TO_FFT_CSR_BASE			0x80000
set SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE		0x90000
set SGDMA_FROM_FFT_CSR_BASE			0xA0000
set SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE	0xB0000
set DATA_BASE					0xC0000
set RESULT_BASE					0xC8000
set FFT_CSR					0xD0000
set DMA_DATA_BASE				0x40000
set DMA_RESULT_BASE				0x48000
set sample_size		128
set triangle_list	{ 0 256 512 768 1024 1280 1536 1792 2048 1792 1536 1280 1024 768 512 256 0 -256 -512 -768 -1024 -1280 -1536 -1792 -2048 -1792 -1536 -1280 -1024 -768 -512 -256 }

proc fft_wave {  waveform } {
	global SGDMA_TO_FFT_CSR_BASE  
	global SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE  
	global SGDMA_FROM_FFT_CSR_BASE 
	global SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE 
	global DATA_BASE 
	global RESULT_BASE 
	global sample_size 
	global DMA_DATA_BASE
	global DMA_RESULT_BASE 
	global FFT_CSR
	global m
	global triangle_list

	master_write_32 $m $FFT_CSR $sample_size

	# we will be only loading the real portion of the waveform which is the upper 16 bits of the 32 bits. 	
	set temp 0
	for {set i 0} {$i < $sample_size} {incr i} {
		if {$waveform == 0} {
			# make small spikes  should give a sinx/x waveform. if I remember my DSP well
			set temp [expr (((($i % 32) < 16) * 0xffff) - 0x8000) & 0xffff] 
		}
		if {$waveform == 1} {
			set temp [ expr  int(((sin(2.0 * 3.1415 * $i / 16.0)*0x7fff))) & 0xffff ]
		}

		if {$waveform == 2} {
			set value [lindex $triangle_list [expr $i % 32]]
			set temp [expr  ($value * 15)& 0xffff]
		}
		master_write_32 $m [expr $DATA_BASE + ($i * 4)] $temp
	}

	# zero out the second half of the on chip memory	         
	for {set i 0} {$i < [expr $sample_size*2]} {incr i} {
		master_write_32 $m [expr $RESULT_BASE + ($i * 4)] 0
	}

	# dump the pre-calculation data and results
	master_read_to_file $m "pre_data_$waveform.bin" $DATA_BASE [expr $sample_size * 4]
	master_read_to_file $m "pre_result_$waveform.bin" $RESULT_BASE [expr ($sample_size * 4) * 2]

	#load the sgdmas up
	master_write_32 $m [expr $SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE + 0] $DMA_DATA_BASE
	master_write_32 $m [expr $SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE + 4] 0
	master_write_32 $m [expr $SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE + 8] [expr $sample_size *4]
	master_write_32 $m [expr $SGDMA_TO_FFT_DESCRIPTOR_SLAVE_BASE + 0xc] 0x80000300


	master_write_32 $m [expr $SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE + 0] 0
	master_write_32 $m [expr $SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE + 4] $DMA_RESULT_BASE
	master_write_32 $m [expr $SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE + 8] [expr $sample_size *8]
	master_write_32 $m [expr $SGDMA_FROM_FFT_DESCRIPTOR_SLAVE_BASE + 0xc] 0x80001000

	#Check to make sure the fft is done
	set status_to [master_read_32 $m $SGDMA_TO_FFT_CSR_BASE 1]
	set status_from [master_read_32 $m $SGDMA_FROM_FFT_CSR_BASE 1]
	if {[expr $status_to & 0x1]} {
		puts "SGDMA to FFT is still busy - $status_to"
	}
	if {[expr $status_from & 0x1]} {
		puts "SGDMA from FFT is still busy - $status_from"
	}
	if {[expr $status_to & $status_from & 0x1]} {
		puts "An SGDMA is unexpectedly still busy..."
	} else {
		puts "No busy SGDMAs as expected..."
	}

	# dump the pre-calculation data and results
	master_read_to_file $m "post_data_$waveform.bin" $DATA_BASE [expr $sample_size * 4]
	master_read_to_file $m "post_result_$waveform.bin" $RESULT_BASE [expr ($sample_size * 4) * 2]
}

