#include <stdint.h>

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
extern void SysTick_Handler(void);  // Khai báo hàm SysTick
extern void PendSV_Handler(void);   // Khai báo ngắt PendSV

void Reset_Handler(void) {
  /* Copy dữ liệu từ Flash sang RAM */
  uint32_t* src = &_sidata;
  uint32_t* dest = &_sdata;
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

// Hàm bẫy lỗi an toàn cho Hệ điều hành
void HardFault_Handler(void) {
  while (1);  // CPU sẽ kẹt ở đây thay vì sập QEMU
}

/* Vector Table bắt buộc của kiến trúc ARM Cortex-M */
__attribute__((section(".isr_vector")))
uint32_t* vector_table[] = {
    (uint32_t*)&_estack,
    (uint32_t*)Reset_Handler,
    0,                            /*Vị trí 2: Offset 0x08 - NMI*/
    (uint32_t*)HardFault_Handler, /* Vị trí 3: Offset 0x0C - Hard Fault */
    0, 0, 0, 0, 0, 0,             /* Vị trí 4-9: Dự trữ */
    0, 0, 0, 0,                   /* Vị trí 10-13: Các lỗi hệ thống khác */
    (uint32_t*)PendSV_Handler,    /* Vị trí 14: PendSV - Dùng cho Context Switch */
    (uint32_t*)SysTick_Handler    /* Vị trí 15: SysTick Timer! Đây là vị trí
                                    ngắt đồng hồ theo tài liệu lõi kiến trúc của ARM Cotex-M3 */
};