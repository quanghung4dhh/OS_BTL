#ifndef RTOS_H
#define RTOS_H
#include <stdint.h>

#define MAX_TASKS 2
#define STACK_SIZE 128

typedef struct {
    uint32_t *strp; // Lưu giá trị thanh ghi SP (PSP) của Task khi Task bị tạm dừng (Khối này quan trọng để context switch)
} TCB;

extern TCB tasks[MAX_TASKS];
extern int currentTask;
extern uint32_t stack[MAX_TASKS][STACK_SIZE];

// Sửa lại cú pháp con trỏ hàm cho đúng chuẩn C
void create_task_stack(void (*task)(void), int id);
void start_schedule(void);   // Sửa tên hàm cho giống trong main.c

#endif
