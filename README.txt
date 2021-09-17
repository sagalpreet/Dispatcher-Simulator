Submitter name: Sagalpreet Singh
Roll No.: 2019CSB1113
Course: CS303
=================================
1. What does this program do

"Dispatcher Simulator" is a project written in C which simulates the functioning of dispatcher using a client-server network. The server responds to client requests under the following constraints:

a) Server recieves the requests on different threads, however there is a limit to the number of connections that a server can communicate on concurrently.
b) Whenever server recieves a request, it enqueues the request into a request queue.
c) Dispatcher, as its name suggests dequeues the requests and executes them. Dispatcher does so by assigning the requests to different threads. There is a predefined limit on the number of threads a dispatcher can use.
d) On each of the threads that dispatcher assigns a task to, there is a pre-defined limit on number of open files and memory consumed. If execution of request exceeds this, the request is not processed further, and an error statement is output after which thread continues with its normal execution.

The request sent by client has to be of the following form (3 Lines followed by EOF - details on how to run in section 3):
<Dynamic Library Name (with absolute address)>
<name of function to be invoked>
<argument to the function>

For this project, the only supported functions are those with a single argument of type (double) and return type (double).
However, any library can be used.

File Structure:
- bin : contains executables
- obj : contains object files
- src : contains source code (c and header files)
- scripts : bash scripts for automated compilation, linking and execution of entire project (build and run)
- execution_results : contains output of all the requests handled by dispatcher (separate file for each thread)
- test_data : contains file containing request data for the request made by client
- unit_tests : contains unit tests for all helper functions
	-bin : binaries of unit tests
- README.txt
- design.pdf

2. A description of how this program works (i.e. its logic)

Different modules have been used to build this project. Following are their implementation details.

a) client
- This program contains the implementation of client. The client sends the request to server and communicates over the network. Client uses the sys/socket.h and arpa/inet.h apis.
- It is a standalone piece of code.
- PORT over which the client has to send requests is taken as a command line argument. The client attempts to build a socket and connect to server over the port. Proper checks are made if the client is able to connect to the server else error messages are logged onto console.
- An optional second command line argument can be used to specify the name client which is "client" by default.
- The program takes input from stdin which specifies the request that has to be communicated over to the server. The format and details are discussed in section 3.

b) server
- This program contains the implementation of server. The server recieves the request over network sent by clients and sends a confirmation as to whether the request has been queued or not. Note that it may so happen that the request is queued but not executed later as it may exceed the resource limit of thread it is assigned to or it may be an illegal request. In any case, an attempt to execute it is made.
- It includes functions spread accross various other files but has a main function.
- The main function takes in input 6 command line arguments: PORT, queue_size, max_connections, max_threads, max_files, max_size (in order)
- This program spawn two threads
- One of the thread functions setups the server and ultimately accepts requests from clients and enqueues them or rejects them.
- The other thread function is a dispatcher
- Following are the 3 functions of this program:
	- runDispatcher: It spawns the max_threads(the command line argument) number of threads each of which check for the requests is pending in the queue and service them.
	- runServer: It attempts to make a socket, connect it to the port and then listens to the client requests. It listens to client requests concurrently on different threads. However the limit to number of concurrent connections is set while the listen function is called. It uses accept function in infinite loop. accept is a blocking call. Whenever, a new connection request from client is made, a check is made if it is successful and then the request using its socket descriptor is assigned to thread function - handleConnections
	- handleConnections: This is a thread function which reads the request from client and attempts to queue the request if the queue isn't already fulled. In case the queue is already full, the request is rejected and 0 is sent to the client over connection. On successful enque, 1 is sent to the client. In either case, after communicating the response to client, the socket connection is closed.
- The server program uses various functions from other modules which are discussed below. These include the implementation of request queue, functions for deep copy and freeing memory pointed to by pointers, request listener which reads the request sent by client stores it as a message (message is the name of structure used to store requests) and most importantly the dispatcher.

c) request_listener
- This module contains a single function - request_listener.
- The client sends the request in form of a string (as it is read from stdin in the client program). The string is read as a stream in request_listener, from which dynamic library name, function name and arguments are read and stored in message structure. The stream is closed and the message struct is returned by the request_listener function.

d) memory
- This module contains helper function used accross different modules. The functions help in memory allcation and deallocation.
- The functions are duplicate_sptr, duplicate_dptr, free_sptr, free_dptr
- duplicate_sptr takes a (char*) as input. It allocates equal memory on heap and deep copies contents from argument pointer's memory to newly allcated memory. This is equivalent to deep copying a string.
- Similarly, duplicate_dptr takes (char**) as input which can be seen as an array of strings. Here, the depth of copy is one more than duplicate_sptr. The function copies all the strings into newly allcoated memory.
- free_sptr frees the memory pointed to by a (char *) pointer.
- free_dptr frees the memory pointed to by (char **) pointer. char** indicates an array of strings, so free_sptr is called for each of the strings before freeing the memory pointed to by char* pointer.

