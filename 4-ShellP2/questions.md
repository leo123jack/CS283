1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: We use fork() followed by execvp() instead of just calling execvp() directly because fork() allows us to create a new child process while execvp() replaces the current process image with a new one. fork() provides the shell with the ability to run external commands in separate processes while maintaining control of the shell process itself.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: If the fork() system call fails it returns a negative value (-1). This failure occurs when the system cannot allocate resources for a new process. In my implementation the failure of fork() is handled by checking if the return value is less than 0.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: The execvp() function is used to execute external commands in Unix like systems. It looks for the command to execute by searching through a list of directories that are specified in the $PATH environment variable. When execvp() is called it receives the name of the command to execute as its first argument and then a list of arguments as its second argument.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**: The wait() system call is used by the parent process to wait for the termination of a child process after it has been forked. Without calling wait() the child process remains which can cause resource leaks and system instability.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: It is a macro used to retrieve the exit status of a child process in the parent process after calling wait() or waitpid(). It extracts the actual exit code returned by the child process which is a value passed to the exit() function or returned from main() by the child.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**: Quoted arguments are handled by parsing through the command string and detecting when quoted sections begin and end. This is necessary as it ensures that arguments containing spaces are treated as a single argument rather than being split into multiple tokens.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: It introduced logic to handle quoted arguments and some challenges were refactoring tokenization logic to handle complexity without breaking previous behavior.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: Signals are a form of communication used to notify a process about certain events or actions. A signal is essentially an interrupt or notification that is sent to a process to inform it about some specific event such as an error, user input, or a request for the process to terminate.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**: The SIGKILL signal is a termination signal that immediately terminates the target process. It cannot be caught, blocked, or ignored by the process. This signal is typically used when other methods of terminating a process fail or if you need to forcibly kill a process that is unresponsive. SIGTERM signal is a termination signal that requests a process to terminate but it allows the process to clean up resources and handle the termination gracefully before exiting. SIGINT signal is typically generated when the user presses Ctrl+C in the terminal. 

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: When a process receives SIGSTOP it is paused and cannot continue executing until it is resumed since it is controlled by the kernel. 
