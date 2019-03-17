# TinyOne

A simple FTP server implement by C.

## Core

- FTP protocol
    - Mode:
        - PORT: client -> PORT -> server, port > 1024
        - PASV: client -> PASV -> server, port < 1024
    - Commands:
        - USER, PASS, SIZE, CWD, PASV, PORT, RETR, STOR, REST, QUIT
    - Response Code
    - Socket Programming
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
    - EOL

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
- [x] Commands complementation

## References

- [FILE TRANSFER PROTOCOL](https://www.w3.org/Protocols/rfc959/)
- [使用 Socket 通信实现 FTP 客户端程序](https://www.ibm.com/developerworks/cn/linux/l-cn-socketftp/)

