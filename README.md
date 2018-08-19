# TCP-Web-Server
Learning socket programming by designing a TCP web server in C

Outline/Summary:
1. Set up a TCP protocol connection (create a socket)
2. Accept connections over this socket
3. Read HTTP requests
4. Fetch the requested file
5. Create and send a HTTP response with the correct body (e.g. including a requested file)

additional requirements: 
- if requested file not found, send 404 Not Found response
- only need to be able to handle GET requests (for now, but will improve later)
- server should wait in a infinite loop for requests

