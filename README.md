# Sadale's CHIP-8 Interpreter (ilo pi toki Sije "CHIP-8" pi jan Sate)

(sina sona e toki pona la o lukin e lipu ni: [mi pali e ilo ni: ona li pali kepeken nasin toki Sije "CHIP-8"](https://sadale.net/tok/75/))

This repo contains my implementation of CHIP-8 Interpreter in C. Originally I wrote it as a weekend project. Now that I'm expanding and repurposing it to be used in a physical game console that I'm developing.

![Screenshot of the CHIP-8 Interpreter running Brix](./doc/Brix%20(by%20Andreas%20Gustafsson)(1990).ch8.jpg?raw=true)

Blogpost of the original implementation of this project is available here: [Weekend Project: CHIP-8 Interpreter/Emulator](https://sadale.net/en/75/)

### Why another one? Aren't there already exist many of them on the internet?

I was a bit bored in a weekend so I decided to take up a challenge and implement a [CHIP-8 Interpreter](https://en.wikipedia.org/wiki/CHIP-8) on my own. For minimal implementation without extension support, I thought that it'd take me a week but I ended up getting it done within a few hours. I was surprised at how easy it is.

Now that I'm developing a physical game console that emulates CHIP-8. The implementation of this repo is meant to be integrated into that physical game console. It has performance feature specifically designed for the hardware of the game console, such as column-major display buffer and 32bit little-endian encoding of the audio pattern buffer.

This implementation is compatible with maybe 80% of the CHIP-8 games listed here: [https://archive.org/details/chip-8-games](https://archive.org/details/chip-8-games). The incompatibility is mainly caused by the quirks between different implementations.

I came up with this implementation on my own solely by reading the reference documents below. I did not read the source code of any other CHIP-8 interpreters, nor any guide of implementing CHIP-8 interpreters.

At first, this implementation only supported COSMAC VIP's instruction set. Since 2026, I've started implementing support of Super-CHIP and part of XO-Chip extensions. For simplicity of implementation, XO-Chip's extension is only partially supported:

* pitch change is not supported in the SDL implementation. However, it's properly implemented in the emulator's core code itself.
* bitplanes selection isn't supported. The game console I'm building is monochrome and cannot support 4 colors.
* 64K RAM isn't supported. The game console I'm building doesn't have that much RAM
* long jump isn't supported because there's no need for that with 4K RAM.

### Dependencies

* C
* SDL2

### Reference Documents

* https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set
* https://github.com/mattmikolay/chip-8/wiki/Mastering-CHIP%E2%80%908
* https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Technical-Reference#references
* https://blog.khutchins.com/posts/chip-8-emulation/
* https://github.com/chip-8/chip-8-database/blob/master/database/quirks.json
* https://chip8.gulrak.net/
* https://chip-8.github.io/extensions/
* https://github.com/JohnEarnest/Octo/blob/gh-pages/docs/SuperChip.md
* https://github.com/JohnEarnest/Octo/blob/gh-pages/docs/XO-ChipSpecification.md

### Acknowledgement

* The main.c of this repo was adapted based on the SDL2 example code from the following repo: [https://github.com/xyproto/sdl2-examples/tree/main](sdl2-examples)
