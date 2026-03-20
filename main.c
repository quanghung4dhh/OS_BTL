#include <stdint.h>

/* Địa chỉ thanh ghi dữ liệu UART0 của bo mạch LM3S6965 */
volatile uint32_t * const UART0_DR = (uint32_t *)0x4000C000;
#define SYSTICK_CTRL  (*((volatile uint32_t *)0xE000E010)) /* Điều khiển */
#define SYSTICK_LOAD  (*((volatile uint32_t *)0xE000E014)) /* Giá trị nạp lại */
#define SYSTICK_VAL   (*((volatile uint32_t *)0xE000E018)) /* Giá trị hiện tại */

/* Hàm in một chuỗi ký tự ra cổng UART */
void print_uart(const char *str) {
    while (*str != '\0') {
        *UART0_DR = (uint32_t)(*str); /* Ghi từng ký tự vào thanh ghi */
        str++;                        /* Chuyển sang ký tự tiếp theo */
    }
}

/* --- 3. CẤU HÌNH NHỊP TIM SYSTICK --- */
void SysTick_Init(uint32_t ticks) {
    SYSTICK_CTRL = 0;           /* Tắt SysTick để cài đặt an toàn */
    SYSTICK_LOAD = ticks - 1;   /* Cài đặt giá trị đếm ngược */
    SYSTICK_VAL  = 0;           /* Xóa bộ đếm hiện tại về 0 */
    
    /* Bật SysTick: Bit 0 (Enable), Bit 1 (Tạo ngắt), Bit 2 (Dùng Clock CPU) */
    SYSTICK_CTRL = (1 << 0) | (1 << 1) | (1 << 2);
}

/* --- 4. HÀM PHỤC VỤ NGẮT (INTERRUPT HANDLER) --- */
/* Cứ mỗi khi SysTick đếm về 0, CPU sẽ VỨT BỎ MỌI VIỆC ĐANG LÀM để nhảy thẳng vào đây */
volatile uint32_t system_ticks = 0;

void SysTick_Handler(void) {
    system_ticks++; /* Tăng biến đếm tổng của OS */
    
    /* Cứ mỗi 1000 lần ngắt (1 giây), in ra màn hình 1 dòng */
    if (system_ticks % 1000 == 0) {
        print_uart("[SysTick] 1 Second passed! Thump... Thump...\n");
    }
}


int main(void) {
    /* Cất tiếng khóc chào đời! */
    print_uart("   HELLO OS WORLD! BOOTING SUCCESS!   \n");
    print_uart("Kernel is running...\n");

    volatile int os_tick = 0; 
    SysTick_Init(12000);
    print_uart("Heartbeat started! CPU entering Idle Mode...\n\n");
    
    while (1) {
        os_tick++;
        /* OS đang chạy ổn định trong vòng lặp vô hạn */
    }
    
    return 0;
}