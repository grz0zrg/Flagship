# FLAGSHIP by The Orz

### 256 bytes linux procedural graphics for Revision 2021 demoparty (wild entry)

this doesn't use any libraries (not even the standard C library) and use two syscalls (open / mmap) to output to the Linux framebuffer device (fbdev), this also doesn't use shell / compression stub so it is a standalone 256 bytes 32 bits ELF (64 bits wasn't tested)

this use some SIMD instructions for saturated arithmetic (using intrinsics)

binary size is 252 bytes using `-march=haswell` other machine type may work (try `-march=native` if it don't) but some compile down to 260 bytes (especially those without AVX ?)

prototyped first with my [software graphics library](https://github.com/grz0zrg/fbg)

this intro led to many improvements (mainly related to 32 bits C + custom headers) to my [C Linux intro framework](https://github.com/grz0zrg/tinycelfgraphics)

![screenshot](flagship_400x225.png?raw=true "Screenshot")

Also have included screenshots of the original version which was 1536x864 (only width is necessary), the 1536 width version is much more visually pleasing but since the result is resolution dependent i couldn't fit it into 256 bytes (maybe it is doable ?)

![1536x864 version](flagship_1536x864_400x225.png?raw=true "Screenshot (1536x864 version)")

requirements:
- 32 bits /dev/fb0 with supported resolution
- framebuffer set to display resolution

how to run:
 - switch to console with Ctrl+Alt+F2 (Ctrl+Alt+F1 to switch back to X)
 - check /dev/fb0 is writable to you (or launch with sudo to bypass this step)
 - check /dev/fb0 current resolution with fbset tool
 - `clear` before running the program and maybe deactivate cursor (not mandatory but nicer)
 - run the appropriate binary

permission issue: add current user to either 'tty' or 'video' group or use 'sudo'

change graphical terminal resolution:
 - check supported resolution with "vbeinfo" in GRUB command prompt
 - "GRUB_GFXMODE=WxH" in /etc/default/grub (where W/H is a value)
 - sudo update-grub

this is a follow-up of the 64-bit TRUESPACE procedural graphics intro, was too busy trying to improve my C intro framework so didn't had enough time to craft a scene at the time of TRUESPACE (especially since 32 bits ELF didn't work yet) but then i had some time... :) most of the code is C with hand crafted ELF headers, it is 32 bits so that many more bytes can be gained over a 64 bits version (due to ELF header)

The algorithm is some kind of CORDIC algorithm and show some kind of fractal.

This show that some interesting intro can be done in pure C.

ELF header is taking 48 bytes and there is ~58 bytes of init code so technically not very far from a 128 bytes if header and init code is ommited !

[pouet link](https://www.pouet.net/prod.php?which=88636)

code by grz built with GCC v11

greetings to all sizecoders

31/03/2021
