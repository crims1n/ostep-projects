#ifndef ZIP_H
#define ZIP_H

void simple_zip(FILE *fp, char *old_char, int *count);
void thread_zip(FILE *fp, char *old_char, int *count);

#endif