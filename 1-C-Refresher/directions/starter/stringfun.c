#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);

//add additional prototypes here
void reverse_string(char *, int);
void print_words(char *, int);
void replace_words(char *, int, char *, char *);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    int i = 0;
    int j = 0;
    int space = 0;

    // Skip leading spaces
    while (*(user_str + i) == ' ' || *(user_str + i) == '\t' || *(user_str + i) == '\n') {
        i++;
    }

    // Process the string
    while (*(user_str + i) != '\0') {
        if (*(user_str + i) == ' ' || *(user_str + i) == '\t' || *(user_str + i) == '\n') {
            // When there is multiple spaces make it 1
            if (!space) {
                *(buff + j++) = ' ';
                space = 1;
            }
        } else {
            *(buff + j++) = *(user_str + i);
            space = 0;
        }
        i++;
    }

    // Remove trailing space
    if (j > 0 && (*(buff + j - 1) == ' ' || *(buff + j - 1) == '\t' || *(buff + j - 1) == '\n')) {
        j--;
    }
    
    // Null-terminate
    *(buff + j) = '\0';  

    if (j > len) {
        return -1;
    }

    return j;
}

void print_buff(char *buff, int len){
    printf("Buffer:  [");
    for (int i = 0; i < len; i++) {
        if (*(buff + i) == '\0') {
            putchar('.');
        } else {
            putchar(*(buff + i));
        }
    }

    putchar(']');
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT
    int wc = 0;
    int in_word = 0;

    // Checks if string is within buffer
    if (str_len > len) {
        exit(1);
    }

    // Count words
    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) != ' ' && *(buff + i) != '\t' && *(buff + i) != '\n') {
            if (!in_word) {
                wc++; 
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }
    
    return wc;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
void reverse_string(char *buff, int len) {
    int left = 0;
    int right = len - 1;

    // Swap characters
    while (left < right) {
        char temp = *(buff + left); 
        *(buff + left) = *(buff + right); 
        *(buff + right) = temp; 
        left++; 
        right--; 
    }
}

void print_words(char *buff, int len) {
    int i = 0;
    int word_start = 0;
    int word_length = 0;
    int word_count = 0;

    while (i < len && *(buff + i) != '\0') {
        // Count letters
        if (*(buff + i) != ' ') {  
            if (word_length == 0) {  
                word_start = i;
            }
            word_length++;
        } else if (word_length > 0) {  
            // Print details
            word_count++;
            printf("%d. ", word_count);
            fwrite(buff + word_start, sizeof(char), word_length, stdout);
            printf("(%d)\n", word_length);
            
            word_length = 0;
        }
        i++;
    }

    // Handles final word
    if (word_length > 0) {
        word_count++;
        printf("%d. ", word_count);
        fwrite(buff + word_start, sizeof(char), word_length, stdout);
        printf("(%d)\n", word_length);
    }
}

void replace_words(char *buff, int len, char *search, char *replace) {
    char *pos;
    int search_len = 0;
    int replace_len = 0;

    // Calculate the length of the search word
    while (*(search + search_len) != '\0') {
        search_len++;
    }

    // Calculate the length of the replace word
    while (*(replace + replace_len) != '\0') {
        replace_len++;
    }

    // Iterate over the buffer to find the first occurrence
    pos = buff;
    int found = 0;

    while (*pos != '\0') {
        // Check if the current part of the string matches the search word
        int i = 0;
        while (*(pos + i) == *(search + i) && *(search + i) != '\0') {
            i++;
        }

        // If found
        if (i == search_len) {
            found = 1;

            // Ensure the replacement fits within the buffer
            if (replace_len > len) {
                printf("Error: Replacement word is too long for the buffer.\n");
                exit(1);
            }

            // If the replacement word is shorter or same length
            if (replace_len <= search_len) {
                // Replace the word directly and adjust the rest of the string
                for (int i = 0; i < replace_len; i++) {
                    *(pos + i) = *(replace + i);
                }

                // If the replacement is shorter fill the remaining characters with '.'
                if (replace_len < search_len) {
                    for (int i = replace_len; i < search_len; i++) {
                        *(pos + i) = '.';
                    }
                }
            }
            
            // If the replacement word is longer than current word
            else if (replace_len > search_len) {
                // Shift all words after replacement
                char *move_pos = pos + search_len;
                while (*move_pos != '\0') {
                    move_pos++;
                }

                // Shift the characters to the right
                while (move_pos >= pos + search_len) {
                    *(move_pos + replace_len - search_len) = *move_pos;
                    move_pos--;
                }

                // Copy the replacement word into the position of the search word
                for (int i = 0; i < replace_len; i++) {
                    *(pos + i) = *(replace + i);
                }
            }
            break;
        }
        pos++;
    }

    // If the search word was not found exit
    if (!found) {
        printf("Error: Search word not found.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]){
    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    //
    //      This is safe because it checks that the first argument begins
    //      with the ' - ' character which denotes it as a flag. So if arv[1]
    //      doesn't exist *argv[1] != '-' will be true which will cause a exit(1).
    //
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    //
    //      This ensures that all the arguments are provided before
    //      running since it expects ' stringfun -[flag] "A string" '
    //      If it doesn't contain the correct amount it will exit(1)
    //
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *)malloc(BUFFER_SZ * sizeof(char));
    

    if (buff == NULL) {
        printf("Memory allocation failed");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        
        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_string(buff, user_str_len);
            break;
        case 'w':
            printf("Word Print\n----------\n");
            print_words(buff, user_str_len);
            printf("\n");

            rc = count_words(buff, BUFFER_SZ, user_str_len);
            printf("Number of words returned: %d\n", rc);

            break;
        case 'x':
            if (argc != 5) {
                printf("Error: Invalid number of arguments for -x\n");
                usage(argv[0]);
                exit(1);
            }

            char *search_word = argv[3];
            char *replace_word = argv[4];
            replace_words(buff, BUFFER_SZ, search_word, replace_word);
            break;
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    // Free buffer
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
//          It is good practice to prove both pointer and length because
//          it allows the function itself to do error handling. Also, if the
//          buffer would change in the future, the functions will still work
//          as intended, and modifications will not be necessary.
