#include "app.h"
#include "string.h"

int main() {
	SUCCESS("Enter kernel process GPID_TERMINAL");

	char buf[SYSCALL_MSG_LEN];
	strcpy(buf, "Finish GPID_TERMINAL Initialization");
	grass->sys_send(GPID_PROCESS, buf, 36);

	while (1) {
		int sender;
		struct term_request req;
		struct term_reply reply;
		grass->sys_recv(GPID_ALL, &sender, buf, SYSCALL_MSG_LEN);
	}
}