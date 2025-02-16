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
#define CHIP8_PC_STACK_SIZE (12U)

#define CHIP8_DISPLAY_WIDTH (64U)
#define CHIP8_DISPLAY_HEIGHT (32U)

struct chip8_cpu {
	uint8_t v[16];
	uint16_t i;
	uint16_t pc[CHIP8_PC_STACK_SIZE];
	uint8_t pc_index;
};

#define CHIP8_REQUEST_WAIT_DISPLAY_REFRESH (1U << 0)

struct chip8_periph {
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint16_t key_held;
	uint16_t key_just_released;
	uint8_t display[CHIP8_DISPLAY_WIDTH*CHIP8_DISPLAY_HEIGHT/8];
	uint8_t random_num;
	uint8_t requests;
};

struct chip8_machine {
	struct chip8_cpu cpu;
	struct chip8_periph periph;
	uint8_t mem[CHIP8_MEMORY_SIZE];
};

void chip8_step(struct chip8_machine *machine);
void chip8_timer_step(struct chip8_machine *machine);
void chip8_init(struct chip8_machine *machine);
