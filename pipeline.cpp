#include "pipeline.h"
#include "builtins.h"
#include "jobs.h"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;


// Add this function definition in your builtins.cpp
#include <string>
#include <vector>
#include <algorithm>

bool is_shell_affecting_builtin(const char *cmd) {
    static const std::vector<std::string> shell_builtins = {
        "cd", "exit", "export", "unset", "jobs", "fg", "bg"
    };
    return std::find(shell_builtins.begin(), shell_builtins.end(), cmd) != shell_builtins.end();
}


pair<vector<Command>, bool> parse_pipeline(char *cmdStr) {
    vector<Command> commands;
    Command cmd;
    bool background = false;

    char *tok = strtok(cmdStr, " ");
    while (tok != NULL) {
        if (strcmp(tok, "|") == 0) {
            commands.push_back(cmd);
            cmd = Command();
        } else if (strcmp(tok, "<") == 0) { tok = strtok(NULL, " "); cmd.input_file = tok ? tok : ""; }
        else if (strcmp(tok, ">") == 0) { tok = strtok(NULL, " "); cmd.output_file = tok ? tok : ""; cmd.append_output = false; }
        else if (strcmp(tok, ">>") == 0) { tok = strtok(NULL, " "); cmd.output_file = tok ? tok : ""; cmd.append_output = true; }
        else if (strcmp(tok, "&") == 0) background = true;
        else cmd.args.push_back(tok);
        tok = strtok(NULL, " ");
    }
    commands.push_back(cmd);
    return {commands, background};
}

// void execute_pipeline(vector<Command> &commands, bool is_background) {
//     int n = commands.size();
// if (n == 1 && is_shell_affecting_builtin(commands[0].args[0].c_str())) {
//         char *argv[commands[0].args.size()+1];
//         for (size_t k = 0; k < commands[0].args.size(); k++) argv[k] = (char*)commands[0].args[k].c_str();
//         argv[commands[0].args.size()] = NULL;
//         execute_builtin(argv, commands[0].args.size());
//         return;
//     }



//     int pipefd[2*n]; // max n pipes
//     for (int i = 0; i < n-1; i++) {
//         if (pipe(pipefd + 2*i) < 0) perror("pipe failed");
//     }

//     vector<pid_t> pids;
//     for (int i = 0; i < n; i++) {
//         pid_t pid = fork();
//         if (pid == 0) {
//         for (auto pid : pids) waitpid(pid, nullptr, 0);
//         foreground_pgid = -1;
//     } else {
//         Job j; j.pgid = pids[0]; j.cmd = commands[0].args[0]; j.stopped = false; j.job_number = next_job_number++;
//         jobs.push_back(j);
//         cout << "[" << j.job_number << "] " << j.pgid << "\n";
//     }
// }           if (i != 0) dup2(pipefd[2*(i-1)], 0);
//             if (i != n-1) dup2(pipefd[2*i+1], 1);

//             for (int j = 0; j < 2*(n-1); j++) close(pipefd[j]);

//             if (!commands[i].input_file.empty()) {
//                 int fd = open(commands[i].input_file.c_str(), O_RDONLY);
//                 if (fd < 0) perror("input open");
//                 dup2(fd, 0); close(fd);
//             }
//             if (!commands[i].output_file.empty()) {
//                 int fd = open(commands[i].output_file.c_str(), O_WRONLY | O_CREAT | (commands[i].append_output ? O_APPEND : O_TRUNC), 0644);
//                 if (fd < 0) perror("output open");
//                 dup2(fd, 1); close(fd);
//             }

//             if (is_builtin(commands[i].args[0].c_str())) {
//                 char *argv[commands[i].args.size()+1];
//                 for (size_t k = 0; k < commands[i].args.size(); k++) argv[k] = (char*)commands[i].args[k].c_str();
//                 argv[commands[i].args.size()] = NULL;
//                 execute_builtin(argv, commands[i].args.size());
//                 exit(0);
//             } else {
//                 vector<char*> argv;
//                 for (auto &a : commands[i].args) argv.push_back((char*)a.c_str());
//                 argv.push_back(NULL);
//                 execvp(argv[0], argv.data());
//                 perror("exec failed"); exit(1);
//             }
//         } else if (pid < 0) perror("fork failed");
//         else pids.push_back(pid);
//     }

//     for (int i = 0; i < 2*(n-1); i++) close(pipefd[i]);

//     if (!is_background) {
//         foreground_pgid = pids[0];
//         for (auto pid : pids) waitpid(pid, nullptr, 0);
//         foreground_pgid = -1;
//     } else {
//         Job j; j.pgid = pids[0]; j.cmd = commands[0].args[0]; j.stopped = false; j.job_number = next_job_number++;
//         jobs.push_back(j);
//         cout << "[" << j.job_number << "] " << j.pgid << "\n";
//     }
// }





