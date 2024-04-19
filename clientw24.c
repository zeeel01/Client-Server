#include <stdio.h> //Used for printing error messages and user prompts to the console.
#include <netdb.h>
#include <netinet/in.h> //Used for sockaddr_in serv_addr, mirror_addr
#include <stdlib.h>     // Used for memory allocation, string manipulation, and converting string representations to integers.
#include <string.h>     //Used for manipulating strings and parsing user input. Ex strcmp() strcpy()
#include <sys/socket.h> //Used for socket-related functions like bind() accept() connect()
#include <sys/types.h>
#include <unistd.h>    //Used for system calls like read, write, close, and fork
#include <arpa/inet.h> // Used for converting IP addresses between different formats using functions like inet_pton and inet_ntop.

#define BUFSIZE 1024 // Setting up the bfr size

int validDate(const char *date) // method to chck date
{
    int year, month, day;                                   // Declaring the variables
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) // Deconstructing given date into day, months and year
    {
        return 0; // If not three arguments, return
    }

    if (year < 0 || month < 1 || month > 12 || day < 1) // if year < 0, month <1 ot month >12, day<1
    {
        return 0; // retunr 0
    }

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // setting the dayzz of 12 monts

    if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) // checking for the leap yeer
    {
        daysInMonth[1] = 29; // in febb dyz are 29
    }

    if (day > daysInMonth[month - 1]) // validating the given day as per pre set ddayz arry
    {
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) // in main code
{
    char buff[BUFSIZE];    // buffer arry
    char command[BUFSIZE]; // filtered commnd wil be here
    char userInput[2048];
    int isValid, portNumber; // variables isValid and portnumber declre

    int socket_fd;                             // descriptor of socket
    struct sockaddr_in serv_addr, mirror_addr; // addresses of server and mirror

    if (argc != 3) // It three arguments are not provided
    {
        fprintf(stderr, "Usage: %s <IP> <Port#>\n", argv[0]); // give usage direction to the user
        exit(1);                                              // exit
    }

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // configuring socket
    {
        // AF_INET: Address family of IPv4
        // SOCK_STREAM: specifies type of socket i.e stream oriented or udatagram
        // 0: Specifies the protocol to be used ) means default underlying protocol
        printf("\n Socket creation error \n"); // if error
        exit(1);                               // leave
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // resetting the server address with 0

    serv_addr.sin_family = AF_INET; // setting the addr family of server address

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    // The inet_pton() function converts an IP address from presentation (string) format to network format (binary)
    // AF_INET: Address family of IPv4
    // &serv_addr.sin_addr: Pointer to the destination structure where the converted address will be stored.
    {
        perror("Invalid address"); // If error occored prit error
        exit(3);                   // give exit
    }

    if (sscanf(argv[2], "%d", &portNumber) != 1) // Get porrt from the second argument
    {
        fprintf(stderr, "Invalid port number\n"); // If port reading error print error
        exit(4);                                  // Exittttt
    }
    serv_addr.sin_port = htons(portNumber); // Set the port number in network byte order

    // trying to connect to server
    if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //- socket_fd: The file descriptor of the socket.
    // serv_addr: A pointer that has server address information
    // sizeof(serv_addr): The size of the sockaddr_in structure.
    {
        printf("\nconnect() failed, exiting"); // If failss print error
        exit(3);                               // exittt
    }

    char status[500];                                  // Get staus if connect to mirror or mirror2
    memset(status, 0, sizeof(status));                 // reset status before
    int val = read(socket_fd, status, sizeof(status)); // get status from servar
    int portStatus = atoi(status);                     // convert the string to int staus
    if (portStatus == 8089)                            // if staus is 8089
    {
        close(socket_fd);                            // close socket
        socket_fd = socket(AF_INET, SOCK_STREAM, 0); // create new server socket

        if (socket_fd == -1) // if fails the servar sacket
        {
            perror("socket");   // print error
            exit(EXIT_FAILURE); // exitt
        }

        serv_addr.sin_family = AF_INET; // setup the addreaa family
        if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
        // The inet_pton() function converts an IP address from presentation (string) format to network format (binary)
        // AF_INET: Address family of IPv4
        // &serv_addr.sin_addr: Pointer to the destination structure where the converted address will be stored
        {
            perror("Invalid address"); // If fails print error
            exit(3);                   // exitt
        }
        portNumber = 8089; // port is 8089 for mirror1

        serv_addr.sin_port = htons(portNumber); //  // Set the port number in network byte order

        if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //- socket_fd: The file descriptor of the socket.
        {
            // serv_addr: A pointer that has server address information
            // sizeof(serv_addr): The size of the sockaddr_in structure.
            printf("\nconnect() failed, exiting"); // If failed print error
            exit(3);                               // exitt
        }
    }
    else if (portStatus == 5001) // if the port is 5001
    {
        close(socket_fd);                            // close sacket
        socket_fd = socket(AF_INET, SOCK_STREAM, 0); // create new server socket

        if (socket_fd == -1) // if return -1
        {
            perror("socket");   // print sacket error
            exit(EXIT_FAILURE); // failed
        }

        serv_addr.sin_family = AF_INET; // setting up address family

        if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
        {
            // The inet_pton() function converts an IP address from presentation (string) format to network format (binary)
            // AF_INET: Address family of IPv4
            // &serv_addr.sin_addr: Pointer to the destination structure where the converted address will be stored
            perror("Invalid address");
            exit(3);
        }
        portNumber = 5001; // new port is 5001 now for mirror2

        serv_addr.sin_port = htons(portNumber); // conerting the port from int to bytes

        if (connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) ///- socket_fd: The file descriptor of the socket.
        {
            // serv_addr: A pointer that has server address information
            // sizeof(serv_addr): The size of the sockaddr_in structure.
            perror("connect()"); // If fail print errpr
            exit(3);             // exit
        }
    }
    while (1) // in infinite loop
    {
        printf("Enter a command to send to server: "); // scaan the cmmnd
        isValid = 1;                                   // initialy valid
        memset(buff, 0, sizeof(buff));                 // reseting buffer to 0
        memset(command, 0, sizeof(command));           // resettign command to 0

        fgets(buff, sizeof(buff), stdin);      // scan user command in the buffer
        memcpy(userInput, buff, sizeof(buff)); // copy the command
        buff[strcspn(buff, "\n")] = 0;         // eleminate \n

        char *command = (strtok(strdup(buff), " ")); // take first token
        if (command == NULL)                         // if null
        {
            isValid = 0; // not validd
        }

        else if (strcmp(command, "w24fn") == 0) // if command is w24fn
        {
            char *searchFile = strtok(NULL, " "); // get the file to be searched
            //printf("%s", searchFile);
            if (searchFile == NULL)               // if file is not given
            {
                isValid = 0; // not valid
            }
            else // otherwise valid
            {
                sprintf(command, "w24fn %s", searchFile); // sent the command to server
            }
        }

        else if (strcmp(command, "dirlist") == 0) // if command is dirlist
        {
            char *option = strtok(NULL, " "); // get optiion
            if (option == NULL)               // if no option, invalis
            {
                isValid = 0; // not valid
            }
            else if (strcmp(option, "-t") == 0 || strcmp(option, "-a") == 0) // if -a or -t valid
            {
                sprintf(command, "dirlist %s", option); // send command to server
            }
        }

        else if (strcmp(command, "w24fz") == 0) // if commans is w24 fz
        {
            char *s1 = strtok(NULL, " ");                                                        // get size 1
            char *s2 = strtok(NULL, " ");                                                        // get size2
            if (s1 == NULL || s2 == NULL || atoi(s1) < 0 || atoi(s2) < 0 || atoi(s2) < atoi(s1)) // validate both sizes
            {
                isValid = 0; // if invalid not valid
            }
            else
            {
                sprintf(command, "w24fz %s %s", s1, s2); // else senf commans to setver
            }
        }

        else if (strcmp(command, "w24fdb") == 0) // if command is w24fdb
        {
            char *dt = strtok(NULL, " ");     // get date
            if (dt == NULL || !validDate(dt)) // if date is not valis
            {
                isValid = 0; // not valid command
            }
            else // otherwise
            {
                sprintf(command, "w24fdb %s", dt); // send command to server
            }
        }

        else if (strcmp(command, "w24fda") == 0) //  if command is w24fda
        {
            char *dt = strtok(NULL, " ");     // get the date
            if (dt == NULL || !validDate(dt)) // if date is not valis
            {
                isValid = 0; // not valid command
            }
            else // otherwise
            {
                sprintf(command, "w24fda %s", dt); // send command to server
            }
        }

        else if (strcmp(command, "w24ft") == 0) // if command is w24ft
        {
            char *ext[3];      // getting the ext[] error
            int ext_count = 0; // counts of extention

            char *token;                                                 // to kenize other to get other extention
            while ((token = strtok(NULL, " ")) != NULL && ext_count < 3) // getting the exrtention
            {
                ext[ext_count] = token; // store in extenstion arrya
                ext_count++;            // increment
            }
            if (ext[0] == NULL) // if first is zero
            {
                isValid = 0; // in vallid
            }
            else // otherwise
            {
                int i = 0; // to count ext
                while (i < ext_count && ext[i] != NULL)
                {
                    sprintf(command + strlen(command), " %s", ext[i]); // generate the command again to sent to server
                    i++;                                               // increase i
                }
            }
        }

        else if (strcmp(command, "quitc") == 0) // if quitc
        {
            sprintf(command, "quitc"); // send quitc to server
        }
        else
        {
            isValid=0;
        }
        //printf("%d",isValid);

        if (isValid==0) // if not valid command
        {
            printf("Please enter valid command\n"); // print invalid command
            continue;                               // skip current iteration
        }

        if (send(socket_fd, command, strlen(command) + 1, 0) == -1) // otherwise, send command to user
        {
            printf("%s", command);  // if not sent print to verify
            perror("Write failed"); // prit error
            break;                  // break
        }
        if (strcmp(command, "quitc") == 0) // if quit
            break;                         // break from loop

        char reply[2000000];    // to get reply from server
        ssize_t total_read = 0; // count of read
        ssize_t val;            // catch read value

        while ((val = read(socket_fd, reply + total_read, sizeof(reply) - total_read)) > 0) // iterativeely read server response
        {
            total_read += val;                                                // add totl rwad bytes
            if (total_read >= sizeof(reply) || reply[total_read - 1] == '\n') // if error in read
            {
                break; // break from loop
            }
        }

        if (val < 0) // if read returns 0
        {
            perror("Read failed"); // print error
            break;                 // break
        }
        printf("%.*s", (int)total_read, reply); // print readed response from server
    }
    close(socket_fd); // close server socket
    return 0;         // return 0
}