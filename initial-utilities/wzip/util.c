#include <stdio.h>
void zip_print(int count, char ch) {
    fwrite(&count, sizeof(int), 1, stdout);

    fwrite(&ch, sizeof(char), 1, stdout);
}