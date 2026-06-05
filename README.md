# Requirements:
1. Ubuntu WSL
2. GCC (arm-none-eabi-gcc)
3. make
4. QEMU

# Setup:
## Install Ubuntu WSL (If you are using Windows)
```bash
wsl --install -d Ubuntu
```
### Then restart your computer
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
# Tasks done:
1. Boot OS
2. Systick
3. Scheduling and Context switch with 2 dummy tasks task0 and task1
4. Mutex Lock
