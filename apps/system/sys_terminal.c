#include "app.h"
#include "string.h"

int parse_input(char* buf, uint len) {
    char c;
    for (int i = 0; i < len - 1; i++) {
        earth->tty_read(&c);
        buf[i] = c;

        switch (c) {
        case 0x0d:  /* Enter     */
            buf[i] = 0;
            earth->tty_write("\r\n", 2);
            return i ? i + 1 : 0;
        case 0x7f:  /* Backspace */
            c = 0;
            if (i) earth->tty_write("\b \b", 3);
            i = i ? i - 2 : i - 1;
        }
        if (c) earth->tty_write(&c, 1);
    }

    buf[len - 1] = 0;
    return len;
}

int main() {
	SUCCESS("Enter kernel process GPID_TERMINAL");
	
	char buf[SYSCALL_MSG_LEN];
	strcpy(buf, "Finish GPID_TERMINAL Initialization");
	grass->sys_send(GPID_PROCESS, buf, 36);

	while (1) {
		int sender;
		struct term_request *req = (void*)buf;
		struct term_reply *reply = (void*)buf;
		grass->sys_recv(GPID_ALL, &sender, (void*)req, SYSCALL_MSG_LEN);

		if (req->len > TERM_BUF_SIZE) FATAL("sys_terminal: request from process %d exceeds TERM_BUF_SIZE", sender);

		switch (req->type) {
			case TERM_INPUT:
				reply->len = parse_input(reply->buf, req->len);
				grass->sys_send(sender, (void*)reply, sizeof(*reply));
				break;
			case TERM_OUTPUT:
				earth->tty_write(req->buf, req->len);
				break;
			default:
				FATAL("sys_terminal: invalid request %d", req->type);
		}
	}
}