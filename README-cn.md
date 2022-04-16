# tinyRPC
[English version](README.md)

Pivot Talent Plan-tinyRPC 项目（仍在开发中）

## 介绍

Pivot Talent Plan-tinyRPC 旨在构建一个功能完备的微型RPC（Remote Procedure Call）框架。

完成本实验后，你能学会linux平台下的网络编程，这些经验也许能在校招中帮到你。

## 前置要求

请务必使用以下系统环境：
* Linux 版本：Ubuntu 20.04 LTS
* cmake 版本 >= 3.22
* gcc/g++ 版本 >= 8.0 或 clang/clang++ 版本 >= 13.0
* 操作系统架构：amd64 aka x86_64
* CPU：支持 AVX 的Intel CPU（通常skylake架构之后都支持）
> 如果你使用其他系统环境，则**不能**保证tinyRPC能正确编译和正常运行。
## 整体结构

整个项目分为几个小实验，在每个实验中只需要在给定的框架中添加你的代码。然后运行测试来观察自己的代码逻辑是否正确。

* [实验0](lab0/lab0_cn.md)
    * 安装开发工具和环境配置
* [实验1](lab1/lab1-cn.md)
    * 使用 epoll 进行 linux 网络编程。
