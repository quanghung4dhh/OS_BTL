
#include <stdint.h>
#include "rtos.h"
#include "Systick.h"
// Tự định nghĩa khối điều khiển hệ thống SCB và ICSR theo đúng tài liệu phần cứng ARM Cortex-M3
#define SCB_ICSR          (*(volatile uint32_t *)0xE000ED04)
#define PENDSV_SET_BIT    (1 << 28)
// Tự định nghĩa các lệnh nội tại của ARM Cortex-M bằng Inline Assembly
#define __set_PSP(val)      __asm volatile ("MSR psp, %0" : : "r" (val) : )
#define __set_CONTROL(val)  __asm volatile ("MSR control, %0" : : "r" (val) : "memory")
#define __ISB()             __asm volatile ("ISB" : : : "memory")
// Địa chỉ thanh ghi bật Clock cho GPIO (RCC)
#define RCC_APB2ENR       (*(volatile uint32_t *)0x40021018)

// Địa chỉ thanh ghi cấu hình chân (CRL, CRH) và xuất dữ liệu (ODR) của Port A
#define GPIOA_CRL         (*(volatile uint32_t *)0x40010800)
#define GPIOA_CRH         (*(volatile uint32_t *)0x40010804)
#define GPIOA_ODR         (*(volatile uint32_t *)0x4001080C)

// Địa chỉ thanh ghi tương tự cho Port B
#define GPIOB_CRL         (*(volatile uint32_t *)0x40010C00)
#define GPIOB_CRH         (*(volatile uint32_t *)0x40010C04)
#define GPIOB_ODR         (*(volatile uint32_t *)0x40010C0C)
static int a = 0;
void test_gpio_high(void)
{
    // 1. Bật xung Clock cho Port A và Port B (Set bit 2 và bit 3 trên RCC_APB2ENR)
    RCC_APB2ENR |= (1 << 2) | (1 << 3);

    // 2. Cấu hình tất cả các chân từ P0 đến P15 thành Output Push-Pull (Tốc độ 50MHz)
    // Trong STM32F103, mỗi chân chiếm 4-bit cấu hình trong thanh ghi CRL và CRH.
    // Ghi 0x33333333 sẽ biến tất cả các chân thành Output Push-Pull.
    GPIOA_CRL = 0x33333333;
    GPIOA_CRH = 0x33333333;

    GPIOB_CRL = 0x33333333;
    GPIOB_CRH = 0x33333333;

    // 3. Kéo HIGH toàn bộ chân bằng cách ghi 1 vào tất cả các bit của thanh ghi ODR
    // Trừ chân nạp SWD (PA13 là SWDIO, PA14 là SWCLK) để tránh bị khóa chip dính cứng
    GPIOA_ODR = 0xFFFF & ~((1 << 13) | (1 << 14));

    GPIOB_ODR = 0xFFFF; // Port B có thể kéo HIGH hết toàn bộ
}
void test_dem_so(void){
	RCC_APB2ENR |= (1 << 2);     // Bật clock Port A
	GPIOA_CRL = 0x33333333;
	int x = 0;
	uint8_t led_code = 0b01111111;
	while (1){
	switch(x){
	case 0: led_code = 0b00111111; break; // 0x3F: Sáng A,B,C,D,E,F
	case 1: led_code = 0b00000110; break; // 0x06: Sáng B,C
	case 2: led_code = 0b01011011; break; // 0x5B: Sáng A,B,D,E,G
	case 3: led_code = 0b01001111; break; // 0x4F: Sáng A,B,C,D,G
    case 4: led_code = 0b01100110; break; // 0x66: Sáng B,C,F,G
	case 5: led_code = 0b01101101; break; // 0x6D: Sáng A,C,D,F,G
	case 6: led_code = 0b01111101; break; // 0x7D: Sáng A,C,D,E,F,G
	case 7: led_code = 0b00000111; break; // 0x07: Sáng A,B,C
	case 8: led_code = 0b01111111; break; // 0x7F: Sáng tất cả (trừ DP)
	case 9: led_code = 0b01101111; break; // 0x6F: Sáng A,B,C,D,F,G
    default : led_code = 0b00000000; break;
	}
	GPIOA_ODR = (GPIOA_ODR & 0xFF00) | led_code;
	x++;
	if (x > 9) x = 0;
	delay(1000);
	}
}
void SysTick_Handler(void)
{
    a++;
    if(a >= 7000){
        SCB_ICSR |= PENDSV_SET_BIT; // Ép bật cờ ngắt PendSV
        a = 0;
    }
}
// Sửa lại hàm SysTick_Handler dùng trực tiếp định nghĩa trên
void start_schedule(void) {
    // Cài đặt PSP (Process Stack Pointer) trỏ vào đỉnh Stack của Task 0
    __set_PSP((uint32_t)tasks[currentTask].strp);

    // Chuyển CPU sang dùng PSP thay vì MSP (Main Stack Pointer)
    __set_CONTROL(0x02);
    __ISB(); // Clear pipeline của ARM

    // Nhảy vào chạy Task đầu tiên
    void (*first_task)(void) = (void(*)(void))stack[0][STACK_SIZE - 2];
    first_task();
}
void trigger_context_switch(void)
{   //Thực hiện pend SV(chuyển sang task mới)
	 SCB_ICSR |= PENDSV_SET_BIT;
}
void task1(void){
	while(1){
		led_on();
		delay(1000);
		led_off();
		delay(1000);
	}
}
void task2(void){
	while(1){
		led_on();
		delay(500);
		led_off();
		delay(500);
	}
}

int main(void)
{
	led_init();
    create_task_stack(test_dem_so, 0);
    create_task_stack(task2, 1);
    systick_init(8000);
    start_schedule();
    // Kéo HIGH toàn bộ các chân cùng một lúc cho Port A, B, C
	// test_gpio_high();
    while(1);
}
