#include <sys/mman.h>
#include <fcntl.h>

#include <x86intrin.h>

#ifdef __x86_64
    #include <asm/unistd_64.h>
#else
    #include <asm/unistd_32.h>
#endif

// framebuffer settings (width / height / components should match actual framebuffer settings)
#define FRAMEBUFFER_COMPONENTS 4
#define FRAMEBUFFER_LENGTH (WIDTH * HEIGHT * FRAMEBUFFER_COMPONENTS)

inline static int sys_open(char *filepath, int flags, int mode) {
    int r;
#ifdef __x86_64
    __asm__ volatile("syscall"
        : "=a"(r)
        : "0"(__NR_open), "D"(filepath), "S"(flags)//, "d"(mode)
        : "cc", "rcx", "r11", "memory");
#else
    __asm__ volatile("int $0x80"
        : "=a"(r)
        : "0"(__NR_open), "b"(filepath), "c"(flags)/*, "d"(mode)*/
        : "cc", "edi", "esi", "memory");
#endif
  return r;
}

inline static unsigned char *sys_mmap(unsigned int *addr, unsigned long length, unsigned long prot, unsigned long flags, unsigned long fd) {
#ifdef __x86_64
    register volatile int r10 __asm__ ("r10") = flags; register volatile int r8 __asm__ ("r8") = fd; unsigned char *r;
    __asm__ volatile ("syscall" : "=a" (r) : "a" (__NR_mmap)/*, "D" (addr)*/, "S" (length), "d" (prot), "r" (r10), "r" (r8) : "cc", "memory", "r11", "rcx");
    return r;
#else
    unsigned int args[1] = { (unsigned int)addr };
    unsigned char *r;

    // is that safe ?
    __asm__ __volatile__(//"push %%ebp\n"
                        "movl 4(%%ebx), %%ebp\n"
                        //"movl 0(%%ebx), %%ebx\n"
                        "int $0x80\n"
                        //"pop %%ebp\n"
                        : "=a"(r)
                        : "a"(__NR_mmap2), "b"(&args),
                            "c"(length), "d"(prot), "S"(flags), "D"(fd));

    return r;
#endif
}

unsigned char sadd8(unsigned char a, unsigned char b) {
    return (b > 255 - a) ? 255 : a + b;
}

void _start() {
#ifdef __i386__
    // 0x0001000E : see elf32.s (this is /dev/fb0 string address)
    int fbfd = sys_open((char *)0x00010020, O_RDWR, 0);
#else
    // 0x100000007 : see elf64.s (this is /dev/fb0 string address packed in the ELF padding / ABI field)
    int fbfd = sys_open((char *)0x100000007, O_RDWR, 0);
#endif
    unsigned char *buffer = (unsigned char *)sys_mmap(0, FRAMEBUFFER_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, fbfd);

    int frame = 0, x = 0, y = 0;
    unsigned int index = 0;
    unsigned int index2 = 0;

    while (1) {
        // CORDIC
        x = (x - (x >> 12)) - y + (frame >> 7);
        y = y + (x >> 1 - (y >> 23)) + ((frame >> 7) & (x >> 2));

        index = ((WIDTH >> 1) + (x >> 13)) + (y >> 13) * WIDTH * 4;
        index2 = ((WIDTH >> 1) - (x >> 13)) + (y >> 13) * WIDTH * 4;

        // clamp
        if (index >= FRAMEBUFFER_LENGTH) { index = 0; index2 = 0; } // if index2 is omitted the binary size get bigger; why ?
        if (index2 >= FRAMEBUFFER_LENGTH) { index2 = 0; }

        // SIMD saturated arithmetic
        // load color (packed into the header; see elf32.s)
        int c2 = *((int *)0x0001002E);
        __m128i fv = _mm_set_epi32(c2, c2, c2, c2);

        // saturated add
        __m128i t = _mm_load_si128((const __m128i *)&buffer[index]);
        __m128i v = _mm_adds_epu8(fv, t);
        _mm_storeu_si32((unsigned char*)&buffer[index], v);
        __m128i t2 = _mm_load_si128((const __m128i *)&buffer[index2]);
        __m128i v2 = _mm_adds_epu8(fv, t2);
        _mm_storeu_si32((unsigned char*)&buffer[index2], v2);

        frame += 60;   
    }
}
