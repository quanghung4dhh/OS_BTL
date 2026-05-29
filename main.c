#include <stdint.h>

/* Địa chỉ thanh ghi dữ liệu UART0 của bo mạch LM3S6965 */
volatile uint32_t* const UART0_DR = (uint32_t*)0x4000C000;
#define SYSTICK_CTRL (*((volatile uint32_t*)0xE000E010)) /* Điều khiển */       // Thanh ghi điều khiển của ngắt đồng hồ
#define SYSTICK_LOAD (*((volatile uint32_t*)0xE000E014)) /* Giá trị nạp lại */  // Thanh ghi chứa con số bắt đầu, sẽ đếm lùi
#define SYSTICK_VAL (*((volatile uint32_t*)0xE000E018)) /* Giá trị hiện tại */  // Thanh ghi hiển thị giá trị đếm hiện tại
#define ICSR (*((volatile uint32_t*)0xE000ED04))                                // Thanh ghi để kích hoạt PendSV
volatile uint32_t system_ticks = 0;                                             // Biến đếm tổng global để đồng bộ

/* Hàm in một chuỗi ký tự ra cổng UART */
void print_uart(const char* str) {
  while (*str != '\0') {
    *UART0_DR = (uint32_t)(*str); /* Ghi từng ký tự vào thanh ghi */
    str++;                        /* Chuyển sang ký tự tiếp theo */
  }
}

/* Hàm in một số nguyên ra UART */
void print_int(uint32_t num) {
  if (num == 0) {
    print_uart("0");
    return;
  }
  char buf[10];  // 10 chữ số vì kiểu số nguyên 32 bit là 4.294.967.295, có đúng 10 chữ số
  int i = 0;
  while (num > 0) {
    buf[i++] = '0' + (num % 10);  // Tách số (chia 10 liên tục) để lấy chữ số và in ra từng chữ số , '0' được cộng thêm vào để lấy mã ASCII của chữ số đó
    num /= 10;
  }
  while (i > 0) {
    *UART0_DR = (uint32_t)buf[--i];  // Sau khi chia xong thì lấy ngược lại chữ số đã lưu vào buffer
  }
}

/* ================================================================= */
/* 1. KHỐI ĐIỀU KHIỂN TIẾN TRÌNH (PCB & STACK)                       */
/* ================================================================= */
#define STACK_SIZE 256
uint32_t task0_stack[STACK_SIZE];
uint32_t task1_stack[STACK_SIZE];

// TCB_t lưu trữ Con trỏ ngăn xếp (SP) của mỗi Task
typedef struct {
  uint32_t* sp;
} TCB_t;

TCB_t tasks[2];
volatile int current_task = 0;
volatile int next_task = 0;
/* ================================================================= */
/* 2. KHỞI TẠO STACK GIẢ (DUMMY STACK FRAME) CHO TASK MỚI            */
/* ================================================================= */
void Task_Init(int task_id, void (*task_func)(void), uint32_t* stack, uint32_t stack_size) {
  uint32_t* sp = &stack[stack_size];

  // Khung ngăn xếp chuẩn phần cứng (Hardware Stack Frame)
  *(--sp) = 0x01000000;           // xPSR: Đặt bit T (Thumb mode) - Bắt buộc
  *(--sp) = (uint32_t)task_func;  // PC: Trỏ tới hàm thực thi của Task
  *(--sp) = 0xFFFFFFFD;           // LR: Mã EXC_RETURN để quay lại dùng PSP
  *(--sp) = 0;                    // R12
  *(--sp) = 0;                    // R3
  *(--sp) = 0;                    // R2
  *(--sp) = 0;                    // R1
  *(--sp) = 0;                    // R0

  // Khung ngăn xếp phần mềm (Software Stack Frame) do PendSV quản lý
  // Lưu 8 thanh ghi từ R4 đến R11
  for (int i = 0; i < 8; i++) {
    *(--sp) = 0;
  }

  tasks[task_id].sp = sp;  // Lưu lại đỉnh Stack vào TCB
}

/* ================================================================= */
/* 3. BỘ LẬP LỊCH THỜI GIAN THỰC (ROUND-ROBIN SCHEDULER)             */
/* ================================================================= */
void SysTick_Handler(void) {
  // Thuật toán Round-Robin: Luân phiên 0 -> 1 -> 0 -> 1...
  next_task = (current_task + 1) % 2;

  system_ticks++;  // Tăng biến đếm của hệ thống
  // Ghi vào bit 28 của thanh ghi ICSR để kích hoạt ngắt PendSV
  ICSR |= (1 << 28);
}

// Hàm delay dùng chung trong hệ thống
void delay_ticks(uint32_t ticks) {
  uint32_t start = system_ticks;
  while ((system_ticks - start) < ticks) {
    // Không làm gì — chờ OS tick trôi qua
    // CPU vẫn chạy bình thường, SysTick vẫn ngắt được
  }
}

/* ================================================================= */
/* 4. CHUYỂN ĐỔI NGỮ CẢNH (CONTEXT SWITCH) - MÃ ASSEMBLY             */
/* ================================================================= */
/* Thuộc tính 'naked' cấm GCC tự động sinh mã push/pop hàm C,
   đảm bảo ta kiểm soát 100% thanh ghi bằng Assembly */
