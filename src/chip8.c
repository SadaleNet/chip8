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

#include "chip8.h"
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

static uint8_t chip8_check_collision(uint8_t old, uint8_t new) {
	// Truth table:
	// old	new	out
	// 0	0	0
	// 0	1	0
	// 1	1	0
	// 1	0	1
	return (old ^ new) & old;
}

void chip8_step(struct chip8_machine *machine) {
	struct chip8_cpu *cpu = &machine->cpu;
	struct chip8_periph *periph = &machine->periph;
	uint8_t *mem = machine->mem;
	uint8_t prevents_stepping = 0;

	uint16_t instruction = mem[cpu->pc[cpu->pc_index]] << 8;
	instruction |= mem[cpu->pc[cpu->pc_index]+1];
	uint8_t *vx = &cpu->v[(instruction & 0x0F00)>>8];
	uint8_t *vy = &cpu->v[(instruction & 0x00F0)>>4];
	uint8_t *vf = &cpu->v[15];
	uint16_t *i = &cpu->i;

	switch(instruction & 0xF000) {
		case 0x0000:
			switch(instruction & 0x00FF) {
				case 0x00E0: // 00E0
					memset(periph->display, 0, sizeof(periph->display));
				break;
				case 0x00EE: // 00EE
					assert(cpu->pc_index > 0);
					cpu->pc_index--;
				break;
				default:
					assert(false); // Machine code execution: Unsupported!
				break;
			}
		break;
		case 0x2000:
			assert(cpu->pc_index+1 < (int)CHIP8_PC_STACK_SIZE);
			cpu->pc_index++;
		// Fallthrough
		case 0x1000:
			cpu->pc[cpu->pc_index] = instruction&0x0FFF;
			prevents_stepping = 1;
		break;
		case 0x3000:
			if(*vx == (instruction & 0x00FF)) {
				cpu->pc[cpu->pc_index] += 2;
			}
		break;
		case 0x4000:
			if(*vx != (instruction & 0x00FF)) {
				cpu->pc[cpu->pc_index] += 2;
			}
		break;
		case 0x5000:
			if(*vx == *vy) {
				cpu->pc[cpu->pc_index] += 2;
			}
		break;
		case 0x6000:
			*vx = instruction&0x00FF;
		break;
		case 0x7000:
			*vx += instruction&0x00FF;
		break;
		case 0x8000:
			switch(instruction&0x000F) {
				case 0x0000: // 8XY0
					*vx = *vy;
					if(machine->quirks & CHIP8_QUIRK_LOGIC) {
						*vf = 0;
					}
				break;
				case 0x0001: // 8XY1
					*vx |= *vy;
					if(machine->quirks & CHIP8_QUIRK_LOGIC) {
						*vf = 0;
					}
				break;
				case 0x0002: // 8XY2
					*vx &= *vy;
					if(machine->quirks & CHIP8_QUIRK_LOGIC) {
						*vf = 0;
					}
				break;
				case 0x0003: // 8XY3
					*vx ^= *vy;
				break;
				case 0x0004: // 8XY4
				{
					uint16_t result = *vx + *vy;
					*vx = result;
					*vf = (result > 0xFF);
				}
				break;
				case 0x0005: // 8XY5
				{
					uint16_t result = *vx - *vy;
					*vx = result;
					*vf = (result <= 0xFF);
				}
				break;
				case 0x0006: // 8XY6
				{
					uint8_t *target = (machine->quirks & CHIP8_QUIRK_SHIFT) ? vx : vy;
					*vf = (*target & 0x01);
					*vx = *target >> 1;
				}
				break;
				case 0x0007: // 8XY7
				{
					uint16_t result = *vy - *vx;
					*vx = result;
					*vf = (result <= 0xFF);
				}
				break;
				case 0x000E: // 8XYE
				{
					uint8_t *target = (machine->quirks & CHIP8_QUIRK_SHIFT) ? vx : vy;
					*vf = (*target & 0x80) >> 7;
					*vx = *target << 1;
				}
				break;
				default:
					assert(false); // Invalid instruction!
				break;
			}
		break;
		case 0x9000:
			if(*vx != *vy) {
				cpu->pc[cpu->pc_index] += 2;
			}
		break;
		case 0xA000:
		{
			*i = instruction & 0x0FFF;
		}
		break;
		case 0xB000:
			if(machine->quirks & CHIP8_QUIRK_JUMP) {
				cpu->pc[cpu->pc_index] = (instruction & 0x0FFF) + *vx;
			} else {
				cpu->pc[cpu->pc_index] = (instruction & 0x0FFF) + cpu->v[0];
			}
			prevents_stepping = 1;
		break;
		case 0xC000:
			*vx = periph->random_num & (instruction & 0x00FF);
		break;
		case 0xD000:
		{
			uint16_t x = *vx % CHIP8_DISPLAY_WIDTH;
			uint16_t y = *vy % CHIP8_DISPLAY_HEIGHT;
			uint16_t row = y*CHIP8_DISPLAY_WIDTH/8;
			*vf = 0x00;
			for(size_t n=0; n<(instruction&0x000F); n++) {
				if(row >= sizeof(periph->display)) {
					break;
				}

				uint8_t display_old = periph->display[row + x/8];
				periph->display[row + x/8] ^= mem[*i+n] >> (x%8);
				if(chip8_check_collision(display_old, periph->display[row + x/8])) {
					*vf = 0x01;
				}
				if(x + 8 < (int)CHIP8_DISPLAY_WIDTH) {
					display_old = periph->display[row + x/8 + 1];
					periph->display[row + x/8 + 1] ^= mem[*i+n] << (8-(x%8));
					if(chip8_check_collision(display_old, periph->display[row + x/8 + 1])) {
						*vf = 0x01;
					}
				}
				row += CHIP8_DISPLAY_WIDTH/8;
			}
			if(machine->quirks & CHIP8_QUIRK_VBLANK) {
				periph->requests |= CHIP8_REQUEST_WAIT_DISPLAY_REFRESH;
			}
		}
		break;
		case 0xE000:
			switch(instruction & 0x00FF) {
				case 0x009E: // E09E
					if(periph->key_held & (1U << *vx)) {
						cpu->pc[cpu->pc_index] += 2;
					}
				break;
				case 0x00A1: // E0A1
					if(!(periph->key_held & (1U << *vx))) {
						cpu->pc[cpu->pc_index] += 2;
					}
				break;
				default:
					assert(false); // Invalid instruction!
				break;
			}
		break;
		case 0xF000:
			switch(instruction & 0x00FF) {
				case 0x0007: // FX07
					*vx = periph->delay_timer;
				break;
				case 0x000A: // FX0A
					if(!periph->key_just_released) {
						prevents_stepping = 1;
					} else {
						for(size_t k=0; k<16; k++) {
							if(periph->key_just_released & (1<<k)) {
								*vx = k;
								break;
							}
						}
					}
				break;
				case 0x0015: // FX15
					periph->delay_timer = *vx;
				break;
				case 0x0018: // FX18
					periph->sound_timer = *vx;
				break;
				case 0x001E: // FX1E
					*i += *vx;
				break;
				case 0x0029: // FX29
					if(*vx > 0xF) {
						*i = 16 * 5;
					} else {
						*i = *vx * 5;
					}
				break;
				case 0x0033: // FX33
					assert(*i + 2 < (int)CHIP8_MEMORY_SIZE);
					mem[*i] = *vx / 100;
					mem[*i+1] = (*vx - mem[*i] * 100) / 10;
					mem[*i+2] = *vx - mem[*i]*100 - mem[*i+1]*10;
				break;
				case 0x0055: // FX55
				{
					uint8_t n = (instruction & 0x0F00)>>8;
					assert(*i >= CHIP8_PROGRAM_START_OFFSET && *i + n < CHIP8_MEMORY_SIZE);
					for(size_t x=0; x<n+1; x++) {
						mem[(*i)++] = cpu->v[x];
					}
					if(machine->quirks & CHIP8_QUIRK_MEMORY_LEAVE_I_UNCHANGED) {
						i -= (n+1);
					} else if (machine->quirks & CHIP8_QUIRK_MEMORY_INCREASE_BY_X) {
						i--;
					}
				}
				break;
				case 0x0065: // FX65
				{
					uint8_t n = (instruction & 0x0F00)>>8;
					assert(*i + n < CHIP8_MEMORY_SIZE);
					for(size_t x=0; x<n+1; x++) {
						cpu->v[x] = mem[(*i)++];
					}
					if(machine->quirks & CHIP8_QUIRK_MEMORY_LEAVE_I_UNCHANGED) {
						i -= (n+1);
					} else if (machine->quirks & CHIP8_QUIRK_MEMORY_INCREASE_BY_X) {
						i--;
					}
				}
				break;
				default:
					assert(false); // Invalid instruction!
				break;
			}
		break;
		default:
			assert(false); // Invalid instruction!
		break;
	}
	if(!prevents_stepping) {
		cpu->pc[cpu->pc_index] += 2;
	}
	periph->key_just_released = 0;
}

void chip8_timer_step(struct chip8_machine *machine) {
	if(machine->periph.delay_timer > 0) {
		machine->periph.delay_timer--;
	}
	if(machine->periph.sound_timer > 0) {
		machine->periph.sound_timer--;
	}
}

void chip8_init(struct chip8_machine *machine, uint32_t quirks) {
	static const uint8_t font_data[] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80, // F
	};
	memcpy(machine->mem, font_data, sizeof(font_data));
	memset(&machine->mem[sizeof(font_data)], 0, sizeof(machine->mem));

	memset(&machine->cpu, 0, sizeof(machine->cpu));
	machine->cpu.pc[0] = CHIP8_PROGRAM_START_OFFSET;

	memset(&machine->periph, 0, sizeof(machine->periph));

	machine->quirks = quirks;
}
