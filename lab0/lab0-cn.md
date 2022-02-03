# 实验0

在这个小节中将会安装必要的开发工具并配置好环境。

## 开发工具

需要安装`gcc/g++`编译器或者`clang/clang++`编译器、`cmake`构建工具、`git`版本控制工具（通常系统自带）。

```bash
sudo apt install build-essential
sudo apt install git
sudo apt install cmake
```

执行完这些linux命令后如果没有报错，则说明安装成功，可以开始正式做实验了。

## 环境配置
所有的依赖项都在项目根目录下的CMakeLists.txt中安装并配置好了，理论上不需要额外操作。如果你想体验配置的过程，或者cmake配置过程中报错了，可以执行下面的命令。

**检查OS和AVX指令支持**

```bash
# 输出不为空则说明是支持的
cat /proc/cpuinfo | grep avx
```

**安装googletest**
```bash
wget -o googletest.zip https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
unzip googletest.zip
mkdir googletest/build && cd googletest/build
cmake ..
make && make install
```

**安装googlebenchmark**
```bash
wget -o googlebenchmark.zip https://github.com/google/benchmark/archive/refs/tags/v1.6.1.zip
unzip googlebenchmark.zip
mkdir googlebenchmark/build && cd googlebenchmark/build
cmake ..
make && make install
```

> 🌟如果你不能顺畅访问github，可以将github域名替换为hub.fastgit.org。这在大陆已经是司空见惯的事情了，别因为这个生气～

**小贴士**：你可能需要sudo权限来执行上述的命令
