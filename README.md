# Requirements:
1. Ubuntu WSL
2. GCC (arm-none-eabi-gcc)
3. make
4. QEMU

# Setup:
## Install Ubuntu WSL
```bash
wsl --install -d Ubuntu
```
## Install other requirements (With Ubuntu terminal):
```bash
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi make qemu-system-arm
```
# Make file and run OS (With Ubuntu terminal):
```bash
git clone https://github.com/quanghung4dhh/OS_BTL.git
cd OS_BTL
make run
```
