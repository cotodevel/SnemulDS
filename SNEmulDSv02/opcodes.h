/***********************************************************/
/* This source is part of SNEmulDS                         */
/* ------------------------------------------------------- */
/* (c) 1997-1999, 2006 archeide, All rights reserved.      */
/* Free for non-commercial use                             */
/***********************************************************/

// SNEMUL (c) 1997 Nicolas

#ifndef __snemul_opcodes_h__
#define __snemul_opcodes_h__

#define P_C  0x01
#define P_Z  0x02
#define P_I  0x04
#define P_D  0x08
#define P_X  0x10
#define P_M  0x20
#define P_V  0x40
#define P_N  0x80

#define P_E  0x100

void I_BRK();
void I_ORA_d_x_i();
void I_COP();
void I_ORA_d_s();
void I_TSB_d();
void I_ORA_d();
void I_ASL_d();
void I_ORA_d_i_l();
void I_PHP();
void I_ORA_i();
void I_ASL_A();
void I_PHD();
void I_TSB_a();
void I_ORA_a();
void I_ASL_a();
void I_ORA_al();
void I_BPL();
void I_ORA_d_y_i();
void I_ORA_d_i();
void I_ORA_d_s_y();
void I_TRB_d();
void I_ORA_d_x();
void I_ASL_d_x();
void I_CLC();
void I_TCS();
void I_JSR_a();
void I_PLP();
void I_ROL_A();
void I_SEC();
void I_PHA();
void I_PHK();
void I_CLI();
void I_TCD();
void I_RTS();
void I_ADC_d();
void I_PLA();
void I_ADC();
void I_BVS();
void I_BVS();
void I_SEI();
void I_ADC_a_y();
void I_TDC();
void I_BRA();
void I_STA_d();
void I_STY_a();
void I_LDA_a();
void I_STA_al();
void I_TYA();
void I_STZ_a();
void I_STA_al_x();
void I_LDY();
void I_LDX();
void I_TAY();
void I_LDA();
void I_TAX();
void I_PLB();
void I_STA_a();
void I_LDA_d_i_l_y();
void I_CLV();
void I_LDA_al_x();
void I_REP();
void I_INY();
void I_DEX();
void I_CMP();
void I_BNE();
void I_CLD();
void I_CPX();
void I_SEP();
void I_INX();
void I_SBC();
void I_BEQ();
void I_XCE();
void I_STZ_d_x();
void I_SED();
void I_ROR_a_x();
void I_LDA_a_x();
void I_STX_a();
void I_TXA();
void I_XBA();
void I_STZ_d();
void I_AND_i();
void I_JMP_a();
void I_JMP_al();
void I_CPY();
void I_AND_d_x_i();
void I_SBC_a_y();
void I_ORA_d_x_i();
void I_STA_d_x();
void I_INC_A();
void I_CMP_a();
void I_LDA_d();
void I_CMP_d();
void I_DEC();
void I_WAI();
void I_BIT();
void I_INC_d();
void I_DEC_d();
void I_TXS();
void I_PHX();
void I_PLX();
void I_LDX_a();
void I_STX_d();
void I_RTI();
void I_DEY();
void I_STA_a_x();
void I_BMI();
void I_LDA_d_i();
void I_LDX_d();
void I_LDY_d();
void I_STY_d();
void I_LSR_A();
void I_LDA_d_i_y();
void I_PHY();
void I_PLY();
void I_ASL_A();
void I_LDA_a_y();
void I_ORA();
void I_CPX_d();
void I_ORA_d();
void I_LDX_a_y();
void I_BCC();
void I_BIT_d();
void I_ROL_a();
void I_PHB();
void I_STZ_a_x();
void I_INC_a_x();
void I_LDY_a();
void I_INC_a();
void I_EOR();
void I_ORA_a_x();
void I_ROL_d_x();
void I_JSL_al();
void I_JML_a_i();
void I_ORA_d_i_l();
void I_LDA_d_i_l();
void I_AND_d();
void I_ROR_d();
void I_STA_d_i_l_y();
void I_CMP_d_i_l_y();
void I_LSR_d();
void I_SBC_d();
void I_ROR_A();
void I_AND_d_i_l_y();
void I_STA_d_i_y();
void I_ROL_d();
void I_NOP();
void I_LDA_d_x();
void I_STX_d_y();
void I_TXY();
void I_TYX();
void I_STA_a_y();
void I_LDY_d_x();
void I_EOR_d();
void I_DEC_a();
void I_PEA();
void I_LDA_d_s();
void I_BCS();
void I_STA_d_i();
void I_RTL();
void I_STA_d_x_i();
void I_CMP_al();
void I_LDA_al();
void I_TSB_d();
void I_ORA_d_s_i_y();
void I_JSR_a_x_i();
void I_PHD();
void I_PLD();
void I_ADC_a();
void I_CPY_a();
void I_LDY_a_x();
void I_CPX_a();
void I_EOR_a();
void I_AND_a();
void I_STA_d_i_l();
void I_CMP_a_y();
void I_CMP_al_x();
void I_ASL_a();
void I_SBC_a();
void I_ADC_al_x();
void I_CMP_d_x();
void I_TRB_d();
void I_ADC_a_x();
void I_BIT_a_x();
void I_CMP_a_x();
void I_ADC_d_x();
void I_BVC();
void I_EOR_a_y();
void I_BIT_a();
void I_MVN();
void I_EOR_a_x();
void I_SBC_a_x();
void I_DEC_a_x();
void I_AND_al_x();
void I_CPY_d();
void I_BRL();
void I_ADC_d_i();
void I_ADC_d_x_i();
void I_AND_a_x();
void I_AND_a_y();
void I_PER();
void I_JMP_a_x_i();
void I_STY_d_x();
void I_DEC_d_x();
void I_INC_d_x();
void I_JMP_a_i();
void I_ORA_d_x();
void I_CMP_d_i_y();
void I_TSX();
void I_STA_d_s();
void I_ASL_d_x();
void I_LDA_d_x_i();
void I_LDX_d_y();
void I_SBC_d_x();
void I_ORA_a_y();
void I_AND_d_x();
void I_ASL_a_x();
void I_LSR_a_x();
void I_ORA_al_x();
void I_EOR_d_x();
void I_ROL_a_x();
void I_BIT_d_x();
void I_TRB_a();
void I_TSC();
void I_SBC_al_x();
void I_ROR_d_x();
void I_ADC_al();
void I_AND_d_i_y();
void I_ADC_d_i_y();
void I_ROR_a();
void I_SBC_d_i_y();
void I_ADC_d_s();
void I_CMP_d_i_l();
void I_CMP_d_i();
void I_SBC_d_s();
void I_LSR_a();
void I_SBC_d_i_l_y();
void I_ADC_d_i_l();
void I_ADC_d_i_l_y();
void I_CMP_d_s();
void I_EOR_d_i_l();
void I_EOR_d_i_l_y();
void I_LDA_d_s_i_y();
void I_EOR_al_x();
void I_SBC_d_i();
void I_AND_d_i();
void I_SBC_al();
void I_AND_al();
void I_AND_d_s();
void I_PEI();
void I_ORA_d_i_y();
void I_AND_d_i_l();
void I_AND_d_s_i_y();
void I_AND_d_x_i();
void I_AND_d_s();
void I_MVP();
void I_EOR_al();
void I_EOR_d_i_y();
void I_EOR_d_i();
void I_EOR_d_s();
void I_EOR_d_s_i_y();
void I_LSR_d_x();
void I_ADC_d_s_i_y();
void I_STA_d_s_i_y();
void I_CMP_d_x_i();
void I_CMP_d_s_i_y();
void I_SBC_d_x_i();
void I_SBC_d_i_l();
void I_SBC_d_s_i_y();
void I_EOR_d_x_i();
void I_ORA_d_i_l_y();



void GoDMI();

extern unsigned short P;
extern unsigned short PC;
extern unsigned char  PB, DB, t;
extern unsigned short A, X, Y, D, S;
extern long Cycles;

#endif
