# TinyOne

A simple FTP server implement by C.

## Core

- FTP protocol
    - PORT: client -> PORT -> server
    - PASV: client -> PASV -> server
- Linux Socket Programming
    - use socket() to create a Socket
    - use connect() to connect server
    - use write() and read() to communicate with each other
    - use close() to close Socket
- Linux System Programming

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
username: admin
password: admin

//command
list
get
quit
```

## Snapshot

![](https://github.com/i0Ek3/TinyOne/blob/master/pic/snapshot.png)

## To-Do

- [x] Download big file
- [x] Upload file
- [x] More command support
- [x] Implement commands to replace system call



