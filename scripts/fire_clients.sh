#!/usr/bin/bash

script_dir=$(dirname "$0")

echo -e "\nCompiling..."
gcc $script_dir/../src/client.c -w -o $script_dir/../bin/client

echo -e "Firing client requests\n"

for i in {1..50}
do
    $script_dir/../bin/client 9999 client_$i < $script_dir/../test_data/client_input &
done

wait
