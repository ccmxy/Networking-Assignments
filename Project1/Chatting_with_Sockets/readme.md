##Description:
 A client-server model chat program with sockets. The client half is in C and the server half is optionally in C or Python. 

##Directions:

1. Open two different linux command line windows in this directory

2. Enter `./makefile` into command line      
  *If there are permissions issues than first enter `./makefile chmod 777` and try again*

### Using python server: 

3. In one of the windows, enter `python ./chatserve.py [portnumber]`

### Using C server (BETA):

3. In one of the windows enter `./chatserve [portnumber]`          
  *If there are permission issues enter `./chatserve chmod 777` and try again*

### Using either
4. In the other window, enter `./chatclient [portnumber] [name of host where chatserve is]          
   *If there are permission issues enter `./chatclient chmod 777` and try again*

5. You will be promted for your name in the chat client window. Enter it.

6. Go to the server window and say something back.

7. Keep chatting or enter `\quit` from either side to end the connection. The server will stay on and wait for new client connections. If you try to connect multiple clients to it at once, it will wait until the connection with one of them has ended to start the next one.


### Example       

###### Server window:           

~/Chatting_with_Sockets$ python chatserve.py 30050                
Starting server. Connect client program to localhost on port 30050       
Connection with new chatter established! Querying for name...        
Colleen> Colleen         
 You> Cool name          
Awaiting response from client...        
Colleen> thanks       
 You> \quit      
You have ended the connection with Colleen      
Listening for connection...      
Connection with new chatter established! Querying for name...       
Colleen> Colleen        
 You> Sorry i left before        
Awaiting response from client...      
Colleen> \quit     
Connection with Colleen closed by client.     
Listening for connection...     

##### Client window:          
~/Chatting_with_Sockets$ ./makefile                  
~/Chatting_with_Sockets$ ./chatclient 30050 localhost     
ServerBob> What is your name?      
 You>Colleen                 
Waiting for response from server...           
ServerBob> Cool name     
 You>thanks       
Waiting for response from server...      
Server Bob has closed this connection. Goodbye.        
~/Chatting_with_Sockets$ ./chatclient 30050 localhost      
ServerBob> What is your name?       
 You>Colleen       
Waiting for response from server...       
ServerBob> Sorry i left before      
 You>\quit     
You have closed the connection        

