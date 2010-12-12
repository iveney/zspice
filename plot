#!/bin/bash

if [ "$#" -lt 1 ];then
	echo "Usage: ./plot filename"
	exit
fi


ptype=""
if [[ "$1" == *_g* ]];then
	ptype=Gain
	scale=logscale
	xlabel="Frequency (Hertz)"
	ylabel="Volts Mag (dB)"
elif [[ "$1" == *_p* ]];then
	ptype=Phase
	scale=logscale
	xlabel="Frequency (Hertz)"
	ylabel="Volts Phase (degree)"
elif [[ "$1" == *_t* ]];then
	ptype=Transient
	scale=nolog
	xlabel="Time (s)"
	ylabel="Voltage (V)"
else
	scale=nolog
	ptype=Unknown
fi

gnuplot << EOF
reset

set terminal postscript enhanced color
set output '| ps2pdf - ${1%%.*}.pdf'
set $scale x
set ylabel "$ylabel"
set xlabel "$xlabel"
set title "Output node of ${1%%_*} netlist"
set grid 
set style data lines
set format x "%3.E"

plot "$1" title "$ptype"
EOF
