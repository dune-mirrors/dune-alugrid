#set xrange [256:16384]
#set yrange [3:300]

set terminal postscript eps color "Helvetica" 22

set logscale 

set key bottom left box
set grid

set pointsize 2

set xtics ( "32" 32, "64" 64, "128" 128, "256" 256, "512" 512, "1024" 1024, "2048" 2048, "4096" 4096 )
# , "8192" 8192, "16224" 16224 )

set xlabel "#cores"
set ylabel "run time"

c = 3 
p "mb_alusfc_403/optimal.dat" using 1:u title "optimal" w l lw c, \
  "mb_bresen_403/scaling.dat" using 1:u title "bresenham" w lp lw c, \
  "mb_alusfc_403/scaling.dat" using 1:u title "alusfc" w lp lw c, \
  "mb_alusfc_403_linkage/scaling.dat" using 1:u title "alusfc_l" w lp lw c, \
  "mb_hsfc_403/scaling.dat" using 1:u title "hsfc" w lp lw c, \
  "mb_zgraph_403/scaling.dat" using 1:u title "zgraph" w lp lw c, \
  "mb_kway_403/scaling.dat" using 1:u title "kway" w lp lw c, \
  "mb_parmetis_403/scaling.dat" using 1:u title "parmetis" w lp lw c
