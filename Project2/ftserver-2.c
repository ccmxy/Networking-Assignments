
#include <assert.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <dirent.h>

char **getFileList(char *dirname, int *numFiles);
int runConnectionP(int controlSocket, char *commandTag, int *dataPort, char* filename);
void runConnectionQ(int dataSocket, char *commandTag, char *filename);

/* Server side program: The server is run in main
and runs a loop of a client connection accept, followed by,
if the client sends a valid command, an attempt to connect to the dataPort
*/
int main(int argc, char **argv)
{       
	int port;  // Port number on which to listen for client connections.
	int status; //misc return statuses
	int serverSocket; //Socket for receiving connection requests
	int controlSocket, dataSocket; //Sockets to accept connections with client
	int dataPort;  //port for connection for sending data on dataPort over connectionQ
	socklen_t addr_len;
	struct sockaddr_in client_addr;  // address for client socket
	char commandTag[30];
	char filename[500];

	// Validate number of commands:
	if (argc != 2) {
		fprintf(stderr, "usage: ftserver <server-port>\n");
		exit(1);
	}

	port = atoi(argv[1]);

	//define the server address.
	struct sockaddr_in server_addr; // Server address
	//AF_INET = domain, sock_stream for TCP socket, protocol 0 for default:
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Create a server-side socket.
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		perror("Error on serverSocket creation");
		exit(1);
	}

	//Bind to the specified IP and port
	bind(serverSocket, (struct sockaddr*) &server_addr, sizeof(server_addr));

	//Listen for connections.
	status = listen(serverSocket, 5);
	if (status == -1) {
		perror("Error on listen to serverSocket");
		exit(1);
	}

	// Provide FTP services to clients until interrupted.
	printf("Starting server on port %d\n", port);
	while (1) {
		// Establish FTP control connection.
		addr_len = sizeof(struct sockaddr_in);
		controlSocket = accept(serverSocket, (struct sockaddr *) &client_addr, &addr_len);
		if (controlSocket == -1) {
			perror("accept");
			exit(1);
		}
		printf("Connection P established!\n");

		// Communicate over FTP control connection.
		status = runConnectionP(controlSocket, commandTag, &dataPort, filename);

		// If status == 0 (commands found valid by runConnectionP),
		// then an attempt to create a data socket is made:
		if (status == 0) {
			//Create data socket for connection Q:
			dataSocket = socket(AF_INET, SOCK_STREAM, 0);
			if (dataSocket == -1) {
				perror("Error on dataSocket creation");
				exit(1);
			}

			client_addr.sin_port = htons(dataPort);
			// Initiate the data connection 
			//Do this repeatedly in case command is found invalid by runConnectionP (sets status to -1)

			do {
				status = connect(dataSocket, (struct sockaddr *) &client_addr, sizeof(client_addr));
			} while (status == -1);

			printf("Connection Q established!\n");

			// Communicate over data connection...
			runConnectionQ(dataSocket, commandTag, filename);
	}
}

	exit(0);
}





/*
int runConnectionP(int controlSocket, char on for communicating over the server that's run in main.
Returns 0 if it finds the command sent by the client valid.
Server in main creates a data socket if this returns 0,
or keeps listening if this returns -1.
*/
int runConnectionP(int controlSocket, char *commandTag, int *dataPort, char* filename)
{
	char port_buffer[500];  // Input packet payload
	char command_buffer[30];
	char* command;
	char* port;
	printf("  Receiving data port (FTP active mode) ...\n");
 	read(controlSocket, port_buffer, 1024);
	printf("Sending ack for port....");
        write(controlSocket, "ACK", 3);
	
	printf("PortNo: %s", port_buffer);
	port = strtok(port_buffer, "\n");
	*dataPort = atoi(port);
	
	printf("  Receiving command ...\n");
 	//Read from socket into command
        recv(controlSocket, &command_buffer, sizeof(command), 0);
	printf("Command buffer: %s\n", command_buffer);
	command = strtok(command_buffer, "\n");

        printf("Command: %s\n", &command);

        //strtok(command, "");
	write(controlSocket, "ACK", 3);
	printf("Sent ack for command\n");
        if (strcmp(command, "-l") == 0){
		printf("User chose list...\n");
		strcpy(commandTag, "LIST");
	}
	if (strcmp(command, "-g") == 0){
		printf("User chose get...\n");
		strcpy(commandTag, "GET");
		int nread = read(controlSocket, filename, 500);
		//add null terminating character:
		filename[nread] = '\0';
	}

	//If command is invalid, send error message to client:
	if (strcmp(command, "-l") != 0 && strcmp(command, "-g") != 0) {
		printf("  Transmitting command error ...\n");
		//Client will close connection Q and exit
		write(controlSocket, "ERROR: BAD COMMAND", strlen("ERROR: BAD COMMAND"));
		return -1;
	}

	//If command is valid, send ACK to client:
	else {
		printf("  Transmitting data-connection go-ahead ...\n");
		//Client will attempt to recieve data on data port
		write(controlSocket, "ACK", strlen("ACK"));
		return 0;
	}
}


