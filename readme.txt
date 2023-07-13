a. Praise Olukilede 

b. 5768620607

c. Completed the EE450 socket programming project (without the extra credit)

d. serverM.cpp - receives list of usernames contained in serverA and serverB. Also facilitates communication between client and backend servers by getting usernames from the client and sending the request to the needed servers and sending the final availiabity time to the client

   serverB.cpp - reads data from b.txt and store the result in a data structure. Upon receiving a list of usernames from the main 				server, it computes the time availability that works for the list of users. It then sends the result back to the main server

   serverA.cpp - similar to serverB.cpp but reads data from a.txt

   client.cpp - prompts the user to enter a list of usernames single space seperated. It sends this input to the main server and 			  displays the reply to the user

   timeslotsFuncs.cpp - Implemenation of shared functions used among the various servers

   timeslotsFuncs.h - header file used to share necessary data structures and functions used in the servers

   Makefile - generates .o/executatble files and also performs clean up if called

e. After the backend servers read the data in the txt files and stores it in the data structure. It then sends each of the 			username to the main server in the form of <username>;<AorB> - such as john;B if john is a username that exist on serverB. 		After sending all the usernames it sends end;<AorB> to let the main server know that it is done transferring usernames.

	The client sends a list of username single space delimeted to the main server. The main server then goes through the list received and sends a comma delimeted list to the respective servers after checking if the usernames exist on that server. 

	The servers after finding the availaibity times sends it to the main server in the form [[2,5],[6,9]];<AorB>

	The main server then combimes the availabilty recieved and sends a reply to the client in the form specified in the project document