__attribute__((naked)) void PendSV_Handler(void) {
  __asm volatile(
      "MRS R0, PSP \n\t"              // Đọc Process Stack Pointer hiện tại
      "CBZ R0, restore_context \n\t"  // Nếu PSP = 0 (lần boot OS đầu tiên), bỏ qua bước lưu

      /* LƯU NGỮ CẢNH TASK CŨ */
      "STMDB R0!, {R4-R11} \n\t"  // Đẩy R4-R11 vào Stack của Task cũ
      "LDR R1, =current_task \n\t"
      "LDR R2, [R1] \n\t"  // R2 = ID của Task đang chạy
      "LDR R3, =tasks \n\t"
      "LSL R4, R2, #2 \n\t"    // Dịch bit (x4) vì kích thước con trỏ là 4 byte
      "STR R0, [R3, R4] \n\t"  // Cất SP mới vào tasks[current_task].sp

      /* NẠP NGỮ CẢNH TASK MỚI */
      "restore_context: \n\t"
      "LDR R1, =next_task \n\t"
      "LDR R2, [R1] \n\t"  // R2 = ID của Task tiếp theo
      "LDR R5, =current_task \n\t"
      "STR R2, [R5] \n\t"  // Cập nhật current_task = next_task

      "LDR R3, =tasks \n\t"
      "LSL R4, R2, #2 \n\t"    // Dịch bit (x4)
      "LDR R0, [R3, R4] \n\t"  // R0 = tasks[next_task].sp (Đọc SP của Task mới)

      "LDMIA R0!, {R4-R11} \n\t"  // Lấy R4-R11 từ Stack của Task mới ra CPU
      "MSR PSP, R0 \n\t"          // Ghi ngược lại SP mới vào thanh ghi PSP

      "LDR LR, =0xFFFFFFFD \n\t"  // Bắt buộc: Báo CPU quay lại chế độ Thread dùng PSP
      "BX LR \n\t"                // Trở về từ ngắt, CPU sẽ tự bung nốt R0-R3, PC
  );
}

/* ================================================================= */
/* 5. CÁC TIẾN TRÌNH APP (USER TASKS)                                */
/* ================================================================= */
void Task0_Run(void) {
  int count0 = 0;  // Biến đếm của riêng Task 0
  while (1) {
    print_uart("Task 0 is running...");
    print_int(count0++);  // In ra con số thực tế
    print_uart("\n");
    delay_ticks(10);  // delay tạo hiệu ứng
  }
}

void Task1_Run(void) {
  int count1 = 0;  // Biến đếm của riêng Task 0
  while (1) {
    print_uart("\tTask 1 is running...");
    print_int(count1++);  // In ra con số thực tế
    print_uart("\n");
    delay_ticks(10);  // delay tạo hiệu ứng
  }
}

/* --- 3. CẤU HÌNH NHỊP TIM SYSTICK --- */
void SysTick_Init(uint32_t ticks) {
  SYSTICK_CTRL = 0;                                          /* Tắt SysTick để cài đặt an toàn */
  SYSTICK_LOAD = ticks - 1; /* Cài đặt giá trị đếm ngược */  // Phải trừ đi 1 vì máy tính đếm cả số 0
  SYSTICK_VAL = 0;                                           /* Xóa bộ đếm hiện tại về 0 */

  /* Bật SysTick: Bit 0 (Enable), Bit 1 (Tạo ngắt), Bit 2 (Dùng Clock CPU) */
  // SYSTICK_CTRL = (1 << 0) | (1 << 1) | (1 << 2);
  // Bật công tắc cho thanh ghi điều khiển
  // 1 << 0 : Bật nguồn cho Systick
  // 1 << 1 : Tạo ngắt khi đếm đến 0
  // 1 << 2 : Dùng xung clock với tần số của CPU là 12MHz
  SYSTICK_CTRL = 0x07;
}

/* --- 4. HÀM PHỤC VỤ NGẮT (INTERRUPT HANDLER) --- */
/* Cứ mỗi khi SysTick đếm về 0, CPU sẽ VỨT BỎ MỌI VIỆC ĐANG LÀM để nhảy thẳng vào đây */
// volatile uint32_t system_ticks = 0;

// void SysTick_Handler(void) {
//   system_ticks++; /* Tăng biến đếm tổng của OS */

//   /* Cứ mỗi 1000 lần ngắt (1 giây), in ra màn hình số giây đã trôi qua */
//   if (system_ticks % 1000 == 0) {
//     print_uart("Tick: ");
//     print_int(system_ticks / 1000);
//     print_uart(" seconds passed \n");
//     if (system_ticks % 10000 == 0) {
//       print_uart("Horray!!! \n");
//     }
//   }
// }

int main(void) {
  /* Cất tiếng khóc chào đời! */
  print_uart("   HELLO OS WORLD! BOOTING SUCCESS!   \n");
  print_uart("Kernel is running...\n");

  // 1. Tạo 2 Task và cấp phát Stack
  Task_Init(0, Task0_Run, task0_stack, STACK_SIZE);
  Task_Init(1, Task1_Run, task1_stack, STACK_SIZE);
  print_uart("Scheduler: Tasks Initialized.\n");

  // 2. Gán PSP = 0 để PendSV biết đây là lần chuyển ngữ cảnh đầu tiên
  __asm volatile("MSR PSP, %0" : : "r"(0));

  volatile int os_tick = 0;

  // 3. Khởi tạo SysTick timer
  SysTick_Init(12000 * 50);  // CPU ảo của QEMU chạy ở tần số 12MHz, 12000 tương ứng đúng với 1ms

  // 4. Chủ động gọi PendSV lần đầu tiên để nạp Task 0 lên chạy
  ICSR |= (1 << 28);
  // print_uart("Tick started! CPU entering Idle Mode...\n\n");

  while (1) {
    os_tick++;
    /* OS đang chạy ổn định trong vòng lặp vô hạn */
  }

  return 0;
}