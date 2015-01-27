#!/bin/sh

perl -MMIME::Base64 -0777 -pe '$_=encode_base64($_)'

