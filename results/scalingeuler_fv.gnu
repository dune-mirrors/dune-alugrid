#set xrange [256:16384]
#set yrange [3:300]

set output "scalingeuler_fv.eps"
set title "ALUGrid main_euler (FV)"

u = 5
load "baseeuler.gnu"