// void execute_pipeline(vector<Command> &commands, bool is_background) {
//     int n = commands.size();

//     // --- Print commands ---
//     for(auto &j: commands) {
//         cout << "Command: ";
//         for(auto &arg : j.args)
//             cout << arg << " ";
//         if (!j.input_file.empty())
//             cout << " < " << j.input_file;
//         if (!j.output_file.empty())
//             cout << (j.append_output ? " >> " : " > ") << j.output_file;
//         cout << endl;
//     }

//     // --- Single builtin executed directly ---
//     if (n == 1 && is_shell_affecting_builtin(commands[0].args[0].c_str())) {
//         char *argv[commands[0].args.size() + 1];
//         for (size_t k = 0; k < commands[0].args.size(); k++)
//             argv[k] = (char *)commands[0].args[k].c_str();
//         argv[commands[0].args.size()] = NULL;
//         execute_builtin(argv, commands[0].args.size());
//         return;
//     }

//     // --- Create pipes ---
//     int pipefd[2 * (n-1)];
//     for (int i = 0; i < n-1; i++)
//         if (pipe(pipefd + 2*i) < 0) perror("pipe failed");

//     cout << "Pipes created for " << n << " commands." << endl;

//     vector<pid_t> pids;

//     for (int i = 0; i < n; i++) {
//         pid_t pid = fork();
//         if (pid == 0) {  // --- Child process ---

//             // --- Set process group ---
//             if (i == 0) setpgid(0, 0);
//             else setpgid(0, pids[0]);

//             // --- Setup stdin/stdout ---
//             if (i != 0) dup2(pipefd[2*(i-1)], STDIN_FILENO);
//             if (i != n-1) dup2(pipefd[2*i + 1], STDOUT_FILENO);

//             // --- Close all unused pipe fds ---
//             for (int j = 0; j < 2*(n-1); j++) {
//                 if ((i != 0 && j == 2*(i-1)) || (i != n-1 && j == 2*i + 1))
//                     continue; // keep duplicated fds open
//                 close(pipefd[j]);
//             }

//             // --- Handle input/output redirection ---
//             if (!commands[i].input_file.empty()) {
//                 int fd = open(commands[i].input_file.c_str(), O_RDONLY);
//                 if (fd < 0) perror("input open");
//                 else { dup2(fd, STDIN_FILENO); close(fd); }
//             }

//             if (!commands[i].output_file.empty()) {
//                 int fd = open(commands[i].output_file.c_str(),
//                               O_WRONLY | O_CREAT | (commands[i].append_output ? O_APPEND : O_TRUNC), 0644);
//                 if (fd < 0) perror("output open");
//                 else { dup2(fd, STDOUT_FILENO); close(fd); }
//             }

//             // --- Handle builtins ---
//             if (is_builtin(commands[i].args[0].c_str())) {
//                 char *argv[commands[i].args.size() + 1];
//                 for (size_t k = 0; k < commands[i].args.size(); k++) {
//                     string arg = commands[i].args[k];
//                     // --- Strip quotes ---
//                     if ((arg.front() == '"' && arg.back() == '"') || 
//                         (arg.front() == '\'' && arg.back() == '\''))
//                         arg = arg.substr(1, arg.size() - 2);
//                     argv[k] = strdup(arg.c_str());
//                 }
//                 argv[commands[i].args.size()] = NULL;
//                 execute_builtin(argv, commands[i].args.size());
//                 exit(0);
//             } else {
//                 vector<char*> argv;
//                 for (auto &a : commands[i].args) {
//                     string arg = a;
//                     if ((arg.front() == '"' && arg.back() == '"') || 
//                         (arg.front() == '\'' && arg.back() == '\''))
//                         arg = arg.substr(1, arg.size() - 2);
//                     argv.push_back(strdup(arg.c_str()));
//                 }
//                 argv.push_back(NULL);
//                 execvp(argv[0], argv.data());
//                 perror("exec failed");
//                 exit(1);
//             }
//         }
//         else if (pid < 0) {
//             perror("fork failed");
//         } else {
//             // --- Parent sets PGID ---
//             if (i == 0) setpgid(pid, pid);
//             else setpgid(pid, pids[0]);
//             pids.push_back(pid);
//         }
//     }

//     // --- Parent closes all pipe fds ---
//     for (int i = 0; i < 2*(n-1); i++) close(pipefd[i]);

