# Lab 1

[Chinese version](lab1-cn.md)

## echo server
> We assume you have basic knowledge about computer networking. If not, you might prefer to learn [this course]() first.

Echo server returns whatever message you sent to it.It's a quite simple model to test network programming. In this piece
we will learn basic knowledge about `socket` & `epoll` on Linux platform.

## socket api
> We refer to **TCP socket** for short as *socket* unless specified later

Everything is file on Unix-like system, including socket. Since TCP stream is borderless, it behaves quite different from common files when on read/write opertions.
