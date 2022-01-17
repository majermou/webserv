# webserv 
a simple HTTP server in c++
# webserv-notes
### request
example of request:
```
GET <sp> <Document Requested> <sp> HTTP/1.0 <crlf>
{<Other Header Information> <crlf>}*
<crlf>
```

- \<sp\> stands for a whitespace character
- \<crlf\> stands for a carraige return-linefeed pair. i.e. a carriage return (ascii character 13) followed by a linefeed (ascii character 10).
- http request ends with two \<crlf\>
### UDP
- UDP is a simple transport-layer protocol. The application writes a message to a UDP socket, which is then encapsulated in a UDP datagram, which is further encapsulated in an IP datagram, which is sent to the destination.
- The problem of UDP is its lack of reliability: if a datagram reaches its final destination but the checksum detects an error, or if the datagram is dropped in the network, it is not automatically retransmitted.
- No connection is established between the client and the server and, for this reason, we say that UDP provides a connection-less service.
- Each UDP datagram is characterized by a length. The length of a datagram is passed to the receiving application along with the data.
### TCP
- TCP provides a connection oriented service, since it is based on connections between clients and servers.
- TCP provides reliability. When a TCP client send data to the server, it requires an acknowledgement in return. If an acknowledgement is not received, TCP automatically retransmit the data and waits for a longer period of time.
- We have mentioned that UDP datagrams are characterized by a length. TCP is instead a byte-stream protocol, without any boundaries at all.
### Socket addresses
- IPv4 socket address structure is named sockaddr_in and is defined by including the <netinet/in.h> header.
```
struct in_addr{
in_addr_t s_addr; /*32 bit IPv4 network byte ordered address*/
};


struct sockaddr_in {
   uint8_t sin_len; /* length of structure (16)*/
   sa_family_t sin_family; /* AF_INET*/
   in_port_t sin_port; /* 16 bit TCP or UDP port number */
   struct in_addr sin_addr; /* 32 bit IPv4 address*/
   char sin_zero[8]; /* not used but always set to zero */
};
```
   
### blocking I/O
- With the blocking I/O, when the client makes a connection request to the server, the socket processing that connection and the corresponding thread that reads from it is blocked until some read data appears. This data is placed in the network buffer until it is all read and ready for processing. Until the operation is complete, the server can do nothing more but wait. When the system call for reading is called, the application is blocked and the context is switched to the kernel. The kernel initiates reading - the data is transferred to the user-space buffer. When the buffer becomes empty, the kernel will wake up the process again to receive the next portion of data to be transferred.
-  in order to handle two clients with this approach, we need to have several threads, i.e. to allocate a new thread for each client connection

### Non-blocking I/O
- The difference is obvious from its name — instead of blocking, any operation is executed immediately
- Non-blocking I/O means that the request is immediately queued and the function is returned. The actual I/O is then processed at some later point.
- So when we call the recv method, it will return to the main thread. The main mechanical difference is that send, recv, connect and accept can return without doing anything at all.
- With this approach, we can perform multiple I/O operations with different sockets from the same thread concurrently. But since we don't know if a socket is ready for an I/O operation, we would have to ask each socket with the same question and essentially spin in an infinite loop (this non-blocking but the still synchronous approach is called I/O multiplexing).
- To get rid of this inefficient loop, we need polling readiness mechanism. In this mechanism, we could interrogate the readiness of all sockets, and they would tell us which one is ready for the new I/O operation and which one is not without being explicitly asked. When any of the sockets is ready, we will perform operations in the queue and then be able to return to the blocking state, waiting for the sockets to be ready for the next I/O operation.
 
### File Descriptor Masks
- Each bit in the mask represents one of all file descriptors possible. (ex fd=0 is bit 0, fd=1 is bit 1, etc.).
- The type fd_set is guaranteed to have a bit for all possible file descriptors. (it is usually implemented as an array of int).
- Always call FD_ZERO on file descriptor masks before using.

### select
```
int select(int maxfdpl, fd_set *read, fd_set *write, fd_set *exceptfd, struct timeval *timeout);
```
- returns number of descriptors set,or 0 if timeout occurs or –1 on error
- If we want to ignore one of the parameters (read, write or except), just set these to NULL.
- maxfdpl should be set to the maximum file descriptor number being used + 1.
- maxfdpl can be set to FD_SETSIZE which is the maximum possible number assigned to file descriptors.
- If we want to check what sockets are available to open a connection so that accept() won’t block we can also use select().
- A socket which has a connection pending on it, is marked as ready for reading.

### Notes
- The client-server model is one of the most used communication paradigms in networked systems. Clients normally communicates with one server at a time. From a server’s perspective, at any point in time, it is not unusual for a server to be communicating with multiple clients. Client need to know of the existence of and the address of the server, but the server does not need to know the address of (or even the existence of) the client prior to the connection being established

   
### Links
all notes are from sources below:
- [CS 60 Computer Networks](https://www.cs.dartmouth.edu/~campbell/cs60/socketprogramming.html)
- [Asynchronous programming. Blocking I/O and non-blocking I/O](https://luminousmen.com/post/asynchronous-programming-blocking-and-non-blocking)
- [Systems Programming](http://www.cs.um.edu.mt/~jcor1/SystemsProgramming/CourseMaterials/15_NonBlockingIOAndAdvancedSocketAccess.pdf)
- [Why FD_SET/FD_ZERO for select() inside of loop?](https://stackoverflow.com/questions/7637765/why-fd-set-fd-zero-for-select-inside-of-loop)
- [Should I call a member function in a constructor](https://stackoverflow.com/questions/26464122/should-i-call-a-member-function-in-a-constructor)
- [RFC 2616 response](https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html)
- [RFC 2616 request](https://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html)
- [RFC 3875 cgi](https://datatracker.ietf.org/doc/html/rfc3875#section-4)
- [Beej’s Guide to Network Programming](https://beej.us/guide/bgnet/pdf/bgnet_usl_c_1.pdf)
- [Programmation de sockets FR](https://roscas.github.io/reseau/reseau-programmation-sockets.html)
- [I/O Multiplexing: The select function](http://www.cs.rpi.edu/courses/fall06/netprog/c07.html)
