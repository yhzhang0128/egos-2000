/*
 * (C) 2024, Cornell University
 * All rights reserved.
 *
 * Description: the terminal system server
 * Manage the tty device; Handle keyboard input and screen output for other apps
 */


#include "app.h"
#include "string.h"

int main() {
	SUCCESS("Enter kernel process GPID_TERMINAL");
	
	char buf[SYSCALL_MSG_LEN];
	strcpy(buf, "Finish GPID_TERMINAL initialization");
	grass->sys_send(GPID_PROCESS, buf, 36);

	while (1) {
		int sender;
		struct term_request* req = (void*)buf;
		struct term_reply* reply = (void*)buf;
		grass->sys_recv(GPID_ALL, &sender, (void*)req, SYSCALL_MSG_LEN);

		if (req->len > TERM_BUF_SIZE) FATAL("sys_terminal: request from process %d exceeds TERM_BUF_SIZE", sender);

		switch (req->type) {
			case TERM_INPUT:
				reply->len = term_read(reply->buf, req->len);
				grass->sys_send(sender, (void*)reply, sizeof(*reply));
				break;
			case TERM_OUTPUT:
				term_write(req->buf, req->len);
				break;
			default:
				FATAL("sys_terminal: invalid request %d", req->type);
		}
	}
}