e) request_queue
- This is the implementation of queue (linked-list based), which contains message structure as its contents. As already mentioned, message structure is used to store requests (see last point in server(b))
- Queue has a bound on its maximum size. The queue allows enqueue and dequeue. In addtion to this it has a function which can be used to initialize the queue on heap and return pointer to allcoated memory.

f) dispatcher
- dispatcher module holds the functions to handle the core functionality of a dispatcher (dispatch function). The helper functions are execute_command and itoa. itoa converts an integer to string. (not all compiler support itoa, so I implemented my own function). execute_command takes a shell command in form of string, execute it and return the value (supports only integer outputs).
- dispatch function is essentially a thread function which was envoked by server's runDispatcher routine. For each thread, a file is created in /execution_results directory. This directory has files with name as thread_id and store the result of functions that were run on that thread by dispatcher. If some function couldn't be run, the error is written to that file.
- The function dequeues requests from the request queue and executes them. It loads dynamic libraries and calls functions. The function also keeps track of memory used and files open through system calls and and breaks the function call if resource usage exceeds the permissible limit.

* Since the queue is common, anytime dequeue or enqueue is performed on it, a mutex lock is put over it to prevent undesirable results because of concurrent operations in multithreading.

3. How to compile and run this program

There are several methods to run the program:

I) Easy Method: Bash scripts to run server and send concurrent requests to it from clients are present in /scripts directory. cd into the directory and execute following 2 commands on two different terminal windows:
	command 1: ./fire_clients.sh (alternatively: sh fire_clients.sh)
	command 2: ./start_server.sh (alternatively: sh start_server.sh)
	(make sure to run these commands from within the scripts directory to avoid any unexpected errors)
	
	The command line arguments are already set in the shell scripts. (Command line arguments can be changed manually in the script or use method 2)
	In this case the client requests are being read from the file client_input in test_data directory. You can change the contents there to send some other request.
	All the clients will send the same request in this case.
	This method is useful in simulating traffic.

II) Easy Method: The compiled binaries are already present in /bin directory.
	i) Before proceeding, delete all the files in execution_results but do not delete the folder itself or you may get errors.
	ii) To run server, simply execute the binary named server using the command: ./server {PORT} {queue_size} {max_connections} {max_threads} {max_files} {max_size}
	iii) To run client, simply execute the binary named client using the command: ./client {PORT}
	iv) client needs to be given the input that will be sent as a request to server.
		- Follwing is the format of request:
			<Dynamic Library Name (with absolute address)>
			<name of function to be invoked>
			<argument to the function>
		- example:
			/lib/x86_64-linux-gnu/libm.so.6
			cos
			0.0
		- after entering the request details, press ctrl+d to denote the end of request. (ctrl+d signifies EOF)
		- alternatively, you can modify the contents of file ./test_data/client_input to have request. (it already contains one for example) and use the following command for client execution
			./client {PORT} < ../test_data/client_input

III) Full Method: This method involves compiling files from scratch and creating object files which are then linked to get the binaries: client and server.
	NOTE: All these commands are to be run inside src folder (you may use absolute paths for all files otherwise)
	a) Compile and link client:
		i) gcc ../src/client.c -w -o ../bin/client
	b) Compile server:
		i) gcc -g -c ../src/server.c -o ../obj/server.o -pthread -ldl
		ii) gcc -g -c ../src/dispatcher.c -o ../obj/dispatcher.o -pthread -ldl
		iii) gcc -g -c ../src/memory.c -o ../obj/memory.o -pthread -ldl
		iv) gcc -g -c ../src/request_listener.c -o ../obj/request_listener.o -pthread -ldl
		v) gcc -g -c ../src/request_queue.c -o ../obj/request_queue.o -pthread -ldl
	c) Linking server:
		i) gcc -o ../bin/server ../obj/server.o ../obj/dispatcher.o ../obj/memory.o ../obj/request_listener.o ../obj/request_queue.o -ldl -pthread
	d) executing server and client is same as method 2 as both the binaries will be present in bin directory
	
After execution the results of server side will be visible in the terminal where server is being run, and similarly for client in which the client is being run.
The outputs of commands run by dispatcher on server side will be written to files in execution_results directory with each file name corresponding to a thread.

** Make sure to remove all files from execution_results folder before next run. You can omit this step if you are using bash shell as it already has this built in. Not removing won't cause any error, its just that it will make interpretations difficult.

UNIT TESTS: unit tests can be found in unit_tests directory. The shell script run_tests.sh can be used to run all the unit tests at once using the command ./run_tests.sh (alternatively, sh run_tests.sh)
The binaries for unit tests are in the subdirectory bin of directory unit_tests.


4. Provide a snapshot of a sample run

Following are the results when shell script is run (a lot of client requests are simultaneously made):

I) server side console:

start_server.sh 

Clearing results from previous execution ...


