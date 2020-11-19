set term png
set output "total.png"
set title "Total time"

set xlabel "Processes"
set ylabel "Time"

plot 'total.csv' with linespoints