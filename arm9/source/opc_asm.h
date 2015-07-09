#ifndef OPC_ASM_H_
#define OPC_ASM_H_

//opc_asm.h

/* Optimise short loop going to those instruction */

IN_DTCM
uint8	OpcShortLoop[] = { 
0, //&&I_BRK,
0, //&&I_ORA_d_x_i,
0, //&&I_COP,
0, //&&I_ORA_d_s,
0, //&&I_TSB_d,
0, //&&I_ORA_d,
0, //&&I_ASL_d,
0, //&&I_ORA_d_i_l,
0, //&&I_PHP,
0, //&&I_ORA_i,
0, //&&I_ASL_A,
0, //&&I_PHD,
0, //&&I_TSB_a,
0, //&&I_ORA_a,
0, //&&I_ASL_a,
0, //&&I_ORA_al,
0, //&&I_BPL,
0, //&&I_ORA_d_i_y,
0, //&&I_ORA_d_i,
0, //&&I_ORA_d_s_i_y,
0, //&&I_TRB_d,
0, //&&I_ORA_d_x,
0, //&&I_ASL_d_x,
0, //&&I_ORA_d_i_l_y,
0, //&&I_CLC,
0, //&&I_ORA_a_y,
0, //&&I_INC_A,
0, //&&I_TCS,
0, //&&I_TRB_a,
0, //&&I_ORA_a_x,
0, //&&I_ASL_a_x,
0, //&&I_ORA_al_x,
0, //&&I_JSR_a,
0, //&&I_AND_d_x_i,
0, //&&I_JSL_al,
0, //&&I_AND_d_s,
0, //&&I_BIT_d,
0, //&&I_AND_d,
0, //&&I_ROL_d,
0, //&&I_AND_d_i_l,
0, //&&I_PLP,
0, //&&I_AND_i,
0, //&&I_ROL_A,
0, //&&I_PLD,
0, //&&I_BIT_a,
0, //&&I_AND_a,
0, //&&I_ROL_a,
0, //&&I_AND_al,
0, //&&I_BMI,
0, //&&I_AND_d_i_y,
0, //&&I_AND_d_i,
0, //&&I_AND_d_s_i_y,
0, //&&I_BIT_d_x,
0, //&&I_AND_d_x,
0, //&&I_ROL_d_x,
0, //&&I_AND_d_i_l_y,
0, //&&I_SEC,
0, //&&I_AND_a_y,
0, //&&I_DEC,
0, //&&I_TSC,
0, //&&I_BIT_a_x,
0, //&&I_AND_a_x,
0, //&&I_ROL_a_x,
0, //&&I_AND_al_x,
0, //&&I_RTI,
0, //&&I_EOR_d_x_i,
0, //&&illegal,
0, //&&I_EOR_d_s,
0, //&&I_MVP,
0, //&&I_EOR_d,
0, //&&I_LSR_d,
0, //&&I_EOR_d_i_l,
0, //&&I_PHA,
0, //&&I_EOR,
0, //&&I_LSR_A,
0, //&&I_PHK,
0, //&&I_JMP_a,
0, //&&I_EOR_a,
0, //&&I_LSR_a,
0, //&&I_EOR_al,
0, //&&I_BVC,
0, //&&I_EOR_d_i_y,
0, //&&I_EOR_d_i,
0, //&&I_EOR_d_s_i_y,
0, //&&I_MVN,
0, //&&I_EOR_d_x,
0, //&&I_LSR_d_x,
0, //&&I_EOR_d_i_l_y,
0, //&&I_CLI,
0, //&&I_EOR_a_y,
0, //&&I_PHY,
0, //&&I_TCD,
0, //&&I_JMP_al,
0, //&&I_EOR_a_x,
0, //&&I_LSR_a_x,
0, //&&I_EOR_al_x,
0, //&&I_RTS,
0, //&&I_ADC_d_x_i,
0, //&&I_PER,
0, //&&I_ADC_d_s,
0, //&&I_STZ_d,
0, //&&I_ADC_d,
0, //&&I_ROR_d,
0, //&&I_ADC_d_i_l,
0, //&&I_PLA,
0, //&&I_ADC,
0, //&&I_ROR_A,
0, //&&I_RTL,
0, //&&I_JMP_a_i,
0, //&&I_ADC_a,
0, //&&I_ROR_a,
0, //&&I_ADC_al,
0, //&&I_BVS,
0, //&&I_ADC_d_i_y,
0, //&&I_ADC_d_i,
0, //&&I_ADC_d_s_i_y,
0, //&&I_STZ_d_x,
0, //&&I_ADC_d_x,
0, //&&I_ROR_d_x,
0, //&&I_ADC_d_i_l_y,
0, //&&I_SEI,
0, //&&I_ADC_a_y,
0, //&&I_PLY,
0, //&&I_TDC,
0, //&&I_JMP_a_x_i,
0, //&&I_ADC_a_x,
0, //&&I_ROR_a_x,
0, //&&I_ADC_al_x,
0, //&&I_BRA,
0, //&&I_STA_d_x_i,
0, //&&I_BRL,
0, //&&I_STA_d_s,
0, //&&I_STY_d,
0, //&&I_STA_d,
0, //&&I_STX_d,
0, //&&I_STA_d_i_l,
0, //&&I_DEY,
0, //&&I_BIT,
0, //&&I_TXA,
0, //&&I_PHB,
0, //&&I_STY_a,
0, //&&I_STA_a,
0, //&&I_STX_a,
0, //&&I_STA_al,
0, //&&I_BCC,
0, //&&I_STA_d_i_y,
0, //&&I_STA_d_i,
0, //&&I_STA_d_s_i_y,
0, //&&I_STY_d_x,
0, //&&I_STA_d_x,
0, //&&I_STX_d_y,
0, //&&I_STA_d_i_l_y,
0, //&&I_TYA,
0, //&&I_STA_a_y,
0, //&&I_TXS,
0, //&&I_TXY,
0, //&&I_STZ_a,
0, //&&I_STA_a_x,
0, //&&I_STZ_a_x,
0, //&&I_STA_al_x,
1, //&&I_LDY,
1, //&&I_LDA_d_x_i,
1, //&&I_LDX,
1, //&&I_LDA_d_s,
1, //&&I_LDY_d,
1, //&&I_LDA_d,
1, //&&I_LDX_d,
1, //&&I_LDA_d_i_l,
0, //&&I_TAY,
1, //&&I_LDA,
0, //&&I_TAX,
0, //&&I_PLB,
1, //&&I_LDY_a,
1, //&&I_LDA_a,
1, //&&I_LDX_a,
1, //&&I_LDA_al,
0, //&&I_BCS,
1, //&&I_LDA_d_i_y,
1, //&&I_LDA_d_i,
1, //&&I_LDA_d_s_i_y,
1, //&&I_LDY_d_x,
1, //&&I_LDA_d_x,
1, //&&I_LDX_d_y,
1, //&&I_LDA_d_i_l_y,
0, //&&I_CLV,
1, //&&I_LDA_a_y,
0, //&&I_TSX,
0, //&&I_TYX,
1, //&&I_LDY_a_x,
1, //&&I_LDA_a_x,
1, //&&I_LDX_a_y,
1, //&&I_LDA_al_x,
0, //&&I_CPY,
0, //&&I_CMP_d_x_i,
0, //&&I_REP,
0, //&&I_CMP_d_s,
0, //&&I_CPY_d,
0, //&&I_CMP_d,
0, //&&I_DEC_d,
0, //&&I_CMP_d_i_l,
0, //&&I_INY,
0, //&&I_CMP,
0, //&&I_DEX,
0, //&&I_WAI,
0, //&&I_CPY_a,
0, //&&I_CMP_a,
0, //&&I_DEC_a,
0, //&&I_CMP_al,
0, //&&I_BNE,
0, //&&I_CMP_d_i_y,
0, //&&I_CMP_d_i,
0, //&&I_CMP_d_s_i_y,
0, //&&I_PEI,
0, //&&I_CMP_d_x,
0, //&&I_DEC_d_x,
0, //&&I_CMP_d_i_l_y,
0, //&&I_CLD,
0, //&&I_CMP_a_y,
0, //&&I_PHX,
0, //&&illegal,
0, //&&I_JML_a_i,
0, //&&I_CMP_a_x,
0, //&&I_DEC_a_x,
0, //&&I_CMP_al_x,
0, //&&I_CPX,
0, //&&I_SBC_d_x_i,
0, //&&I_SEP,
0, //&&I_SBC_d_s,
0, //&&I_CPX_d,
0, //&&I_SBC_d,
0, //&&I_INC_d,
0, //&&I_SBC_d_i_l,
0, //&&I_INX,
0, //&&I_SBC,
0, //&&I_NOP,
0, //&&I_XBA,
0, //&&I_CPX_a,
0, //&&I_SBC_a,
0, //&&I_INC_a,
0, //&&I_SBC_al,
0, //&&I_BEQ,
0, //&&I_SBC_d_i_y,
0, //&&I_SBC_d_i,
0, //&&I_SBC_d_s_i_y,
0, //&&I_PEA,
0, //&&I_SBC_d_x,
0, //&&I_INC_d_x,
0, //&&I_SBC_d_i_l_y,
0, //&&I_SED,
0, //&&I_SBC_a_y,
0, //&&I_PLX,
0, //&&I_XCE,
0, //&&I_JSR_a_x_i,
0, //&&I_SBC_a_x,
0, //&&I_INC_a_x,
0, //&&I_SBC_al_x
};	

