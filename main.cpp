#include "pipeline.h"
#include "jobs.h"
#include "history.h"
#include "trie.h"

#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ctime>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <bits/stdc++.h>
#include <readline/readline.h>
#include <readline/history.h>
using namespace std;

#define HISTORY_FILE "history.txt"
#define MAX_HISTORY_DISPLAY 10
#define MAX_HISTORY_STORE 20
char *shell_homedir = "";

std::string prevDir = "";
TrieNode *root;

void buildTrieFromFile(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        buffer[strcspn(buffer, "\n")] = 0;
        insertCommand(root, buffer);
    }
    fclose(fp);
}

char *commandGenerator(const char *text, int state)
{
    static vector<string> matches;
    static size_t list_index;
    if (state == 0)
    {
        matches.clear();
        list_index = 0;
        TrieNode *curr = root;
        for (int i = 0; text[i]; i++)
        {
            int idx = (int)text[i];
            if (!curr->children[idx])
                return nullptr;
            curr = curr->children[idx];
        }
        vector<string> results;
        dfs(curr, string(text), results);
        matches = results;
    }
    if (list_index < matches.size())
    {
        return strdup(matches[list_index++].c_str());
    }
    return nullptr;
}

char **commandCompletion(const char *text, int start, int end)
{
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, commandGenerator);
}
string showprompt(const string &homeDir, const string &username, const string &systemname)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    string currDir = cwd;
    if (currDir.find(shell_homedir) == 0)
    {
        if (currDir == shell_homedir)
            currDir = "~";
        else
            currDir.replace(0, strlen(shell_homedir), "~");
    }
    return username + "@" + systemname + ":" + currDir + "> ";
}

void cd_fn(char *argv[], int count)
{
    if (count > 2)
    {
        std::cout << "Invalid arguments\n";
        return;
    }

    string newdir = "";
    string homedir = getenv("HOME");
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    if (count == 1)
    {
        newdir = homedir;
    }
    else if (count == 2)
    {
        if (strcmp(argv[1], "-") == 0)
        {
            if (prevDir.empty())
            {
                std::cout << "No previous directory\n";
                return;
            }
            newdir = prevDir;
        }
        else if (strcmp(argv[1], ".") == 0)
        {
            return;
        }
        else if (strcmp(argv[1], "..") == 0)
        {
            newdir = "..";
        }
        else if (strcmp(argv[1], "~") == 0)
        {
            newdir = homedir;
        }
        else
        {
            newdir = argv[1];
        }
    }

    if (chdir(newdir.c_str()) != 0)
    {
        perror("cd failed");
    }
    else
    {
        prevDir = cwd;
    }
}

void echo_fn(char *argv[], int count)
{
    if (count == 1)
    {
        cout << endl;
        return;
    }
    for (int i = 1; i < count; i++)
    {
        string arg = argv[i];
        // if (arg.size() >= 2 &&
        //     ((arg.front() == '"' && arg.back() == '"') ||
        //      (arg.front() == '\'' && arg.back() == '\'')))
        // {
        //     arg = arg.substr(1, arg.size() - 2);
        // }
        cout << arg;
        if (i < count - 1)
            cout << " ";
    }
    cout << endl;
}

void pwd_fn()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        cout << cwd << endl;
    else
        perror("pwd error");
}

// void ls(string &directory, int count)
// {
//     DIR *d = opendir(directory.c_str());
//     if (!d)
//     {
//         perror(("cannot open " + directory).c_str());
//         return;
//     }
//     vector<string> fls;
//     struct dirent *file;
//     while ((file = readdir(d)) != NULL)
//     {
//         string s = file->d_name;
//         if (s[0] == '.')
//             continue;
//         fls.push_back(s);
//     }
//     closedir(d);
//     sort(fls.begin(), fls.end());
//     if (count == 1)
//     {
//         string header = directory + ":\n";
//         write(STDOUT_FILENO, header.c_str(), header.size()); // Use write instead of cout
//     }
//     for (auto &i : fls)
//     {
//         string entry = i + " ";
//         write(STDOUT_FILENO, entry.c_str(), entry.size()); // Use write instead of cout
//     }
//     write(STDOUT_FILENO, "\n", 1); // Add a newline at the end
// }

