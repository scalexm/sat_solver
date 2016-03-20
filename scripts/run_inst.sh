#! /bin/bash




echo "solve K-sat for which value of K ?"
read K
echo "how many variables ?"
read V

printf "$K\n$V\nno flag" | ./give_data.sh |python plot.py

for flag in -moms -dlis -rand -wl; do
    printf "$K\n$V\n$flag" | ./give_data.sh |python plot.py
done
rm .result
rm .tmp.cnf
