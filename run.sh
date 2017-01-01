#!/bin/bash
FILE=main.c
OUTPUT=main

#Valores de ejecucion
ITER=1000
VALUES=( 1 1024 1048576 10485760 104857600 524288000 1073741824 )
TEXT=( "1B" "1KB" "1MB" "10MB" "100MB" "500MB" "1GB" )
PROCESS=2
RAFAGA=100
HOSTFILE="myhostfile"

#Directorios de resultados
RES="./res/report"
BIN="./res/bin"
COMP="./res/comp"
PLOT="./res/gnuplot"
GRAPHIC="./res/graphic"

#Preparaicon de directorios
mkdir -p $RES
mkdir -p $BIN
mkdir -p $COMP
mkdir -p $PLOT
mkdir -p $GRAPHIC

#Compilacion de ficheros
echo -n "Compilando.."
echo "OK!"
mpicc $FILE -O3 -o $BIN/$OUTPUT >> $COMP/comp.txt 2>&1
EXEC="mpirun -n $PROCESS --hostfile $HOSTFILE $BIN/$OUTPUT $RAFAGA"

#Preparacion de resultados
echo "" > $RES/res.txt
echo "#Size  Time" > $PLOT/data.dat

# Ejecucion
echo "Ejecutando..."
for ((i=0;i<${#VALUES[@]};i++))
do
    echo -n "Array de "${TEXT[i]} - ""
    echo "======================" >> $RES/res.txt
    echo "Array of: "${TEXT[i]}"" >> $RES/res.txt
    res=$($EXEC "${VALUES[i]}")
    echo $res >> $RES/res.txt
    dummy=$(echo $res | awk '/Time for Packet/{print $NF}')
    echo -n "$dummy " >> $PLOT/data.dat
    echo "${TEXT[i]} " >> $PLOT/data.dat
    echo "OK!"
done

#Preparacion de fichero para la grafica
#PNG
echo "set terminal pngcairo enhanced font 'Verdana,10'" > $PLOT/gnu_plot.png.txt
echo "set output '$GRAPHIC/res.png'" >> $PLOT/gnu_plot.png.txt
echo "set title 'Ejecución de $RAFAGA ráfagas'" >> $PLOT/gnu_plot.png.txt
echo "set ylabel 'Tiempo (s)'" >> $PLOT/gnu_plot.png.txt
echo "set xlabel 'Tamaño del paquete'" >> $PLOT/gnu_plot.png.txt
echo "plot '$PLOT/data.dat' using 1:xtic(2) with linespoints" >> $PLOT/gnu_plot.png.txt
#TEX
echo "set terminal tex" > $PLOT/gnu_plot.tex.txt
echo "set output '$GRAPHIC/res.tex'" >> $PLOT/gnu_plot.tex.txt
echo "set title 'Ejecución de $RAFAGA ráfagas'" >> $PLOT/gnu_plot.tex.txt
echo "set ylabel 'Tiempo (s)'" >> $PLOT/gnu_plot.tex.txt
echo "set xlabel 'Tamaño del paquete'" >> $PLOT/gnu_plot.tex.txt
echo "plot '$PLOT/data.dat' using 1:xtic(2) with linespoints" >> $PLOT/gnu_plot.tex.txt


echo -n "Realizando graficas..."
gnuplot $PLOT/gnu_plot.png.txt 2> /dev/null
gnuplot $PLOT/gnu_plot.tex.txt 2> /dev/null
echo "OK!"
