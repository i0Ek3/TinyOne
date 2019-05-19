# TinyOne

A simple FTP server implement by C.

## Basic Introduction

- File Transfer Protocol
- Working in the application layer of the TCP/IP protocol family
- FTP will establish two connections, separate the command from the data
- Transfer file model
    - PORT: client -> PORT -> server, port > 1024
    - PASV: client -> PASV -> server, port < 1024

## FTP Process

- Launch FTP
- Establish control connection
- Establish a data connection and transfer files
- Close FTP


## Socket Programming Process

- Socket Client
    - use socket() to create a Socket
    - use connect() to connect server
    - use write() and read() to communicate with each other
    - use close() to close Socket
- Socket Server
    - use socket() to create a Socket
    - use bind() to bind Socket
    - use listen() to listen Socket
    - use accept() to recieve request
    - use write() and read() to communicate with each other
    - use close() to close Socket


## Architect

```Shell
.
├── README.md
├── base
│   ├── to_base.c
│   └── to_base.h
├── bin
│   ├── toc
│   └── tos
├── client
│   ├── makefile
│   ├── to_client.c
│   └── to_client.h
├── pic
│   ├── snapshot.png
│   └── structure.png
└── server
    ├── auth.txt
    ├── makefile
    ├── tmp.txt
    ├── to_server.c
    └── to_server.h
```

## How-To

```Shell
// build
$ git clone https://github.com/i0Ek3/TinyOne
$ cd TinyOne
$ cd client ; make
$ cd server ; make

// run
$ ./tos port
$ ./toc ip port

or run toc and tos directly under /bin.

// login
username: admin   username: admin
password: admin   password: 

//commands
list or ls
get or download
put or upload
quit or q
```

## Snapshot

![](https://github.com/i0Ek3/TinyOne/blob/master/pic/snapshot.png)

## Issue

- Execute tos and toc under /bin will appear "No such file or directory" error, cause of server cannot locate auth.txt. 

![](https://github.com/i0Ek3/TinyOne/blob/master/pic/404.png)


## To-Do

- [x] More commands support
- [x] Breakpoint resume
- [x] Server-side synchronization display

## References

- [FILE TRANSFER PROTOCOL](https://www.w3.org/Protocols/rfc959/)
- [使用 Socket 通信实现 FTP 客户端程序](https://www.ibm.com/developerworks/cn/linux/l-cn-socketftp/)

