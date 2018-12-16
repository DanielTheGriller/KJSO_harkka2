// Käyttöjärjestelmät ja Systeemiohjelmointi 2018
// Daniel Linna 0509355
// Harjoitystyö Project 2: Unix Shell
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAXNUM 63

void errormsg (int i){
	char error_message[64];
	switch (i) {
		case 1:
			strcpy(error_message, "Usage: cd [DIRECTORY] NOTE: Only one argument accepted\n");
			break;
		case 2:
			strcpy(error_message, "Command not found\n");
			break;	
		case 3:
			strcpy(error_message, "Directory not found\n");
			break;		
		case 4:
			strcpy(error_message, "Cannot open file\n");
			break;		
		case 5:
			strcpy(error_message, "Invalid input\n");
			break;	
		case 6:
			strcpy(error_message, "Too many files\n");
			break;	
		default:
			strcpy(error_message, "An error occured\n");
			break;
	}
	write(STDERR_FILENO, error_message, strlen(error_message));
}

int main (int argc, char * argv[]){
	char * command, * line, path[32], altpath[36], * arguments[MAXNUM];
	size_t maxlen = 1024;
	int i, pid , mode = 0;
	FILE *input;
	// If no arguments passed, read input from stdin
	if (argc == 1){
		input = stdin;
	} else if (argc == 2) {
	// If arguments, read from passed file
		input = fopen(argv[1], "r");
		// Set mode to 1 which is reading from file
		mode = 1;
		// Error and exit if cannot open file
		if (input == NULL){
			errormsg(4);
			exit(1);
		}
	} else {
		errormsg(6);
		exit(1);
	}
	
	// Allocate memory for line
	line = (char *)malloc(maxlen * sizeof(char));
	// Error if memory is not allocated
	if (line == NULL){
		errormsg(0);
		exit(1);
	}
	// Actual loop for the shell
	while (1){
		// Prompt if not reading input from file
		if (mode == 0){
			printf("wish> ");
		}
		// Exit when reaching EOF
		if (getline(&line, &maxlen,input) == EOF){
			exit(0);
		}
		// Start from beginning if no input
		if (strlen(line) <= 1){
			errormsg(5);
			continue;
		}
		// copy /bin/ to path variable for execv
		strcpy(path, "/bin/");
		// copy /use to altpath variable, in case cannot find command from /bin/
		strcpy(altpath, "/usr");

		// Set the last character of line to null
		line[strlen(line)-1] = '\0';

		// Configure variables for next task
		i = 0;
		command = line;

		// Separate different arguments to arguments[] with strtok
		while ((arguments[i] = strtok(command, " ")) != NULL) {

			//printf("Argument %d: %s\n", i, arguments[i]);
			i++;
			command = NULL;
		}

		// If command is exit, stop the shell
		if(strcmp(arguments[0], "exit") == 0){
			exit(0);
		}

		// If command is cd, run chdir after argument check
		if (strcmp(arguments[0], "cd") == 0){

		// If no arguments passed, display error
			if (arguments[1] == NULL){
				errormsg(1);
				continue;

		// If more than one argument, display error
			} else if (arguments[2] != NULL){
				errormsg(1);
				continue;
			}
			if (chdir(arguments[1]) == -1){
				errormsg(3);
			}
			continue;
		}

		// Get input command to path for execv
		strcat(path, arguments[0]);

		// run the command
		switch (pid = fork()) {
			case -1:
				// In case of error
				errormsg(2);
				continue;
			case 0:
				// Child process
				// Check if have access to /bin/command
				if (access(path, X_OK) == 0){
					execv(path, arguments);
					// Print error if process returns from execv
					errormsg(2);
					continue;
				} else {
					// If no access from 1st one, try with /usr/bin/command
					if (access(altpath, X_OK) == 0){
						strcat(altpath, path);
						execv(altpath, arguments);
						// Print error if process returns from execv
						errormsg(2);
						continue;
					}
					// If still no access, print error
					errormsg(2);
					continue;
				}

			default:
				// Parent process, wait for child to complete
				waitpid(pid, NULL, 0);

				break;
		}

	}

	exit(0);
}
