1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

> **Answer**: To ensure that all child processes complete before the shell continues accepting user input we used the waitpid() system call in the parent process. If the shell forgets to call waitpid() on all child processes then it could lead to zombie processes, resource leaks, and incorrect shell behavior.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

> **Answer**: After calling dup2() it is necessary to close unused pipe ends because it makes resource management more efficient, it prevents the pipe buffer from becoming full, and it ensures that each command in the pipeline get the correct input.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

> **Answer**: cd is a built-in command because it modifies the shell working directory directly and built-in commands are more efficient. Some challenges that would arise if it was implemented as a external process would be the directory not changing when cd is executed.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

> **Answer**: We could replace the fixed size arrays with dynamically allocated arrays or linked lists to increase memory usage, but it will cause higher code complexity and would require more error handling.