#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#define STARTUP_MESSAGE "=-=-=-= Divider Program =-=-=-=\n"

char *g_fifo = "/tmp/divider";
char *g_prog_name = "divider";
int g_fd;

void writeNumberToPipe(int output);
int get1UserInput(const char* prompt);
void checkMonitor();
void handleError(const char *prompt1, const char *prompt2);
int ensureNoDivisionByZero(int user_input);
void writeToPipe(int sent);

int main(int argc, char *argv[]){
    
    printf(STARTUP_MESSAGE);

    checkMonitor();

    // main loop
    while (1){
        // gets input from user
        int input1 = get1UserInput("Input 1: ");
        writeToPipe(input1);

        int input2 = get1UserInput("Input 2: ");
        input2 = ensureNoDivisionByZero(input2);

        // performs the operation
        int result = input1 / input2;
        printf("Result = %d\n\n", result);

        // writes result to FIFO
        writeToPipe(input2);
        writeToPipe(result);
    }
    exit(EXIT_SUCCESS);
}

void checkMonitor(){

    // if monitor didn't open the FIFO, the program terminates
    if(access(g_fifo, F_OK) == -1){
        printf("Please launch the worker monitor first!\n");
        exit(EXIT_SUCCESS);
    } else {
        // open fifo for writing
        g_fd = open(g_fifo, O_WRONLY);
        if(g_fd == -1){

            // error handling
            handleError("opening fifo in ", g_prog_name);
        }
    }
}

// writes output to pipe
void writeToPipe(int sent){

    // write result into fifo    
    int write_return_val = write(g_fd, &sent, sizeof(int));
    if(write_return_val == -1){

        // error handling
        perror("writing to fifo in divider subprogram");
        exit(EXIT_FAILURE);
    }
    printf("write return value for number write func: ");
    printf("%d\n\n", write_return_val);
    fflush(stdout);
}

// gets 1 user input with a prompt, and ensures it is an int
int get1UserInput(const char *prompt){
    int result;

    // gets valid input from user
    while(1){
        printf("%s", prompt);
        if(scanf("%d", &result) == 0){
            printf("Invalid input! Please enter an integer value.\n\n");
            
            // clear input buffer
            while(getchar() != '\n');
        } else {
            break;
        }
    }
    return result;
}


// ensures the second user input is not zero
int ensureNoDivisionByZero(int user_input){
    // checks for division by zero
    while(1){
        if(user_input == 0){
            printf("You cannot divide by zero! Please enter a non-zero value.\n\n");
            user_input = get1UserInput("Input 2: ");
        } else {
            break;
        }
    }
    return user_input;
}

// prints error message with perror and combines 2 prompts to specify the location of the error, then exits the program
void handleError(const char *prompt1, const char *prompt2){
        char err_msg[50];
        sprintf(err_msg, "%s%s", prompt1, prompt2);
        perror(err_msg);
        exit(EXIT_FAILURE); // returns 1
}
