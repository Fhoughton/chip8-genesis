#include <genesis.h>
#include <string.h>

/*General stuff*/
bool game_on = TRUE;



/*Game stuff*/
void start_game(){
	if(game_on == FALSE){
		game_on = TRUE;
	}
}

void end_game(){
	if(game_on == TRUE){
		game_on = FALSE;
	}
}

void myJoyHandler( u16 joy, u16 changed, u16 state)
{
	if (joy == JOY_1)
	{
		/*Start game if START is pressed*/
		if (state & BUTTON_START)
		{
			
		}
		
		if (state & BUTTON_C)
		{
			
		}
	}
}



/*Emulator definitions*/

//constants
const unsigned char chip8_fontset[80] =
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

//loop variables
unsigned short opcode;
bool drawFlag;

//memory
unsigned char memory[4096]; //0x000-0x1FF - Chip 8 interpreter (contains font set in emu); 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F); 0x200-0xFFF - Program ROM and work RAM
unsigned char V[16];
unsigned short I;
unsigned short pc;
unsigned char gfx[64 * 32];

//interrupts
unsigned char delay_timer;
unsigned char sound_timer;

//stack
unsigned short stack[16];
unsigned short sp;

//input
unsigned char key[16];

//preset roms, uses partial initialization
const int UFO_length = 224;

const unsigned char UFO[224] = {
	0xA2, 0xCD, 0x69, 0x38, 0x6A, 0x08, 0xD9, 0xA3, 0xA2, 0xD0, 0x6B, 0x00,
	0x6C, 0x03, 0xDB, 0xC3, 0xA2, 0xD6, 0x64, 0x1D, 0x65, 0x1F, 0xD4, 0x51,
	0x67, 0x00, 0x68, 0x0F, 0x22, 0xA2, 0x22, 0xAC, 0x48, 0x00, 0x12, 0x22,
	0x64, 0x1E, 0x65, 0x1C, 0xA2, 0xD3, 0xD4, 0x53, 0x6E, 0x00, 0x66, 0x80,
	0x6D, 0x04, 0xED, 0xA1, 0x66, 0xFF, 0x6D, 0x05, 0xED, 0xA1, 0x66, 0x00,
	0x6D, 0x06, 0xED, 0xA1, 0x66, 0x01, 0x36, 0x80, 0x22, 0xD8, 0xA2, 0xD0,
	0xDB, 0xC3, 0xCD, 0x01, 0x8B, 0xD4, 0xDB, 0xC3, 0x3F, 0x00, 0x12, 0x92,
	0xA2, 0xCD, 0xD9, 0xA3, 0xCD, 0x01, 0x3D, 0x00, 0x6D, 0xFF, 0x79, 0xFE,
	0xD9, 0xA3, 0x3F, 0x00, 0x12, 0x8C, 0x4E, 0x00, 0x12, 0x2E, 0xA2, 0xD3,
	0xD4, 0x53, 0x45, 0x00, 0x12, 0x86, 0x75, 0xFF, 0x84, 0x64, 0xD4, 0x53,
	0x3F, 0x01, 0x12, 0x46, 0x6D, 0x08, 0x8D, 0x52, 0x4D, 0x08, 0x12, 0x8C,
	0x12, 0x92, 0x22, 0xAC, 0x78, 0xFF, 0x12, 0x1E, 0x22, 0xA2, 0x77, 0x05,
	0x12, 0x96, 0x22, 0xA2, 0x77, 0x0F, 0x22, 0xA2, 0x6D, 0x03, 0xFD, 0x18,
	0xA2, 0xD3, 0xD4, 0x53, 0x12, 0x86, 0xA2, 0xF8, 0xF7, 0x33, 0x63, 0x00,
	0x22, 0xB6, 0x00, 0xEE, 0xA2, 0xF8, 0xF8, 0x33, 0x63, 0x32, 0x22, 0xB6,
	0x00, 0xEE, 0x6D, 0x1B, 0xF2, 0x65, 0xF0, 0x29, 0xD3, 0xD5, 0x73, 0x05,
	0xF1, 0x29, 0xD3, 0xD5, 0x73, 0x05, 0xF2, 0x29, 0xD3, 0xD5, 0x00, 0xEE,
	0x01, 0x7C, 0xFE, 0x7C, 0x60, 0xF0, 0x60, 0x40, 0xE0, 0xA0, 0xF8, 0xD4,
	0x6E, 0x01, 0x6D, 0x10, 0xFD, 0x18, 0x00, 0xEE
};

