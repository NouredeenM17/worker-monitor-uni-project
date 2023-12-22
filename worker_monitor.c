#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#define NO_OF_WORKERS 4
#define STARTUP_MESSAGE "=-=-=-= Worker Monitor =-=-=-=\n"

char *g_worker_fifos[] = {"/tmp/adder","/tmp/subtractor","/tmp/multiplier","/tmp/divider"};
char *g_worker_names[] = {"adder", "subtractor", "multiplier", "divider"};
char *g_worker_operation_signs[] = {"+","-","*","/"};
pthread_t g_thread_ids[NO_OF_WORKERS];
pthread_mutex_t g_lock;

void createThread(int worker_no);
void joinThread(int worker_no);
void *monitorWorker(void *worker_no_ptr);
int readFromPipe(const char *prog_name, const char *fifo);
//void ensureNoDivisionByZero(int *user_input, int operation_num);
void handleError(const char *prompt1, const char *prompt2);
void initPipe(int operation_num);
void clearFifo(const char *fifo);


// main function
int main(){

    printf(STARTUP_MESSAGE);
    printf("Waiting for workers...\n");

    pthread_mutex_init(&g_lock, NULL);

    for (int i = 0; i < 2; i++){
        initPipe(i);
        createThread(i);
    }

    for (int i = 0; i < 2; i++){
        joinThread(i);
    }

    pthread_mutex_destroy(&g_lock);

    
    return 0;
}

void createThread(int worker_no){
    pthread_create(&g_thread_ids[worker_no], NULL, monitorWorker, &worker_no);
}

void joinThread(int worker_no){
    pthread_join(g_thread_ids[worker_no], NULL);
}

void *monitorWorker(void *worker_no_ptr){
    int input1, input2, result;

    int *worker_no_int_ptr = (int*)worker_no_ptr;
    int worker_no = *worker_no_int_ptr;
    

    while (1){

        input1 = readFromPipe(g_worker_names[worker_no], g_worker_fifos[worker_no]);
        printf("%s:\n%d\n%s\n", g_worker_names[worker_no], input1, g_worker_operation_signs[worker_no]);

        //pthread_mutex_lock(&g_lock);

        input2 = readFromPipe(g_worker_names[worker_no], g_worker_fifos[worker_no]);
        result = readFromPipe(g_worker_names[worker_no], g_worker_fifos[worker_no]);

        printf("%d\n=\n%d\n", input2, result);
        printf("Waiting for workers...\n");

        //pthread_mutex_unlock(&g_lock);
    }
    
}

// initializes the FIFO of specified operation number
void initPipe(int operation_num){

    // return function if FIFO file exists
    if(access(g_worker_fifos[operation_num], F_OK) != -1){
        return;
    }

    // create FIFO if it doesn't exists
    if(mkfifo(g_worker_fifos[operation_num], 0666) == -1){

        // error handling
        handleError("mkfifo in ", g_worker_names[operation_num]);
    }
}

// // ensures the second user input is not zero
// void ensureNoDivisionByZero(int *user_input, int operation_num){
//     // checks for division by zero
//     while(1){
//         if(operation_num == 3 && user_input[1] == 0){
//             printf("You cannot divide by zero! Please enter a non-zero value.\n\n");
//             user_input[1] = get1UserInput("Input 2: ");
//         } else {
//             break;
//         }
//     }
// }

// reads from specified pipe
int readFromPipe(const char *prog_name, const char *fifo){
    int fd;
    int buffer;

    // open fifo for reading
    fd = open(fifo, O_RDONLY);
    if(fd == -1){

        // error handling
        handleError("opening fifo in ", prog_name);
    }

    // read from fifo
    if(read(fd, &buffer, sizeof(int)) == -1){

        // error handling
        handleError("reading fifo in ", prog_name);
    }
    
    // close fifo
    close(fd);

    return buffer;
}

void clearFifo(const char *fifo){
    char buffer[256];
    int fd = open(fifo, O_RDONLY | O_NONBLOCK);
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




