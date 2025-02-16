# Sadale's CHIP-8 Interpreter

This repo contains my implementation of CHIP-8 Interpreter

![Screenshot of the CHIP-8 Interpreter running Brix](./doc/Brix (by Andreas Gustafsson)(1990).ch8.jpg)

Blogpost of this project available here: [Sadale's CHIP-8 Interpreter](https://sadale.net/en/75/)

### Why another one? Aren't there already exist many of them on the internet?

I was a bit bored this weekend so I decided to take up a challenge and implement a [CHIP-8 Interpreter](https://en.wikipedia.org/wiki/CHIP-8) on my own. I thought that it'd take me a week but I ended up getting it done within a few hours. I was surprised at how easy it is.

It's compatible with maybe 80% of the CHIP-8 games listed here: [https://archive.org/details/chip-8-games](https://archive.org/details/chip-8-games). The incompatibility is mainly caused by the quirks between different implementations.

I came up with this implementation on my own solely by reading the reference documents below. I did not read the source code of any other CHIP-8 interpreters, nor any guide of implementing CHIP-8 interpreters.

This implementation only supports COSMAC VIP's instruction set. Machine code is not implemented. It does not support any extensions for now. Maybe I'll find a time to implement that in the future.

It's not performance-optimized. It probably has a few bugs on it that I haven't found yet. Maybe I'll work on that if I find any use of it.

For simplicity of implementation, the beeper is emulated by lighting up the bar at the bottom of the display (as shown on the screenshot above, the dark-gray bra at the bottom would light up when the beeper is active).

### Dependencies

* C
* SDL2

### Reference Documents

* https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set
* https://github.com/mattmikolay/chip-8/wiki/Mastering-CHIP%E2%80%908
* https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Technical-Reference#references
* https://blog.khutchins.com/posts/chip-8-emulation/
* https://github.com/chip-8/chip-8-database/blob/master/database/quirks.json

### Acknowledgement

* The main.c of this repo was adapted based on the code from the following repo: [https://github.com/xyproto/sdl2-examples/tree/main](sdl2-examples)
