set terminal png size 2000,1000 enhanced font 'Verdana,12' linewidth 4
set output 'plot.png'
set title "Single axis (x) position approximation with rough 10ms sampling, integration using rectangular method, and no sensor movement."
set xlabel "Time (s)"
set ylabel "Acceleration, Velocity, Position"
set xtic auto
set ytic auto
set autoscale
set key at 10,-2.5
plot "output.dat" using ($1/100):2 title "Linear Acceleration (m/s/s)" with lines, \
 "output.dat" using ($1/100):3 title "Velocity (m/s)" with lines, \
 "output.dat" using ($1/100):4 title "Position (m)" with lines

