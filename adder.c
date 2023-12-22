#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#define STARTUP_MESSAGE "=-=-=-= Adder Program =-=-=-=\n"

char *g_fifo = "/tmp/adder";
char *g_prog_name = "adder";

void sendInt(int output);
int get1UserInput(const char* prompt);
void setup();
void handleError(const char *prompt1, const char *prompt2);
void clearFifo();

int main(int argc, char *argv[]){
    
    printf(STARTUP_MESSAGE);

    setup();

    // main loop
    while (1){
        // gets input from user
        int input1 = get1UserInput("Input 1: ");
        sendInt(input1);

        int input2 = get1UserInput("Input 2: ");
        sendInt(input2);

        // performs the operation
        int result = input1 + input2;
        printf("Result = %d\n\n", result);

        // writes result to FIFO
        sendInt(result);
    }
    exit(EXIT_SUCCESS);
}

void setup(){

    // return function if FIFO file exists
    if(access(g_fifo, F_OK) != -1){
        return;
    }

    // create FIFO if it doesn't exists
    if(mkfifo(g_fifo, 0666) == -1){

        // error handling
        handleError("mkfifo in ", g_prog_name);
    }
}

// writes output to pipe
void sendInt(int output){
    // open fifo
    int fd;
    fd = open(g_fifo, O_WRONLY);
    if(fd == -1){
        
        // error handling
        perror("opening fifo in adder subprogram");
        exit(EXIT_FAILURE);
    }

    // write result into fifo    
    if(write(fd, &output, sizeof(int)) == -1){

        // error handling
        perror("writing to fifo in adder subprogram");
        exit(EXIT_FAILURE);
    }
    
    // close fifo
    close(fd);
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

void clearFifo(){
    char buffer[256];
    int fd = open(g_fifo, O_RDONLY | O_NONBLOCK);
    while(read(fd, buffer, sizeof(buffer)) > 0) {
        // continue reading until the pipe is empty
    }
    close(fd);
}

// prints error message with perror and combines 2 prompts to specify the location of the error, then exits the program
void handleError(const char *prompt1, const char *prompt2){
        char err_msg[50];
        sprintf(err_msg, "%s%s", prompt1, prompt2);
        perror(err_msg);
        exit(EXIT_FAILURE); // returns 1
}