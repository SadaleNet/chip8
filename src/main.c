// This file was heavily modified based on the content from the following repo: https://github.com/xyproto/sdl2-examples/tree/main

// Copyright 2023 Alexander F. Rødseth
// Copyright 2025 Wong "Sadale" Cho Ching <me@sadale.net>
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include "chip8.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct chip8_machine chip8;
#define BORDER_WIDTH (20U)
#define PIXEL_SCALE (4U)
#define FRAME_DURATION_MS (1000/60) // 60Hz for display refresh
#define CYCLE_PER_FRAME (20)

#define CHIP8_QUIRK_PLATFORM_VIP (CHIP8_QUIRK_VBLANK|CHIP8_QUIRK_LOGIC)
#define CHIP8_QUIRK_PLATFORM_SCHIP (CHIP8_QUIRK_SHIFT|CHIP8_QUIRK_MEMORY_LEAVE_I_UNCHANGED|CHIP8_QUIRK_JUMP|CHIP8_QUIRK_HIRES_COLLISION)
#define CHIP8_QUIRK_PLATFORM_XOCHIP (CHIP8_QUIRK_WRAP|CHIP8_QUIRK_LORES_WIDE_SPRITE|CHIP8_QUIRK_RESIZE_CLEAR_SCREEN)

int main(int argc, char **argv)
{
	if(argc < 2) {
		fprintf(stderr, "Usage: %s <chip8rom.ch8>\n", argv[0]);
		return 1;
	}

	srand(time(NULL));
	chip8_init(&chip8, CHIP8_QUIRK_PLATFORM_SCHIP);
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL) {
		fprintf(stderr, "Failed to open the file: %s\n", argv[1]);
		return 1;
	}
	fread(&chip8.mem[CHIP8_PROGRAM_START_OFFSET], CHIP8_MEMORY_SIZE-CHIP8_PROGRAM_START_OFFSET, 1, fp);
	if (ferror(fp)) {
		fprintf(stderr, "Failed to read the file's content: %s\n", argv[1]);
		return 1;
	}
	fclose(fp);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Window* win = SDL_CreateWindow("Chip8", 100, 100, CHIP8_DISPLAY_WIDTH*PIXEL_SCALE, CHIP8_DISPLAY_HEIGHT*PIXEL_SCALE+BORDER_WIDTH, SDL_WINDOW_SHOWN);

	if (win == NULL) {
		fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL) {
		fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_DestroyWindow(win);
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Surface* surface = SDL_CreateRGBSurface(0, CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT, 1, 0, 0, 0, 0);
	memset(surface->pixels, 0x00, CHIP8_DISPLAY_WIDTH*CHIP8_DISPLAY_HEIGHT/8);

	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_RenderClear(ren);
	SDL_RenderPresent(ren);

	uint8_t skip_next_render = 0;

	uint32_t current_tick = SDL_GetTicks();
	uint32_t next_frame_tick = current_tick+FRAME_DURATION_MS;
	uint16_t key_held_previous = 0;
	uint8_t sound_indicator_prev = 2;

	uint32_t cycle_counter = 0;

	SDL_AudioSpec audio_spec;
    SDL_zero(audio_spec);
	audio_spec.freq = 4000;
	audio_spec.format = AUDIO_U8;
	audio_spec.channels = 1;
	audio_spec.samples = 128;
	audio_spec.callback = NULL;
	SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec, NULL, 0);
    SDL_PauseAudioDevice(audio_device, 0);

	while (1) {
		current_tick = SDL_GetTicks();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					return 0;
				break;
			}
		}
		
		// Derive key pressed states
		const Uint8* keystate = SDL_GetKeyboardState(NULL);
		chip8.periph.key_held =
		(((!!keystate[SDL_SCANCODE_X])<<0) | ((!!keystate[SDL_SCANCODE_1])<<1) | ((!!keystate[SDL_SCANCODE_2])<<2) | ((!!keystate[SDL_SCANCODE_3])<<3) |
		((!!keystate[SDL_SCANCODE_Q])<<4) | ((!!keystate[SDL_SCANCODE_W])<<5) | ((!!keystate[SDL_SCANCODE_E])<<6) | ((!!keystate[SDL_SCANCODE_A])<<7) |
		((!!keystate[SDL_SCANCODE_S])<<8) | ((!!keystate[SDL_SCANCODE_D])<<9) | ((!!keystate[SDL_SCANCODE_Z])<<10) | ((!!keystate[SDL_SCANCODE_C])<<11) |
		((!!keystate[SDL_SCANCODE_4])<<12) | ((!!keystate[SDL_SCANCODE_R])<<13) | ((!!keystate[SDL_SCANCODE_F])<<14) | ((!!keystate[SDL_SCANCODE_V])<<15)
		);
		chip8.periph.key_just_released = (key_held_previous^chip8.periph.key_held)&key_held_previous;
		key_held_previous = chip8.periph.key_held;

		chip8.periph.random_num = rand();
		if(!(chip8.periph.requests & CHIP8_REQUEST_WAIT_DISPLAY_REFRESH)) {
			chip8_step(&chip8);
			if(chip8.periph.requests & CHIP8_REQUEST_EXIT_EMULATOR) {
				printf("Exit request received. Emulation completed!\n");
				break;
			}
		}

		if(++cycle_counter >= CYCLE_PER_FRAME) {
			while(SDL_GetTicks() < next_frame_tick) {
				SDL_Delay(1);
			}
		}

		// Audio handling. Pitch: Not implemented.
		if(chip8.periph.sound_timer > 0 && SDL_GetQueuedAudioSize(audio_device) < 128) {
			static uint8_t buffer[CHIP8_AUDIO_BUFFER_SIZE*8];
			for(size_t i=0; i<CHIP8_AUDIO_BUFFER_SIZE/4; i++) {
				for(size_t j=0; j<32; j++) {
					buffer[i*32 + j] = (chip8.periph.audio[i] & (1U << (31-j))) ? 255 : 0;
				}
			}
			SDL_QueueAudio(audio_device, buffer, sizeof(buffer));
		}

		if(SDL_GetTicks() >= next_frame_tick) {
			// Beep indicator
			uint8_t sound_indicator = (chip8.periph.sound_timer <= 0x01);
			if(sound_indicator != sound_indicator_prev) {
				if(chip8.periph.sound_timer <= 0x01) {
					SDL_SetRenderDrawColor(ren, 22, 22, 22, 255);
				} else {
					SDL_SetRenderDrawColor(ren, 222, 222, 222, 255);
				}
				SDL_Rect rect;
				rect.x = 0;
				rect.y = CHIP8_DISPLAY_HEIGHT*PIXEL_SCALE;
				rect.w = CHIP8_DISPLAY_WIDTH*PIXEL_SCALE;
				rect.h = BORDER_WIDTH;
				SDL_RenderFillRect(ren, &rect);
				sound_indicator_prev = sound_indicator;
				SDL_RenderPresent(ren);
			}

			// Render display
			if(!skip_next_render) {
				for(size_t x=0; x<CHIP8_DISPLAY_WIDTH; x++) {
					for(size_t y=0; y<CHIP8_DISPLAY_HEIGHT; y++) {
						if(chip8.periph.display[(x*CHIP8_DISPLAY_HEIGHT+y)/8] & (1<<(y%8))) {
							SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
						} else {
							SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
						}
						SDL_Rect rect;
						rect.x = x*PIXEL_SCALE;
						rect.y = y*PIXEL_SCALE;
						rect.w = PIXEL_SCALE;
						rect.h = PIXEL_SCALE;
						SDL_RenderFillRect(ren, &rect);
					}
				}
				SDL_RenderPresent(ren);
			}
			chip8.periph.requests &= ~CHIP8_REQUEST_WAIT_DISPLAY_REFRESH;

			// Handle timers
			chip8_timer_step(&chip8);
			next_frame_tick += FRAME_DURATION_MS;

			if(SDL_GetTicks() < next_frame_tick) {
				skip_next_render = 0;
			} else {
				// Our emulator is lagging. Skipping the next frame rendering
				printf("skip! %u\n", SDL_GetTicks());
				skip_next_render = 1;
			}
			cycle_counter = 0;
		}
	}

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return EXIT_SUCCESS;
}
