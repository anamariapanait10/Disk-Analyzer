#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

#define HELP "0"
#define ADD "1"
#define PRIORITY "2"
#define SUSPEND "3"
#define RESUME "4"
#define REMOVE "5"
#define INFO "6"
#define LIST "7"
#define PRINT "8"

int* ids = (int *) malloc(100 * sizeof(int));
for (int i = 0; i < 100; i++)
    ids[i] = i;

#endif