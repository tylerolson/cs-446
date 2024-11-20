#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define ANSI_COLOR_GRAY "\x1b[30m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_WHITE "\x1b[37m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define TERM_CLEAR() printf("\033[H\033[J")
#define TERM_GOTOXY(x, y) printf("\033[%d;%dH", (y), (x))

typedef struct _thread_data_t {
    int localTid;
    const int* data;
    int numVals;
    pthread_mutex_t* lock;
    long long int* totalSum;
} thread_data_t;

void print_progress(int localTid, size_t value) {
    pid_t tid = syscall(SYS_gettid);
    TERM_GOTOXY(0, localTid + 1);

    char prefix[256];
    size_t bound = 100;
    sprintf(prefix, "%d: %ld (ns) \t[", tid, value);
    const char suffix[] = "]";
    const size_t prefix_length = strlen(prefix);
    const size_t suffix_length = sizeof(suffix) - 1;
    char* buffer = calloc(bound + prefix_length + suffix_length + 1, 1);
    size_t i = 0;

    strcpy(buffer, prefix);
    for (; i < bound; ++i) {
        buffer[prefix_length + i] = i < value / 10000 ? '#' : ' ';
    }
    strcpy(&buffer[prefix_length + i], suffix);

    if (!(localTid % 7))
        printf(ANSI_COLOR_WHITE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);
    else if (!(localTid % 6))
        printf(ANSI_COLOR_BLUE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);
    else if (!(localTid % 5))
        printf(ANSI_COLOR_RED "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);
    else if (!(localTid % 4))
        printf(ANSI_COLOR_GREEN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);
    else if (!(localTid % 3))
        printf(ANSI_COLOR_CYAN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);
    else if (!(localTid % 2))
        printf(ANSI_COLOR_YELLOW "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);
    else if (!(localTid % 1))
        printf(ANSI_COLOR_MAGENTA "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);
    else
        printf("\b%c[2K\r%s\n", 27, buffer);

    fflush(stdout);
    free(buffer);
}

void* arraySum(void* arg) {
    thread_data_t* thread_data = (thread_data_t*)arg;

    while (1) {
        struct timespec start, end;
        long latency_max = 0;

        long long int threadSum = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < thread_data->numVals; i++) {
            threadSum += thread_data->data[i];
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        long latency = (end.tv_sec - start.tv_sec) * 1000000000 +
                       (end.tv_nsec - start.tv_nsec);

        if (latency > latency_max) {
            latency_max = latency;
        }

        pthread_mutex_lock(thread_data->lock);
        *(thread_data->totalSum) += threadSum;
        pthread_mutex_unlock(thread_data->lock);

        print_progress(thread_data->localTid, latency_max);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return -1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Number of threads must be positive\n");
        return -1;
    }

    int* data = (int*)malloc(2000000 * sizeof(int));
    if (!data) {
        printf("Failed to allocate memory for data array\n");
        return -1;
    }

    long long int totalSum = 0;
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Failed to initialize mutex\n");
        free(data);
        return -1;
    }

    TERM_CLEAR();

    thread_data_t* thread_data =
        (thread_data_t*)malloc(num_threads * sizeof(thread_data_t));
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

    if (!thread_data || !threads) {
        printf("Failed to allocate memory for thread structures\n");
        free(data);
        free(thread_data);
        free(threads);
        pthread_mutex_destroy(&lock);
        return -1;
    }

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].localTid = i;
        thread_data[i].data = data;
        thread_data[i].numVals = 2000000;
        thread_data[i].lock = &lock;
        thread_data[i].totalSum = &totalSum;

        if (pthread_create(&threads[i], NULL, arraySum, &thread_data[i]) != 0) {
            printf("Failed to create thread %d\n", i);
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            free(data);
            free(thread_data);
            free(threads);
            pthread_mutex_destroy(&lock);
            return -1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(data);
    free(thread_data);
    free(threads);
    pthread_mutex_destroy(&lock);

    return 0;
}