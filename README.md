# Project Description
This is a simple HTTP web-server and client. Its has been written in C with the POSIX library in mind. The HTTP protocol that this server follows is the 1945 HTTP 1.0 RFC. The server currently handles POST/GET/HEAD requests from the client.

## How to run the Client
+ Step one gcc -o client client.c
+ ./client [GET|HEAD|POST] <path> [Server IP] [port]

## Steps to run the server
+ If we are running the server on the default port. Remember to set the file to admin / root priviledges.
+ otherwise, this is the follow this convention to run the server ./server [-i ip address ] [-p port]

## Technologies used
+ Database: nbdm database.
+ written in C with POSIX libraries in mind.