//     // --- Wait for foreground or manage background ---
//     if (!is_background) {
//         for (auto pid : pids)
//             waitpid(pid, nullptr, 0);
//     } else {
//         Job j;
//         j.pgid = pids[0];
//         j.cmd = commands[0].args[0];
//         j.stopped = false;
//         j.job_number = next_job_number++;
//         jobs.push_back(j);
//         cout << "[" << j.job_number << "] " << j.pgid << "\n";
//     }
// }


void execute_pipeline(vector<Command> &commands, bool is_background) {
    int n = commands.size();
    
        // --- Print commands ---
    // for(auto &j: commands) {
    //     cout << "Command: ";
    //     for(auto &arg : j.args)
    //         cout << arg << " ";
    //     if (!j.input_file.empty())
    //         cout << " < " << j.input_file;
    //     if (!j.output_file.empty())
    //         cout << (j.append_output ? " >> " : " > ") << j.output_file;
    //     cout << endl;
    // }


    // --- Builtin commands executed directly ---
    if (n == 1 && is_shell_affecting_builtin(commands[0].args[0].c_str())) {
        char *argv[commands[0].args.size() + 1];
        for (size_t k = 0; k < commands[0].args.size(); k++)
            argv[k] = (char *)commands[0].args[k].c_str();
        argv[commands[0].args.size()] = NULL;
        execute_builtin(argv, commands[0].args.size());
        return;
    }

    // --- Create required pipes ---
    int pipefd[2 * n];
    for (int i = 0; i < n - 1; i++) {
        if (pipe(pipefd + 2 * i) < 0)
            perror("pipe failed");
    }

    vector<pid_t> pids;

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // ========== 🔹 NEW: Set process group for child ==========
            if (i == 0) {
                // First command in pipeline
                setpgid(0, 0); // create new process group for this job
            } else {
                // All other commands in pipeline join the same group
                setpgid(0, pids[0]);
            }
            // =========================================================

            // --- Set up pipes for input/output ---
            if (i != 0)
                dup2(pipefd[2 * (i - 1)], 0);
            if (i != n - 1)
                dup2(pipefd[2 * i + 1], 1);

            for (int j = 0; j < 2 * (n - 1); j++)
                close(pipefd[j]);

            // --- Handle input/output redirections ---
            if (!commands[i].input_file.empty()) {
                int fd = open(commands[i].input_file.c_str(), O_RDONLY);
                if (fd < 0)
                    perror("input open");
                dup2(fd, 0);
                close(fd);
            }
            if (!commands[i].output_file.empty()) {
                int fd = open(commands[i].output_file.c_str(), O_WRONLY | O_CREAT |
                                                       (commands[i].append_output ? O_APPEND : O_TRUNC),
                              0644);
                if (fd < 0)
                    perror("output open");
                dup2(fd, 1);
                close(fd);
            }

            // --- Execute command ---
            if (is_builtin(commands[i].args[0].c_str())) {
                char *argv[commands[i].args.size() + 1];
                for (size_t k = 0; k < commands[i].args.size(); k++)
                    argv[k] = (char *)commands[i].args[k].c_str();
                argv[commands[i].args.size()] = NULL;
                execute_builtin(argv, commands[i].args.size());
                exit(0);
            } else {
                vector<char *> argv;
                for (auto &a : commands[i].args)
                    argv.push_back((char *)a.c_str());
                argv.push_back(NULL);
                execvp(argv[0], argv.data());
                perror("exec failed");
                exit(1);
            }
        } 
        else if (pid < 0) {
            perror("fork failed");
        } 
        else {
            // ========== 🔹 NEW: Parent assigns process group too ==========
            if (i == 0)
                setpgid(pid, pid);      // First child starts a new PGID
            else
                setpgid(pid, pids[0]);  // Others join that PGID
            // =============================================================
            pids.push_back(pid);
        }
    }

    for (int i = 0; i < 2 * (n - 1); i++)
        close(pipefd[i]);

    if (!is_background) {
        // Foreground job: assign terminal and wait
        foreground_pgid = pids[0];

        // ========== 🔹 NEW: Give terminal control to foreground job ==========
        tcsetpgrp(STDIN_FILENO, foreground_pgid);
        // =============================================================

        for (auto pid : pids)
            waitpid(pid, nullptr, 0);

        // ========== 🔹 NEW: Restore terminal control to shell ==========
        tcsetpgrp(STDIN_FILENO, getpgrp());

        foreground_pgid = -1;
    } else {
        // Background job
        Job j;
        j.pgid = pids[0];
        j.cmd = commands[0].args[0];
        j.stopped = false;
        j.job_number = next_job_number++;
        jobs.push_back(j);
        cout << "[" << j.job_number << "] " << j.pgid << "\n";
    }
}



void helper(char *cmdStr) {
    auto [commands, is_background] = parse_pipeline(cmdStr);
    execute_pipeline(commands, is_background);
}
