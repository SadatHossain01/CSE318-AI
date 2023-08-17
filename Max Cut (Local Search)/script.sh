#!/bin/bash

rm -rf out.txt
touch out.txt

g++ -std=c++14 -O3 solve.cpp -o solve

grand_command=""

for i in {1..54}
do
    run_command="./solve input/g${i}.rud >> out.txt"
    echo $run_command
    # concatenate run_command to grand_command
    if [ -z "$grand_command" ]
    then
        grand_command="$run_command"
    else
        grand_command="$grand_command & $run_command"
    fi
done

echo $grand_command
eval $grand_command