void ls(string &directory, int count)
{
    DIR *d = opendir(directory.c_str());
    if (!d)
    {
        perror(("cannot open " + directory).c_str());
        return;
    }
    vector<string> fls;
    struct dirent *file;
    while ((file = readdir(d)) != NULL)
    {
        string s = file->d_name;
        if (s[0] == '.')
            continue;
        fls.push_back(s);
    }
    closedir(d);
    sort(fls.begin(), fls.end());
    if (count == 1)
        cout << directory << ":" << endl;
    bool is_terminal = isatty(STDOUT_FILENO);
    for (auto &i : fls)
    {
        if (is_terminal)
            cout << i << " ";
        else
            cout << i << "\n"; // newline when output is a pipe
    }
    if (is_terminal)
        cout << endl;
}

void ls_with_a(string directory, int count)
{
    DIR *d = opendir(directory.c_str());
    if (!d)
    {
        perror(("cannot open " + directory).c_str());
        return;
    }
    vector<string> fls;
    struct dirent *file;
    while ((file = readdir(d)) != NULL)
        fls.push_back(file->d_name);
    closedir(d);
    sort(fls.begin(), fls.end());
    if (count == 1)
        cout << directory << ":" << endl;
    bool is_terminal = isatty(STDOUT_FILENO);

    // for (auto &i : fls)
    //     cout << i << " ";
    // cout << endl;
    for (auto &i : fls)
    {
        if (is_terminal)
            cout << i << " ";
        else
            cout << i << "\n"; // newline when output is a pipe
    }
    if (is_terminal)
        cout << endl;
}

void ls_with_l(const string &path, int show_hidden, int count)
{
    DIR *d = opendir(path.c_str());
    if (!d)
    {
        perror(("cannot open " + path).c_str());
        return;
    }
    vector<string> fls;
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL)
    {
        string name = entry->d_name;
        if (!show_hidden && name[0] == '.')
            continue;
        fls.push_back(name);
    }
    closedir(d);
    sort(fls.begin(), fls.end());

    long long total = 0;
    for (auto &f : fls)
    {
        struct stat st;
        string full = path + "/" + f;
        if (stat(full.c_str(), &st) == 0)
            total += st.st_blocks;
    }
    if (count == 1)
        cout << path << ":" << endl;
    cout << "total " << (total / 2) << endl;

    for (auto &f : fls)
    {
        struct stat st;
        string full = path + "/" + f;
        if (stat(full.c_str(), &st) == -1)
            continue;
        cout << (S_ISDIR(st.st_mode) ? 'd' : S_ISLNK(st.st_mode) ? 'l'
                                                                 : '-');
        cout << ((st.st_mode & S_IRUSR) ? 'r' : '-');
        cout << ((st.st_mode & S_IWUSR) ? 'w' : '-');
        cout << ((st.st_mode & S_IXUSR) ? 'x' : '-');
        cout << ((st.st_mode & S_IRGRP) ? 'r' : '-');
        cout << ((st.st_mode & S_IWGRP) ? 'w' : '-');
        cout << ((st.st_mode & S_IXGRP) ? 'x' : '-');
        cout << ((st.st_mode & S_IROTH) ? 'r' : '-');
        cout << ((st.st_mode & S_IWOTH) ? 'w' : '-');
        cout << ((st.st_mode & S_IXOTH) ? 'x' : '-');

        cout << " " << st.st_nlink;

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        cout << " " << (pw ? pw->pw_name : to_string(st.st_uid));
        cout << " " << (gr ? gr->gr_name : to_string(st.st_gid));

        cout << " " << st.st_size;

        char timebuf[80];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
        cout << " " << timebuf;

        cout << " " << f << endl;
    }
    cout << endl;
}

void pinfo(char *args, bool p)
{
    pid_t pid;
    char path[100], buffer[1000], exe_path[1000];
    FILE *file;
    if (!p)
        pid = getpid();
    else
        pid = atoi(args);

    sprintf(path, "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (!file)
    {
        cout << "Process " << pid << " not found\n";
        return;
    }

    fgets(buffer, sizeof(buffer), file);
    fclose(file);

    char state;
    unsigned long vmsize;
    sscanf(buffer, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u",
           &state, &vmsize);

    pid_t fg_pgid = tcgetpgrp(STDIN_FILENO); // foreground process group of terminal
    pid_t pgid = getpgid(pid);               // process group of target process

    cout << "Process Status -- " << state;
    if (fg_pgid == pgid)
        cout << "+";
    cout << "\n";
    cout << "memory -- " << vmsize << " {Virtual Memory}\n";

    sprintf(path, "/proc/%d/exe", pid);
    int len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0';
        string exeStr(exe_path);

        if (shell_homedir)
        {
            string home(shell_homedir);
            if (exeStr.find(home) == 0)
                exeStr.replace(0, home.length(), "~");
        }

        cout << "Executable Path -- " << exeStr << "\n";
    }
    else
        cout << "Executable Path -- [unknown]\n";
}

