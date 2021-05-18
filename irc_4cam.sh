#!/bin/bash
EXECDIR=/home/kmlee/irc_example
export LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/

${EXECDIR}/irc_module 0013001c-5113-3437-3335-373400000000 1 &
${EXECDIR}/irc_module 0015002c-5119-3038-3732-333700000000 2 &
${EXECDIR}/irc_module 8010800b-5113-3437-3335-373400000000 3 &
${EXECDIR}/irc_module 00070029-5102-3038-3835-393400000000 4 &

