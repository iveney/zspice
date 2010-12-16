#!/bin/bash
# utility script to draw the output by zspice via gnuplot
# if `pdf' is used in the second option, the output will be redirected to 
# a pdf file.

if [ "$#" -lt 1 ];then
	echo "Usage: ./plot filename [output type]"
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

TERM=aqua
OUTPUT=
if [[ "$2" == "pdf" ]];then
	TERM="postscript enhanced color"
	OUTPUT="'| ps2pdf - ${1%%.*}.pdf'"
fi

gnuplot << EOF
reset

set terminal $TERM
set output $OUTPUT
set $scale x
set ylabel "$ylabel"
set xlabel "$xlabel"
set title "Output node of ${1%%_*} netlist"
set grid 
set style data lines
set format x "%3.E"

plot "$1" title "$ptype"
EOF
