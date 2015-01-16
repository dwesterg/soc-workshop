#!/bin/bash
export http_proxy=localhost:8080
export https_proxy=localhost:8080

ssh -M -S my_socket -fnNT -L 8080:sj-proxy.altera.com:8080 sj-interactive1

make arc_build_all

ssh -S my_socket -O exit sj-interactive1
