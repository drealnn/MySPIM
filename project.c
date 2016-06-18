/*
Daniel Sledd
*/
#include "spimcore.h"

unsigned concatenate(unsigned x, unsigned y);

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

	switch(ALUControl)
	{
		// add
		case(0b000):
			*ALUresult = A + B;
			break;
		// subtract
		case (0b001):
			*ALUresult = A - B;

			break;
		// signed slt
		case (0b010):

			if ((int)A < (int)B)
				*ALUresult = 1;
			else
				*ALUresult = 0;

			break;
		// unsigned slt
		case (0b011):

			if (A < B)
				*ALUresult = 1;
			else
				*ALUresult = 0;

			break;
		case (0b100):
			*ALUresult = A & B;

			break;
		case (0b101):
			*ALUresult = A | B;

			break;
		case (0b110):
			*ALUresult = B << 16;

			break;
	}

	if (*ALUresult == 0)
		*Zero = 1;
	else
		*Zero = 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
	// if we are trying to access unallocated memory (beyond 64kB block), or if our address is not word aligned, halt the program.
	if (((PC) > 65535) || ((PC) < 0))
		return 1;
	else if ((PC % 4) != 0)
		return 1;

	// fetch the instruction at index (PC>>2) from memory
    *instruction = Mem[PC>>2];

   
    return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{

    unsigned opMask =       0b11111100000000000000000000000000;
    unsigned r1Mask =       0b00000011111000000000000000000000;
    unsigned r2Mask =       0b00000000000111110000000000000000;
    unsigned r3Mask =       0b00000000000000001111100000000000;
    unsigned functMask =    0b00000000000000000000000000111111;
    unsigned offsetMask =   0b00000000000000001111111111111111;
    unsigned jsecMask =     0b00000011111111111111111111111111;

    *op = (instruction & opMask) >> (32-6);

    *r1 = (instruction & r1Mask) >> (32-11);
    *r2 = (instruction & r2Mask) >> (32-16);
    *r3 = (instruction & r3Mask) >> (32-21);
    *funct = instruction & functMask;
    *offset = instruction & offsetMask;
    *jsec = instruction & jsecMask;


}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    // look at opcode to update the appropriate controls
    // if opcode is 0, then we are performing an R-type instruction (the funct field is looked at for specific ALU operation)

    switch(op)
    {
        // R-Type instruction
        case(0b000000):
            controls->RegDst = 1;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 7;
            controls->MemWrite = 0;
            controls->ALUSrc = 0;
            controls->RegWrite = 1;
            break;
        // Add immediate
        case (0b001000):
            controls->RegDst = 0;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 0;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;
        // Load word
        case (0b100011):
            controls->RegDst = 0;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 1;
            controls->MemtoReg = 1;
            controls->ALUOp = 0;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;
        // Save word
        case (0b101011):
            controls->RegDst = 2;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 2;
            controls->ALUOp = 0;
            controls->MemWrite = 1;
            controls->ALUSrc = 1;
            controls->RegWrite = 0;
            break;
        // Load upper immediate
        case (0b001111):
            controls->RegDst = 0;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 0b110;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;
		// Branch on equal
		case (0b000100):
            controls->RegDst = 2;
            controls->Jump = 0;
            controls->Branch = 1;
            controls->MemRead = 0;
            controls->MemtoReg = 2;
            controls->ALUOp = 0b001;
            controls->MemWrite = 0;
            controls->ALUSrc = 0;
            controls->RegWrite = 0;
            break;
		// set less than immediate
		case (0b001010):
            controls->RegDst = 0;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 0b010;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;

		// set less than immediate unsigned
		case (0b001001):
            controls->RegDst = 0;
            controls->Jump = 0;
            controls->Branch = 0;
            controls->MemRead = 0;
            controls->MemtoReg = 0;
            controls->ALUOp = 0b011;
            controls->MemWrite = 0;
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;
		// jump
		case (0b000010):
            controls->RegDst = 2;
            controls->Jump = 1;
            controls->Branch = 0;
            controls->MemRead = 2;
            controls->MemtoReg = 2;
            controls->ALUOp = 0b000;
            controls->MemWrite = 0;
            controls->ALUSrc = 2;
            controls->RegWrite = 0;
            break;
		// Unknown OpCode - Halt the program
        default:
            return 1;
            break;
    }
	return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
	// get data from the registers
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
	unsigned signMask = 0b1000000000000000;
	unsigned append1 = 0b11111111111111110000000000000000;
	char signBit = (signMask & offset) >> 15;

	// if our signbit is 0, our offset is positive (shift in 16 more 0's for 32 bit extended_value)
	if (signBit == 0)
	{
		*extended_value = offset;
	}
	// our offset is negative (shift in 16 more 1's for 32 bit extended_value)
	else
	{
		*extended_value = offset | append1;
	}
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	// Not using extended value
	if (ALUSrc == 0)
	{
		// R-Type instruction
		if (ALUOp == 0b111)
		{
			// look at funct field for appropriate ALU instruction
			switch(funct)
			{
				// Add
				case(0b100000):
					ALU(data1, data2, 0, ALUresult, Zero);
					break;
				// Subtract
				case(0b100010):
					ALU(data1, data2, 1, ALUresult, Zero);
					break;
				// AND
				case(0b100100):
					ALU(data1, data2, 0b100, ALUresult, Zero);
					break;
				// OR
				case(0b100101):
					ALU(data1, data2, 0b101, ALUresult, Zero);
					break;
				// Set on less than
				case(0b101010):
					ALU(data1, data2, 0b010, ALUresult, Zero);
					break;
				// Set on less than unsigned
				case(0b101011):
					ALU(data1, data2, 0b011, ALUresult, Zero);
					break;
				// No matching funct, halt the program.
				default:
					return 1;
					break;
			}
		}
		// is there a non-R-type instruction that uses both registers?
		// for now, if we are using both registers, but it is not an R-type, halt the program.
		else
		{
			switch(ALUOp)
			{
			//Add
			case(0):
				ALU(data1, data2, 0, ALUresult, Zero);
				break;
			//Subtract
			case(1):
				ALU(data1, data2, 1, ALUresult, Zero);
				break;
			//Set less Than
			case(0b010):
				ALU(data1, data2, 0b010, ALUresult, Zero);
				break;
			//Set less than unsigned
			case(0b011):
				ALU(data1, data2, 0b011, ALUresult, Zero);
				break;
			// And
			case(0b100):
				ALU(data1, data2, 0b100, ALUresult, Zero);
				break;
			// OR
			case(0b101):
				ALU(data1, data2, 0b101, ALUresult, Zero);
				break;
			// shift left data2 value by 16 bits
			case(0b110):
				ALU(data1, data2, 0b110, ALUresult, Zero);
				break;
			// no matching ALUOp, halt the program
			default:
				return 1;
				break;
			}
		}

	}
	// We use the Extended_value
	else if (ALUSrc == 1)
	{
		switch(ALUOp)
		{
			//Add
			case(0):
				ALU(data1, extended_value, 0, ALUresult, Zero);
				break;
			//Subtract
			case(1):
				ALU(data1, extended_value, 1, ALUresult, Zero);
				break;
			//Set less Than
			case(0b010):
				ALU(data1, extended_value, 0b010, ALUresult, Zero);
				break;
			//Set less than unsigned
			case(0b011):
				ALU(data1, extended_value, 0b011, ALUresult, Zero);
				break;
			// And
			case(0b100):
				ALU(data1, extended_value, 0b100, ALUresult, Zero);
				break;
			// OR
			case(0b101):
				ALU(data1, extended_value, 0b101, ALUresult, Zero);
				break;
			// shift left extended value by 16 bits
			case(0b110):
				ALU(data1, extended_value, 0b110, ALUresult, Zero);
				break;
			// no matching ALUOp, halt the program
			default:
				return 1;
				break;
		}

	}else if (ALUSrc != 2)
		return 1;

	// calculated the ALUresult and Zero values, continue...
	return 0;

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
	// read from memory
	// separate function will write to register
	if (MemRead == 1)
	{
		// if our ALUresult doesn't represent a word aligned address or its out of bounds, halt the program.
		if ((ALUresult % 4) != 0)
			return 1;
		else if (((ALUresult) > 65535) || ((ALUresult) < 0))
			return 1;

		*memdata = Mem[ALUresult>>2];


	}
	// write to memory
	else if (MemWrite == 1)
	{
		// if our ALUresult doesn't represent a word aligned address or its out of bounds, halt the program.
		if ((ALUresult % 4) != 0)
			return 1;
		else if (((ALUresult) > 65535) || ((ALUresult) < 0))
			return 1;

		Mem[ALUresult>>2] = data2;
	}

	return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
	// If we are writing to a register
	if (RegWrite == 1)
	{
		// if its not a load instruction
		if (MemtoReg == 0)
		{
			// destination register is specified by r3 (R-type instruction)
			if (RegDst == 1)
			{
				// write data from ALUresult to the register at address r3
				Reg[r3] = ALUresult;
			}
			// destination register is specified by r2 (I or J-type instruction)
			else if (RegDst == 0)
			{
				Reg[r2] = ALUresult;
			}
		}
		// Load from Memory into register
		else if (MemtoReg == 1)
		{
			//Load from Memory into destination register (r2)
			if (RegDst == 0)
			{
				Reg[r2] = memdata;
			}

		}
	}
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC = *PC+4;
	// if jump is asserted, then we use pseudodirect addressing
	if (Jump == 1)
	{
		//shift jsec by 2 to left and concatenate (PC + 4) to the beginning of jsec
		int mask = 0b11110000000000000000000000000000;
		*PC = *PC & mask;
		*PC = *PC >> 28;
		//*PC = (*PC | (jsec << 2));
		*PC = concatenate((*PC), jsec << 2);

	}
	// if branch is asserted, and both registers are equal, then we branch on equal using PC relative addressing
	else if (Branch == 1 && Zero == 1)
	{
		//shift extended_value by 2 to left and add that to (PC+4)
		*PC = (*PC) + (extended_value << 2);
	}
	// else increment PC to PC + 4


}

unsigned concatenate(unsigned x, unsigned y)
{
	unsigned pow = 10;
	while (y >= pow)
	{
		pow *= 10;
	}
	return (x*pow + y);

}

//Wunderbar!
