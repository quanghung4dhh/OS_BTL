# Tên file đầu ra
TARGET = os_kernel

# Các công cụ
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
QEMU = qemu-system-arm

# Cờ biên dịch cho Cortex-M3
CFLAGS = -c -mcpu=cortex-m3 -mthumb -O0 -g -Wall
LDFLAGS = -nostdlib -T linker.ld

# Danh sách các file object cần tạo
OBJS = startup.o main.o

# Lệnh mặc định khi gõ "make"
all: $(TARGET).elf

# Cách tạo ra file .elf từ các file .o
$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET).elf

# Cách tạo ra file .o từ file .c
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

# Lệnh dọn dẹp khi gõ "make clean"
clean:
	rm -f *.o *.elf

# Lệnh chạy giả lập khi gõ "make run"
run: $(TARGET).elf
	$(QEMU) -machine lm3s6965evb -cpu cortex-m3 -nographic -kernel $(TARGET).elf