#include "rtos.h"
extern void schedule(void);
__attribute__((naked)) void PendSV_Handler(void)
{
     __asm volatile(
        // Lưu stack của task vào
        "MRS R0, PSP                \n"
        "STMDB R0!, {R4-R11}        \n" // Sau lệnh này R0 đang lưu trữ R4.
        // Tính toán ra địa chỉ của TCB task hiện tại.
        "LDR R1, =tasks             \n"
        "LDR R2, =currentTask       \n"
        "LDR R3, [R2]               \n"
        "LSL R3, R3, #2             \n"
        "ADD R1, R1, R3             \n"
    	// Lưu giá trị R0 vào ô nhớ R1 đang trỏ tới.
        "STR R0, [R1]               \n"

        /* --- SỬA Ở ĐÂY: BẢO VỆ LR --- */
        "PUSH {LR}                  \n" // Cất giá trị EXC_RETURN (0xFFFFFFFD) vào Main Stack
        "BL schedule                \n" // Nhảy sang hàm C chọn Task mới (sẽ ghi đè LR)
        "POP {LR}                   \n" // Trả lại giá trị EXC_RETURN gốc cho LR để thoát ngắt
        /* ---------------------------- */

        "LDR R1, =tasks             \n"
        "LDR R2, =currentTask       \n"
        "LDR R3, [R2]               \n"
        "LSL R3, R3, #2             \n"
        "ADD R1, R1, R3             \n"
        "LDR R0, [R1]               \n"

        "LDMIA R0!, {R4-R11}        \n" // Khi nạp xong đến R11, R0 tự động tăng tiến lên và dừng ở vị trí tiếp theo trên Stack — chính là nơi đang chứa R0-R3, R12, LR, PC, xPSR (các thanh ghi do phần cứng quản lý)
        "MSR PSP, R0                \n"
        "BX LR                      \n"
    );
}
