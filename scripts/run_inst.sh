#! /bin/bash




echo "solve K-sat for which value of K ?"
read K
echo "how many variables ?"
read V
paramcl=( -moms -dlis -rand -vsids "-wl" "-wl -vsids" "-wl -rand")

allparam=( -moms -dlis -rand -wl -cl "-wl -rand" "-cl -forget" )
for x in "${paramcl[@]}"; do
    allparam+=("-cl $x")
    allparam+=("-cl $x -forget")
done;
#printf "$K\n$V\nno flags" | ./give_data.sh |python plot.py

allparam=( "-cl -moms -forget" )

for flag in "${allparam[@]}"; do
    printf "$K\n$V\n$flag" | ./give_data.sh |python plot.py
done
rm -f .result
rm -f .tmp.cnf