bool found = false;
void search_rec(const string &dir, string &target)
{
    DIR *d = opendir(dir.c_str());
    if (!d)
        return;
    struct dirent *file;
    while ((file = readdir(d)) != NULL)
    {
        string name = file->d_name;
        if (name == "." || name == "..")
            continue;
        if (name == target)
        {
            cout << "True\n";
            found = true;
            closedir(d);
            return;
        }
        string path = dir + "/" + name;
        struct stat st;
        if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        {
            search_rec(path, target);
            if (found)
            {
                closedir(d);
                return;
            }
        }
    }
    closedir(d);
}

void search(string target)
{
    found = false;
    search_rec(".", target);
    if (!found)
        cout << "False\n";
}

bool is_builtin(const char *cmd)
{ // cout<<"Checking if builtin: "<<cmd<<endl;
    return (strcmp(cmd, "cd") == 0 || strcmp(cmd, "pwd") == 0 ||
            strcmp(cmd, "echo") == 0 || strcmp(cmd, "ls") == 0 ||
            strcmp(cmd, "pinfo") == 0 || strcmp(cmd, "search") == 0);
}

void execute_builtin(char *argvec[], int argcount)
{
    if (strcmp(argvec[0], "cd") == 0)
        cd_fn(argvec, argcount);
    else if (strcmp(argvec[0], "pwd") == 0)
        pwd_fn();
    else if (strcmp(argvec[0], "echo") == 0)
        echo_fn(argvec, argcount);
    else if (strcmp(argvec[0], "ls") == 0)
    {
        // cout << "Command is ls" << endl;
        if (argcount == 1)
        {
            string s1 = ".";
            ls(s1, 0);
        }
        else
        {
            vector<string> paths;
            int l{}, a{};
            int i = 1;
            while (argvec[i] != NULL)
            {
                if (strcmp(argvec[i], "-a") == 0)
                {
                    a = 1;
                }
                else if (strcmp(argvec[i], "-l") == 0)
                {
                    l = 1;
                }
                else if (strcmp(argvec[i], "-al") == 0 || strcmp(argvec[i], "-la") == 0)
                {
                    l = 1;
                    a = 1;
                }
                else
                {
                    string temp = argvec[i];
                    if (temp[0] == '~')
                        temp.replace(0, 1, getenv("HOME"));
                    paths.push_back(temp);
                }

                i++;
            }
            vector<string> files;
            vector<string> directories;

            // separate files and directories
            for (auto &p : paths)
            {
                struct stat st;
                if (stat(p.c_str(), &st) != 0)
                {
                    perror(("cannot access " + p).c_str());
                    continue;
                }
                if (S_ISDIR(st.st_mode))
                    directories.push_back(p);
                else
                    files.push_back(p);
            }
            // cout<<"#of files: "<<files.size()<<endl;
            // cout<<"#of dirs: "<<directories.size()<<endl;
            int count{};
            if (paths.size() > 1)
                count = 1;
            //  cout << "count=" << count << endl;

            int file_count = {};
            for (auto &f : files)
            {
                if (l)
                {
                    // print single file info
                    struct stat st;
                    if (stat(f.c_str(), &st) == 0)
                    {
                        cout << (S_ISDIR(st.st_mode) ? 'd' : S_ISLNK(st.st_mode) ? 'l'
                                                                                 : '-');
                        cout << ((st.st_mode & S_IRUSR) ? 'r' : '-');
                        cout << ((st.st_mode & S_IWUSR) ? 'w' : '-');
                        cout << ((st.st_mode & S_IXUSR) ? 'x' : '-');
                        cout << ((st.st_mode & S_IRGRP) ? 'r' : '-');
                        cout << ((st.st_mode & S_IWGRP) ? 'w' : '-');
                        cout << ((st.st_mode & S_IXGRP) ? 'x' : '-');
                        cout << ((st.st_mode & S_IROTH) ? 'r' : '-');
                        cout << ((st.st_mode & S_IWOTH) ? 'w' : '-');
                        cout << ((st.st_mode & S_IXOTH) ? 'x' : '-');

                        cout << " " << st.st_nlink;

                        struct passwd *pw = getpwuid(st.st_uid);
                        struct group *gr = getgrgid(st.st_gid);
                        cout << " " << (pw ? pw->pw_name : to_string(st.st_uid));
                        cout << " " << (gr ? gr->gr_name : to_string(st.st_gid));

                        cout << " " << st.st_size;

                        char timebuf[80];
                        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
                        cout << " " << timebuf;

                        cout << " " << f << endl;
                    }
                }
                else
                {
                    file_count++;
                    cout << f << " ";
                }
                cout << "Done" << endl;
            }
            if (file_count && (!l))
                cout << endl;

            if (a == 1)
            {
                // both
                if (l == 1)
                {
                    if (paths.size() == 0 && directories.size() == 0)
                    {
                        ls_with_l(".", 1, count);
                    }
                    else
                    {
                        for (auto j : directories)
                        {
                            ls_with_l(j, 1, count);
                        }
                    }
                }

                else
                {
                    if (paths.size() == 0 && directories.size() == 0)
                    {
                        ls_with_a(".", count);
                    }
                    else
                    {
                        for (auto j : directories)
                        {
                            ls_with_a(j, count);
                        }
                    }
                }
            }
            else if (l == 1)
            {
                if (paths.size() == 0 && directories.size() == 0)
                {
                    ls_with_l(".", 0, count);
                }
                else
                {
                    for (auto j : directories)
                    {
                        ls_with_l(j, 0, count);
                    }
                }
            }
            else
            {
                for (auto j : directories)
                {
                    ls(j, count);
                }
            }
        }
    }
    else if (strcmp(argvec[0], "pinfo") == 0)
    {
        if (argcount == 1)
            pinfo(nullptr, false);
        else
            pinfo(argvec[1], true);
    }
    else if (strcmp(argvec[0], "search") == 0)
    {
        if (argcount != 2)
            cout << "Requires Target file\n";
        else
            search(argvec[1]);
    }
}

