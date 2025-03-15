
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"

int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);


    return rc;
}

int stop_server(int svr_socket){
    return close(svr_socket);
}

int boot_server(char *ifaces, int port){
    int svr_socket;
    int ret;
    struct sockaddr_in addr;
    int enable = 1;

    // Create the server socket
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket == -1) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    // Set socket options to reuse address
    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    // Configure server address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (ifaces) {
        if (inet_pton(AF_INET, ifaces, &addr.sin_addr) <= 0) {
            perror("inet_pton");
            close(svr_socket);
            return ERR_RDSH_COMMUNICATION;
        }
    } else {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    // Bind the socket
    ret = bind(svr_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    // Listen for incoming connections
    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}


int process_cli_requests(int svr_socket){
    int cli_socket;
    int rc = OK;    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        // Accept a client connection
        cli_socket = accept(svr_socket, (struct sockaddr *)&client_addr, &client_len);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        // Handle client requests
        rc = exec_client_requests(cli_socket);

        // Close the client socket
        close(cli_socket);

        if (rc < 0) {
            break;
        }
    }

    return rc;
}

int exec_client_requests(int cli_socket) {
    int io_size;
    command_list_t cmd_list;
    int rc;
    int cmd_rc;
    char *io_buff;
    extern int build_cmd_list(char *cmd_line, command_list_t *clist);

    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL){
        return ERR_RDSH_SERVER;
    }

    while (1) {
        // Receive data from client
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);
        if (io_size <= 0) {
            perror("recv");
            free(io_buff);
            return ERR_RDSH_COMMUNICATION;
        }

        // Null-terminate the received data
        io_buff[io_size] = '\0';

        // Check for exit commands
        if (strcmp(io_buff, "exit") == 0) {
            free(io_buff);
            return OK;
        } else if (strcmp(io_buff, "stop-server") == 0) {
            free(io_buff);
            return OK_EXIT;
        }

        // Build command list
        rc = build_cmd_list(io_buff, &cmd_list);
        if (rc != OK) {
            send_message_string(cli_socket, "Error: Failed to parse command.\n");
            send_message_eof(cli_socket);
            continue;
        }

        // Handle CD
        if (cmd_list.num == 1 && strcmp(cmd_list.commands[0].argv[0], "cd") == 0) {
            if (cmd_list.commands[0].argc < 2) {
                // No directory provided
                const char *home = getenv("HOME");
                if (home) {
                    if (chdir(home) != 0) {
                        send_message_string(cli_socket, "cd: Failed to change directory.\n");
                    }
                } else {
                    send_message_string(cli_socket, "cd: HOME environment variable not set.\n");
                }
            } else {
                // Change to the specified directory
                if (chdir(cmd_list.commands[0].argv[1]) != 0) {
                    send_message_string(cli_socket, "cd: No such file or directory.\n");
                }
            }
            send_message_eof(cli_socket);
            continue;
        }

        // Execute command pipeline
        cmd_rc = rsh_execute_pipeline(cli_socket, &cmd_list);

        if (cmd_rc < 0) {
            send_message_string(cli_socket, "Error: Command execution failed.\n");
        }

        send_message_eof(cli_socket);
    }

    free(io_buff);
    return OK;
}


int send_message_eof(int cli_socket){
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len){
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}


int send_message_string(int cli_socket, char *buff){
    int bytes_sent;

    // Send message
    bytes_sent = send(cli_socket, buff, strlen(buff), 0);
    if (bytes_sent == -1) {
        perror("send");
        return ERR_RDSH_COMMUNICATION;
    }

    // Send EOF
    if (send_message_eof(cli_socket) != OK) {
        return ERR_RDSH_COMMUNICATION;
    }

    return OK;
}


int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int pipes[clist->num - 1][2];
    pid_t pids[clist->num];
    int pids_st[clist->num];
    int exit_code;

    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_RDSH_COMMUNICATION;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return ERR_RDSH_COMMUNICATION;
        }
        
        // Child process
        if (pids[i] == 0) { 
            if (i == 0) {
                // Read from cli_sock
                dup2(cli_sock, STDIN_FILENO);
            } else {
                // Read from previous pipe
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            if (i == clist->num - 1) {
                // Write output to cli_sock
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                // Write output to next pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipes
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Try accessing the command list
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], &pids_st[i], 0);
    }

    // Get exit code
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);

    // Check if any process returned EXIT_SC
    for (int i = 0; i < clist->num; i++) {
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC) {
            exit_code = EXIT_SC;
        }
    }

    return exit_code;
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype)
    {
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}
