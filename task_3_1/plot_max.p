set term png
set output "max.png"
set title "Max time among processes"

set xlabel "Processes"
set ylabel "Time"

plot 'max.csv' with linespoints