#pragma once
#include <string>
#include <vector>
using namespace std;

struct Command {
    vector<string> args;
    string input_file;
    string output_file;
    bool append_output;
    Command() : append_output(false) {}
};

pair<vector<Command>, bool> parse_pipeline(char *cmdStr);
void execute_pipeline(vector<Command> &commands, bool is_background);
void helper(char *cmdStr);
