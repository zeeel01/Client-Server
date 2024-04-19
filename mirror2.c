// Include Statements
#include <stdio.h>      // for standard input-output
#include <netdb.h>      // for network database operations
#include <netinet/in.h> // for internet address family
#include <stdlib.h>     // for standard library functions
#include <string.h>     // for string manipulation
#include <sys/socket.h> // for socket-related functions
#include <sys/types.h>  // for system call data types
#include <sys/signal.h> // for signal handling
#include <unistd.h>     // for system calls
#include <arpa/inet.h>  // for internet-related operations
#include <sys/wait.h>   // for process management
#include <errno.h>      // for system error handling
#include <sys/stat.h>   // for file system operations
#include <time.h>       // for time-related functions
#include <dirent.h>     // for directory manipulation
#include <stdbool.h>    // for boolean type and values

// defining the constants
#define MAX_PATH_LENGTH 4096      // maximmum path length
#define BUFSIZE 1024              // buffer size
#define RESULT_BUFFER_SIZE 200000 // result buffer size
#define PORT 5001                 // port for miror 2

bool lookForFile(const char *filename, const char *currentPath, char *result); // function to search for a file
void mirrorRedirection2(int client_fd);                                        // function for miror redirection 2
void perform_w24fn(char *filename, int csd);                                   // perform w24fn
void getFilePermission(mode_t mode, char *str);                                // get file permission
bool lookForFile(const char *dir_path, const char *filename, char *result);    // function to search for a file
void perform_dirlist_t(int csd);                                               // perform directory list t
void perform_dirlist_a(int csd);                                               // perform directory list a
void perform_w24fz(int sockfd, char *s1_str, char *s2_str);                    // perform w24fz
int validDate(const char *date);                                               // validate date
void perform_w24fdb(int sockfd, char *date1_str);                              // perform w24fdb
void perform_w24fda(int sockfd, char *date1_str);                              // perform w24fda
void perform_w24ft(int sockfd, char *extensions[]);                            // perform w24ft
void quit(int sockfd);                                                         // quit
void crequest(int csd);                                                        // c request

void perform_w24fn(char *filename, int csd) // method to perform serch file requirement
{
    char rootPath[MAX_PATH_LENGTH];          // variable taking the root path
    strcpy(rootPath, getenv("HOME"));        // getting the root dir copying it to rootPath variable
    char result[BUFSIZE];                    // declaring result variable later used as reply to cli
    lookForFile(rootPath, filename, result); // calling lookForFile where there is search logic
    send(csd, result, strlen(result), 0);    // after returned result by method it is send to cli using send
}

void getFilePermission(mode_t mode, char *str) // method to get permision of the requested file
{
    const char chars[] = "rwxrwxrwx"; // initializing all permissions to variable
    for (int i = 0; i < 9; i++)       // iterating through permissions
    {
        str[i] = (mode & (1 << (8 - i))) ? chars[i] : '-'; // checking and assigning permissions
    }
    str[9] = '\0'; // adding null terminator to the string
}

bool lookForFile(const char *dir_path, const char *filename, char *result) // method that performs file serch and returns true or false
{
    DIR *dir = opendir(dir_path); // opening directory
    struct dirent *entry;         // declaring directory entry
    struct stat file_stat;        // declaring file status structure

    if (dir == NULL) // checking if directory is valid
    {
        perror("opendir");  // printing error if directory is not valid
        exit(EXIT_FAILURE); // exiting with failure status
    }

    while ((entry = readdir(dir)) != NULL) // looping through directory entries
    {
        char full_path[PATH_MAX];                                                 // declaring full path variable
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name); // constructing of the full path

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) // checking for current and parent dir
            continue;                                                            // skipping if current or parent dir

        if (lstat(full_path, &file_stat) < 0) // getting file status
        {
            perror("lstat"); // printing error if failed to get file status
            continue;        // continuing to next iter
        }

        if (S_ISDIR(file_stat.st_mode)) // checking if it is a dir
        {
            if (lookForFile(full_path, filename, result)) // recursively searching in subdirectories
                return true;                              // returning true if file found
        }
        else if (S_ISREG(file_stat.st_mode) && strcmp(entry->d_name, filename) == 0) // checking if it is a regular file with matching name
        {
            char date_created[20];                                                                             // declaring variable for creation date
            char permissions_str[10];                                                                          // declaring variable for permissions
            getFilePermission(file_stat.st_mode, permissions_str);                                             // getting permissions
            strftime(date_created, sizeof(date_created), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime)); // formatting creation date
            memset(result, 0, sizeof(result));                                                                 // resetting result buffer
            snprintf(result, PATH_MAX,
                     "Filename: %s\nSize: %lld bytes\nDate Created: %s\nFile Permissions: %s\n",
                     entry->d_name, (long long)file_stat.st_size, date_created, permissions_str); // constructing result message
            closedir(dir);                                                                        // closing dir
            return true;                                                                          // returning true if file is found
        }
    }

    closedir(dir);                        // closing dir
    memset(result, 0, BUFSIZE);           // resetting result buffer
    sprintf(result, "File not found.\n"); // setting message for file as not found
    return false;                         // returning false when file is not found
}

