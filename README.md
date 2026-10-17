# Custom Posix Shell -- Assignment 2 

## 📌 Overview

This project is a **custom Posix shell** implemented in **C++17** as part
of the Advanced Operating Systems course.\
It mimics many of the functionalities of a standard shell (like `bash`)
and adds features such as:

-   Execution of built-in commands (`cd`, `ls`, `pwd`, `echo`, `pinfo`,
    `search`, `history`)\
-   Foreground and background job management with job control (`Ctrl+C`,
    `Ctrl+Z`)\
-   Input/output redirection (`>`, `>>`, `<`)\
-   Pipelining of commands (`cmd1 | cmd2 | cmd3`)\
-   Persistent command history (last **20 commands saved**, last **10
    displayed**)\
-   Autocompletion of **commands** and **files** using a **Trie** and
    GNU Readline.

## ⚙️ Project Structure

    .
    ├── main.cpp         # Entry point, handles prompt, input loop, orchestration
    ├── history.cpp      # Persistent history management (last 20 commands)
    ├── jobs.cpp         # Job control, signals, foreground/background handling
    ├── pipeline.cpp     # Parsing/execution of pipelines and redirection
    ├── trie.cpp         # Trie-based command autocompletion
    ├── trie.h           # Trie header file
    ├── run_shell.sh     # Script to build and run the shell
    ├── commands.txt     # Auto-generated list of available commands (via compgen)
    └── history.txt      # Persistent command history (created at runtime)

## 🚀 Features

### 1. Prompt

The shell prompt displays:

    <username>@<systemname>:<current_directory> >

-   Home directory (`$HOME`) is replaced with `~`.\
-   Updates dynamically using `getcwd()`.

### 2. Built-in Commands

#### cd

-   Change the current working directory.\
-   Supports:
    -   `cd` → go to home directory\
    -   `cd ..` → go one directory up\
    -   `cd .` → stay in current directory\
    -   `cd -` → go to previous directory\
    -   `cd ~` → go to home directory\
-   Updates `prevDir` for `cd -`.

#### pwd

-   Prints the current working directory using `getcwd()`.

#### echo

-   Prints arguments to standard output.\
-   Strips surrounding quotes (`" "` or `' '`).

#### ls

-   Lists files in a directory.\
-   Supports:
    -   `ls` → list non-hidden files\
    -   `ls -a` → include hidden files\
    -   `ls -l` → detailed listing (permissions, owner, size, time)\
    -   `ls -al` or `ls -la` → both options\
    -   Handles multiple directories and files.\
-   Special handling for:
    -   `ls ~` → home directory\
    -   `ls .` → current directory\
    -   `ls ..` → parent directory

#### pinfo \[pid\]

-   Displays information about the given process (or current shell if no
    `pid`).\
-   Reads from `/proc/<pid>/stat` and `/proc/<pid>/exe`.\
-   Outputs:
    -   Process status (+ if in foreground)\
    -   Virtual memory usage\
    -   Executable path (with `~` substitution).

#### search `<filename>`{=html}

-   Recursively searches from current directory (`.`).\
-   Prints `True` if found, otherwise `False`.

### 3. History

-   Maintains up to **20 commands** across sessions.\
-   Stored in `history.txt`.\
-   Avoids duplicates (consecutive same commands not stored).\
-   Commands:
    -   `history` → show last 10 commands\
    -   `history <num>` → show last `<num>` commands (capped at 10).

Up/Down arrow navigation is handled by **GNU Readline**.

### 4. Job Control

-   **Foreground process group tracking** (`foreground_pgid`).\
-   **Signal handling**:
    -   `Ctrl+C` (`SIGINT`) → kills current foreground process.\
    -   `Ctrl+Z` (`SIGTSTP`) → suspends current foreground process.\
-   Background jobs are listed and cleaned using `waitpid()` in
    `check_background_jobs()`.\
-   Properly manages stopped and running jobs with job numbers.

### 5. Pipelining & Redirection

-   Multiple commands can be connected with pipes (`|`).\
-   Supports redirection:
    -   `>` → write to file (overwrite)\
    -   `>>` → append to file\
    -   `<` → read input from file\
-   Combination of pipes and redirection is supported.

### 6. Autocompletion

-   **Trie-based command completion** + **file completion**.\
-   Uses **GNU Readline's completion API** (`rl_completion_matches`).\
-   Sources for commands:
    -   System executables (`compgen -c`)\
    -   Built-ins (`compgen -b`)\
    -   Aliases (`compgen -a`)\
    -   Custom commands added manually.\
-   Files/directories are fetched from the current working directory.

## 🛠️ How to Compile & Run

1.  Give execution permission to the script:

    ``` bash
    chmod +x run_shell.sh
    ```

2.  Run the shell:

    ``` bash
    ./run_shell.sh
    ```

The script will: 1. Use `compgen` to dump all available commands
(executables, builtins, aliases) into `commands.txt`.\
2. Compile all `.cpp` files with `g++ -std=c++17`.\
3. Link with **readline** and **ncurses** libraries.\
4. Run the compiled shell.

## 🧩 Example Session

    suparshwa@ubuntu:~> pwd
    /home/suparshwa
    suparshwa@ubuntu:~> cd Documents
    suparshwa@ubuntu:~/Documents> ls -al
    .  ..  file1.txt  file2.cpp
    suparshwa@ubuntu:~/Documents> echo "Hello World"
    Hello World
    suparshwa@ubuntu:~/Documents> search file2.cpp
    True
    suparshwa@ubuntu:~/Documents> pinfo
    Process Status -- S+
    memory -- 123456 {Virtual Memory}
    Executable Path -- /bin/bash
    suparshwa@ubuntu:~/Documents> history
    ls -al
    echo "Hello World"
    search file2.cpp
    pinfo
    history
    suparshwa@ubuntu:~/Documents> exit
    Terminated

## 📂 Files Generated During Execution

-   `commands.txt` → Auto-generated list of all commands (via compgen).\
-   `history.txt` → Stores last 20 commands across sessions.


