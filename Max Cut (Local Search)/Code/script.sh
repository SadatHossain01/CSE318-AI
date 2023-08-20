#!/bin/bash

rm -rf ../Results/out.txt
touch ../Results/out.txt

g++ -std=c++14 -O3 solve.cpp -o solve

grand_command=""
type=("greedy-1" "greedy-2" "semi-greedy-1" "semi-greedy-2" "randomized")

for i in {1..54}
do
    for j in {0..4}
    do
        command="./solve ../input/g${i}.rud ${type[$j]} >> ../Results/out.txt"
        echo $command

        if [ -z "$grand_command" ]
        then
            grand_command="$command"
        else
            grand_command="$grand_command & $command"
        fi
    done
done

echo $grand_command
eval $grand_command
rm solve