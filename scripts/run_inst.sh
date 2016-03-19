#! /bin/bash

solve=../bin/resol
echo "solve K-sat for which value of K ?"
read K
echo "how many variables ?"
read V
echo "$K" > .result
echo "$V" >> .result
echo "no flag" >> .result
for i in {20..60}; do
    for j in {1..100}; do
        (printf "$K\n$V\n$i\n" | ./rand_inst.py) > .tmp.cnf
        TIMEFORMAT="%3R";ligne=$((time (timeout 10s $solve .tmp.cnf >/dev/null) )2>&1)
        echo "$ligne">> .result
        echo "$ligne"
    done
done
./plot.py < .result

for flag in -moms -dlis -rand -wl do
    echo "$K" > .result
    echo "$V" >> .result
    echo "$flag" >> .result
    for i in {20..60}; do
        for j in {1..100}; do
            (printf "$K\n$V\n$i\n" | ./rand_inst.py) > .tmp.cnf
            TIMEFORMAT="%3R";ligne=$((time (timeout 10s $solve $flag .tmp.cnf >/dev/null) )2>&1)
            echo "$ligne">> .result
            echo "$ligne"
        done
    done
    ./plot.py < .result
done
rm .result
rm .tmp.cnf