void perform_dirlist_t(int csd) // it is void type method that lists all the directories as per date
{
    const char *home_directory = getenv("HOME"); // getting home directory
    if (home_directory == NULL)                  // checking if home directory is valid
    {
        printf("Error: HOME environment variable not set.\n"); // printing error if home directory is not set
        return;                                                // returning from method
    }

    char command[MAX_PATH_LENGTH + 100];                                                                                                    // declaring command buffer
    snprintf(command, sizeof(command), "find %s -type d -printf '%%p\\n' -printf '%%T@\\n' | sort -k 2 | cut -f 1 -d ' '", home_directory); // constructing command to list directories by date

    FILE *fp = popen(command, "r"); // executing command using popen
    if (fp == NULL)                 // checking if popen failed
    {
        perror("popen error"); // printing error if popen failed
        return;                // returning from method
    }

    char buffer[MAX_PATH_LENGTH];    // declaring buffer for directory names
    char result[RESULT_BUFFER_SIZE]; // declaring buffer for result
    result[0] = '\0';                // Initialize the result buffer

    while (fgets(buffer, sizeof(buffer), fp) != NULL) // reading directories from file pointer
    {
        // Check if adding this directory would exceed the buffer size
        if (strlen(result) + strlen(buffer) >= RESULT_BUFFER_SIZE) // checking if result buffer would overflow
        {
            printf("Result buffer overflowed\n"); // printing message if buffer overflow
            break;                                // breaking loop
        }
        strcat(result, buffer); // appending directory name to result buffer
    }

    if (pclose(fp) == -1) // closing file pointer and checking if it failed
    {
        perror("pclose error"); // printing error if pclose failed
        return;                 // returning from method
    }

    // Send the directory list over the socket
    send(csd, result, strlen(result), 0); // sending directory list over the socket
    memset(result, 0, BUFSIZE);           // resetting result buffer
}

void perform_dirlist_a(int csd) // method that lists all the dirs according to the alphabeticall order
{
    const char *home_directory = getenv("HOME"); // getting home directory
    if (home_directory == NULL)                  // checking if home directory is valid
    {
        printf("Error: HOME environment variable not set.\n"); // printing error if home directory is not set
        return;                                                // returning from method
    }

    char command[MAX_PATH_LENGTH + 50];                                                            // declaring command buffer
    snprintf(command, sizeof(command), "find %s -type d -printf '%%p\\n' | sort", home_directory); // constructing command to list directories alphabetically

    FILE *fp = popen(command, "r"); // executing command using popen
    if (fp == NULL)                 // checking if popen failed
    {
        perror("popen error"); // printing error if popen failed
        return;                // returning from method
    }

    char buffer[MAX_PATH_LENGTH];    // declaring buffer for directory names
    char result[RESULT_BUFFER_SIZE]; // declaring buffer for result
    result[0] = '\0';                // Initialize the result buffer

    while (fgets(buffer, sizeof(buffer), fp) != NULL) // reading directories from file pointer
    {
        // Check if adding this directory would exceed the buffer size
        if (strlen(result) + strlen(buffer) >= RESULT_BUFFER_SIZE) // checking if result buffer would overflow
        {
            printf("Result buffer overflowed\n"); // printing message if buffer overflow
            break;                                // breaking loop
        }
        strcat(result, buffer); // appending directory name to result buffer
    }

    if (pclose(fp) == -1) // closing file pointer and checking if it failed
    {
        perror("pclose error"); // printing error if pclose failed
        return;                 // returning from method
    }

    // Send the directory list over the socket
    send(csd, result, strlen(result), 0); // sending directory list over the socket
    memset(result, 0, BUFSIZE);           // resetting result buffer
}

