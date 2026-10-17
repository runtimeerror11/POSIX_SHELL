#!/bin/bash


{
    compgen -c      # all commands
    compgen -b      # all shell built-ins
    compgen -a      # all aliases
} | sort -u > commands.txt

g++ -std=c++17 -Wall -Wextra main.cpp trie.cpp history.cpp jobs.cpp pipeline.cpp -o main -lreadline -lncurses
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi
./main

#  chmod +x run_shell.sh
# ./run_shell.sh
