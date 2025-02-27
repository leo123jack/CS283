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

int exec_local_cmd_loop() {
    char input[SH_CMD_MAX];
    command_list_t clist;

    while (1) {
        printf("%s", SH_PROMPT);
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        // Process input line
        input[strcspn(input, "\n")] = '\0';
        trim_spaces(input);

        // Handles exit
        if (strcmp(input, EXIT_CMD) == 0) {
            break;
        }  

        // Skip empty
        if (strlen(input) == 0) {
            continue;
        }

        // Parse command line into components
        if (build_cmd_list(input, &clist) != OK) {
            continue;
        }

        // Execute command
        if (execute_pipeline(&clist) == OK_EXIT) {
            break;
        }
        free_cmd_list(&clist);
    }

    free_cmd_list(&clist);
    return OK;
}

Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    }
    if (strcmp(input, "exit") == 0) {
        return BI_CMD_EXIT;
    }
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds bic = match_command(cmd->argv[0]);
    if (bic == BI_CMD_CD) {
        if (cmd->argc < 2) {
            const char *home = getenv("HOME");
            if (home && chdir(home) != 0) {
                perror("cd");
            }
        } else if (chdir(cmd->argv[1]) != 0) {
            perror("cd");
        } else if (cmd->argc > 2) {
            fprintf(stderr, "cd: too many arguments\n");
        }
    } else if (bic == BI_CMD_EXIT) {
        return BI_CMD_EXIT;
    }
    return bic;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd) {
    strncpy(cmd->_cmd_buffer, cmd_line, SH_CMD_MAX);
    cmd->_cmd_buffer[SH_CMD_MAX - 1] = '\0';

    int tokenCount = 0;
    char *cur = cmd->_cmd_buffer;
    char *tokenStart = NULL;
    bool inQuote = false;

    // Tokenize command
    while (*cur) {
        if (*cur == '\"') {
            inQuote = !inQuote;
            *cur = '\0';
            if (!inQuote && tokenStart) {
                cmd->argv[tokenCount++] = tokenStart;
                tokenStart = NULL;
            } else {
                tokenStart = cur + 1;
            }
        } else if (isspace(*cur) && !inQuote) {
            // Handle space delimiter outside quotes
            if (tokenStart) {
                *cur = '\0';
                cmd->argv[tokenCount++] = tokenStart;
                tokenStart = NULL;
            }
        } else if (!tokenStart) {
            tokenStart = cur;
        }
        cur++;
    }

    // Add final token if exists
    if (tokenStart) {
        cmd->argv[tokenCount++] = tokenStart;
    }
    cmd->argc = tokenCount;
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token, *rest = cmd_line;
    clist->num = 0;

    // Tokenize pipeline components
    while ((token = strtok_r(rest, PIPE_STRING, &rest))) {
        if (clist->num >= CMD_MAX) {
            fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            return ERR_TOO_MANY_COMMANDS;
        }

        trim_spaces(token);
        if (strlen(token) == 0) {
            continue;
        }

        // Allocate and parse command buffer
        cmd_buff_t *cmd = &clist->commands[clist->num];
        if (alloc_cmd_buff(cmd) != OK) {
            return ERR_MEMORY;
        }
        if (build_cmd_buff(token, cmd) != OK) {
            return ERR_CMD_ARGS_BAD;
        }
        clist->num++;
    }

    return clist->num > 0 ? OK : WARN_NO_CMDS;
}

int execute_pipeline(command_list_t *clist) {
    int num_commands = clist->num;
    int pipefds[2 * (num_commands - 1)];
    pid_t pids[num_commands];

    // Handle single built in commands
    if (num_commands == 1) {
        cmd_buff_t *cmd = &clist->commands[0];
        Built_In_Cmds bic = match_command(cmd->argv[0]);
        if (bic != BI_NOT_BI) {
            if (bic == BI_CMD_EXIT) {
                return OK_EXIT;
            }
            exec_built_in_cmd(cmd);
            return OK;
        }
    }

    // Create pipe connections
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    // Create child processes
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            return ERR_EXEC_CMD;
        }

        // Child process
        if (pids[i] == 0) {  
            // Connect input from previous command
            if (i > 0) {
                dup2(pipefds[(i-1)*2], STDIN_FILENO);
            }

            // Connect output to next command
            if (i < num_commands - 1) {
                dup2(pipefds[i*2 + 1], STDOUT_FILENO);
            }

            // Close all pipe ends in child
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }

            // Prepare and execute command
            cmd_buff_t *cmd = &clist->commands[i];
            cmd->argv[cmd->argc] = NULL;
            execvp(cmd->argv[0], cmd->argv);
            perror("execvp");
            exit(ERR_EXEC_CMD);
        }
    }

    // Close all pipes in parent process
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    // Wait for all child processes
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return OK;
}

void trim_spaces(char *str) {
    if (!str) {
        return;
    }

    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}

// Free memory for a single command buffer
int free_cmd_buff(cmd_buff_t *cmd) {
    if (cmd->_cmd_buffer) {
        free(cmd->_cmd_buffer);
        cmd->_cmd_buffer = NULL;
    }
    return OK;
}

// Free all command buffers in a command list
int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    return OK;
}

// Allocate memory for command buffer
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) {
        perror("malloc");
        return ERR_MEMORY;
    }
    return OK;
}