void perform_w24fz(int sockfd, char *s1_str, char *s2_str) // method to perform creting tar that satisfy the size condtion
{
    if (s1_str == NULL || s2_str == NULL) // checking if input strings are valid
    {
        printf("Invalid syntax. Please try again.\n"); // printing error message if input strings are invalid
    }
    else // if input strings are valid
    {
        long long size1 = atoll(s1_str); // Convert to long long for bytes
        long long size2 = atoll(s2_str); // Convert to long long for bytes

        if (size1 < 0 || size2 < 0 || size1 > size2) // checking if size range is valid
        {
            printf("Invalid size range. Please try again.\n"); // printing error message if size range is invalid
        }
        else // if size range is valid
        {
            char *root_path = getenv("HOME"); // getting home directory
            if (root_path == NULL)            // checking if home directory is valid
            {
                printf("Could not get the home directory\n"); // printing error message if home directory is not valid
                return;                                       // returning from method
            }

            // Find files matching the size range in bytes
            char buff_command[BUFSIZE]; // declaring buffer for command
            sprintf(buff_command, "find %s -type f -size +%lldc -size -%lldc -print0 | xargs -0 tar -czf %s/w24project/temp.tar.gz",
                    root_path, size1, size2, root_path); // constructing command to find and compress files

            int status = system(buff_command); // executing command

            // Check if the files were found and tar created successfully
            if (status == -1) // if system command execution failed
            {
                printf("No file found.\n");                                // printing message if no files found
                char tar_success_msg[] = "No file found.\n";               // message to send over socket
                send(sockfd, tar_success_msg, strlen(tar_success_msg), 0); // sending message over socket
            }

            else // if system command executed successfully
            {
                wait(NULL);                                 // waiting for child process to finish
                printf("Tar file created successfully.\n"); // printing message if tar file created successfully
                // Send tar created successfully message
                char tar_success_msg[] = "Tar file created successfully.\n"; // message to send over socket
                send(sockfd, tar_success_msg, strlen(tar_success_msg), 0);   // sending message over socket
            }
        }
    }
}

int validDate(const char *date) // func that checks if the date gicen in the command is valid or not
{
    int year, month, day;                                   // declaring variables for year, month, and day
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) // parsing date string
    {
        return 0; // returning 0 if parsing failed
    }

    if (year < 0 || month < 1 || month > 12 || day < 1) // checking if year, month, or day is out of range
    {
        return 0; // returning 0 if any value is out of range
    }

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // array to store days in each month

    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) // checking for leap year
    {
        daysInMonth[1] = 29; // updating days in February for leap year
    }

    if (day > daysInMonth[month - 1]) // checking if day exceeds maximum days in month
    {
        return 0; // returning 0 if day exceeds maximum days
    }

    return 1; // returning 1 if date is valid
}

void perform_w24fdb(int sockfd, char *date1_str) // method to perform w24fdb that returns tar that were created before given date
{
    if (!validDate(date1_str)) // checking if date is valid
    {
        printf("Invalid date format or values. YYYY-MM-DD format\n"); // printing error message if date is invalid
    }
    else // if date is valid
    {
        char *root_path = getenv("HOME"); // getting home directory
        if (root_path == NULL)            // checking if home directory is valid
        {
            printf("Could not get the home directory\n"); // printing error message if home directory is not valid
            return;                                       // returning from method
        }

        // Find files matching the date range
        char buff_command[BUFSIZE]; // declaring buffer for command
        sprintf(buff_command, "find %s -type f ! -newermt \"%s\" -print0 | xargs -0 tar -czf %s/w24project/temp.tar.gz",
                root_path, date1_str, root_path); // constructing command to find and compress files based on date
        // printf("%s", buff_command);               // printing command for debugging
        int status = system(buff_command); // executing command

        // Check if the files were found and tar created successfully

        if (status == -1) // if system command execution failed
        {
            printf("No file found.\n");                                // printing message if no files found
            char tar_success_msg[] = "No files found.\n";              // message to send over socket
            send(sockfd, tar_success_msg, strlen(tar_success_msg), 0); // sending message over socket
        }

        else // if system command executed successfully
        {
            printf("Tar file created successfully.\n"); // printing message if tar file created successfully

            // Send tar created successfully message
            char tar_success_msg[] = "Tar file created successfully.\n"; // message to send over socket
            send(sockfd, tar_success_msg, strlen(tar_success_msg), 0);   // sending message over socket
        }
    }
}

