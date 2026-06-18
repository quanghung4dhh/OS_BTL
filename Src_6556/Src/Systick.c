#include "systick.h"
 // const mặc định trỏ vào địa chỉ thanh ghi
volatile uint32_t * const USART2_SR = (uint32_t *)0x40004400;
volatile uint32_t * const USART2_DR = (uint32_t *)0x40004404;
#define SYSTICK_CTRL (*(volatile uint32_t *)0xE000E010)
#define SYSTICK_LOAD (*(volatile uint32_t *)0xE000E014)
#define SYSTICK_VAL  (*(volatile uint32_t *)0xE000E018)
#define RCC_APB2ENR   (*(volatile uint32_t*)0x40021018)
#define GPIOC_CRH (*(volatile uint32_t*)0x40011004)
#define GPIOC_ODR (*(volatile uint32_t*)0x4001100C)


void systick_init(uint32_t ticks){
    SYSTICK_LOAD = ticks - 1;
    SYSTICK_VAL  = 0;
    SYSTICK_CTRL = 0x07; // enable + interrupt + clock
}

void print_uart(const char *str) {
    while (*str != '\0') {
        // Đợi cho đến khi thanh ghi đệm TDR trống (bit số 7 trong SR bằng 1)
        while (!(*USART2_SR & (1 << 7)));

        *USART2_DR = (uint32_t)(*str); /* Ghi từng ký tự vào thanh ghi */
        str++;                        /* Chuyển sang ký tự tiếp theo */
    }
}
void delay(uint32_t count)
{
    // Thêm volatile ở đây để Compiler bắt buộc CPU phải đếm từng số một, không được xóa
    volatile uint32_t local_count = count * 1000; // Nhân thêm 1000 vì chip 32-bit chạy rất nhanh, 500 là quá nhỏ
    while(local_count--);
}
// cau hinh chuc nang chan
void led_init(){
	// bat clock GPIOC
    RCC_APB2ENR |= (1 << 4);
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |=  (0x2 << 20);
}

void led_on(){
    GPIOC_ODR &= ~(1 << 13);
}

void led_off(){
    GPIOC_ODR |= (1 << 13);
}
