// Copyright (c) 2025 Wong "Sadale" Cho Ching <me@sadale.net>. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <stdint.h>

#define CHIP8_PROGRAM_START_OFFSET (0x200U)
#define CHIP8_MEMORY_SIZE (4096U)
#define CHIP8_PC_STACK_SIZE (16U) // 12 for original CHIP, 16 for SuperCHIP. Let's support 16. There's no harm.

#define CHIP8_DISPLAY_WIDTH (128U)
#define CHIP8_DISPLAY_HEIGHT (64U)

#define CHIP8_QUIRK_SHIFT (1U<<0)
#define CHIP8_QUIRK_MEMORY_LEAVE_I_UNCHANGED (1U<<1)
#define CHIP8_QUIRK_MEMORY_INCREASE_BY_X (1U<<2)
#define CHIP8_QUIRK_WRAP (1U<<3)
#define CHIP8_QUIRK_JUMP (1U<<4)
#define CHIP8_QUIRK_VBLANK (1U<<5)
#define CHIP8_QUIRK_LOGIC (1U<<6)

#define CHIP8_QUIRK_LORES_WIDE_SPRITE (1U<<7)
#define CHIP8_QUIRK_LORES_TALL_SPRITE (1U<<8) // Also draws 8x16 in hires
#define CHIP8_QUIRK_LORES_SCROLL_DIV2 (1U<<9)
#define CHIP8_QUIRK_HIRES_COLLISION (1U<<10)
#define CHIP8_QUIRK_RESIZE_CLEAR_SCREEN (1U<<11)
#define CHIP8_QUIRK_VF_ORDER (1U<<12) // No clue on what it does. Unimplemented.

struct chip8_cpu {
	uint8_t v[16];
	uint16_t i;
	uint16_t pc[CHIP8_PC_STACK_SIZE];
	uint8_t pc_index;
};

#define CHIP8_REQUEST_WAIT_DISPLAY_REFRESH (1U << 0)
#define CHIP8_REQUEST_EXIT_EMULATOR (1U << 1)

struct chip8_periph {
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint16_t key_held;
	uint16_t key_just_released;
	uint8_t high_res;
	uint8_t display[CHIP8_DISPLAY_HEIGHT*CHIP8_DISPLAY_WIDTH/8]; // column-major, first column is leftmost. Each column is 64bit, the top bit is LSB.
	uint8_t random_num;
	uint8_t requests;
	uint8_t storage_flags[16];
};

struct chip8_machine {
	struct chip8_cpu cpu;
	struct chip8_periph periph;
	uint8_t mem[CHIP8_MEMORY_SIZE];
	uint32_t quirks;
};

void chip8_step(struct chip8_machine *machine);
void chip8_timer_step(struct chip8_machine *machine);
void chip8_init(struct chip8_machine *machine, uint32_t quirks);
