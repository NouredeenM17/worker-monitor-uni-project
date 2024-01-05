#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>


#define NO_OF_WORKERS 4
#define STARTUP_MESSAGE "=-=-=-= Worker Monitor =-=-=-=\nPlease launch all workers before proceeding.\n"

char *g_worker_fifos[] = {"/tmp/adder","/tmp/subtractor","/tmp/multiplier","/tmp/divider"};
char *g_worker_names[] = {"adder", "subtractor", "multiplier", "divider"};
char *g_worker_operation_signs[] = {"+","-","*","/"};
pthread_t g_thread_ids[NO_OF_WORKERS];
int g_file_descriptors[NO_OF_WORKERS];
pthread_mutex_t g_lock;

void createThread(int worker_no);
void joinThread(int worker_no);
void *monitorWorker(void *worker_no_ptr);
int readFromPipe(int worker_no);
void handleError(const char *prompt1, const char *prompt2);
void initPipe(int worker_no);
void handle_sigint(int sig);


// main function
int main(){

    // temination signal handler
    signal(SIGINT, handle_sigint);

    printf(STARTUP_MESSAGE);

    pthread_mutex_init(&g_lock, NULL);

    for (int i = 0; i < NO_OF_WORKERS; i++){
        int worker_no = i;
        initPipe(worker_no);
    }

    for (int i = 0; i < NO_OF_WORKERS; i++){
        int worker_no = i;
        createThread(worker_no);
    }
    
    printf("Waiting for workers...\n");

    for(int i = 0; i < NO_OF_WORKERS; i++){
        int worker_no = i;
        joinThread(worker_no);
    }

    pthread_mutex_destroy(&g_lock);

    return 0;
}

void createThread(int worker_no){

    void *ptr = malloc(sizeof(int));
    *((int*)ptr) = worker_no;

    pthread_create(&g_thread_ids[worker_no], NULL, monitorWorker, ptr);
}

void joinThread(int worker_no){
    pthread_join(g_thread_ids[worker_no], NULL);
}

void *monitorWorker(void *worker_no_ptr){

    int *worker_no_int_ptr = (int*)worker_no_ptr;
    int worker_no = *worker_no_int_ptr;

    free(worker_no_ptr);

    char *worker_name = g_worker_names[worker_no];
    char *worker_fifo = g_worker_fifos[worker_no];
    char *worker_operation_sign = g_worker_operation_signs[worker_no];

    while (1){

        int input1;
        input1 = readFromPipe(worker_no);

        pthread_mutex_lock(&g_lock);

        printf("%s:\n%d\n%s\n", worker_name, input1, worker_operation_sign);
        fflush(stdout);

        int input2;
        input2 = readFromPipe(worker_no);
        printf("%d\n",input2);
        fflush(stdout);

        int result;
        result = readFromPipe(worker_no);
        printf("=\n%d\n",result);
        fflush(stdout);

        printf("Waiting for workers...\n");
        fflush(stdout);

        pthread_mutex_unlock(&g_lock);

    }
    
}

// initializes the FIFO of specified worker number
void initPipe(int worker_no){

    // delete FIFO if it exists
    if(access(g_worker_fifos[worker_no], F_OK) != -1){
        if (unlink(g_worker_fifos[worker_no]) == -1) {
            handleError("unlink in ", g_worker_names[worker_no]);
        }
    }

    // create FIFO
    if(mkfifo(g_worker_fifos[worker_no], 0666) == -1){

        // error handling
        handleError("mkfifo in ", g_worker_names[worker_no]);
    }

    // open fifo for reading
    int fd;
    fd = open(g_worker_fifos[worker_no], O_RDONLY);
    if(fd == -1){

        // error handling
        handleError("opening fifo in ", g_worker_names[worker_no]);
    }


    // store the file descriptor in the global variable
    g_file_descriptors[worker_no] = fd;
}

// reads from specified pipe
int readFromPipe(int worker_no){
    int fd = g_file_descriptors[worker_no];
    int buffer;

    // read from fifo    
    int read_return_val = read(fd, &buffer, sizeof(int));
    if(read_return_val == -1){

        // error handling
        handleError("reading fifo in ", g_worker_names[worker_no]);
    }

    return buffer;
}

// prints error message with perror and combines 2 prompts to specify the location of the error, then exits the program
void handleError(const char *prompt1, const char *prompt2){
        char err_msg[50];
        sprintf(err_msg, "%s%s", prompt1, prompt2);
        perror(err_msg);
        exit(EXIT_FAILURE); // returns 1
}

void handle_sigint(int sig) {
    printf("termination signal\n");
    fflush(stdout);

    for(int i = 0; i < NO_OF_WORKERS; i++){
        int worker_no = i;
        close(g_file_descriptors[worker_no]);
        unlink(g_worker_fifos[worker_no]);
    }

    pthread_mutex_destroy(&g_lock);
    exit(0);
}


