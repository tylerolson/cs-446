#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct _thread_data_t {
    const int *data;
    int startInd;
    int endInd;
    pthread_mutex_t *lock;
    long long int *totalSum;
} thread_data_t;

int readFile(const char *filename, int **data);
void *arraySum(void *arg);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <filename> <num_threads>\n", argv[0]);
        return -1;
    }

    const char *filename = argv[1];
    int num_threads = atoi(argv[2]);
    int *data = NULL;
    int num_values = readFile(filename, &data);

    if (num_values == -1) {
        return -1;
    }

    if (num_threads > num_values) {
        printf("Too many threads requested\n");
        free(data);
        return -1;
    }

    long long int totalSum = 0;
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    thread_data_t *thread_data = malloc(num_threads * sizeof(thread_data_t));
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    int chunk_size = num_values / num_threads;
    int remainder = num_values % num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].data = data;
        thread_data[i].startInd =
            i * chunk_size + (i < remainder ? i : remainder);
        thread_data[i].endInd =
            (i + 1) * chunk_size + (i < remainder ? i + 1 : remainder);
        thread_data[i].lock = &lock;
        thread_data[i].totalSum = &totalSum;

        pthread_create(&threads[i], NULL, arraySum, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);

    long long milliseconds = (end.tv_sec - start.tv_sec) * 1000LL +
                             (end.tv_usec - start.tv_usec) / 1000LL;

    printf("Final sum: %lld\n", totalSum);
    printf("Total execution time: %lld ms\n", milliseconds);

    free(data);
    free(thread_data);
    free(threads);
    pthread_mutex_destroy(&lock);

    return 0;
}

int readFile(const char *filename, int **data) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("File not found...\n");
        return -1;
    }

    int capacity = 1000;
    int count = 0;
    *data = malloc(capacity * sizeof(int));

    int value;
    while (fscanf(file, "%d", &value) == 1) {
        if (count >= capacity) {
            capacity *= 2;
            *data = realloc(*data, capacity * sizeof(int));
        }
        (*data)[count++] = value;
    }

    fclose(file);
    return count;
}

void *arraySum(void *arg) {
    thread_data_t *thread_data = (thread_data_t *)arg;
    long long int threadSum = 0;

    for (int i = thread_data->startInd; i < thread_data->endInd; i++) {
        threadSum += thread_data->data[i];
    }

    pthread_mutex_lock(thread_data->lock);
    *(thread_data->totalSum) += threadSum;
    pthread_mutex_unlock(thread_data->lock);

    return NULL;
}