//runConnectQ: This is the function for communicating over the data
//All communication in this function takes place over the
// data socket.
// runConnectionQ also closes the data socket.
void runConnectionQ (int dataSocket, char *commandTag, char *filename)
{
	char **fileList; //char** to hold list of files
	int numFiles;   //counter for for loop for transmitting the listings
	fileList = getFileList(".", &numFiles);
	char ack[3];
	// The client requests transmission of filenames in the current directory.
	if (strcmp(commandTag, "LIST") == 0) {

		/*Transfer each filename one by one,
		receiving an ACK each time.
		ftclient runs a loop that stops when DONE
		is received.*/
	printf("  Transmitting file listing ...\n");
		int i = 0;
		for ( i = 0; i < numFiles; i++) {
			int number = htonl(18);
			//write directories tag to socket
			write(dataSocket, "directories\n\0", strlen("directories"));
			//Recv ack for tag
			read(dataSocket, ack, 3);
			//write file name to socket
			write(dataSocket, fileList[i], strlen(fileList[i]));
			//recv ack for file name
			read(dataSocket, ack, 3);
			printf("Ack for %s received...\n", fileList[i]);
		}	//End of for loop
					//Write done so client knows that file transfer is complete
           	write(dataSocket, "DONE", strlen("DONE"));
		printf("Sent DONE to let client know that the list of files has completed.\n");
		
	} //end of if list

	// The client requests transmission of a file.
	else if (strcmp(commandTag, "GET") == 0) {
		char buffer[500]; // File reader storage buffer
		FILE *infile;   // Reference to input file
    		printf("FILE NAME IS %s\n", filename);

			// Attempt to open the file.
			infile = fopen(filename, "r");
			if (infile == 0) {
				printf("  Transmitting file-read-access error ...\n");
				write(dataSocket, "ERROR: ", strlen("ERROR: "));
				read(dataSocket, ack, 3);
				write(dataSocket, "File not found.", strlen("File not found."));
				read(dataSocket, ack, 3);

				write(dataSocket, "DONE", strlen("DONE"));
			} //end if if file could not be opened

		else{ //Valid file name
			// Transfer tag text_transfer so ftclient.py knows what to do:
			write(dataSocket, "text_transfer", strlen("text_transfer"));
			printf("Set file tag...\n");
			//Ack for 'text_transfer' tag:
			read(dataSocket, ack, 3);
			printf("Ack received for text transfer tag...\n");
			//Transfer the name of the file so ftclient.py
			//knows what to save it to:
			write(dataSocket, filename, strlen(filename));
			//Get ACK for transfer of filename:
			read(dataSocket, ack, 3);
			printf("Ack received for file name...\n");
			// Open the file and send the contents over the data connection:
			printf("Opening file %s to retreive text to send to client...\n", filename);
			int fd = open(filename, O_RDWR);
			read(fd, buffer, 500);
			close(fd);
			buffer[(strlen(buffer) + 1)] = '\n';
			printf("Writing to socket...\n");
			write(dataSocket, buffer, strlen(buffer));
			if (ferror(infile)) {
				perror("fread");
			}
			fclose(infile);
			printf("File closed\n");
		} //end of if VALID file name
		


}
		//Write DONE to the dataSocket so that it knows 
		//that the transfer is complete, and it should now 
		//close connectionP and then send an ACK so that this 
		//end will know to close connectionQ: 
		printf("Writing final DONE to socket dataSocket\n");
		write(dataSocket, "DONE", strlen("DONE"));
 		printf("Sent DONE to socket to signify end of data trasnfer\n");
		read(dataSocket, ack, 3);
		printf("Client sent ACK for done signal: OK to close connection.\n");
		// Close connectionQ:
		int status = close(dataSocket);
		if (status == -1) {
			printf("Failed to close connection Q!\n");
			exit(1);
		}
		printf("Connection Q closed.\n");
}


/*
function to return a list of every file in the directory that
the server is run in and set numFiles to the number of files in the directory.
*/

char ** getFileList(char* dirname, int *numFiles)
{
    char **fileList;      //List of files to return 
    DIR *d;             //Directory object
    struct dirent *dir;
    struct stat info;     //Directory information

    d = opendir(".");
    // Build a list of filenames in the given directory.
    *numFiles = 0;
    fileList = NULL;
    if(d){
        while ((dir = readdir(d)) != NULL) {
            stat(dir->d_name, &info);
            if (S_ISDIR(info.st_mode)) {
                continue;
            }

            {
                if (fileList == NULL) {
                    fileList = malloc(sizeof(char *));
                    } else {
                    fileList = realloc(fileList, (*numFiles + 1) * sizeof(char *));
                }
                fileList[*numFiles] = malloc((strlen(dir->d_name) + 1) * sizeof(char));
                strcpy(fileList[*numFiles], dir->d_name);
                (*numFiles)++;
            }
        }
    }

    closedir(d);
    return fileList;
}


