#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "util.h"
void *worker();

typedef struct __arg {
    // pass to
    size_t start;
    size_t stop;
    FILE *fp;

    // misc data that we may need?
    int count;
    char old_char;

    // get back
    int arr_size;
    int *num_arr;
    char *char_arr;

} arg_t;

void simple_zip(FILE *fp, char *old_char, int *count) {
    char current_char;
    while ((current_char = fgetc(fp)) != EOF) {
        if (*old_char == '\0') {
            *old_char = current_char;
            (*count)++;
        } else if (current_char != *old_char) {
            zip_print(*count, *old_char);
            *count = 1;
            *old_char = current_char;
        } else {
            (*count)++;
        }
    }
}

arg_t *arg_init(size_t start_val, size_t stop_val, FILE *fp) {
    arg_t *args = malloc(sizeof(arg_t));
    args->start = start_val;
    args->stop = stop_val;
    args->fp = fp;
    args->count = 0;
    args->old_char = '\0';
    args->arr_size = 0;
    return args;
}

void thread_zip(const char *filename, char *old_char, int *count) {
    pthread_t t[3];
    FILE *fp = fopen(filename, "r");
    int fd = fileno(fp);
    struct stat sb;
    fstat(fd, &sb);
    size_t fsize = sb.st_size;
    // char *src = mmap(0, fsize, PROT_READ, MAP_PRIVATE, fd, 0);

    int step = fsize / 3;

    arg_t *one_args = arg_init(0, step, fp);
    arg_t *two_args = arg_init(step, step * 2, fp);
    arg_t *three_args = arg_init(step * 2, fsize, fp);

    pthread_create(&t[0], NULL, worker, (void *)one_args);
    pthread_create(&t[1], NULL, worker, (void *)two_args);
    pthread_create(&t[2], NULL, worker, (void *)three_args);

    for (size_t i = 0; i < 3; i++) {
        pthread_join(t[i], NULL);
    }

    if (one_args->char_arr[0] == *old_char) {
        one_args->num_arr[0] += *count;
    } else if (*old_char != '\0') {
        zip_print(*count, *old_char);
    }

    if (one_args->char_arr[(one_args->arr_size) - 1] == two_args->char_arr[0]) {
        one_args->char_arr[(one_args->arr_size) - 1] = '\0';
        two_args->num_arr[0] += one_args->num_arr[(one_args->arr_size) - 1];
    }

    if (two_args->char_arr[(two_args->arr_size) - 1] == three_args->char_arr[0]) {
        two_args->char_arr[(two_args->arr_size) - 1] = '\0';
        three_args->num_arr[0] += two_args->num_arr[(two_args->arr_size) - 1];
    }

    *old_char = three_args->char_arr[(three_args->arr_size) - 1];
    *count = three_args->num_arr[(three_args->arr_size) - 1];
    three_args->char_arr[(three_args->arr_size) - 1] = '\0';

    for (size_t i = 0; i < one_args->arr_size; i++) {
        if (one_args->char_arr[i] != '\0') {
            fwrite(&one_args->num_arr[i], sizeof(int), 1, stdout);
            printf("%c", one_args->char_arr[i]);
        }
    }
    for (size_t i = 0; i < two_args->arr_size; i++) {
        if (two_args->char_arr[i] != '\0') {
            fwrite(&two_args->num_arr[i], sizeof(int), 1, stdout);
            printf("%c", two_args->char_arr[i]);
        }
    }
    for (size_t i = 0; i < (three_args->arr_size) - 1; i++) {
        if (three_args->char_arr[i] != '\0') {
            fwrite(&three_args->num_arr[i], sizeof(int), 1, stdout);
            printf("%c", three_args->char_arr[i]);
        }
    }
    fclose(fp);
    free(one_args->char_arr);
    free(one_args->num_arr);
    free(one_args);
    free(two_args->char_arr);
    free(two_args->num_arr);
    free(two_args);
    free(three_args->char_arr);
    free(three_args->num_arr);
    free(three_args);
}

void *worker(void *ptr) {
    arg_t *args = (arg_t *)ptr;
    struct stat sb;
    int fd = fileno(args->fp);
    fstat(fd, &sb);
    size_t fsize = sb.st_size;
    char *src = mmap(0, fsize, PROT_READ, MAP_PRIVATE, fd, 0);

    size_t mem_size = 4096;
    char *tmp_char_arr = malloc(sizeof(char) * mem_size);
    int *tmp_num_arr = malloc(sizeof(int) * mem_size);

    char tmp_old_char = args->old_char;
    int tmp_count = args->count;
    int tmp_arr_size = args->arr_size;

    for (size_t i = args->start; i < args->stop; i++) {
        char current_char = src[i];
        if (tmp_old_char == '\0') {
            tmp_old_char = current_char;
            tmp_count++;
            tmp_arr_size++;
        } else if (current_char != tmp_old_char) {
            if (tmp_arr_size > mem_size) {
                mem_size *= 2;
                tmp_char_arr = realloc(tmp_char_arr, sizeof(char) * mem_size);
                tmp_num_arr = realloc(tmp_num_arr, sizeof(int) * mem_size);
            }
            tmp_char_arr[(tmp_arr_size)-1] = tmp_old_char;
            tmp_num_arr[(tmp_arr_size)-1] = tmp_count;
            tmp_arr_size++;
            tmp_count = 1;
            tmp_old_char = current_char;
        } else {
            tmp_count++;
        }
    }
    // args->char_arr = realloc(args->char_arr, (sizeof(char) * (args->arr_size)));
    // args->num_arr = realloc(args->num_arr, (sizeof(size_t) * (args->arr_size)));
    tmp_char_arr[(tmp_arr_size)-1] = tmp_old_char;
    tmp_num_arr[(tmp_arr_size)-1] = tmp_count;

    args->char_arr = tmp_char_arr;
    args->num_arr = tmp_num_arr;
    args->count = tmp_count;
    args->old_char = tmp_old_char;
    args->arr_size = tmp_arr_size;
}