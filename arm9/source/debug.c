#if 0
void APU_printOpcode(uint8 opcode, uint16 pc, uint8 pc1, uint8 pc2)
{
  sint8	buf[50];
  
  switch(opcode) {
    case 0xE8 : sprintf(buf,"MOV A,#%02X", pc1); break;
    case 0xE6 : sprintf(buf,"MOV A,(X)"); break;
    case 0xBF : sprintf(buf,"MOV A,(X)+"); break;
    case 0xE4 : sprintf(buf,"MOV A,%02X", pc1); break;
    case 0xF4 : sprintf(buf,"MOV A,%02X+X", pc1); break;
    case 0xE5 : sprintf(buf,"MOV A,%04X", pc1+pc2*0x100); break;
    case 0xF5 : sprintf(buf,"MOV A,%04X+X", pc1+pc2*0x100); break;
    case 0xF6 : sprintf(buf,"MOV A,%04X+Y", pc1+pc2*0x100); break;
    case 0xE7 : sprintf(buf,"MOV A,(%02X+X)", pc1); break;
    case 0xF7 : sprintf(buf,"MOV A,(%02X)+Y", pc1); break;
    case 0xCD : sprintf(buf,"MOV X,#%02X", pc1); break;
    case 0xF8 : sprintf(buf,"MOV X,%02X", pc1); break;
    case 0xF9 : sprintf(buf,"MOV X,%02X+Y", pc1); break;
    case 0xE9 : sprintf(buf,"MOV X,%04X", pc1+pc2*0x100); break;
    case 0x8D : sprintf(buf,"MOV Y,#%02X", pc1); break;
    case 0xEB : sprintf(buf,"MOV Y,%02X", pc1); break;
    case 0xFB : sprintf(buf,"MOV Y,%02X+X", pc1); break;
    case 0xEC : sprintf(buf,"MOV Y,%04X", pc1+pc2*0x100); break;

    case 0xC6 : sprintf(buf,"MOV (X),A"); break;
    case 0xAF : sprintf(buf,"MOV (X)+,A"); break;
    case 0xC4 : sprintf(buf,"MOV %02X,A", pc1); break;
    case 0xD4 : sprintf(buf,"MOV %02X+X,A", pc1); break;
    case 0xC5 : sprintf(buf,"MOV %04X,A", pc1+pc2*0x100); break;
    case 0xD5 : sprintf(buf,"MOV %04X+X,A", pc1+pc2*0x100); break;
    case 0xD6 : sprintf(buf,"MOV %04X+Y,A", pc1+pc2*0x100); break;
    case 0xC7 : sprintf(buf,"MOV (%02X+X),A", pc1); break;
    case 0xD7 : sprintf(buf,"MOV (%02X)+Y,A", pc1); break;
    case 0xD8 : sprintf(buf,"MOV %02X,X", pc1); break;
    case 0xD9 : sprintf(buf,"MOV %02X+Y,X", pc1); break;
    case 0xC9 : sprintf(buf,"MOV %04X,X", pc1+pc2*0x100); break;
    case 0xCB : sprintf(buf,"MOV %02X,Y", pc1); break;
    case 0xDB : sprintf(buf,"MOV %02X+X,Y", pc1); break;
    case 0xCC : sprintf(buf,"MOV %04X,Y", pc1+pc2*0x100); break;

    case 0x7D : sprintf(buf,"MOV A,X"); break;
    case 0xDD : sprintf(buf,"MOV A,Y"); break;
    case 0x5D : sprintf(buf,"MOV X,A"); break;
    case 0xFD : sprintf(buf,"MOV Y,A"); break;
    case 0x9D : sprintf(buf,"MOV X,SP"); break;
    case 0xBD : sprintf(buf,"MOV SP,X"); break;
    case 0xFA : sprintf(buf,"MOV %02X,%02X", pc2, pc1); break;
    case 0x8F : sprintf(buf,"MOV %02X,#%02X", pc2, pc1); break;

    case 0x88 : sprintf(buf,"ADC A,#%02X", pc1); break;
    case 0x86 : sprintf(buf,"ADC A,(X)"); break;
    case 0x84 : sprintf(buf,"ADC A,%02X", pc1); break;
    case 0x94 : sprintf(buf,"ADC A,%02X+X", pc1); break;
    case 0x85 : sprintf(buf,"ADC A,%04X", pc1+pc2*0x100); break;
    case 0x95 : sprintf(buf,"ADC A,%04X+X", pc1+pc2*0x100); break;
    case 0x96 : sprintf(buf,"ADC A,%04X+Y", pc1+pc2*0x100); break;
    case 0x87 : sprintf(buf,"ADC A,(%02X+X)", pc1); break;
    case 0x97 : sprintf(buf,"ADC A,(%02X)+Y", pc1); break;
    case 0x99 : sprintf(buf,"ADC (X),(Y)"); break;
    case 0x89 : sprintf(buf,"ADC %02X,%02X", pc2, pc1); break;
    case 0x98 : sprintf(buf,"ADC %02X,#%02X", pc2, pc1); break;

    case 0xA8 : sprintf(buf,"SBC A,#%02X", pc1); break;
    case 0xA6 : sprintf(buf,"SBC A,(X)"); break;
    case 0xA4 : sprintf(buf,"SBC A,%02X", pc1); break;
    case 0xB4 : sprintf(buf,"SBC A,%02X+X", pc1); break;
    case 0xA5 : sprintf(buf,"SBC A,%04X", pc1+pc2*0x100); break;
    case 0xB5 : sprintf(buf,"SBC A,%04X+X", pc1+pc2*0x100); break;
    case 0xB6 : sprintf(buf,"SBC A,%04X+Y", pc1+pc2*0x100); break;
    case 0xA7 : sprintf(buf,"SBC A,(%02X+X)", pc1); break;
    case 0xB7 : sprintf(buf,"SBC A,(%02X)+Y", pc1); break;
    case 0xB9 : sprintf(buf,"SBC (X),(Y)"); break;
    case 0xA9 : sprintf(buf,"SBC %02X,%02X", pc2, pc1); break;
    case 0xB8 : sprintf(buf,"SBC %02X,#%02X", pc2, pc1); break;

    case 0x68 : sprintf(buf,"CMP A,#%02X", pc1); break;
    case 0x66 : sprintf(buf,"CMP A,(X)"); break;
    case 0x64 : sprintf(buf,"CMP A,%02X", pc1); break;
    case 0x74 : sprintf(buf,"CMP A,%02X+X", pc1); break;
    case 0x65 : sprintf(buf,"CMP A,%04X", pc1+pc2*0x100); break;
    case 0x75 : sprintf(buf,"CMP A,%04X+X", pc1+pc2*0x100); break;
    case 0x76 : sprintf(buf,"CMP A,%04X+Y", pc1+pc2*0x100); break;
    case 0x67 : sprintf(buf,"CMP A,(%02X+X)", pc1); break;
    case 0x77 : sprintf(buf,"CMP A,(%02X)+Y", pc1); break;
    case 0x79 : sprintf(buf,"CMP (X),(Y)"); break;
    case 0x69 : sprintf(buf,"CMP %02X,%02X", pc2, pc1); break;
    case 0x78 : sprintf(buf,"CMP %02X,#%02X", pc2, pc1); break;
    case 0xC8 : sprintf(buf,"CMP X,#%02X", pc1); break;
    case 0x3E : sprintf(buf,"CMP X,%02X", pc1); break;
    case 0x1E : sprintf(buf,"CMP X,%04X", pc1+pc2*0x100); break;
    case 0xAD : sprintf(buf,"CMP Y,#%02X", pc1); break;
    case 0x7E : sprintf(buf,"CMP Y,%02X", pc1); break;
    case 0x5E : sprintf(buf,"CMP Y,%04X", pc1+pc2*0x100); break;

    case 0x28 : sprintf(buf,"AND A,#%02X", pc1); break;
    case 0x26 : sprintf(buf,"AND A,(X)"); break;
    case 0x24 : sprintf(buf,"AND A,%02X", pc1); break;
    case 0x34 : sprintf(buf,"AND A,%02X+X", pc1); break;
    case 0x25 : sprintf(buf,"AND A,%04X", pc1+pc2*0x100); break;
    case 0x35 : sprintf(buf,"AND A,%04X+X", pc1+pc2*0x100); break;
    case 0x36 : sprintf(buf,"AND A,%04X+Y", pc1+pc2*0x100); break;
    case 0x27 : sprintf(buf,"AND A,(%02X+X)", pc1); break;
    case 0x37 : sprintf(buf,"AND A,(%02X)+Y", pc1); break;
    case 0x39 : sprintf(buf,"AND (X),(Y)"); break;
    case 0x29 : sprintf(buf,"AND %02X,%02X", pc2, pc1); break;
    case 0x38 : sprintf(buf,"AND %02X,#%02X", pc2, pc1); break;

    case 0x08 : sprintf(buf,"OR A,#%02X", pc1); break;
    case 0x06 : sprintf(buf,"OR A,(X)"); break;
    case 0x04 : sprintf(buf,"OR A,%02X", pc1); break;
    case 0x14 : sprintf(buf,"OR A,%02X+X", pc1); break;
    case 0x05 : sprintf(buf,"OR A,%04X", pc1+pc2*0x100); break;
    case 0x15 : sprintf(buf,"OR A,%04X+X", pc1+pc2*0x100); break;
    case 0x16 : sprintf(buf,"OR A,%04X+Y", pc1+pc2*0x100); break;
    case 0x07 : sprintf(buf,"OR A,(%02X+X)", pc1); break;
    case 0x17 : sprintf(buf,"OR A,(%02X)+Y", pc1); break;
    case 0x19 : sprintf(buf,"OR (X),(Y)"); break;
    case 0x09 : sprintf(buf,"OR %02X,%02X", pc2, pc1); break;
    case 0x18 : sprintf(buf,"OR %02X,#%02X", pc2, pc1); break;

    case 0x48 : sprintf(buf,"EOR A,#%02X", pc1); break;
    case 0x46 : sprintf(buf,"EOR A,(X)"); break;
    case 0x44 : sprintf(buf,"EOR A,%02X", pc1); break;
    case 0x54 : sprintf(buf,"EOR A,%02X+X", pc1); break;
    case 0x45 : sprintf(buf,"EOR A,%04X", pc1+pc2*0x100); break;
    case 0x55 : sprintf(buf,"EOR A,%04X+X", pc1+pc2*0x100); break;
    case 0x56 : sprintf(buf,"EOR A,%04X+Y", pc1+pc2*0x100); break;
    case 0x47 : sprintf(buf,"EOR A,(%02X+X)", pc1); break;
    case 0x57 : sprintf(buf,"EOR A,(%02X)+Y", pc1); break;
    case 0x59 : sprintf(buf,"EOR (X),(Y)"); break;
    case 0x49 : sprintf(buf,"EOR %02X,%02X", pc2, pc1); break;
    case 0x58 : sprintf(buf,"EOR %02X,#%02X", pc2, pc1); break;

    case 0xBC : sprintf(buf,"INC A"); break;
    case 0xAB : sprintf(buf,"INC %02X", pc1); break;
    case 0xBB : sprintf(buf,"INC %02X+X", pc1); break;
    case 0xAC : sprintf(buf,"INC %04X", pc1+pc2*0x100); break;
    case 0x3D : sprintf(buf,"INC X"); break;
    case 0xFC : sprintf(buf,"INC Y"); break;
    case 0x9C : sprintf(buf,"DEC A"); break;
    case 0x8B : sprintf(buf,"DEC %02X", pc1); break;
    case 0x9B : sprintf(buf,"DEC %02X+X", pc1); break;
    case 0x8C : sprintf(buf,"DEC %04X", pc1+pc2*0x100); break;
    case 0x1D : sprintf(buf,"DEC X"); break;
    case 0xDC : sprintf(buf,"DEC Y"); break;

    case 0x1C : sprintf(buf,"ASL A"); break;
    case 0x0B : sprintf(buf,"ASL %02X", pc1); break;
    case 0x1B : sprintf(buf,"ASL %02X+X", pc1); break;
    case 0x0C : sprintf(buf,"ASL %04X", pc1+pc2*0x100); break;

    case 0x5C : sprintf(buf,"LSR A"); break;
    case 0x4B : sprintf(buf,"LSR %02X", pc1); break;
    case 0x5B : sprintf(buf,"LSR %02X+X", pc1); break;
    case 0x4C : sprintf(buf,"LSR %04X", pc1+pc2*0x100); break;

    case 0x3C : sprintf(buf,"ROL A"); break;
    case 0x2B : sprintf(buf,"ROL %02X", pc1); break;
    case 0x3B : sprintf(buf,"ROL %02X+X", pc1); break;
    case 0x2C : sprintf(buf,"ROL %04X", pc1+pc2*0x100); break;

    case 0x7C : sprintf(buf,"ROR A"); break;
    case 0x6B : sprintf(buf,"ROR %02X", pc1); break;
    case 0x7B : sprintf(buf,"ROR %02X+X", pc1); break;
    case 0x6C : sprintf(buf,"ROR %04X", pc1+pc2*0x100); break;

    case 0x9F : sprintf(buf,"XCN"); break;

    case 0xBA : sprintf(buf,"MOVW YA,%02X", pc1); break;
    case 0xDA : sprintf(buf,"MOVW %02X,YA", pc1); break;

    case 0x3A : sprintf(buf,"INCW %02X", pc1); break;
    case 0x1A : sprintf(buf,"DECW %02X", pc1); break;
    case 0x7A : sprintf(buf,"ADDW YA,%02X", pc1); break;
    case 0x9A : sprintf(buf,"SUBW YA,%02X", pc1); break;
    case 0x5A : sprintf(buf,"CMPW YA,%02X", pc1); break;

    case 0xCF : sprintf(buf,"MUL YA"); break;
    case 0x9E : sprintf(buf,"DIV YA,X"); break;

    case 0x2F : sprintf(buf,"BRA %04X", pc+(sint8)pc1+2); break;
    case 0xF0 : sprintf(buf,"BEQ %04X", pc+(sint8)pc1+2); break;
    case 0xD0 : sprintf(buf,"BNE %04X", pc+(sint8)pc1+2); break;
    case 0xB0 : sprintf(buf,"BCS %04X", pc+(sint8)pc1+2); break;
    case 0x90 : sprintf(buf,"BCC %04X", pc+(sint8)pc1+2); break;
    case 0x70 : sprintf(buf,"BVS %04X", pc+(sint8)pc1+2); break;
    case 0x50 : sprintf(buf,"BVC %04X", pc+(sint8)pc1+2); break;
    case 0x30 : sprintf(buf,"BMI %04X", pc+(sint8)pc1+2); break;
    case 0x10 : sprintf(buf,"BPL %04X", pc+(sint8)pc1+2); break;
    case 0x5F : sprintf(buf,"JMP %04X", pc1+pc2*0x100); break;
    case 0x1F : sprintf(buf,"JMP (%04X+X)", pc1+pc2*0x100); break;

    case 0x3F : sprintf(buf,"CALL %04X", pc1+pc2*0x100); break;
    case 0x6F : sprintf(buf,"RET"); break;

    case 0x2D : sprintf(buf,"PUSH A"); break;
    case 0x4D : sprintf(buf,"PUSH X"); break;
    case 0x6D : sprintf(buf,"PUSH Y"); break;
    case 0x0D : sprintf(buf,"PUSH PSW"); break;

    case 0xAE : sprintf(buf,"POP A"); break;
    case 0xCE : sprintf(buf,"POP X"); break;
    case 0xEE : sprintf(buf,"POP Y"); break;
    case 0x8E : sprintf(buf,"POP PSW"); break;

    case 0x2E : sprintf(buf,"CBNE %02X, %04X",   pc1, pc+(sint8)pc2+3); break;
    case 0xDE : sprintf(buf,"CBNE %02X+X, %04X", pc1, pc+(sint8)pc2+3); break;
    case 0x6E : sprintf(buf,"DBNZ %02X, %04X",   pc1, pc+(sint8)pc2+3); break;
    case 0xFE : sprintf(buf,"DBNZ Y, %04X", pc+(sint8)pc1+2); break;

    case 0x60 : sprintf(buf,"CLRC"); break;
    case 0x80 : sprintf(buf,"SETC"); break;
    case 0xED : sprintf(buf,"NOTC"); break;
    case 0xE0 : sprintf(buf,"CLRV"); break;
    case 0x20 : sprintf(buf,"CLRP"); break;
    case 0x40 : sprintf(buf,"SETP"); break;

    case 0x0E : sprintf(buf,"TSET1 %04X", pc1+pc2*0x100); break;
    case 0x4E : sprintf(buf,"TCLR1 %04X", pc1+pc2*0x100); break;
    case 0xAA : sprintf(buf,"MOV1 C,%04X", pc1+pc2*0x100); break;
    case 0xCA : sprintf(buf,"MOV1 %04X,C", pc1+pc2*0x100); break;
    case 0xEA : sprintf(buf,"NOT1 %04X", pc1+pc2*0x100); break;
    case 0x4A : sprintf(buf,"AND1 C,%04X", pc1+pc2*0x100); break;
    case 0x6A : sprintf(buf,"AND1 C,/%04X", pc1+pc2*0x100); break;
    case 0x0A : sprintf(buf,"OR1 C,%04X", pc1+pc2*0x100); break;
    case 0x2A : sprintf(buf,"OR1 C,/%04X", pc1+pc2*0x100); break;
    case 0x8A : sprintf(buf,"EOR1 C,%04X", pc1+pc2*0x100); break;

    case 0x4F : sprintf(buf,"PCALL %02X", pc1); break;

    case 0x02 : sprintf(buf,"SET1 1,%02X", pc1); break;
    case 0x12 : sprintf(buf,"CLR1 1,%02X", pc1); break;
    case 0x22 : sprintf(buf,"SET1 2,%02X", pc1); break;
    case 0x32 : sprintf(buf,"CLR1 2,%02X", pc1); break;
    case 0x42 : sprintf(buf,"SET1 3,%02X", pc1); break;
    case 0x52 : sprintf(buf,"CLR1 3,%02X", pc1); break;
    case 0x62 : sprintf(buf,"SET1 4,%02X", pc1); break;
    case 0x72 : sprintf(buf,"CLR1 4,%02X", pc1); break;
    case 0x82 : sprintf(buf,"SET1 5,%02X", pc1); break;
    case 0x92 : sprintf(buf,"CLR1 5,%02X", pc1); break;
    case 0xA2 : sprintf(buf,"SET1 6,%02X", pc1); break;
    case 0xB2 : sprintf(buf,"CLR1 6,%02X", pc1); break;
    case 0xC2 : sprintf(buf,"SET1 7,%02X", pc1); break;
    case 0xD2 : sprintf(buf,"CLR1 7,%02X", pc1); break;
    case 0xE2 : sprintf(buf,"SET1 8,%02X", pc1); break;
    case 0xF2 : sprintf(buf,"CLR1 8,%02X", pc1); break;

    case 0x01 : sprintf(buf,"TCALL 0"); break;
    case 0x11 : sprintf(buf,"TCALL 1"); break;
    case 0x21 : sprintf(buf,"TCALL 2"); break;
    case 0x31 : sprintf(buf,"TCALL 3"); break;
    case 0x41 : sprintf(buf,"TCALL 4"); break;
    case 0x51 : sprintf(buf,"TCALL 5"); break;
    case 0x61 : sprintf(buf,"TCALL 6"); break;
    case 0x71 : sprintf(buf,"TCALL 7"); break;
    case 0x81 : sprintf(buf,"TCALL 8"); break;
    case 0x91 : sprintf(buf,"TCALL 9"); break;
    case 0xA1 : sprintf(buf,"TCALL 10"); break;
    case 0xB1 : sprintf(buf,"TCALL 11"); break;
    case 0xC1 : sprintf(buf,"TCALL 12"); break;
    case 0xD1 : sprintf(buf,"TCALL 13"); break;
    case 0xE1 : sprintf(buf,"TCALL 14"); break;
    case 0xF1 : sprintf(buf,"TCALL 15"); break;

    case 0xDF : sprintf(buf,"DAA (N.I.)"); break;
    case 0xBE : sprintf(buf,"DAS (N.I.)"); break;
    case 0xA0 : sprintf(buf,"EI (N.I.)"); break;
    case 0xC0 : sprintf(buf,"SI (N.I.)"); break;

    case 0x00 : sprintf(buf,"NOP"); break;
    case 0xEF : sprintf(buf,"SLEEP"); break;
    case 0xFF : sprintf(buf,"STOP"); break;

    default   : sprintf(buf,"?(%02X)", opcode); break;
  }
  FS_printlog(buf);
  FS_printlog("\n");
}

