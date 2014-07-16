#!/bin/sh


cd arch_binary_outputs
outputs=`ls *_*`
cd ..
for outputFile in $outputs
do
  if cmp -s $1 arch_binary_outputs/$outputFile
  then
    echo "#define INTENSE_IO_COMPATIBLE_ARCHITECTURE BinaryBaseSerialiser::ARCH_$outputFile"
    exit
  fi
done
echo "#define INTENSE_IO_COMPATIBLE_ARCHITECTURE BinaryBaseSerialiser::ARCH_unknown"
