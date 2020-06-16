#include "SimpleChip8.h"
#include "chip8.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <cstdio>
#include <iostream>

chip8 myChip8;
SDL_Renderer* rend;
SDL_Texture* tex;
const int scale_factor = 8;

int main(int argc, char* argv[])
{
    SDL_Window* window = initGraphics();

    myChip8.loadApplication("C:\\Users\\jjask\\Downloads\\myChip8-bin-src\\sirpinski.c8");
    std::cout << "Loaded program to RAM";

    bool quit = false;
    SDL_Event e;
    int pitch = 64 * 32 * sizeof(Uint32);
    Uint32* pixels = new Uint32[pitch];
    memset(pixels, 255, pitch);

    while(!quit)
    {
        // Clear screen
        SDL_RenderClear(rend);

        // Handle events
        while(SDL_PollEvent(&e) != 0)
        {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                break;

                case SDL_KEYDOWN:
                    keyboardDown(e.key.keysym.sym);
                break;

                case SDL_KEYUP:
                    keyboardUp(e.key.keysym.sym);
                break;
            }
        }

        myChip8.emulateCycle();

        // Draw each pixel to the screen
        if (myChip8.drawFlag) {
            
	        for(int y = 0; y < 32; ++y)		
		        for(int x = 0; x < 64; ++x)
			        if(myChip8.gfx[(y * 64) + x] == 1) {
				        pixels[(y * 64) + x] = 0xFFFFFFFF;
                    }
                    else {
				        pixels[(y * 64) + x] = 0x00000000;
                    }
            SDL_UpdateTexture(tex, NULL, pixels, 64 * sizeof(Uint32));
            myChip8.drawFlag = false;
        }

        SDL_RenderClear(rend);
        SDL_RenderCopy(rend, tex, NULL, NULL);
        SDL_RenderPresent(rend);
    }

    delete[] pixels;
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void keyboardDown(SDL_Keycode key)
{
    switch (key) {
        case SDLK_1:
            myChip8.key[0x1] = 1;
            break;
        case SDLK_2:
            myChip8.key[0x2] = 1;
            break;
        case SDLK_3:
            myChip8.key[0x3] = 1;
            break;
        case SDLK_4:
            myChip8.key[0xC] = 1;
            break;

        case SDLK_q:
            myChip8.key[0x4] = 1;
            break;
        case SDLK_w:
            myChip8.key[0x5] = 1;
            break;
        case SDLK_e:
            myChip8.key[0x6] = 1;
            break;
        case SDLK_r:
            myChip8.key[0xD] = 1;
            break;

        case SDLK_a:
            myChip8.key[0x7] = 1;
            break;
        case SDLK_s:
            myChip8.key[0x8] = 1;
            break;
        case SDLK_d:
            myChip8.key[0x9] = 1;
            break;
        case SDLK_f:
            myChip8.key[0xE] = 1;
            break;

        case SDLK_z:
            myChip8.key[0xA] = 1;
            break;
        case SDLK_x:
            myChip8.key[0x0] = 1;
            break;
        case SDLK_c:
            myChip8.key[0xB] = 1;
            break;
        case SDLK_v:
            myChip8.key[0xF] = 1;
            break;
    }
}
void keyboardUp(SDL_Keycode key)
{
    switch (key) {
        case SDLK_1:
            myChip8.key[0x1] = 0;
            break;
        case SDLK_2:
            myChip8.key[0x2] = 0;
            break;
        case SDLK_3:
            myChip8.key[0x3] = 0;
            break;
        case SDLK_4:
            myChip8.key[0xC] = 0;
            break;

        case SDLK_q:
            myChip8.key[0x4] = 0;
            break;
        case SDLK_w:
            myChip8.key[0x5] = 0;
            break;
        case SDLK_e:
            myChip8.key[0x6] = 0;
            break;
        case SDLK_r:
            myChip8.key[0xD] = 0;
            break;

        case SDLK_a:
            myChip8.key[0x7] = 0;
            break;
        case SDLK_s:
            myChip8.key[0x8] = 0;
            break;
        case SDLK_d:
            myChip8.key[0x9] = 0;
            break;
        case SDLK_f:
            myChip8.key[0xE] = 0;
            break;

        case SDLK_z:
            myChip8.key[0xA] = 0;
            break;
        case SDLK_x:
            myChip8.key[0x0] = 0;
            break;
        case SDLK_c:
            myChip8.key[0xB] = 0;
            break;
        case SDLK_v:
            myChip8.key[0xF] = 0;
            break;
    }
}

SDL_Window* initGraphics() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("SimpleChip8 - a CHIP-8 emulator", // window's title
        100, 250, // coordinates on the screen, in pixels, of the window's upper left corner
        64 * scale_factor, 32 * scale_factor, // window's length and height in pixels
    SDL_WINDOW_OPENGL);

    // triggers the program that controls
    // your graphics hardware and sets flags
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
  
    // creates a renderer to render our images
    rend = SDL_CreateRenderer(window, -1, render_flags);
    SDL_RenderSetScale(rend, scale_factor, scale_factor);

    tex = SDL_CreateTexture(rend,
        SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
        64, 32);

    return window;
}