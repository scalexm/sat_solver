#! /bin/bash




echo "solve K-sat for which value of K ?"
read K
echo "how many variables ?"
read V
param=( -moms -dlis -rand)

allparam=( 'no flags' -wl -cl '-wl -vsids' '-cl -wl -vsids')
for x in ${param[@]}; do
    allparam+=("-wl $x")
    allparam+=("-cl $x")
    allparam+=("-wl -cl $x")
    allparam+=("-cl $x -forget")
    allparam+=("-wl -cl $x -forget")
done;
printf "$K\n$V\nno flag" | ./give_data.sh |python plot.py

for flag in "${allparam[@]}"; do
    printf "$K\n$V\n$flag" | ./give_data.sh |python plot.py
done
rm -f .result
rm -f .tmp.cnf
