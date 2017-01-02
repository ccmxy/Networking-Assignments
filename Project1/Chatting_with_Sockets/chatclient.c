#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

void stripWhitespaces(char *str);

int main(int argc, char* argv[]) {
	//create a socket
	int network_socket;	
	//AF_INET = domain, sock_stream for TCP socket, protocol 0 for default
	network_socket = socket(AF_INET, SOCK_STREAM, 0);
	// specify address for socket:
	struct sockaddr_in server_address;
	//Set family of address:
	server_address.sin_family = AF_INET;
	//Set port as first command line argument:
	server_address.sin_port = htons(atoi(argv[1]));
	//Specify ip address as secton command line argument:
	server_address.sin_addr.s_addr = (atoi(argv[2]));
	
	//Connect to server
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address)); 
	//Error check connection
	if (connection_status == -1){
		printf("ERROR: Connection failed");
	}

	// receive data from the server
	char server_response[256];
	char client_msg[256];
	int quit = 0;
	while(quit == 0){
		recv(network_socket, &server_response, sizeof(server_response), 0);
		strtok(server_response, "\n\r");
		//If server has sent quit, print quit message:
		if (strcmp(server_response, "\\quit") == 0) {
			printf("Server Bob has closed this connection. Goodbye.\n");
			quit = 1;
			break;
		}
                //Print out server's response:
		printf("ServerBob> %s\n", server_response);
		//Prompt user:
		printf(" You>");
		fgets(client_msg, 500, stdin);
		//Send user message to server:
		send(network_socket, client_msg, sizeof(client_msg), 0);		
		if (strcmp(client_msg, "\\quit\n") != 0) {
			printf("Waiting for response from server...\n");
		}
		else{ //If \quit command from this side, close the connection:
			close(network_socket);
			printf("You have closed the connection.\n");
			quit = 1;
			break;
		}
	}
	return 0;
}


