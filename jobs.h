#pragma once
#include <vector>
#include <string>
using namespace std;

struct Job {
    pid_t pgid;
    string cmd;
    bool stopped;
    int job_number;
};

extern vector<Job> jobs;
extern int next_job_number;
extern pid_t foreground_pgid;

void check_background_jobs();
void sigint_handler(int sig);
void sigtstp_handler(int sig);
