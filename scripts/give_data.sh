#! /bin/bash

if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    timer=timeout
else #Assuming mac OS
    timer=gtimeout
fi

solve=../bin/resol

read K
read V
read flag

echo "$K"
echo "$V"
echo "$flag"

if [ "$flag" == "no flags" ]; then
    flag=""
fi

for i in {20..60}; do
    for j in {1..100}; do
        (printf "$K\n$V\n$i\n" | python3 rand_inst.py) > .tmp.cnf
        TIMEFORMAT="%3R";ligne=$((time  ($timer 10s $solve $flag .tmp.cnf >/dev/null) )2>&1)
        echo "$ligne"
    done
done