/*Emulator core*/
void reset_emulator() 
{
	pc		= 0x200;		// Program counter starts at 0x200 (Start adress program)
	opcode	= 0;			// Reset current opcode	
	I		= 0;			// Reset index register
	sp		= 0;			// Reset stack pointer

	// Clear display
	for(int i = 0; i < 2048; ++i)
	{	
		gfx[i] = 0;
	}
	
	// Clear stack, registers and key inputs
	for(int i = 0; i < 16; ++i)
	{
		stack[i] = 0;
		V[i] = 0;
		key[i] = 0;
	}
	
	for(int i = 0; i < 16; ++i)
	{	
		
	}

	// Clear memory
	for(int i = 0; i < 4096; ++i)
	{
		memory[i] = 0;
	}
	
	// Load fontset
	for(int i = 0; i < 80; ++i)
	{
		memory[i] = chip8_fontset[i];
	}		

	//reset timers
	delay_timer = 0;
	sound_timer = 0;

	//clear screen
	drawFlag = TRUE;
}

int rand() 
{
	return random();
}

void emulate_cyle() 
{
	//load instruction
	opcode = memory[pc] << 8 | memory[pc + 1]; //use bitwise or to merge two bytes into single instruction
	
	// Process opcode
	switch(opcode & 0xF000)
	{		
		case 0x0000:
			switch(opcode & 0x000F)
			{
				case 0x0000: // 0x00E0: Clears the screen
					for(int i = 0; i < 2048; ++i)
						gfx[i] = 0x0;
					drawFlag = TRUE;
					pc += 2;
				break;

				case 0x000E: // 0x00EE: Returns from subroutine
					--sp;			// 16 levels of stack, decrease stack pointer to prevent overwrite
					pc = stack[sp];	// Put the stored return address from the stack back into the program counter					
					pc += 2;		// Don't forget to increase the program counter!
				break;

				//default:
				//	printf ("Unknown opcode [0x0000]: 0x%X\n", opcode);					
			}
		break;

		case 0x1000: // 0x1NNN: Jumps to address NNN
			pc = opcode & 0x0FFF;
		break;

		case 0x2000: // 0x2NNN: Calls subroutine at NNN.
			stack[sp] = pc;			// Store current address in stack
			++sp;					// Increment stack pointer
			pc = opcode & 0x0FFF;	// Set the program counter to the address at NNN
		break;
		
		case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
			if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
			if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x6000: // 0x6XNN: Sets VX to NN.
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			pc += 2;
		break;
		
		case 0x7000: // 0x7XNN: Adds NN to VX.
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			pc += 2;
		break;
		
		case 0x8000:
			switch(opcode & 0x000F)
			{
				case 0x0000: // 0x8XY0: Sets VX to the value of VY
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't					
					if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) 
						V[0xF] = 1; //carry
					else 
						V[0xF] = 0;					
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;					
				break;

				case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) 
						V[0xF] = 0; // there is a borrow
					else 
						V[0xF] = 1;					
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
				break;

				case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])	// VY-VX
						V[0xF] = 0; // there is a borrow
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];				
					pc += 2;
				break;

				case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
				break;

				//default:
				//	printf ("Unknown opcode [0x8000]: 0x%X\n", opcode);
			}
		break;
		
		case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
			if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
		break;

		case 0xA000: // ANNN: Sets I to the address NNN
			I = opcode & 0x0FFF;
			pc += 2;
		break;
		
		case 0xB000: // BNNN: Jumps to the address NNN plus V0
			pc = (opcode & 0x0FFF) + V[0];
		break;
		
		case 0xC000: // CXNN: Sets VX to a random number and NN
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
		break;
	
		case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
					 // Each row of 8 pixels is read as bit-coded starting from memory location I; 
					 // I value doesn't change after the execution of this instruction. 
					 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, 
					 // and to 0 if that doesn't happen
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
						{
							V[0xF] = 1;                                    
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}
						
			drawFlag = TRUE;			
			pc += 2;
		}
		break;
			
		case 0xE000:
			switch(opcode & 0x00FF)
			{
				case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
					if(key[V[(opcode & 0x0F00) >> 8]] != 0)
						pc += 4;
					else
						pc += 2;
				break;
				
				case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
					if(key[V[(opcode & 0x0F00) >> 8]] == 0)
						pc += 4;
					else
						pc += 2;
				break;

				//default:
				//	printf ("Unknown opcode [0xE000]: 0x%X\n", opcode);
			}
		break;
		
		case 0xF000:
			switch(opcode & 0x00FF)
			{
				case 0x0007: // FX07: Sets VX to the value of the delay timer
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
				break;
								
				case 0x000A: // FX0A: A key press is awaited, and then stored in VX		
				{
					bool keyPress = FALSE;

					for(int i = 0; i < 16; ++i)
					{
						if(key[i] != 0)
						{
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress = TRUE;
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

				case 0x001E: // FX1E: Adds VX to I
					if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
						V[0xF] = 1;
					else
						V[0xF] = 0;
					I += V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;

				case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					pc += 2;
				break;

				case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I plus 2
					memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;					
					pc += 2;
				break;

				case 0x0055: // FX55: Stores V0 to VX in memory starting at address I					
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						memory[I + i] = V[i];	

					// On the original interpreter, when the operation is done, I = I + X + 1.
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;

				case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I					
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						V[i] = memory[I + i];			

					// On the original interpreter, when the operation is done, I = I + X + 1.
					I += ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;

				//default:
				//	printf ("Unknown opcode [0xF000]: 0x%X\n", opcode);
			}
		break;

		//default:
		//	printf ("Unknown opcode: 0x%X\n", opcode);
	}	

	// Update timers
	if(delay_timer > 0)
		--delay_timer;

	if(sound_timer > 0)
	{
		/*
		if(sound_timer == 1)
		{
			printf("BEEP!\n");
		}
		*/
		--sound_timer;
	}
}

void draw_graphics() 
{
	for(u16 y = 0; y < 32; ++y)		
		for(u16 x = 0; x < 64; ++x)
			if(gfx[(y * 64) + x] != 0)
			{
				BMP_setPixelFast(x,y,0x00); // Disabled
				BMP_drawText("t", x, y);
			}
}

int main()
{	
	/*Input*/
	JOY_init();
	JOY_setEventHandler( &myJoyHandler );

	/*Pre-run*/
	reset_emulator();
    BMP_init(TRUE, BG_A, PAL0, FALSE);
	
	//load the UFO rom into memory
	for(int i = 0; i < UFO_length; ++i)
	{
		memory[i + 512] = UFO[i];
	}

	while(1)
	{
		if(game_on == TRUE)
		{
			BMP_reset();
			
			/*Core emulation loop*/
			//emulate cycle
			emulate_cyle();
			
			//draw graphics
			draw_graphics();
			//BMP_setPixelFast(0,2,0xFFFFFF);
			//BMP_drawText("trans z:", 0, 2);
			
			//store input state

			
		}
		else //if we aren't emulating a game we must be in the menu 
		{
		
		}
		//VDP_drawText("Hello!",10,1);
		//VDP_waitVSync();
	}

	return 0;
}
