# Lab0

In this piece you will install development tools and configure the environment step by step.

## Dev tools Installation

you need install `gcc/g++`, `cmake`, `git`(optional) on your linux.

```bash
sudo apt install build-essential
sudo apt install git
sudo apt install cmake
```

After executing those commands successfully, you are ready to go :)

## Env configuration
All the dependency are installed and configured automatically by cmake, see [CMakeLists.txt](../CMakeLists.txt) for detail.
But you can still try it with those following commands.

**Check your OS & CPU Architecture support**
ensure your architecture supports AVX instructions.
```bash
# for linux
cat /proc/cpuinfo | grep avx

# for OS X
sysctl -a | grep machdep.cpu.features | grep AVX
```
**Install googletest**
```bash
wget -o googletest.zip https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
unzip googletest.zip
mkdir googletest/build && cd googletest/build
cmake ..
make && make install
```

**Install googlebechmark**
```bash
wget -o googlebenchmark.zip https://github.com/google/benchmark/archive/refs/tags/v1.6.1.zip
unzip googlebenchmark.zip
mkdir googlebenchmark/build && cd googlebenchmark/build
cmake ..
make && make install
```
> 🌟If you have trouble accessing github, just replace the domain with `hub.fastgit.org`. It's commonplace in mainland China, so take it easy and enjoy～

**NOTE**:You might need `sudo` to execute those commands.