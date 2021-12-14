// Operating Systems: sample code  (c) Tomáš Hudec
// Communication: Sockets
// Single-Client Server: echo
// IPv4/IPv6
// getaddrinfo(3), gai_strerror(3), freeaddrinfo(3), inet_ntop(3),
// socket(2), setsockopt(2), bind(2), listen(2), accept(2), recv(2)/read(2), send(2)/write(2), close(2)

// Modified: 2016-11-28, 2021-12-01

#include <stdio.h>			// basic I/O routines
#include <sys/types.h>			// standard system types
#include <netinet/in.h>			// internet address structures
#include <sys/socket.h>			// socket interface functions
#include <netdb.h>			// host to IP resolution
#include <arpa/inet.h>			// convert address to a string: inet_ntop
#include <stdlib.h>			// exit
#include <string.h>			// memset, memcpy
#include <unistd.h>			// read, write
#include <signal.h>

#define	SERVICE		"5665"		// port of our echo server
// Note: ports below 1024 are RESERVED (for super-user), see RFC 1700 and IPPORT_RESERVED in /usr/include/netinet/in.h

#define	QUEUE_SIZE	5		// how many pending connections to keep in the queue

#define	BUFLEN		1024		// buffer length

struct addrinfo *servinfo = NULL;	// host-to-IP translation

// this should be called upon exit
void cleanup_servinfo(void)
{
	if (NULL != servinfo) {
		freeaddrinfo(servinfo);
		servinfo = NULL;
	}
}

// get the pointer to the internet address member in the socket struct
void *get_in_addr(struct sockaddr *addr)
{
	switch (addr->sa_family) {
	case AF_INET:	// IPv4
		return &((struct sockaddr_in *)addr)->sin_addr;
	case AF_INET6:	// IPv6
		return &((struct sockaddr_in6 *)addr)->sin6_addr;
	default:
		return NULL;
	}
}

// get the port member in the socket struct
in_port_t get_in_port(struct sockaddr *addr)
{
	switch (addr->sa_family) {
	case AF_INET:	// IPv4
		return ((struct sockaddr_in *)addr)->sin_port;
	case AF_INET6:	// IPv6
		return ((struct sockaddr_in6 *)addr)->sin6_port;
	default:
		return 0;
	}
}

