#include "builtins.h"
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;


char* shell_homedir = "";

std::string prevDir="";  // make this global

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
    cout << "DEBUG: chdir to " << newdir << endl;
if (chdir(newdir.c_str()) != 0) {
    perror("cd failed");
} else {
    prevDir = cwd;
    char test[1024];
    getcwd(test, sizeof(test));
    cout << "DEBUG: now in " << test << endl;
}

    // if (chdir(newdir.c_str()) != 0)
    // {
    //     perror("cd failed");
    // }
    // else
    // {
    //     prevDir = cwd;
    // }
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
        if (arg.size() >= 2 &&
            ((arg.front() == '"' && arg.back() == '"') ||
             (arg.front() == '\'' && arg.back() == '\'')))
        {
            arg = arg.substr(1, arg.size() - 2);
        }
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
    for (auto &i : fls)
        cout << i << " ";
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
    for (auto &i : fls)
        cout << i << " ";
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
{
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

                int count{};
                if (paths.size() > 1)
                    count = 1;
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
                }
                if (file_count && (!l))
                    cout << endl;

                if (a == 1)
                {
                    // both
                    if (l == 1)
                    {
                        if (directories.size() == 0)
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
                        if (directories.size() == 0)
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
                    if (directories.size() == 0)
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
