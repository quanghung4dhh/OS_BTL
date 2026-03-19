#include <stdint.h>

/* Địa chỉ thanh ghi dữ liệu UART0 của bo mạch LM3S6965 */
volatile uint32_t * const UART0_DR = (uint32_t *)0x4000C000;

/* Hàm in một chuỗi ký tự ra cổng UART */
void print_uart(const char *str) {
    while (*str != '\0') {
        *UART0_DR = (uint32_t)(*str); /* Ghi từng ký tự vào thanh ghi */
        str++;                        /* Chuyển sang ký tự tiếp theo */
    }
}

int main(void) {
    /* Cất tiếng khóc chào đời! */
    print_uart("   HELLO OS WORLD! BOOTING SUCCESS!   \n");
    print_uart("Kernel is running...\n");

    volatile int os_tick = 0; 
    
    while (1) {
        os_tick++;
        /* OS đang chạy ổn định trong vòng lặp vô hạn */
    }
    
    return 0;
}