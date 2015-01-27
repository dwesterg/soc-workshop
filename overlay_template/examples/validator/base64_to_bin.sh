#!/bin/sh

perl -MMIME::Base64 -pe '$_=decode_base64($_)'

