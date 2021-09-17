#!/usr/bin/bash

script_dir=$(dirname "$0")

echo -e "\nCompiling...\n"
gcc $script_dir/../unit_tests/dispatcher.c -w -o $script_dir/../unit_tests/bin/dispatcher -ldl -pthread
gcc $script_dir/../unit_tests/memory.c -w -o $script_dir/../unit_tests/bin/memory -ldl -pthread
gcc $script_dir/../unit_tests/request_listener.c -w -o $script_dir/../unit_tests/bin/request_listener -ldl -pthread
gcc $script_dir/../unit_tests/request_queue.c -w -o $script_dir/../unit_tests/bin/request_queue -ldl -pthread

echo -e "Running Unit Tests...\n"

echo -e "\nTesting Dispatcher...\n"
$script_dir/../unit_tests/bin/dispatcher
echo -e "\nTesting Memory...\n"
$script_dir/../unit_tests/bin/memory
echo -e "\nTesting Request Listener...\n"
$script_dir/../unit_tests/bin/request_listener
echo -e "\nTesting Request Queue...\n"
$script_dir/../unit_tests/bin/request_queue
