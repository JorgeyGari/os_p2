//  MSH main file
// Write your msh source code here

/* CODE BY: Laura Belizón Merchán and Jorge Lázaro Ruiz
USAGE OF THIS CODE IS STRICTLY FOR REFERENCE ONLY, DO NOT COPY */

//#include "parser.h"
#include <stddef.h> /* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8

// Pipe descriptors
#define READ 0
#define WRITE 1

// Buffer size for mycp
#define BUFFER_SIZE 1024

// files in case of redirection
char filev[3][64];

// to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	// signal(SIGINT, siginthandler);
	exit(0);
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char ***argvv, int num_command)
{
	// reset first
	for (int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for (i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}

/**
 * Main sheell  Loop
 */
int main(int argc, char *argv[])
{
	/**** Do not delete this code.****/
	int end = 0;
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];

	if (!isatty(STDIN_FILENO))
	{
		cmd_line = (char *)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF)
		{
			if (strlen(cmd_line) <= 0)
				return 0;
			cmd_lines[end] = (char *)malloc(strlen(cmd_line) + 1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush(stdin);
			fflush(stdout);
		}
	}

	/*********************************/

	char ***argvv = NULL;
	int num_commands;

	// We need to declare some variables and environment variables before entering the while loop
	const char *var = "Acc";
	const char *val = "0";
	setenv(var, val, 1);
	char resultStr[1024]; // An auxiliary variable so we can use sprintf() later on

	while (1)
	{
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		// Prompt
		write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

		// Get command
		//********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
		executed_cmd_lines++;
		if (end != 0 && executed_cmd_lines < end)
		{
			command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
		}
		else if (end != 0 && executed_cmd_lines == end)
		{
			return 0;
		}
		else
		{
			command_counter = read_command(&argvv, filev, &in_background); // NORMAL MODE
		}
		//************************************************************************************************

		/************************ STUDENTS CODE ********************************/
		if (command_counter > 0)
		{
			if (command_counter > MAX_COMMANDS)
			{
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}
			else
			{
				/* Print command
				print_command(argvv, filev, in_background);
				printf("------------------------\n");
				*/

				pid_t pid;

				int p1[2];

				// Saving the original input, output and error file descriptors in order to restore them later
				int original_in = dup(STDIN_FILENO);
				int original_out = dup(STDOUT_FILENO);
				int original_err = dup(STDERR_FILENO);

				for (int i = 0; i < command_counter; i++)
				{
					/* MYCALC */
					if (strcmp(argvv[i][0], "mycalc") == 0)
					{
						if ((argvv[i][3] != NULL && argvv[i][4] == NULL))
						{ // This way we can check that the command received exactly 3 arguments

							// Using strcmp() and atoi() we check that all of the arguments are of the expected type
							if (((strcmp(argvv[i][1], "0") != 0 && atoi(argvv[i][1]) != 0) || (strcmp(argvv[i][1], "0") == 0 && atoi(argvv[i][1]) == 0)) && (strcmp(argvv[i][2], "0") != 0 && atoi(argvv[i][2]) == 0) && ((strcmp(argvv[i][3], "0") != 0 && atoi(argvv[i][3]) != 0) || (strcmp(argvv[i][3], "0") == 0 && atoi(argvv[i][3]) == 0)))
							{
								int result, quotient;

								// Cast the arguments as integers so we can operate with them
								int num1 = atoi(argvv[i][1]);
								int num2 = atoi(argvv[i][3]);

								if (strcmp(argvv[i][2], "add") == 0)
								{
									result = num1 + num2;
									char *acc = getenv(var);				 // Save the current value of Acc
									int accInt = atoi(acc);					 // Cast as an integer
									int resultInt = result + accInt;		 // Add the new result to the accumulated sum
									sprintf(resultStr, "%d", resultInt);	 // Cast the accumulated sum as a string
									int envCode = setenv(var, resultStr, 1); // Change the value of the environment variable Acc
									if (envCode < 0)						 // Check for errors when setting up the environment variable
									{
										perror("Environment variable could not be set");
									}
									fprintf(stderr, "[OK] %d + %d = %d; Acc %s\n", num1, num2, result, resultStr); // We print the success message in the standard error output
								}

								else if (strcmp(argvv[i][2], "mod") == 0)
								{
									if (num2 != 0)
									{
										quotient = num1 / num2;
										result = num1 % num2;
										fprintf(stderr, "[OK] %d %% %d = %d; Quotient %d\n", num1, num2, result, quotient); // We print the success message in the standard error output
									}
									else
									{
										fprintf(stderr, "[ERROR] Division by zero.\n"); // We print the error message in the standard error output
									}
								}

								else
								{																										   // If the operation is neither "add" nor "mod"
									fprintf(stderr, "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n"); // We print the error message in the standard errror output
								}
							}
							else
							{																										   // If one of the arguments is not of the expected type
								fprintf(stderr, "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n"); // We print the error message in the standard error output
							}
						}
						else
						{																										   // If it received an incorrect number of arguments
							fprintf(stderr, "[ERROR] The structure of the command is mycalc <operand_1> <add/mod> <operand_2>\n"); // We print the error message in the standard error output
						}
					}

					/* MYCP */
					else if (strcmp(argvv[i][0], "mycp") == 0)
					{
						if (argvv[i][2] != NULL && argvv[i][3] == NULL) // Check the command has received exactly 2 arguments
						{
							int original, copy, readCode, writeCode, *buffer[BUFFER_SIZE];
							int success = 1;

							original = open(argvv[i][1], O_RDONLY); // Open the original file with system call open() and reading permissions
							if (original == -1)
							{															  // Check for errors
								fprintf(stdout, "[ERROR] Error opening original file\n"); // Print the error message in the standard output
								success = 0;
							}

							copy = open(argvv[i][2], O_CREAT | O_WRONLY | O_TRUNC, 0666); // Open the file where we will copy the original file with system call open() and writing permissions (or create if it doesn't exist)
							if (copy == -1)
							{															  // Check for errors
								fprintf(stdout, "[ERROR] Error opening the copy file\n"); // Print the error message in the standard output
								success = 0;
							}

							do
							{													// Read/write loop
								readCode = read(original, buffer, BUFFER_SIZE); // Read the contents of the original file
								if (readCode == -1)
								{ // Check for errors
									perror("Could not read the original file");
									success = 0;
								}

								writeCode = write(copy, buffer, readCode);
								if (writeCode == -1)
								{ // Check for errors
									perror("Could not write on the copied file");
									success = 0;
								}
							} while (readCode > 0);

							if (success == 1)
							{
								fprintf(stderr, "[OK] Copy has been successful between %s and %s\n", argvv[i][1], argvv[i][2]); // Print the success message in the standard error output
							}
						}
						else
						{
							fprintf(stdout, "[ERROR] The structure of the command is mycp <original file> <copied file>\n"); // Print error message in the standard output
						}
					}

					else
					{ /* EXTERNAL COMMANDS */

						int pipeCode = pipe(p1); // Creating the pipe
						if (pipeCode < 0)
						{ // Check for errors when creating pipe
							perror("Error when creating pipe");
						}

						// If we are in the last child process
						if (i == command_counter - 1)
						{
							close(p1[WRITE]);
							// We don't need to duplicate the output file descriptor because we are not going to need the output of the last child
						}
						else
						{
							dup2(p1[WRITE], STDOUT_FILENO); // Duplicate the output file descriptor
							close(p1[WRITE]);				// Close the descriptor to write into the pipe
						}

						pid = fork(); // Create child process

						if (pid == 0)
						{											  // Child process
							if (strcmp(filev[0], "0") != 0 && i == 0) // First child
							{										  // Check if there is an input redirection
								close(STDIN_FILENO);				  // Close the standard input
								open(filev[0], O_RDONLY);			  // Open the input with reading permissions
							}
							if (strcmp(filev[1], "0") != 0 && i == command_counter - 1) // Last child
							{															// Check if there is an output redirection
								close(STDOUT_FILENO);									// Close the standard output
								open(filev[1], O_CREAT | O_WRONLY, 0666);				// Open with writing permissions and, if the file doesn't exist, create it
							}
							if (strcmp(filev[2], "0") != 0)
							{											  // Check if there is an error redirection
								close(STDERR_FILENO);					  // Close the standard error file
								open(filev[2], O_CREAT | O_WRONLY, 0666); // Same permissions as output redirection
							}
							close(p1[READ]);

							execvp(argvv[i][0], argvv[i]); // To execute the commands, we use one of the system calls from the exec() family

							// If we reached these lines, some error must have occurred
							perror("Error when executing command");
							exit(-1);
						}
						else if (pid != -1)
						{ // Parent process
							if (in_background == 1)
							{
								printf("[%d]\n", pid);
								dup2(p1[READ], STDIN_FILENO);	   // Use the pipe to connect the next input and the output of this process
								dup2(original_out, STDOUT_FILENO); // Restore the original output file descriptor
								close(p1[READ]);				   // Now we can close the pipe
							}
							else
							{ // Parent process only waits if the command is executed in foreground
								int waitCode = wait(&status);
								if (waitCode < 0)
								{
									perror("Error while waiting");
									exit(-1);
								}
								dup2(p1[READ], STDIN_FILENO);	   // Use the pipe to connect the next input and the output of this process
								dup2(original_out, STDOUT_FILENO); // Restore the original output file descriptor
								close(p1[READ]);				   // Now we can close the pipe
							}
						}
						else
						{
							perror("Error when using fork"); // Check for errors when using system call fork()
							exit(-1);
						}
					}
				}
				dup2(original_in, STDIN_FILENO);	// Restore the standard input file descriptor
				int closeCode = close(original_in); // And now we can close the input...
				if (closeCode < 0)
				{ // Check for errors while closing
					perror("Error while closing input");
				}
				closeCode = close(original_out); // ...and the output
				if (closeCode < 0)
				{
					perror("Error while closing output");
				}
			}
		}
	}

	return 0;
}
