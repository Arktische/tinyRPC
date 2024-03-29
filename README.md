# tinyRPC

tinyRPC networking lab(**STILL ON DEVELOPING**).

## Introduction

The Pivot Talent Plan networking lab builds a RPC(Remote Procedure Call) framework with variable application layer
protocol support.

After finishing this lab, you'll have an overview of networking programming under linux platform. This may help you in
interview especially for campus recruiting.

## Prerequisites

Be sure to use the following environment：
* cmake version >= 3.22
* gcc/g++ version >= 11.0 or clang/clang++ version >= 11.0
* OS arch: amd64 aka x86_64
* CPU: Intel CPU with AVX support (commonly after skylake)

> Ubuntu 22.04 satisfy those requirements by default package manager `apt`

> If you use other platforms, the project is **NOT** guaranteed to compile and run correctly.
## Architecture

The whole project is divided into few steps, in each piece you will write your code into the skeleton code.

* [lab0](lab0/lab0.md)
    * installation of development tools & env configuration
* [lab1](lab1/lab1.md)
    * basic practice of linux networking programming with epoll.
* [net](net/net.md)
    * networking library with epoll support
* [net2](net2/net2.md)
    * yet another networking library with io_uring support.

## Open Source License Declaration
[dota-benchmark](https://github.com/miloyip/dtoa-benchmark/blob/master/license.txt)
[tinyrpc](https://github.com/Gooddbird/tinyrpc/blob/main/LICENSE)