Compiling source code ...
Linking object files ...
Setting up Server ...

PORT: 9999
QUEUE SIZE: 10
MAX CONNECTIONS: 10
MAX THREADS: 8
MAX FILES ON EACH THREAD: 1
MAX MEMORY ON EACH THREAD: 100 kB

Waiting for new connection ...
Connection established with socket descriptor 4

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 14

Waiting for new connection ...
Connection established with socket descriptor 4

Waiting for new connection ...
Connection established with socket descriptor 17

Waiting for new connection ...
Connection established with socket descriptor 16

Waiting for new connection ...
Connection established with socket descriptor 20

Waiting for new connection ...
Connection established with socket descriptor 21

Waiting for new connection ...
Connection established with socket descriptor 22

Waiting for new connection ...
Connection established with socket descriptor 23

Waiting for new connection ...
Connection established with socket descriptor 24

Waiting for new connection ...
Connection established with socket descriptor 25

Waiting for new connection ...
Connection established with socket descriptor 26

Waiting for new connection ...
Connection established with socket descriptor 27

Waiting for new connection ...
Connection established with socket descriptor 28

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 16

Waiting for new connection ...
Connection established with socket descriptor 35

Waiting for new connection ...
Connection established with socket descriptor 36

Waiting for new connection ...
Connection established with socket descriptor 37

Waiting for new connection ...
Connection established with socket descriptor 35

Waiting for new connection ...
Connection established with socket descriptor 38

Waiting for new connection ...
Connection established with socket descriptor 16

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 16

Waiting for new connection ...
Connection established with socket descriptor 17

Waiting for new connection ...
Connection established with socket descriptor 23

Waiting for new connection ...
Connection established with socket descriptor 17

Waiting for new connection ...
Connection established with socket descriptor 16

Waiting for new connection ...
Connection established with socket descriptor 22

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 14

Waiting for new connection ...
Connection established with socket descriptor 19

Waiting for new connection ...
Connection established with socket descriptor 17

Waiting for new connection ...
Connection established with socket descriptor 23

Waiting for new connection ...
Connection established with socket descriptor 17

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 14

Waiting for new connection ...
Connection established with socket descriptor 17

Waiting for new connection ...
Connection established with socket descriptor 19

Waiting for new connection ...
Connection established with socket descriptor 22

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 14

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 17

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 14

Waiting for new connection ...
Connection established with socket descriptor 16

Waiting for new connection ...
Connection established with socket descriptor 13

Waiting for new connection ...
Connection established with socket descriptor 19

Waiting for new connection ...



II) client side console:

./fire_clients.sh 

Compiling...
Firing client requests

client_1's request queued
client_4's request queued
client_7's request queued
client_5's request queued
client_8's request queued
client_2's request queued
client_22's request queued
client_27's request queued
client_26's request queued
client_21's request queued
client_9's request queued
client_23's request queued
client_33's request queued
client_31's request queued
client_14's request queued
client_34's request queued
client_25's request queued
client_13's request rejected
client_48's request queued
client_20's request rejected
client_6's request rejected
client_36's request rejected
client_50's request rejected
client_30's request rejected
client_12's request rejected
client_3's request rejected
client_35's request rejected
client_28's request rejected
client_38's request rejected
client_32's request rejected
client_43's request rejected
client_42's request rejected
client_44's request rejected
client_39's request rejected
client_45's request rejected
client_10's request rejected
client_46's request rejected
client_17's request rejected
client_40's request rejected
client_16's request rejected
client_19's request rejected
client_24's request rejected
client_49's request rejected
client_15's request rejected
client_29's request rejected
client_18's request rejected
client_41's request rejected
client_47's request rejected
client_37's request rejected
client_11's request rejected

III) execution results (i.e the output/errors on executing client requests by dispatcher) are present in execution_results directory. They can be seen below (results are written for each thread)

38265:
Successfully Executed cos(0.0) = 1.000000
Successfully Executed cos(0.0) = 1.000000

38266:
Successfully Executed cos(0.0) = 1.000000
Successfully Executed cos(0.0) = 1.000000
Successfully Executed cos(0.0) = 1.000000

38267:
Successfully Executed cos(0.0) = 1.000000
Successfully Executed cos(0.0) = 1.000000

38268:
Successfully Executed cos(0.0) = 1.000000
Successfully Executed cos(0.0) = 1.000000

38269:
MEMORY LIMIT EXCEEDED
Successfully Executed cos(0.0) = 1.000000

38270:
Successfully Executed cos(0.0) = 1.000000
Successfully Executed cos(0.0) = 1.000000

38271:
MEMORY LIMIT EXCEEDED
OPEN FILES LIMIT EXCEEDED
Successfully Executed cos(0.0) = 1.000000

38272:
MEMORY LIMIT EXCEEDED
Successfully Executed cos(0.0) = 1.000000

************
As can be verified, 18 requests were accepted, and for each of those the output is present at either of the 8 threads.
