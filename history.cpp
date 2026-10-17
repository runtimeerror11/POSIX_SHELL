#include "history.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define HISTORY_FILE "history.txt"
#define MAX_HISTORY_DISPLAY 10
#define MAX_HISTORY_STORE 20

// Load history from file
void loadHistory() {
    if (read_history(HISTORY_FILE) != 0) {
        perror("Failed to load history");
    }
}

// Save history to file
void saveHistory() {
    HIST_ENTRY **hist = history_list();
    if (!hist) return;

    int total = history_length;
    int start = total > MAX_HISTORY_STORE ? total - MAX_HISTORY_STORE : 0;

    FILE *fp = fopen(HISTORY_FILE, "w");
    if (!fp) return;

    for (int i = start; i < total; i++) {
        fprintf(fp, "%s\n", hist[i]->line);
    }

    fclose(fp);
}

// Add a command to history, avoiding duplicates
void addCommandToHistory(const char *cmd) {
    if (strlen(cmd) > 0) {
        HIST_ENTRY **hist = history_list();
        int total = history_length;

        // Check if the command already exists in the history
        if (total > 0 && strcmp(hist[total - 1]->line, cmd) == 0) {
            return; // Skip adding duplicate commands
        }

        // Add the command to the history
        add_history(cmd);
    }
}

// Display the last `num` commands from history
void showHistory(int num) {
    HIST_ENTRY **hist = history_list();
    if (!hist) return;

    int total = history_length;

    // Limit the number of commands displayed
    if (num > MAX_HISTORY_DISPLAY) num = MAX_HISTORY_DISPLAY;
    if (num > total) num = total;

    int start = total - num;
    for (int i = start; i < total; i++) {
        printf("%s\n", hist[i]->line);
    }
}