1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**: fgets() is a good choice for this application as fgets() reads input until either a newline or the EOF is encountered which is good for line based inputs in a shell environment. It also ensures that the input is properly terminated with a \0 and that is important when working with C strings.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**: Using malloc() for cmd_buff allows for dynamic memory allocation which allows the program to be flexible when handling different sized input and improves memory usage compared to a fixed size array. It also prepares the program for cases where the input might exceed the set buffer size making it more efficient.

3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: It is necessary because it ensures that the commands are correctly parsed and executed. If spaces are not removed beforehand issues may arise when executing commands in the shell like misinterpreting a argument.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**: ls > output.txt, ls non_existent_dir 2> error.txt, ls non_existent_dir &> output.txt. Some challenges that we might face when implementing them is that we have to make sure that the shell opens the file correctly as if the file doesn't exists it should be created but if it does it shouldn't be overwritten. Another is error handling as we want to redirect error messages and leave normal output alone so they need to be managed separately. For the last one we need to make sure that both types of output go into the file in the right order.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**: Redirection is about changing where data goes while piping is more about connecting commands together. So redirection controls where input/output go and piping lets commands work together by passing data directly.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**: Separating STDOUT and STDERR allows for clearer error handling as it gives you more flexibility and control over how data is managed and displayed which can help you identify and fix issues in your code.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**: Our shell should display both STDOUT and STDERR and if a command fails it should produce an error message in STDERR which clearly displayed what went wrong. When there is a case where a command outputs both STDOUT and STDERR we should output it separately so that the user can clearly see between regular output and the error messgge. We can also merge the two outputs as well if the user wants both output and errors together.