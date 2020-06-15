#include "stddef.h"

#pragma once

class chip8 {
    public:
		chip8();
		~chip8();
		
		bool drawFlag;

		void emulateCycle();
		//void debugRender();
		bool loadApplication(const char* filename);

        // Graphics buffer
        unsigned char  gfx[64 * 32];	// Total amount of pixels: 2048

        // Keypad
		unsigned char  key[16];

    private:
	    unsigned char memory[0xFFF]; // Memory size: 4096 (4KB)

        // Current opcode
        unsigned short opcode;

        // Registers
        unsigned char V[0xF];

        // Instruction pointer
        unsigned short I;

        // Program counter
        unsigned short pc;

        unsigned char delay_timer;
        unsigned char sound_timer;

        unsigned short stack[16];

        // Stack pointer
        unsigned short sp;

        void initialize();
};
