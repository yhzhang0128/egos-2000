#include "string.h"
#include "app.h"
#include "egos.h"

void loop(int count) {
    int counter = 0;
    for(int i=0;i<count*QUANTUM; i++) {
        float ret = (float)i / counter;
        counter++;
    }
}

void sleep(int time) {
  /* Student code goes here (Premptive scheduler)
   * call proc_sleep to sleep for time * QUANTUM.
   * */
}

int main(int argc, char** argv) {
    if (argc == 1) {
        loop(100);
    } else if (strcmp(argv[1], "sleep") == 0) {
        INFO("Sleep for 10 QUANTUM...\n");
        sleep(10);
        INFO("                   ...and waked up\n");
    }
}
