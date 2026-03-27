#include <stdint.h>

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
extern void SysTick_Handler(void); //Khai báo hàm SysTick

void Reset_Handler(void) {
    /* Copy dữ liệu từ Flash sang RAM */
    uint32_t *src = &_sidata;
    uint32_t *dest = &_sdata;
    while (dest < &_edata) {
        *dest++ = *src++;
    }

    /* Xóa vùng nhớ .bss (đặt bằng 0) */
    dest = &_sbss;
    while (dest < &_ebss) {
        *dest++ = 0;
    }

    /* Chuyển quyền điều khiển cho Hệ điều hành của bạn */
    main();

    /* Bẫy an toàn nếu main() kết thúc */
    while (1);
}

/* Vector Table bắt buộc của kiến trúc ARM Cortex-M */
__attribute__((section(".isr_vector")))
uint32_t *vector_table[] = {
    (uint32_t *)&_estack,
    (uint32_t *)Reset_Handler,
    0, 0, 0, 0, 0, 0, 0, 0,     /* Vị trí 2-9: Dự trữ */
    0, 0, 0, 0, 0,              /* Vị trí 10-14: Các lỗi hệ thống khác */
    (uint32_t *)SysTick_Handler /* Vị trí 15: SysTick Timer! Đây là vị trí
                                  ngắt đồng hồ theo tài liệu lõi kiến trúc của ARM Cotex-M3 */
};