void perform_w24fda(int sockfd, char *date1_str) // method that returns tar file that contains files with created date greater than provided date
{
    if (!validDate(date1_str)) // checking if date is valid
    {
        printf("Invalid date format or values. YYYY-MM-DD format\n"); // printing error message if date is invalid
    }
    else
    {
        char *root_path = getenv("HOME"); // getting home directory
        if (root_path == NULL)            // checking if home directory is valid
        {
            printf("Could not get the home directory\n"); // printing error message if home directory is not valid
            return;                                       // returning from method
        }

        // Find files matching the date range
        char buff_command[BUFSIZE]; // declaring buffer for command
        sprintf(buff_command, "find %s -type f -newermt \"%s\" -print0 | xargs -0 tar -czf %s/w24project/temp.tar.gz",
                root_path, date1_str, root_path); // constructing command to find and compress files based on date
        int status = system(buff_command);        // executing command

        // Check if the files were found and tar created successfully
        if (status == -1) // if system command execution failed
        {
            printf("No file found.\n");                                // printing message if no files found
            char tar_success_msg[] = "No files found.\n";              // message to send over socket
            send(sockfd, tar_success_msg, strlen(tar_success_msg), 0); // sending message over socket
        }
        else
        {
            printf("Tar file created successfully.\n"); // printing message if tar file created successfully

            // Send tar created successfully message
            char tar_success_msg[] = "Tar file created successfully.\n"; // message to send over socket
            send(sockfd, tar_success_msg, strlen(tar_success_msg), 0);   // sending message over socket
        }
    }
}

void perform_w24ft(int sockfd, char *extensions[]) // method that performs w24ft that returns tar file that contains only the mentioned ext
{
    char *root_path = getenv("HOME"); // getting home directory
    if (root_path == NULL)            // checking if home directory is valid
    {
        printf("Could not get the home directory\n"); // printing error message if home directory is not valid
        return;                                       // returning from method
    }

    // Check if any of the specified files are present
    char buff_command[BUFSIZE];                               // declaring buffer for command
    sprintf(buff_command, "find %s -type f \\( ", root_path); // constructing command to find files with specified extensions
    int i = 0;
    while (i < 3 && extensions[i] != NULL) // iterating through extensions array
    {
        sprintf(buff_command + strlen(buff_command), "-iname \"*.%s\" -o ", extensions[i]); // appending extension to command
        i++;
    }

    sprintf(buff_command + strlen(buff_command), "-false \\) -print0 | xargs -0 tar -czf %s/w24project/temp.tar.gz", root_path); // finalizing command to compress files

    int status = system(buff_command); // executing command

    // Check if the files were found
    if (status == -1) // if system command execution failed
    {
        char error_msg[] = "No file found.\n";         // message to send over socket
        send(sockfd, error_msg, strlen(error_msg), 0); // sending message over socket
    }

    else
    {
        // Send the tar created successfully message
        char tar_success_msg[] = "Tar file created successfully.\n"; // message to send over socket
        send(sockfd, tar_success_msg, strlen(tar_success_msg), 0);   // sending message over socket
    }
}

void quit(int sockfd) // the method theat is performed when cli says quitc
{
    printf("Client exited\n"); // printing message when client exits
    close(sockfd);             // closing socket connection
    exit(0);                   // exiting program
}