uint8	*g_ApuTrace = (uint8*)memUncached(0x2FE0000);
uint32	*g_ApuTrace32 = (uint32*)memUncached(0x2FE0000);

void	APU_printLog()
{
	uint8	opcode;
	uint8	pc1;
	uint8	pc2;
	uint16	realpc;
	uint8	A;
	uint32	X;
	uint32	Y;
	uint8	S;
	uint8	P;
	int		i;
	
	for (i = 0; i < 8192;)
	{ 
/*		opcode = g_ApuTrace[i++];
		A = g_ApuTrace[i++];
		realpc = g_ApuTrace[i++];
		realpc += (g_ApuTrace[i++]<<8);		
		pc1 = g_ApuTrace[i++];
		pc2 = g_ApuTrace[i++];
		X = g_ApuTrace[i++];
		Y = g_ApuTrace[i++];*/
		
		uint8 *pc = (uint8 *)g_ApuTrace32[i++];
		A = g_ApuTrace32[i++] >> 24;
		X = g_ApuTrace32[i++];
		Y = g_ApuTrace32[i++];
/*		X = g_ApuTrace32[i++] >> 24;Z
		Y = g_ApuTrace32[i++] >> 24;*/
		
		realpc = pc - APU_RAM_ADDRESS;
		opcode = APU_RAM_ADDRESS[realpc];
		pc1 = APU_RAM_ADDRESS[realpc+1];
		pc2 = APU_RAM_ADDRESS[realpc+2];

		/* todo add A X Y S P */
		FS_flog("%08x %02X %02X %02X A:%02X X:%08X Y:%08X %04X  ", 
			 g_ApuTrace32+i, opcode, pc1, pc2, A, X, Y, realpc);
		APU_printOpcode(opcode, realpc, pc1, pc2);
	}
}
#endif
