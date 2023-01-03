#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#include <stdio.h> // for reading and writing from/to the command line
#include <stdlib.h> // for the malloc function
#include <string.h>
#include <errno.h> 
#include <sys/types.h> //pid_t type
#include <inttypes.h>
#include <iso646.h>


char* read_from_file(char* file) {
	FILE* fptr;
	char buffer;
	char* result = "";
	fptr = fopen(file, "r"); //open file in read mode

	if (fptr == NULL) {
		perror("Error opening file!\n");
		return NULL;
	}

	int r;
	while ((r = fread(fptr, &buffer, 1) > 0) //read content of file
		strcat(result, buffer);

	if (ferror(fptr)) { //check if error
		perror("Error reading from file!\n");
		return NULL;
	}
	fclose(fptr);
	return result;
}

int write_to_file(char* file, char* text) {
	char* fptr;
	fptr = fopen(file, "w"); //open file in write mode; if it does not exist, it will be created

	if (fptr == NULL) {
		perror("Error opening file!\n");
		return errno;
	}

	int w = fwrite(text, 1, sizeof(text), fptr); //write text to file
	if (w < 0) {
		perror("Error writing to file!\n");
		return errno;
	}
	fclose(fptr);
	return 0;
}

pid_t read_pid() {
	pid_t result;
	char* r = read_from_file(daemon_pid_path);

	if (r == NULL)
		return (pid_t)0;

	intmax_t xmax;
	char* tmp;

	//convert string to pid_t;
	errno = 0;
	xmax = strtoimax(r, &tmp, 10);
	if (errno != 0 || tmp == r || *tmp != '\0' || xmax != (pid_t)xmax) {
		perror("Bad PID!\n");
		return (pid_t)0;
	}
	result = (pid_t)xmax;

	return result;
}


#endif