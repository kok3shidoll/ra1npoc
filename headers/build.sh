#!/bin/bash

rm -rf legacy_kpf.h
rm -rf legacy_ramdisk.h
rm -rf Pongo_bin.h

xxd -i legacy_kpf > legacy_kpf.h
xxd -i legacy_ramdisk > legacy_ramdisk.h
xxd -i Pongo.bin > Pongo_bin.h
