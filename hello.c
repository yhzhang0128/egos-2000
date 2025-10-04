void terminal_write(const char *str, int len) {
    for (int i = 0; i < len; i++) {
        //大家幾乎都是使用 QEMU 的 virt 虛擬機器作為標準平台
        //QEMU 會模擬出一塊包含 CPU、記憶體和各種 I/O 裝置的電路板。這些裝置會被分配到不同的「實體位址 (Physical Address)」上，這就是所謂的「記憶體對映 I/O (Memory-Mapped I/O)」。
        //大以下是 virt 機器一個典型的、簡化的記憶體地圖：

//起始位址	大小	裝置/區域	說明
//0x02000000	4KB	CLINT	核心本地中斷控制器 (處理計時器和軟體中斷)
//0x0C000000	~4MB	PLIC	平台級中斷控制器 (處理外部裝置中斷，如磁碟)
//0x10000000	8B	UART0	序列埠 (用於在終端機上印出文字)
//0x10001000	4KB+	VIRTIO-MMIO	VirtIO 裝置 (如磁碟控制器、網路卡)
//0x80000000	128MB+	DRAM (主記憶體)	主要記憶體的起始位置
        *(char*)(0x10000000) = str[i];
    }
}

/* Uncomment this code block
 * when implementing formatted output
 */

#include <string.h>  // for strlen() and strcat()
#include <stdlib.h>  // for itoa()
#include <stdarg.h>  // for va_start(), va_end() and va_arg()

void to_hex(int num, char *hexStr) {


    char hexChars[] = "0123456789abcdef";


    int i = 0, temp;


    while (num != 0) {


        temp = num % 16;


        hexStr[i++] = hexChars[temp];


        num /= 16;


    }


    hexStr[i] = '\0';



    for (int j = 0; j < i / 2; j++) {


        char temp = hexStr[j];


        hexStr[j] = hexStr[i - j - 1];


        hexStr[i - j - 1] = temp;


    }


}






void format_to_str(char* out, const char* fmt, va_list args) {
    for(out[0] = 0; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            strncat(out, fmt, 1);
        } else {
            fmt++;
            if (*fmt == 's') {
                strcat(out, va_arg(args, char*));
            }
            else if (*fmt == 'c') {
                char temp_char_str[2];
                // 1. 使用 int 型別取得參數，並轉換回 char
                temp_char_str[0] = (char)va_arg(args, int);
                // 2. 加上空字元結尾，使其成為一個合法的 C 字串
                temp_char_str[1] = '\0';
                // 3. 將這個只有一個字元的字串附加到輸出
                strcat(out, temp_char_str);
                //strcat(out, va_arg(args, char));
            } else if (*fmt == 'd') {
                itoa(va_arg(args, int), out + strlen(out), 10);

            }
            else if (*fmt == 'x' ){
                //itoa(va_arg(args, int), out + strlen(out), 16);
                char hexStr[10];
                int i =  (int)va_arg(args, int);
                to_hex(i, hexStr);
                char* test = &hexStr[1];
                if (hexStr[0] == '0'){
                 strcat(out, test);
                //  char* msg = "H!\n\r";
                 //    terminal_write(msg, 4);
                }
                else{
                 strcat(out, hexStr);
                   //char* msg = "T!\n\r";
                    // terminal_write(msg, 4);
                }

            }
        }
    }
}

//回傳不定參數函數的字串大小
unsigned int format_to_str_len(const char* fmt, va_list args) {
    unsigned int count = 0;
    for(; *fmt != '\0'; fmt++) {
        if (*fmt != '%') {
            count++;
        } else {
            fmt++;
            if (*fmt == 's') {
                count = count + strlen(va_arg(args, char*));

            }
      }
    }
    return count;
 }



int printf(const char* format, ...) {
    //char buf[512];
    va_list args;
    va_start(args, format);

    unsigned int len = format_to_str_len(format, args);

    char *buf = malloc(len);

    format_to_str(buf, format, args);
    va_end(args);
    terminal_write(buf, strlen(buf));

    return 0;
}


/* Uncomment this code block
 * when implementing dynamic memory allocation
 */

extern char __heap_start, __heap_end;
static char* brk = &__heap_start;
char* _sbrk(int size) {
    if (brk + size > (char*)&__heap_end) {
        terminal_write("_sbrk: heap grows too large\r\n", 29);
        return NULL;
    }

    char* old_brk = brk;
    brk += size;
    return old_brk;
}


int main() {
    char* msg = "Hello, World!\n\r";
    terminal_write(msg, 15);

    /* Uncomment this line of code
     * when implementing formatted output
     */
    /* printf("%s-%d is awesome!", "egos", 2000); */

    return 0;
}
