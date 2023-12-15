#include <stdio.h>
#include <stdint.h>
#include <lcom/lcf.h>
#include <lcom/proj.h>

#include "labs/manager.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/src/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/src/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char **argv) {
  // printf("Hello world!\n");

  if(init_all() != OK) {
    printf("Error in init_all\n");
    return 1;
  }

  if(main_game_loop() != OK) {
    printf("Main game loop error\n");
    return 1;
  }

  if(end_all() != OK) {
    printf("Error in end_all\n");
    return 1;
  }

  return 0;
}
