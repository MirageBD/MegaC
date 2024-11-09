#ifndef __INPUT_H
#define __INPUT_H

#define KEYBOARD_INSERTDEL		(0*8 + 0)					; backspace in xemu
#define KEYBOARD_RETURN			(0*8 + 1)
#define KEYBOARD_CURSORRIGHT	(0*8 + 2)
#define KEYBOARD_CURSORDOWN		(0*8 + 7)
#define KEYBOARD_CURSORLEFT		(0*8 + 2 + 64)
#define KEYBOARD_CURSORUP		(0*8 + 7 + 64)
#define KEYBOARD_KEY0			(4*8 + 3)
#define KEYBOARD_KEY1			(7*8 + 0)
#define KEYBOARD_KEY2			(7*8 + 3)
#define KEYBOARD_KEY3			(1*8 + 0)
#define KEYBOARD_KEY4			(1*8 + 3)
#define KEYBOARD_KEY5			(2*8 + 0)
#define KEYBOARD_KEY6			(2*8 + 3)
#define KEYBOARD_KEY7			(3*8 + 0)
#define KEYBOARD_KEY8			(3*8 + 3)
#define KEYBOARD_KEY9			(4*8 + 0)
#define KEYBOARD_ESC			(7*8 + 1)
#define KEYBOARD_NOKEY			(255)

#define KEYBOARD_F1				(0*8 + 4)
#define KEYBOARD_F3				(0*8 + 5)
#define KEYBOARD_F5				(0*8 + 6)
#define KEYBOARD_F7				(0*8 + 3)

void keyboard_update();
uint8_t keyboard_keyreleased(uint8_t key);
uint8_t keyboard_keypressed(uint8_t key);

#endif