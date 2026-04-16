#include <stdint.h>

/* Địa chỉ thanh ghi dữ liệu UART0 của bo mạch LM3S6965 */
volatile uint32_t * const UART0_DR = (uint32_t *)0x4000C000;
#define SYSTICK_CTRL  (*((volatile uint32_t *)0xE000E010)) /* Điều khiển */ //Thanh ghi điều khiển của ngắt đồng hồ
#define SYSTICK_LOAD  (*((volatile uint32_t *)0xE000E014)) /* Giá trị nạp lại */ //Thanh ghi chứa con số bắt đầu, sẽ đếm lùi
#define SYSTICK_VAL   (*((volatile uint32_t *)0xE000E018)) /* Giá trị hiện tại */ //Thanh ghi hiển thị giá trị đếm hiện tại 

/* Hàm in một chuỗi ký tự ra cổng UART */
void print_uart(const char *str) {
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
    char buf[10]; //10 chữ số vì kiểu số nguyên 32 bit là 4.294.967.295, có đúng 10 chữ số
    int i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);  //Tách số (chia 10 liên tục) để lấy chữ số và in ra từng chữ số , '0' được cộng thêm vào để lấy mã ASCII của chữ số đó
        num /= 10;
    }
    while (i > 0) {
        *UART0_DR = (uint32_t)buf[--i]; //Sau khi chia xong thì lấy ngược lại chữ số đã lưu vào buffer
    }
}

/* --- 3. CẤU HÌNH NHỊP TIM SYSTICK --- */
void SysTick_Init(uint32_t ticks) {
    SYSTICK_CTRL = 0;           /* Tắt SysTick để cài đặt an toàn */ 
    SYSTICK_LOAD = ticks - 1;   /* Cài đặt giá trị đếm ngược */ //Phải trừ đi 1 vì máy tính đếm cả số 0
    SYSTICK_VAL  = 0;           /* Xóa bộ đếm hiện tại về 0 */
    
    /* Bật SysTick: Bit 0 (Enable), Bit 1 (Tạo ngắt), Bit 2 (Dùng Clock CPU) */
    SYSTICK_CTRL = (1 << 0) | (1 << 1) | (1 << 2); 
    //Bật công tắc cho thanh ghi điều khiển
    // 1 << 0 : Bật nguồn cho Systick
    // 1 << 1 : Tạo ngắt khi đếm đến 0
    // 1 << 2 : Dùng xung clock với tần số của CPU là 12MHz
    SYSTICK_CTRL = 0x07;

}

/* --- 4. HÀM PHỤC VỤ NGẮT (INTERRUPT HANDLER) --- */
/* Cứ mỗi khi SysTick đếm về 0, CPU sẽ VỨT BỎ MỌI VIỆC ĐANG LÀM để nhảy thẳng vào đây */
volatile uint32_t system_ticks = 0;

void SysTick_Handler(void) {
    system_ticks++; /* Tăng biến đếm tổng của OS */
    
    /* Cứ mỗi 1000 lần ngắt (1 giây), in ra màn hình số giây đã trôi qua */
    if (system_ticks % 1000 == 0) {
        print_uart("Tick: ");
        print_int(system_ticks / 1000);
        print_uart(" seconds passed \n");
        if (system_ticks % 10000 == 0) {
          print_uart("Horray!!! \n");
        }
    }
}


int main(void) {
    /* Cất tiếng khóc chào đời! */
    print_uart("   HELLO OS WORLD! BOOTING SUCCESS!   \n");
    print_uart("Kernel is running...\n");

    volatile int os_tick = 0; 
    SysTick_Init(12000);  //CPU ảo của QEMU chạy ở tần số 12MHz, 12000 tương ứng đúng với 1ms
    print_uart("Tick started! CPU entering Idle Mode...\n\n");
    
    while (1) {
        os_tick++;
        /* OS đang chạy ổn định trong vòng lặp vô hạn */
    }
    
    return 0;
}