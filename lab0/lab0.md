# Lab0
In this piece you will install development tools and configure the environment step by step.

## Installation
you need install `gcc/g++`, `cmake`, `git`(optional) on your linux.
```bash
sudo apt install build-essential
sudo apt install git
sudo apt install cmake
sudo apt install googletest
```
After executing those commands successfully, you are ready to go :)

## Env configuration
You can execute `setup.sh` to configure all the dev environment. Or you can follow the next few steps:
```bash
sudo apt install googletest
cd /usr/src/googletest/ && mkdir build && cd build
cmake ../
make && make install
```
**NOTE**:You might need `sudo` to execute those commands.