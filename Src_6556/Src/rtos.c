#include "rtos.h"
// Vừa phải có mảng 1 chiều để lưu trữ từng con trỏ TCB của từng task, vừa phải có mảng 2 chiều để chứa stack của từng task.
TCB tasks[MAX_TASKS];
uint32_t stack[MAX_TASKS][STACK_SIZE]; // Mảng 2 chiều này là hàng là task index, cột là ô nhớ lưu các thanh ghi PSP, LR, R0, R1,...
int currentTask = 0;

void create_task_stack(void (*task)(void), int id)
{
	// Trỏ vào vị trí đỉnh của cái được cấp phát.
    uint32_t *sp = &stack[id][STACK_SIZE - 1];
    // Lưu từ cao xuống thấp
    *(sp--) = 0x01000000;     // xPSR
    *(sp--) = (uint32_t)task; // PC
    *(sp--) = 0xFFFFFFFD;     // LR

    // R12, R3, R2, R1, R0
    for(int i = 0; i < 5; i++) *(sp--) = 0;

    // R11-R4
    for(int i = 0; i < 8; i++) *(sp--) = 0;

    tasks[id].strp = sp + 1;
}

void schedule(void)
{
    currentTask = (currentTask + 1) % MAX_TASKS;
}
