#!/bin/bash

if [ "$#" -lt 1 ];then
	echo "Usage: ./plot filename"
	exit
fi

ptype=""
if [[ "$1" == *_g* ]];then
	ptype=Gain
	ylabel="Volts Mag (dB)"
elif [[ "$1" == *_p* ]];then
	ptype=Phase
	ylabel="Volts Mag (dB)"
else
	ptype=Unknown
fi

gnuplot << EOF
reset

set logscale x
set ylabel "$ylabel"
set xlabel "$xlabel"
set title "Output node of ${1%%_*} netlist"
set grid 
set style data lines
set format x "%3.E"

plot "$1" title "$ptype"
EOF
