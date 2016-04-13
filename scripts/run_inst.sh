#! /bin/bash




echo "solve K-sat for which value of K ?"
read K
echo "how many variables ?"
read V
paramcl=( -moms -dlis -rand -vsids "-wl -vsid")

allparam=( -wl -cl "-wl -cl" "-cl -forget" "-wl -cl -forget" )
for x in ${param[@]}; do
    allparam+=("-cl $x")
    allparam+=("-cl $x -forget")
done;
printf "$K\n$V\nno flag" | ./give_data.sh |python plot.py

for flag in "${allparam[@]}"; do
    printf "$K\n$V\n$flag" | ./give_data.sh |python plot.py
done
rm -f .result
rm -f .tmp.cnf