void crequest(int csd) // func that process the client requests
{
    printf("Client connected.\n"); // Print when a client connects
    char buffer[1024];             // buffer for incoming data
    int valread;                   // variable to store the number of bytes read

    while ((valread = read(csd, buffer, sizeof(buffer))) > 0) // reading data from client
    {
        printf("Incoming command: %s\n", buffer); // printing incoming command

        char *token = strtok(buffer, " "); // tokenizing incoming command

        if (token == NULL) // checking if command is empty
        {
            printf("Invalid command.\n"); // printing error message for invalid command
        }
        else if (strcmp(token, "w24fn") == 0) // checking for w24fn command
        {
            char *filename = strtok(NULL, " "); // getting filename from command
            if (filename == NULL)               // checking if filename is missing
            {
                printf("Invalid Filename.\n"); // printing error message for invalid filename
            }
            else
            {
                perform_w24fn(filename, csd); // calling function to perform w24fn operation
            }
        }
        else if (strcmp(token, "dirlist") == 0) // checking for dirlist command
        {
            char *home = getenv("HOME"); // getting home directory
            if (home == NULL)            // checking if home directory is not found
            {
                printf("Could not get the home directory\n"); // printing error message for missing home directory
                return;                                       // returning from the function
            }

            const char *type = strtok(NULL, " "); // getting directory list type from command
            if (type == NULL)                     // checking if directory list type is missing
            {
                printf("not a valid command\n"); // printing error message for invalid command
                continue;                        // continuing to next iteration
            }
            else if (strcmp(type, "-a") == 0) // checking for all directories option
            {
                perform_dirlist_a(csd); // calling function to perform directory list with all directories
            }
            else if (strcmp(type, "-t") == 0) // checking for directories sorted by date option
            {
                perform_dirlist_t(csd); // calling function to perform directory list sorted by date
            }
        }
        else if (strcmp(token, "w24fdb") == 0) // checking for w24fdb command
        {
            char *date1_str = strtok(NULL, " "); // getting date from command

            perform_w24fdb(csd, date1_str); // calling function to perform w24fdb operation
        }
        else if (strcmp(token, "w24fda") == 0) // checking for w24fda command
        {
            char *date1_str = strtok(NULL, " "); // getting date from command

            perform_w24fda(csd, date1_str); // calling function to perform w24fda operation
        }

        else if (strcmp(token, "w24fz") == 0) // checking for w24fz command
        {
            char *s1_str = strtok(NULL, " "); // getting size1 from command
            char *s2_str = strtok(NULL, " "); // getting size2 from command

            perform_w24fz(csd, s1_str, s2_str); // calling function to perform w24fz operation
        }

        else if (strcmp(token, "w24ft") == 0) // checking for w24ft command
        {

            char *ext[3]; // Assuming a maximum of 10 extensions, adjust size as needed
            int ext_count = 0;

            // Tokenize the remaining parts of the buffer into ext[] until encountering a null
            while ((token = strtok(NULL, " ")) != NULL && ext_count < 3) // tokenizing extension from command
            {
                ext[ext_count] = token; // storing extension
                ext_count++;            // incrementing extension count
            }
            perform_w24ft(csd, ext); // calling function to perform w24ft operation
        }
        else if (strcmp(token, "quitc") == 0) // checking for quit command
        {
            quit(csd); // calling function to quit
            break;     // breaking the loop
        }
    }

    if (valread == -1) // checking if reading failed
    {
        perror("read"); // printing error message
    }
    printf("Client Exitted\n"); // printing message when client exits
    close(csd);                 // closing client socket connection
    exit(0);                    // exiting from the program
}

int main(int argc, char *argv[]) // In main Driver code
{
    int sd, csd, portNumber;    // varibles for client and server socket descriptors and port number
    socklen_t len;              // declaring socket length variable
    struct sockaddr_in servAdd; // declaring server address structure

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // creating socket
    {
        fprintf(stderr, "Cannot create socket\n"); // printing error message if socket creation fails
        exit(1);
    }

    servAdd.sin_family = AF_INET;                // setting the address family
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY); // setting the adress
    servAdd.sin_port = htons((uint16_t)PORT);    // setting the port number
    // after conerting to byte

    bind(sd, (struct sockaddr *)&servAdd, sizeof(servAdd)); // binding socket

    listen(sd, 5); // listening for 5 clients

    while (1) // infinite loop
    {
        csd = accept(sd, (struct sockaddr *)NULL, NULL); //  accept the client connection
        printf("Incoming connection accepted.\n");       // Print when a connection is accepted
        int cli_fork = fork();                           // fork for curren tclient
        if (cli_fork == 0)                               // child ill handle current client
        {
            close(sd);     // close server socket
            crequest(csd); // call crequest to handle client's
            // request once connected
        }
        else if (cli_fork > 0) // if parent process
        // listen for other clients
        {
            close(csd);                            // closee the client socket
            while (waitpid(-1, NULL, WNOHANG) > 0) // kill the zombie processes
                ;
        }
    }
}