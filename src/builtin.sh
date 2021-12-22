#!/bin/bash

rm -rf payload/

mkdir payload/

cd ../dumpfiles

xxd -i pongoOS > ../src/payload/pongoOS.h

# T7000
xxd -i t7000_overwrite2 >> ../src/payload/t7000.h
xxd -i t7000_stage2 >> ../src/payload/t7000.h

# S8000
xxd -i s8000_overwrite2 >> ../src/payload/s8000.h
xxd -i s8000_stage2 >> ../src/payload/s8000.h

# T8010
xxd -i t8010_overwrite1 >> ../src/payload/t8010.h
xxd -i t8010_overwrite2 >> ../src/payload/t8010.h
xxd -i t8010_stage2 >> ../src/payload/t8010.h

# T8015
xxd -i t8015_overwrite1 >> ../src/payload/t8015.h
xxd -i t8015_overwrite2 >> ../src/payload/t8015.h
xxd -i t8015_stage2 >> ../src/payload/t8015.h
