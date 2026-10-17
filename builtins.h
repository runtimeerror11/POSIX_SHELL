// builtins.h
#ifndef BUILTINS_H
#define BUILTINS_H

#include <iostream>
#include <string>
extern char* shell_homedir;  // declare it for other files
extern std::string prevDir;

#include <string>
using namespace std;

void cd_fn(char *argv[], int count);
void echo_fn(char *argv[], int count);
void pwd_fn();
void ls(const string &directory, int count);
void ls_with_a(string directory, int count);
void ls_with_l(const string &path, int show_hidden, int count);
void pinfo(char *args, bool p);
void search(string target);
bool is_builtin(const char *cmd);
void execute_builtin(char *argvec[], int argcount);

#endif
