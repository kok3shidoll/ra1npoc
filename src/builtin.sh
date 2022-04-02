#!/bin/bash

rm -rf payload/

mkdir payload/

cd ../dumpfiles

xxd -i pongoOS > ../src/payload/pongoOS.h

# S5L8960
xxd -i s5l8960_overwrite1 >> ../src/payload/s5l8960.h
xxd -i s5l8960_overwrite2 >> ../src/payload/s5l8960.h
xxd -i s5l8960_stage2 >> ../src/payload/s5l8960.h

# T7000
xxd -i t7000_overwrite2 >> ../src/payload/t7000.h
xxd -i t7000_stage2 >> ../src/payload/t7000.h

# S8000/S8003
xxd -i s8000_overwrite2 >> ../src/payload/s8000.h
xxd -i s8000_stage2 >> ../src/payload/s8000.h

# S8001
xxd -i s8001_overwrite1 >> ../src/payload/s8001.h
xxd -i s8001_overwrite2 >> ../src/payload/s8001.h
xxd -i s8001_stage2 >> ../src/payload/s8001.h

# T8010
xxd -i t8010_overwrite1 >> ../src/payload/t8010.h
xxd -i t8010_overwrite2 >> ../src/payload/t8010.h
xxd -i t8010_stage2 >> ../src/payload/t8010.h

# T8011
xxd -i t8011_overwrite1 >> ../src/payload/t8011.h
xxd -i t8011_overwrite2 >> ../src/payload/t8011.h
xxd -i t8011_stage2 >> ../src/payload/t8011.h

# T8012
xxd -i t8012_overwrite1 >> ../src/payload/t8012.h
xxd -i t8012_overwrite2 >> ../src/payload/t8012.h
xxd -i t8012_stage2 >> ../src/payload/t8012.h

# T8015
xxd -i t8015_overwrite1 >> ../src/payload/t8015.h
xxd -i t8015_overwrite2 >> ../src/payload/t8015.h
xxd -i t8015_stage2 >> ../src/payload/t8015.h