int main()
{
    // Set up signal handlers
    char cwd[PATH_MAX]; // fixed-size buffer
    std::cout.setf(std::ios::unitbuf);

    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
        shell_homedir = cwd; // just assign pointer (points to buffer)
        std::cout << "Current dir: " << shell_homedir << std::endl;
    }
    else
    {
        perror("getcwd error");
    }
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGTTOU, SIG_IGN); // ignore terminal output signals

    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    string username = pw ? pw->pw_name : "user";

    struct utsname sysinfo;
    uname(&sysinfo);
    string systemname = sysinfo.nodename;

    string homeDir = getenv("HOME");
    root = createNode();

    // Build trie with commands from file
    buildTrieFromFile("commands.txt");

    // Add custom shell commands
    const char *myCommands[] = {"cd", "ls", "echo", "pwd", "pinfo", "search", "history", nullptr};
    for (int i = 0; myCommands[i]; i++)
        insertCommand(root, myCommands[i]);

    // Setup readline completion function
    rl_attempted_completion_function = commandCompletion;

    // --- Initialize history ---
    using_history();
    loadHistory();

    // while (true)
    // {
    //     string prompt = showprompt(homeDir, username, systemname);
    //     char *input = readline(prompt.c_str());
    //     if (!input)
    //         break;

    //     if (strlen(input) == 0)
    //     {
    //         free(input);
    //         continue;
    //     }
    //     addCommandToHistory(input);
    //     add_history(input);
    //     // Handle multiple commands separated by ';'
    //     char *line = strdup(input);
    //     char *cmdStr = strtok(line, ";");
    //     while (cmdStr != NULL)
    //     {   if (strncmp(input, "history", 7) == 0)
    //     {
    //         int num = MAX_HISTORY_DISPLAY; // default last 10
    //         if (strlen(input) > 7 && input[7] == ' ')
    //         {
    //             num = atoi(input + 8);
    //         }
    //         showHistory(num);
    //         free(input);
    //         continue;
    //     }

    //         helper(cmdStr);              // pipeline module
    //         cmdStr = strtok(NULL, ";");
    //     }
    // free(line);
    // char *line = strdup(input);
    // char *cmdStr = strtok(line, ";");

    // while (cmdStr != NULL)
    // {
    //     // --- Trim leading spaces and newlines ---
    //     while (*cmdStr == ' ' || *cmdStr == '\n')
    //         cmdStr++;

    //     // --- Trim trailing spaces and newlines ---
    //     int len = strlen(cmdStr);
    //     while (len > 0 && (cmdStr[len - 1] == ' ' || cmdStr[len - 1] == '\n'))
    //         cmdStr[--len] = '\0';

    //     // Skip empty commands
    //     if (strlen(cmdStr) == 0)
    //     {
    //         cmdStr = strtok(NULL, ";");
    //         continue;
    //     }

    //     // --- Handle history command ---
    //     if (strncmp(cmdStr, "history", 7) == 0)
    //     {
    //         int num = MAX_HISTORY_DISPLAY; // default last 10
    //         if (strlen(cmdStr) > 7 && cmdStr[7] == ' ')
    //             num = atoi(cmdStr + 8);
    //         showHistory(num);
    //     }
    //     else
    //     {
    //         // --- Pass command to pipeline module ---
    //         helper(cmdStr);
    //     }

    //     cmdStr = strtok(NULL, ";");
    // }

    // free(line);

    //         free(input);
    //         saveHistory();
    //     }

    //     saveHistory(); // save last 20 commands
    //     // cout << "Terminated\n";
    //     return 0;
    // }



    //15:27 9/10/2025
  //  while (true)
    // {
    //     // Show prompt
    //     string prompt = showprompt(homeDir, username, systemname);
    //     char *input = readline(prompt.c_str());
    //     if (!input)
    //         break; // Ctrl+D pressed

    //     // Skip empty input
    //     if (strlen(input) == 0)
    //     {
    //         free(input);
    //         continue;
    //     }

    //     // Add to history
    //     addCommandToHistory(input);
    //     add_history(input);

    //     // Duplicate input to safely tokenize
    //     char *line = strdup(input);
    //     cout << "niput command read -->" << line << endl;
    //     char *cmdStr = strtok(line, ";");

    //     while (cmdStr != NULL)
    //     {
    //         // Trim leading spaces
    //         cout << "command in main while loop--->" << cmdStr << endl;
    //         while (*cmdStr == ' ')
    //             cmdStr++;

    //         if (strncmp(cmdStr, "history", 7) == 0)
    //         {
    //             int num = MAX_HISTORY_DISPLAY; // default
    //             if (strlen(cmdStr) > 7 && cmdStr[7] == ' ')
    //             {
    //                 num = atoi(cmdStr + 8);
    //             }
    //             showHistory(num);
    //         }
    //         else
    //         {
    //             helper(cmdStr); // Execute the command/pipeline
    //         }

    //         // Move to the next command
    //         cmdStr = strtok(NULL, ";");
    //     }
    //     char *line = strdup(input);
    //     char *cmdStr = strtok(line, ";");

    //     char *cmdStr = strtok(line, ";");

    //     free(line);

    //     // free(line);
    //     free(input);

    //     // Optional: persist last 20 commands after each input
    //     saveHistory();
    // }
