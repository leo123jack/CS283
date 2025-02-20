#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// Prototype
void trim_spaces(char *str);
extern void print_dragon();

int exec_local_cmd_loop() {
    char input[SH_CMD_MAX];
    cmd_buff_t cmd;
    
    // Allocate memory for command buffer
    cmd._cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd._cmd_buffer) {
        perror("malloc");
        return ERR_MEMORY;
    }
    
    while (1) {
        printf("%s", SH_PROMPT);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove the trailing newline then trim
        input[strcspn(input, "\n")] = '\0';
        trim_spaces(input);
        
        // Check if the user wants to exit
        if (strcmp(input, EXIT_CMD) == 0)
            break;
        
        // If the command is empty print a warning
        if (strlen(input) == 0) {
            printf("%s", CMD_WARN_NO_CMD);
            continue;
        }
        
        // Build the command buffer
        if (build_cmd_buff(input, &cmd) != 0) {
            fprintf(stderr, "Error parsing command\n");
            continue;
        }
        
        // If the command is built-in execute it
        if (match_command(cmd.argv[0]) != BI_NOT_BI) {
            exec_built_in_cmd(&cmd);
            continue;
        }
        
        // Fork and execute an external command
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        // Child process
        if (pid == 0) {  
            cmd.argv[cmd.argc] = NULL;
            execvp(cmd.argv[0], cmd.argv);
            perror("execvp");
            exit(1);
            
        // Parent process
        } else {  
            int status;
            waitpid(pid, &status, 0);
        }
    }
    
    free_cmd_buff(&cmd);
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    // Check for built-in commands
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    else if (strcmp(input, "dragon") == 0) {
        print_dragon();
    }

    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bic = match_command(cmd->argv[0]);
    if (bic == BI_CMD_CD) {
        if (cmd->argc < 2) {
            // if no directory is provided
        } else if (cmd->argc == 2) {
            if (chdir(cmd->argv[1]) != 0)
                perror("cd");
        } else {
            fprintf(stderr, "cd: too many arguments\n");
        }
    }
    return bic;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd) {
    // Copy the input line into the internal command buffer
    strncpy(cmd->_cmd_buffer, cmd_line, SH_CMD_MAX);
    cmd->_cmd_buffer[SH_CMD_MAX - 1] = '\0';

    int tokenCount = 0;
    char *cur = cmd->_cmd_buffer;
    char *tokenStart = NULL;
    bool inQuote = false;

    while (*cur != '\0') {
        // If we see a quote
        if (*cur == '\"') {
            if (inQuote) {
                // End of quoted token.
                *cur = '\0';
                if (tokenStart != NULL && tokenCount < CMD_ARGV_MAX)
                    cmd->argv[tokenCount++] = tokenStart;
                tokenStart = NULL;
                inQuote = false;
            } else {
                // Start of a quoted token
                inQuote = true;
                tokenStart = cur + 1;
            }
        } 
        // Outside quote
        else if (isspace((unsigned char)*cur) && !inQuote) {
            if (tokenStart) {
                *cur = '\0';
                if (tokenCount < CMD_ARGV_MAX)
                    cmd->argv[tokenCount++] = tokenStart;
                tokenStart = NULL;
            }
        } 
        // Regular characters
        else {
            if (!tokenStart)
                tokenStart = cur;
        }
        cur++;
    }

    // If the last token extends to the end add it
    if (tokenStart && tokenCount < CMD_ARGV_MAX) {
        cmd->argv[tokenCount++] = tokenStart;
    }

    cmd->argc = tokenCount;
    return 0;
}

void trim_spaces(char *str) {
    if (!str)
        return;

    // Trim leading spaces
    char *start = str;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != str)
        memmove(str, start, strlen(start) + 1);

    // Trim trailing spaces
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

int free_cmd_buff(cmd_buff_t *cmd) {
    if (cmd->_cmd_buffer) {
        free(cmd->_cmd_buffer);
        cmd->_cmd_buffer = NULL;
    }
    return OK;
}