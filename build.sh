#!/bin/bash
mkdir -p stamp
#dont add memtest to novtech board, FPGA too small
#touch stamp/NOVTECH_NOVSOMCV_LITE.qsys_add_memtest_components.tcl.stamp

make -j8 all
