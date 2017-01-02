#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int port = atoi(argv[1]); 

	int server_socket, client_socket;
	//create server socket:
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	//define the server address
	struct sockaddr_in server_address, client_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	
	//Ensure that sock address can be reused:
	int yes = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));


	//Bind socket to specified IP and port
	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	listen(server_socket, 5);

	while(1){
	char server_message[500], client_response[500], handle_buffer[500] = "What is your name?";	
	client_socket = accept(server_socket, NULL, NULL);
	//Send prompt for handle in handle_buffer:	
	send(client_socket, handle_buffer, sizeof(server_message), 0);
	//Receive handle back into handle_buffer:
	recv(client_socket, &handle_buffer, sizeof(handle_buffer), 0);
	//char return_name[255] = "Nice name"; 
	//send(client_socket, return_name, sizeof(return_name), 0);
	//printf("Message sent to new chatter %s: Nice Name\n", handle_buffer);
	strtok(handle_buffer, "\n"); //Remove newline
	//char client_name[500] = handle_buffer;
	//client_response = "Name Returned";
	int quit = 0;
	while(quit == 0){
		printf("%s>  %s ", handle_buffer, client_response); //Print client response
                if(strcmp(client_response, "\\quit") == 0){
			printf("\nClient has closed the connection");
			break;
		}
		printf("\n You> "); //Prompt
		//fflush(stdout);
                fgets(server_message, 500, stdin); //Get server message from stdin
		strtok(server_message, "\n");
		send(client_socket, server_message, sizeof(server_message), 0); //Send it
		if(strcmp(server_message, "\\quit") == 0){
			printf("Closing connection to %s", handle_buffer);
			close(client_socket);
			quit = 1;
			break;
		}
		printf("\nWaiting for response from %s...\n", handle_buffer);
                recv(client_socket, &client_response, sizeof(client_response), 0); //Receive client response	

        }
	printf("Listening for connection....");

}
	return 0;
}
