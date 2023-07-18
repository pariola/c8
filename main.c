#include "cpu.c"
#include "display.c"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  srand(time(NULL)); // seed random no generator

  create_window();

  struct Chip8 *ch8 = init_cpu();

  int ok = load_rom(ch8, argv[1]);
  if (!ok) {
    free(ch8); // free allocation
    printf("error: couldn't open rom @ %s\n", "");
    exit(-1);
  }

  int keep_open = 1;
  while (keep_open) {
    exec(ch8); // emulator cycle

    keep_open = listen_events(ch8); // listen for UI events

    build_texture(ch8); // redraw graphics

    usleep(17 * 1000); // 60 hz = 16.666 ms
  }

  close_window();
  free(ch8);

  return 0;
}