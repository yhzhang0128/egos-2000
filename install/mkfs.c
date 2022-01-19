#include <stdio.h>
#include <string.h>
#include <unistd.h>

char buf[1024 * 1024];

int main() {
    freopen("disk.img", "w", stdout);

    /* paging area */
    memset(buf, 0, sizeof(buf));
    write(1, buf, 1024 * 1024);

    /* grass kernel */
    
    /* file system */
    
    fclose(stdout);
    return 0;
}
