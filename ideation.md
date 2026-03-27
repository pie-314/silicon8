`this file contains my thinking throughtout, this is not supposed to be taken as guide`

As of now i belive following functions are required

```
void load_rom(rom_file); // this is for handling of .ch8 files, copy data into RAM
bool init_sdl(void); // SDL boilerplate stuff and all
void init_chip8(Chip8 *chip); // Initalization of system, all values to 0
void emulate(Chip8 *chip); // this will act as main process it will do all 
void render_display(Chip8 *chip); // just checks for sprites and updates screen
void cleanup(); // final cleanup
void fetch(Chip8 *chip) // this function will fetch two bytes and returns a opcode
```


#define MEMORY_SIZE 4096 // memory in chip8 is 4KB
#define REGISTER_COUNT 16 // there are 16 registers
#define STACK_LEVELS 16

// the screen is usually 64*32
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define KEY_COUNT 16


chip8 layout
```

typedef struct {
  uint8_t memory[4096]; // RAM
  uint8_t V[16];        // Registers V0-VF
  uint16_t I;           // Index register
  uint16_t pc;          // Program counter

  uint16_t stack[16]; // Stack
  uint8_t sp;         // Stack pointer

  uint8_t delay_timer;
  uint8_t sound_timer;

  uint8_t display[64 * 32]; // Screen pixels
  uint8_t keypad[16];       // Input
  const char *rom_file;
} Chip8;
```

