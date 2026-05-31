#include "app.h"

int main() {
  const uint usec_cnt = 5000000;
  printf("Start to sleep for %d microseconds\n\r", usec_cnt);
  sleep(usec_cnt);
  printf("Woke up again after %d microseconds\n\r", usec_cnt);
}