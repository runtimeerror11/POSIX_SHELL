#include "jobs.h"
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

vector<Job> jobs;
int next_job_number = 1;
pid_t foreground_pgid = -1;

void check_background_jobs()
{
    int status;
    for (auto it = jobs.begin(); it != jobs.end();)
    {
        pid_t ret = waitpid(-it->pgid, &status, WNOHANG | WUNTRACED);
        if (ret > 0)
        {
            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                cout << "[" << it->job_number << "] Done\t" << it->cmd << endl;
                it = jobs.erase(it);
            }
            else if (WIFSTOPPED(status))
            {
                it->stopped = true;
                ++it;
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }

    // Reset next_job_number if all jobs are done
    if (jobs.empty())
        next_job_number = 1;
    else
        next_job_number = jobs.back().job_number + 1;
}
// Signal handler for background process cleanup
// void sigchld_handler(int sig) {
//     pid_t pid;
//     int status;
//     while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
//         // Remove from background_pids vector
//         background_pids.erase(remove(background_pids.begin(), background_pids.end(), pid), background_pids.end());
//         cout << "\n[" << pid << "] finished\n";
//     }
// }

// Signal handler for CTRL-C (SIGINT)
void sigint_handler(int sig)
{
    if (foreground_pgid > 0)
    {
        // Send SIGINT to foreground process group
        kill(-foreground_pgid, SIGINT);
        cout << "\n";
    }
    else
    {
        // No foreground process - just print newline
        cout << "\n";
    }
}

// Signal handler for CTRL-Z (SIGTSTP)
void sigtstp_handler(int sig)
{
    if (foreground_pgid > 0)
    {
        // Send SIGTSTP to foreground process group
        kill(-foreground_pgid, SIGTSTP);
        foreground_pgid = -1;
        cout << endl;
    }
    else
    {
        // No foreground process - just print newline
        cout << "\n";
    }
}
