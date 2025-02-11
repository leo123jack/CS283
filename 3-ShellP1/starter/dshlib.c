#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */

// Prototype 
void trim_spaces(char *str);

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    clist->num = 0;

    // Tokenize the command line
    char *cmd_token = strtok(cmd_line, PIPE_STRING);
    while (cmd_token != NULL) {

        // Check if we exceeded the maximum number of commands
        if (clist->num >= CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Trim leading and trailing spaces
        trim_spaces(cmd_token);

        // Split the command
        char *exe = cmd_token;
        char *args = strchr(cmd_token, SPACE_CHAR);

        if (args != NULL) {
            // Split the executable and arguments
            *args = '\0';
            args++;
            trim_spaces(args);
        }

        // Check if the executable is too long
        if (strlen(exe) >= EXE_MAX || (args != NULL && strlen(args) >= ARG_MAX)) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Populate the command structure
        strncpy(clist->commands[clist->num].exe, exe, EXE_MAX);
        if (args != NULL) {
            strncpy(clist->commands[clist->num].args, args, ARG_MAX);
        } else {
            clist->commands[clist->num].args[0] = '\0';
        }

        // Increment the command count
        clist->num++;

        // Get the next command
        cmd_token = strtok(NULL, PIPE_STRING);
    }

    // No commands were provided
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }

    return OK;
}

// Helper function
void trim_spaces(char *str) {
    // Trim leading spaces
    char *start = str;
    while (isspace(*start)) {
        start++;
    }
    memmove(str, start, strlen(start) + 1);

    // Trim trailing spaces
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end--;
    }
    *(end + 1) = '\0';
}