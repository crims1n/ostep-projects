#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "util.h"
#include "zip.h"

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }

    char old_char = '\0';
    int count = 0;

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            fprintf(stderr, "Error opening file\n");
            return 1;
        }
        int fd = fileno(fp);
        struct stat sb;
        fstat(fd, &sb);

        if (sb.st_size > 4096) {
            thread_zip(argv[i], &old_char, &count);
        } else {
            simple_zip(fp, &old_char, &count);
        }

        fclose(fp);
    }
    zip_print(count, old_char);

    return 0;
}
