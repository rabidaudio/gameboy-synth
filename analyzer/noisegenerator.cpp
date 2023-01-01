#include <stdio.h>
#include <string>
#include "../Source/Gb_Snd_Emu-0.1.4/Basic_Gb_Apu.h"
#include "../Source/Gb_Snd_Emu-0.1.4/Wave_Writer.h"

static Basic_Gb_Apu apu;

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("invalid arg count: %d", argc);
    return 1;
  }

  printf("%s", argv[1]);
  int i = std::stoi(argv[1]);
  if (i < 0 || i > 255) {
    printf("invalid arg: %d", i);
    return 1;
  }

  Wave_Writer wave(44100);
  wave.stereo(true);

  if (apu.set_sample_rate(44100) != 0) {
    return 2;
  }
  apu.write_register(0xFF26, 0x80);  // turn on
  apu.write_register(0xFF24, 0x7F);  // full volume
  apu.write_register(0xFF25, 0x88);  // enable noise osc

  apu.write_register(0xFF1F + 3, i);  // noise settings

  apu.write_register(0xFF1F + 2, 0xF0);  // set full volume, no envelope
  apu.write_register(0xFF1F + 4, 0x80);  // trigger

  blip_sample_t buffer[2048];
  // read 6 frames
  for (int i = 0; i < 6; i++) {
    apu.end_frame();
    int count = apu.read_samples(buffer, 2048);
    wave.write(buffer, count);
  }
}
