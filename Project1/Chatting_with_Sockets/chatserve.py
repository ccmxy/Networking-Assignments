import sys
from sys import stdin
import SocketServer
import argparse

class MyTCPHandler(SocketServer.BaseRequestHandler):
    """
    The RequestHandler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """
    def handle(self):
	print "Connection with new chatter established! Querying for name..."
        server_msg = "What is your name?\n"
        sent = self.request.send(server_msg)
	self.client_response = self.request.recv(500).split("\n", 1)[0]
        client_name = self.client_response
        quit = 0
        while quit < 1:
            print client_name + "> " + self.client_response
           # if client sends \quit, break loop  
            if (self.client_response == "\quit"):
                quit = 1
                print "Connection with " + client_name + " closed by client."     
                break
            sys.stdout.write(' You> ')
            sys.stdout.flush()
            server_msg = stdin.readline()
           # self.request is the TCP socket connected to the client 
            self.request.send(server_msg)
           # if server sends \quit, close connection with client and break loop
            if(server_msg == "\quit\n"):
                quit = 1
                self.request.close()
                print "You have ended the connection with " + client_name 
                break
            print "Awaiting response from client..."
            self.client_response = self.request.recv(500).split("\n",1)[0]
        print "Listening for connection..."


if __name__ == "__main__":

    #Specify command line arguments
    parser = argparse.ArgumentParser(
        description='''Server side of a two-way chat program which creates a socket
             to listen for incoming connections, using a port that you specify in the 
                 command line''')
    parser.add_argument('port', metavar='port', type=int,
         help='This is the port number that the socket will be bound to.')

    args = parser.parse_args()

    HOST, PORT = "localhost", (args.port)

    # Create the server, binding to localhost on specified port
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

    print "Starting server. Connect client program to " + str(HOST) +  " on port " + str(PORT) 

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