int main(int argc, char *argv[])
{
	int rc;				// system calls return value
	int s;				// socket descriptor
	int cs;				// new connection's socket descriptor
	struct sockaddr_storage client;	// client's internet address info
	socklen_t client_addr_size;	// size of client's address struct
	char c_addr[INET6_ADDRSTRLEN];	// client's address as a string
	in_port_t c_port;		// client's port number
	struct addrinfo hints;		// host-to-IP and service translation
	struct addrinfo *si;		// for traversing the linked list of addresses
	int yes = 1;			// for socket releasing
	char buf[BUFLEN + 1];		// buffer for incoming data
	ssize_t r;			// the number of bytes read by recv(2)/read(2)
	ssize_t w;			// the number of bytes written by send(2)/write(2)

	// print id
	// configuration of SIGCHLD signal action, so the child processes won't turn into ZOMBIE state
	struct sigaction sa_config; 	// structure used to carry the configuration
	sa_config.sa_handler = SIG_DFL;		// sa_handler - specify the handler of signal (SIG_DFL - default handler is used)
	sa_config.sa_flags = SA_NOCLDWAIT;	// sa_flags - affect behavior of the signal (SA_NOCLDWAIT - don't create zombie processes or status information on child termination)
	// change the SIGCHLD signal action
		// SIGCHLD - signal that child processes created by fork send on termination to parent
		// &sa_config - configuration of signal actions behaviour
		// NULL - if non-NULL value is provided -> the previous action configuration is saved to it (we ignore it)
	sigaction(SIGCHLD, &sa_config, NULL); 


	// set the listening address
	memset(&hints, 0, sizeof(hints));	// clear out the struct
	hints.ai_family = AF_UNSPEC;		// use both IPv4/IPv6 (AF_INET for IPv4, AF_INET6 for IPv6)
	hints.ai_socktype = SOCK_STREAM;	// TCP stream socket
	hints.ai_flags = AI_PASSIVE;		// to fill in the local IP (if the first arg is NULL)
	
	char * port;
	// check if valid process argument is provided
	// and is greater than value 1024 (ports with lesser values are reserved by system -> bind would fail with permission denied)
	// upper limit is not validated, as if we choose greater port value than 65535, system will choose (probably) randomly port for us
	if (argc > 1 && atoi(argv[1]) > 1024) {
		port = argv[1]; // use the provided argument as port
	} else {
		port = SERVICE; // if the conditions are not satisfied, then use the default port 5665
	}
	
	rc = getaddrinfo(NULL, port, &hints, &servinfo);	// set the IP and port numbers
	if (rc) {
		// print the error message using gai_strerror(3)
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
		return EXIT_FAILURE;
	}
	// servinfo should be freed upon exit using freeaddrinfo(3)
	atexit(cleanup_servinfo);

	// the servinfo from getaddrinfo is a linked list of addresses
	// several addresses are returned, if the network host is multi-homed,
	// or if the same service is available from multiple socket protocols (SOCK_STREAM and SOCK_DGRAM)
	if (servinfo->ai_next) {
		fprintf(stderr, "Multiple local addresses detected, the first one available will be used:\n");
		for (si = servinfo; NULL != si; si = si->ai_next) {
			inet_ntop(si->ai_family, get_in_addr(si->ai_addr), c_addr, sizeof(c_addr));
			c_port = ntohs(get_in_port(si->ai_addr));
			fprintf(stderr, "  IPv%d address: %s, port %d\n",
				si->ai_family == AF_INET ? 4 : 6, c_addr, c_port);
		}
	}

	// try all available addresses, use the first possible
	for (si = servinfo; NULL != si; si = si->ai_next) {
		// allocate a socket, use address family, socket type and protocol from servinfo
		s = socket(si->ai_family, si->ai_socktype, si->ai_protocol);
		if (s < 0) {
			perror("socket");
			continue;
		}

		// reuse the socket in case of the "Address already in use" error message
		rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		if (rc) {
			perror("setsockopt");
			close(s);
			continue;
		} 

		// bind the socket to the address and port
		rc = bind(s, si->ai_addr, si->ai_addrlen);
		if (rc) {
			perror("bind");
			close(s);
			continue;
		}

		break;	// socket allocating and binding was was successful
	}

	if (NULL == si) {
		fprintf(stderr, "No local address was available to bind to.\n");
		return EXIT_FAILURE;
	}

	// get real port if zero was specified
	rc = getsockname(s, si->ai_addr, &si->ai_addrlen);
	if (rc) {
		perror("getsockname");
		close(s);
		return EXIT_FAILURE;
	}

	inet_ntop(si->ai_family, get_in_addr(si->ai_addr), c_addr, sizeof(c_addr));
	c_port = ntohs(get_in_port(si->ai_addr));
	fprintf(stderr, "Using local IPv%d address: %s, port %d\n",
		si->ai_family == AF_INET ? 4 : 6, c_addr, c_port);

	// the servinfo is no more needed
	cleanup_servinfo();

	// ask the OS to listen for incoming connections
	// specify that up to QUEUE_SIZE pending connection requests will be queued by the OS
	// if we are not directly awaiting them using the accept(2), when they arrive
	rc = listen(s, QUEUE_SIZE);
	if (rc) {
		perror("listen");
		return EXIT_FAILURE;
	}

	// enter an accept-read-write-close infinite loop
	while (1) {
		// the accept(2) system call will wait for a connection
		// when one is established, a new socket will be created to form it, and
		// the client variable will hold the address of the client that connected to us
		// the s socket, will still be available for future accept(2) statements
		client_addr_size = sizeof(client);	// maximum size, accept(2) will set the actual size
		cs = accept(s, (struct sockaddr *)&client, &client_addr_size);
		if (cs < 0){			// check for errors, if any, enter accept mode again
			perror("accept: descriptor couldn't be opened");
			continue;
		}
		// convert the client address to a string and show it
		inet_ntop(client.ss_family, get_in_addr((struct sockaddr *)&client), c_addr, sizeof(c_addr));
		c_port = ntohs(get_in_port((struct sockaddr *)&client));	// convert port from network to host byte order
		fprintf(stderr, "echo server: got an IPv%d connection from %s, port %d\n",
			client.ss_family == AF_INET ? 4 : 6, c_addr, c_port);
		
		pid_t child; // PID of the child process

		// create a child process
		if ((child = fork()) == -1) {
			perror("fork: process couldn't be created");
		}

		if (child == 0) { // child process will serve requests from newly connected client
			close(s); // close parent descriptor as it won't be needed in child
		
			// we've got a new connection, do the job
			// if error on recv or send -> terminate the process
			while(1) {
				if((r = recv(cs, buf, BUFLEN, 0)) == -1){	// read the request
					perror("recv");
					return EXIT_FAILURE;
				} else if(r == 0) { // on no input we consider that client closed the connection
					close(cs); // we close client socket and end process 
					fprintf(stderr, "echo server: closed connection sd = %d\n", cs);
					return EXIT_SUCCESS;
				} else {
					if((w = send(cs, buf, r, 0)) < -1){	// send back what we've received
						perror("send");
						return EXIT_FAILURE;
					}
				}
			}
		}
		close(cs); // close new connection's descriptor as it won't be needed in parent
	}
	return EXIT_SUCCESS;
}