// 
    // saveHistory(); // final save on exit
    // return 0;

    while (true)
{
    // --- Show prompt and read input ---
    string prompt = showprompt(homeDir, username, systemname);
    char *input = readline(prompt.c_str());
    if (!input)
        break; // Ctrl+D pressed

    if (strlen(input) == 0)
    {
        free(input);
        continue;
    }

    addCommandToHistory(input);
    add_history(input);

    cout << "Input command read --> " << input << endl;

    // --- Split by ';' using stringstream ---
    string inputStr(input);
    stringstream ss(inputStr);
    string segment;
    vector<string> commands;

    while (getline(ss, segment, ';'))
    {
        // Trim leading/trailing spaces
        while (!segment.empty() && isspace(segment.front()))
            segment.erase(segment.begin());
        while (!segment.empty() && isspace(segment.back()))
            segment.pop_back();

        if (!segment.empty())
            commands.push_back(segment);
    }

    // --- Process each command separately ---
    for (string &cmd : commands)
    {
        // cout << "Command token --> " << cmd << endl;

        if (cmd.rfind("history", 0) == 0) // starts with "history"
        {
            int num = MAX_HISTORY_DISPLAY;
            if (cmd.length() > 7 && cmd[7] == ' ')
                num = atoi(cmd.c_str() + 8);
            showHistory(num);
        }
        else
        {
            char *cmdCopy = strdup(cmd.c_str());
            helper(cmdCopy);
            free(cmdCopy);
        }
    }

    free(input);
    saveHistory();
}

saveHistory(); // final save on exit
return 0;

}