/*
uint8	OpcShortLoop[] = { 
0, //&&I_BRK,
0, //&&I_ORA_d_x_i,
0, //&&I_COP,
0, //&&I_ORA_d_s,
0, //&&I_TSB_d,
0, //&&I_ORA_d,
0, //&&I_ASL_d,
0, //&&I_ORA_d_i_l,
0, //&&I_PHP,
0, //&&I_ORA_i,
0, //&&I_ASL_A,
0, //&&I_PHD,
0, //&&I_TSB_a,
0, //&&I_ORA_a,
0, //&&I_ASL_a,
0, //&&I_ORA_al,
0, //&&I_BPL,
0, //&&I_ORA_d_i_y,
0, //&&I_ORA_d_i,
0, //&&I_ORA_d_s_i_y,
0, //&&I_TRB_d,
0, //&&I_ORA_d_x,
0, //&&I_ASL_d_x,
0, //&&I_ORA_d_i_l_y,
0, //&&I_CLC,
0, //&&I_ORA_a_y,
0, //&&I_INC_A,
0, //&&I_TCS,
0, //&&I_TRB_a,
0, //&&I_ORA_a_x,
0, //&&I_ASL_a_x,
0, //&&I_ORA_al_x,
0, //&&I_JSR_a,
0, //&&I_AND_d_x_i,
0, //&&I_JSL_al,
0, //&&I_AND_d_s,
0, //&&I_BIT_d,
0, //&&I_AND_d,
0, //&&I_ROL_d,
0, //&&I_AND_d_i_l,
0, //&&I_PLP,
0, //&&I_AND_i,
0, //&&I_ROL_A,
0, //&&I_PLD,
0, //&&I_BIT_a,
0, //&&I_AND_a,
0, //&&I_ROL_a,
0, //&&I_AND_al,
0, //&&I_BMI,
0, //&&I_AND_d_i_y,
0, //&&I_AND_d_i,
0, //&&I_AND_d_s_i_y,
0, //&&I_BIT_d_x,
0, //&&I_AND_d_x,
0, //&&I_ROL_d_x,
0, //&&I_AND_d_i_l_y,
0, //&&I_SEC,
0, //&&I_AND_a_y,
0, //&&I_DEC,
0, //&&I_TSC,
0, //&&I_BIT_a_x,
0, //&&I_AND_a_x,
0, //&&I_ROL_a_x,
0, //&&I_AND_al_x,
0, //&&I_RTI,
0, //&&I_EOR_d_x_i,
0, //&&illegal,
0, //&&I_EOR_d_s,
0, //&&I_MVP,
0, //&&I_EOR_d,
0, //&&I_LSR_d,
0, //&&I_EOR_d_i_l,
0, //&&I_PHA,
0, //&&I_EOR,
0, //&&I_LSR_A,
0, //&&I_PHK,
0, //&&I_JMP_a,
0, //&&I_EOR_a,
0, //&&I_LSR_a,
0, //&&I_EOR_al,
0, //&&I_BVC,
0, //&&I_EOR_d_i_y,
0, //&&I_EOR_d_i,
0, //&&I_EOR_d_s_i_y,
0, //&&I_MVN,
0, //&&I_EOR_d_x,
0, //&&I_LSR_d_x,
0, //&&I_EOR_d_i_l_y,
0, //&&I_CLI,
0, //&&I_EOR_a_y,
0, //&&I_PHY,
0, //&&I_TCD,
0, //&&I_JMP_al,
0, //&&I_EOR_a_x,
0, //&&I_LSR_a_x,
0, //&&I_EOR_al_x,
0, //&&I_RTS,
0, //&&I_ADC_d_x_i,
0, //&&I_PER,
0, //&&I_ADC_d_s,
0, //&&I_STZ_d,
0, //&&I_ADC_d,
0, //&&I_ROR_d,
0, //&&I_ADC_d_i_l,
0, //&&I_PLA,
0, //&&I_ADC,
0, //&&I_ROR_A,
0, //&&I_RTL,
0, //&&I_JMP_a_i,
0, //&&I_ADC_a,
0, //&&I_ROR_a,
0, //&&I_ADC_al,
0, //&&I_BVS,
0, //&&I_ADC_d_i_y,
0, //&&I_ADC_d_i,
0, //&&I_ADC_d_s_i_y,
0, //&&I_STZ_d_x,
0, //&&I_ADC_d_x,
0, //&&I_ROR_d_x,
0, //&&I_ADC_d_i_l_y,
0, //&&I_SEI,
0, //&&I_ADC_a_y,
0, //&&I_PLY,
0, //&&I_TDC,
0, //&&I_JMP_a_x_i,
0, //&&I_ADC_a_x,
0, //&&I_ROR_a_x,
0, //&&I_ADC_al_x,
0, //&&I_BRA,
0, //&&I_STA_d_x_i,
0, //&&I_BRL,
0, //&&I_STA_d_s,
0, //&&I_STY_d,
0, //&&I_STA_d,
0, //&&I_STX_d,
0, //&&I_STA_d_i_l,
0, //&&I_DEY,
0, //&&I_BIT,
0, //&&I_TXA,
0, //&&I_PHB,
0, //&&I_STY_a,
0, //&&I_STA_a,
0, //&&I_STX_a,
0, //&&I_STA_al,
0, //&&I_BCC,
0, //&&I_STA_d_i_y,
0, //&&I_STA_d_i,
0, //&&I_STA_d_s_i_y,
0, //&&I_STY_d_x,
0, //&&I_STA_d_x,
0, //&&I_STX_d_y,
0, //&&I_STA_d_i_l_y,
0, //&&I_TYA,
0, //&&I_STA_a_y,
0, //&&I_TXS,
0, //&&I_TXY,
0, //&&I_STZ_a,
0, //&&I_STA_a_x,
0, //&&I_STZ_a_x,
0, //&&I_STA_al_x,
0, //&&I_LDY,
0, //&&I_LDA_d_x_i,
0, //&&I_LDX,
0, //&&I_LDA_d_s,
0, //&&I_LDY_d,
0, //&&I_LDA_d,
0, //&&I_LDX_d,
0, //&&I_LDA_d_i_l,
0, //&&I_TAY,
0, //&&I_LDA,
0, //&&I_TAX,
0, //&&I_PLB,
0, //&&I_LDY_a,
0, //&&I_LDA_a,
0, //&&I_LDX_a,
0, //&&I_LDA_al,
0, //&&I_BCS,
0, //&&I_LDA_d_i_y,
0, //&&I_LDA_d_i,
0, //&&I_LDA_d_s_i_y,
0, //&&I_LDY_d_x,
0, //&&I_LDA_d_x,
0, //&&I_LDX_d_y,
0, //&&I_LDA_d_i_l_y,
0, //&&I_CLV,
0, //&&I_LDA_a_y,
0, //&&I_TSX,
0, //&&I_TYX,
0, //&&I_LDY_a_x,
0, //&&I_LDA_a_x,
0, //&&I_LDX_a_y,
0, //&&I_LDA_al_x,
0, //&&I_CPY,
0, //&&I_CMP_d_x_i,
0, //&&I_REP,
0, //&&I_CMP_d_s,
0, //&&I_CPY_d,
0, //&&I_CMP_d,
0, //&&I_DEC_d,
0, //&&I_CMP_d_i_l,
0, //&&I_INY,
0, //&&I_CMP,
0, //&&I_DEX,
0, //&&I_WAI,
0, //&&I_CPY_a,
0, //&&I_CMP_a,
0, //&&I_DEC_a,
0, //&&I_CMP_al,
0, //&&I_BNE,
0, //&&I_CMP_d_i_y,
0, //&&I_CMP_d_i,
0, //&&I_CMP_d_s_i_y,
0, //&&I_PEI,
0, //&&I_CMP_d_x,
0, //&&I_DEC_d_x,
0, //&&I_CMP_d_i_l_y,
0, //&&I_CLD,
0, //&&I_CMP_a_y,
0, //&&I_PHX,
0, //&&illegal,
0, //&&I_JML_a_i,
0, //&&I_CMP_a_x,
0, //&&I_DEC_a_x,
0, //&&I_CMP_al_x,
0, //&&I_CPX,
0, //&&I_SBC_d_x_i,
0, //&&I_SEP,
0, //&&I_SBC_d_s,
0, //&&I_CPX_d,
0, //&&I_SBC_d,
0, //&&I_INC_d,
0, //&&I_SBC_d_i_l,
0, //&&I_INX,
0, //&&I_SBC,
0, //&&I_NOP,
0, //&&I_XBA,
0, //&&I_CPX_a,
0, //&&I_SBC_a,
0, //&&I_INC_a,
0, //&&I_SBC_al,
0, //&&I_BEQ,
0, //&&I_SBC_d_i_y,
0, //&&I_SBC_d_i,
0, //&&I_SBC_d_s_i_y,
0, //&&I_PEA,
0, //&&I_SBC_d_x,
0, //&&I_INC_d_x,
0, //&&I_SBC_d_i_l_y,
0, //&&I_SED,
0, //&&I_SBC_a_y,
0, //&&I_PLX,
0, //&&I_XCE,
0, //&&I_JSR_a_x_i,
0, //&&I_SBC_a_x,
0, //&&I_INC_a_x,
0, //&&I_SBC_al_x
};
*/	

#endif /*OPC_ASM_H_*/

