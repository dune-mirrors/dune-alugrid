#set xrange [256:16384]
#set yrange [3:300]

set terminal postscript eps color "Helvetica" 22

set logscale 

set key bottom left box
set grid

set pointsize 2

set xtics ( "128" 128, "256" 256, "512" 512, "1024" 1024, "2048" 2048, "4096" 4096 )
# , "8192" 8192, "16224" 16224 )

set xlabel "#cores"
set ylabel "run time"

c = 3 
p "me_alusfc_306/optimal.dat" using 1:u title "optimal" w l lw c, \
  "me_alusfc_306/scaling.dat" using 1:u title "alusfc" w lp lw c, \
  "me_hsfc_306/scaling.dat" using 1:u title "hsfc" w lp lw c
