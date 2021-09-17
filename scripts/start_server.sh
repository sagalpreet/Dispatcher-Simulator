#!/usr/bin/bash

script_dir=$(dirname "$0")

echo -e "\nClearing results from previous execution ...\n"
rm -f $script_dir/../execution_results/*

echo -e "\nCompiling source code ..."

gcc -g -c $script_dir/../src/server.c -o $script_dir/../obj/server.o -pthread -ldl
gcc -g -c $script_dir/../src/dispatcher.c -o $script_dir/../obj/dispatcher.o -pthread -ldl
gcc -g -c $script_dir/../src/memory.c -o $script_dir/../obj/memory.o -pthread -ldl
gcc -g -c $script_dir/../src/request_listener.c -o $script_dir/../obj/request_listener.o -pthread -ldl
gcc -g -c $script_dir/../src/request_queue.c -o $script_dir/../obj/request_queue.o -pthread -ldl

# gcc -Wall -g -c ../src/server.c -o ../obj/server.o -pthread -ldl
# gcc -Wall -g -c ../src/dispatcher.c -o ../obj/dispatcher.o -pthread -ldl
# gcc -Wall -g -c ../src/memory.c -o ../obj/memory.o -pthread -ldl
# gcc -Wall -g -c ../src/request_listener.c -o ../obj/request_listener.o -pthread -ldl
# gcc -Wall -g -c ../src/request_queue.c -o ../obj/request_queue.o -pthread -ldl

echo "Linking object files ..."

gcc -o $script_dir/../bin/server $script_dir/../obj/server.o $script_dir/../obj/dispatcher.o $script_dir/../obj/memory.o $script_dir/../obj/request_listener.o $script_dir/../obj/request_queue.o -ldl -pthread


echo -e "Setting up Server ...\n"

# port queue_size max_connections max_threads max_files max_size

$script_dir/../bin/server 9999 10 10 8 1 100

