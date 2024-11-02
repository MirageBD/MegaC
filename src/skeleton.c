#include <stdio.h>
#include <stdint.h>

// C stuff that I want to test converting to asm. Makefile creates a file called exe/skeleton.s.
// This failed miserably when I wanted to see what structs looked like in assembly.

static struct Foo x;

static struct Foo
{
	uint8_t a;
	uint8_t b;
};

void bar()
{
    x.a = 1;
    x.b = 2;
    printf("%d", x.a);
}
