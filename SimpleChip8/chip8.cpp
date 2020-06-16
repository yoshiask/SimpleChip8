#include "chip8.h"
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

unsigned char chip8_fontset[80] =
{
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
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

chip8::chip8()
{
	// empty
    initialize();
}

chip8::~chip8()
{
	// empty
}


void chip8::initialize() {
    // Initialize registers and memory once

    pc     = 0x200;  // Program counter starts at 0x200
    opcode = 0;      // Reset current opcode
    I      = 0;      // Reset index register
    sp     = 0;      // Reset stack pointer

    // Clear display
    for (int i = 0; i < 64 * 32; i++)
        gfx[i] = 0;

    // Clear stack
    for(int i = 0; i < 16; ++i)
        stack[i] = 0;

    // Clear registers V0-VF
    for(int i = 0; i < 0xF; ++i)
        V[i] = 0;

    // Clear memory
    for(int i = 0; i < 0xFFF; ++i)
        memory[i] = 0;

    // Load fontset to memory 0x050-0x0A0
    for(int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;
}

void chip8::emulateCycle()
{
    // Fetch Opcode
    opcode = memory[pc] << 8 | memory[pc + 1];
    printf("Executing opcode 0x%X\n", opcode);

    // Decode & Execute Opcode
    switch (opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x000F)
            {
                case 0x0000: // 0x00E0: Clears the screen
                    for (int i = 0; i < 64 * 32; i++) {
                        gfx[i] = 0;
                    }
                    drawFlag = true;
					pc += 2;
                break;
 
                case 0x000E: // 0x00EE: Returns from subroutine
					--sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
					pc = stack[sp];	// Put the stored return address from the stack back into the program counter					
					pc += 2;		// Don't forget to increase the program counter!
                break;
 
                default:
                    printf("Unknown opcode [0x0000]: 0x%X\n", opcode);          
            }
        break;

        case 0x1000: // 1NNN: Jumps to address NNN.
            pc = opcode & 0xFFF;
        break;

        case 0x2000: // 2NNN: Calls subroutine at NNN.
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0xFFF;
        break;

        case 0x3000: // 3XNN: Skips the next instruction if VX equals NN.
            if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
        break;

        case 0x4000: // 4XNN: Skips the next instruction if VX does not equal NN.
            if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
        break;

        case 0x5000: // 5XY0: Skips the next instruction if VX equals VY.
            if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
        break;

        case 0x6000: // 6XNN: Sets VX to NN
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
        break;

        case 0x7000: // 7XNN: Add NN to VX, carry flag not changed
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
        break;

        // Mathematical and bit operators
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: // 8XY0: Sets VX to the value of VY.
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                break;

                case 0x0001: // 8XY1: Sets VX to VX or VY. (Bitwise OR operation)
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] | V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x0002: // 8XY2: Sets VX to VX and VY. (Bitwise AND operation)
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] & V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x0003: // 8XY3: Sets VX to VX xor VY. (Bitwise XOR operation)
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] ^ V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1; // Set the carry bit
                    else
                        V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2; 
                break;

                case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                    if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                        V[0xF] = 0; // Unset the carry bit
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2; 
                break;

                case 0x0006: // 8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2; 
                break;

                case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 0; // Unset the carry bit
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2; 
                break;

                case 0x000E: // 8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2; 
                break;

                default:
                    printf("Unknown opcode [0x8000]: 0x%X\n", opcode); 
            }
        break;

        case 0x9000: // 9XY0: Skips the next instruction if VX does not equal VY.
            if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
        break;

        case 0xA000: // ANNN: Sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
        break;

        case 0xB000: // BNNN: Jumps to the address NNN plus V0.
            pc = (opcode & 0x0FFF) + V[0];
        break;

        case 0xC000: // CXNN: Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
            V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            pc += 2;
        break;

        case 0xD000: /* DXYN:
             * Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels
             * and a height of N pixels. Each row of 8 pixels is read as bit-coded
             * starting from memory location I; I value doesn’t change after the
             * execution of this instruction. As described above, VF is set to 1
             * if any screen pixels are flipped from set to unset when the sprite
             * is drawn, and to 0 if that doesn’t happen */
        {
            unsigned short x = V[(opcode & 0x0F00) >> 8];
            unsigned short y = V[(opcode & 0x00F0) >> 4];
            unsigned short height = opcode & 0x000F;
            unsigned short pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++)
            {
                pixel = memory[I + yline];
                for(int xline = 0; xline < 8; xline++)
                {
                    if((pixel & (0x80 >> xline)) != 0)
                    {
                        if(gfx[(x + xline + ((y + yline) * 64))] == 1)
                            V[0xF] = 1;
                        gfx[x + xline + ((y + yline) * 64)] ^= 1;
                    }
                }
            }
 
            drawFlag = true;
            pc += 2;
        }
        break;

        case 0xE000:
            switch(opcode & 0x00FF)
            {
                case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
                    if(key[V[(opcode & 0x0F00) >> 8]] == 1)
                        pc += 4;
                    else
                        pc += 2;
                break;

                case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed.
                    if(key[V[(opcode & 0x0F00) >> 8]] == 0)
                        pc += 4;
                    else
                        pc += 2;
                break;
            }
        break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // FX07: Sets VX to the value of the delay timer.
                    V[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                break;

                case 0x000A: // FX0A: A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                {
                    bool keyPress = false;

					for(int i = 0; i < 16; ++i)
					{
						if(key[i] != 0)
						{
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress = true;
						}
					}

					// If we didn't received a keypress, skip this cycle and try again.
					if(!keyPress)
						return;

					pc += 2;
                }
                break;

                case 0x0015: // FX15: Sets the delay timer to VX
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x0018: // FX18: Sets the sound timer to VX
                    sound_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

                case 0x001E: // FX1E: Adds VX to I. VF is set to 1 when there is a range overflow (I+VX>0xFFF), and to 0 when there isn't.
                    if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1; // Set VF
                    else
                        V[0xF] = 0;
                    I += V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                break;

				case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
				break;

                case 0x0033: // FX33: take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
                    memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                break;

				case 0x0055: // FX55: Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
					for (int o = 0; o <= ((opcode & 0x0F00) >> 8); o++) {
                        memory[I + o] = V[o];
                    }
					pc += 2;
				break;

				case 0x0065: // FX65: Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
					for (int o = 0; o <= ((opcode & 0x0F00) >> 8); o++) {
                        V[o] = memory[I + o];
                    }
					pc += 2;
				break;
            }
        break;

        default:
            printf("Unknown opcode: 0x%X\n", opcode);
    }

    // TODO: Limit clock cycle to ~60Hz for more accurate emulation
    Sleep(1000/60);

    // Update timers
    if(delay_timer > 0)
        --delay_timer;

    if(sound_timer > 0)
    {
        if(sound_timer == 1)
            printf("BEEP!\n");
        --sound_timer;
    }
}

bool chip8::loadApplication(const char* filename) {
    // TODO: Add safety checks, for example if reading the file fails or does not fit in RAM
    
    // Load the program into emulator's memory
	// Max program size is 3584, since chip8's interpreter uses the first 512 of its 4K RAM
	const int BUFFERSIZE = 0xFFF - 0x200;
	FILE* romFile = fopen(filename, "rb");
	char* buffer = new char[BUFFERSIZE];
	int bytes_read = fread(buffer, sizeof(char), BUFFERSIZE, romFile);
	for(int i = 0; i < BUFFERSIZE; ++i)
		memory[i + 0x200] = buffer[i];
    fclose(romFile);

    return true;
}