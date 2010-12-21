// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

// Additional copyrights go to Duddie and Tratax (c) 2004

#include "../DSPIntCCUtil.h"
#include "../DSPIntUtil.h"
#include "../DSPEmitter.h"
#include "../DSPAnalyzer.h"
#include "x64Emitter.h"
#include "ABI.h"
using namespace Gen;

// CLR $acR
// 1000 r001 xxxx xxxx
// Clears accumulator $acR
//
// flags out: --10 0100
//void DSPEmitter::clr(const UDSPInstruction opc)
//{
//	u8 reg = (opc >> 11) & 0x1;

//	dsp_set_long_acc(reg, 0);
//	Update_SR_Register64(0);
//	zeroWriteBackLog();
//}

// CLRL $acR.l
// 1111 110r xxxx xxxx
// Clears (and rounds!) $acR.l - low 16 bits of accumulator $acR.
//
// flags out: --xx xx00
//void DSPEmitter::clrl(const UDSPInstruction opc)
//{
//	u8 reg = (opc >> 8) & 0x1;
//	s64 acc = dsp_round_long_acc(dsp_get_long_acc(reg));

//	zeroWriteBackLog();

//	dsp_set_long_acc(reg, acc);
//	Update_SR_Register64(acc);
//}

//----

// ANDCF $acD.m, #I
// 0000 001r 1100 0000
// iiii iiii iiii iiii
// Set logic zero (LZ) flag in status register $sr if result of logic AND of
// accumulator mid part $acD.m with immediate value I is equal I.
//
// flags out: -x-- ----
//void DSPEmitter::andcf(const UDSPInstruction opc)
//{
//	u8 reg  = (opc >> 8) & 0x1;

//	u16 imm = dsp_fetch_code();
//	u16 val = dsp_get_acc_m(reg);
//	Update_SR_LZ(((val & imm) == imm) ? true : false);
//}

// ANDF $acD.m, #I
// 0000 001r 1010 0000
// iiii iiii iiii iiii
// Set logic zero (LZ) flag in status register $sr if result of logical AND
// operation of accumulator mid part $acD.m with immediate value I is equal
// immediate value 0.
//
// flags out: -x-- ----
//void DSPEmitter::andf(const UDSPInstruction opc)
//{
//	u8 reg  = (opc >> 8) & 0x1;

//	u16 imm = dsp_fetch_code();
//	u16 val = dsp_get_acc_m(reg);
//	Update_SR_LZ(((val & imm) == 0) ? true : false);
//}

//----

// TST
// 1011 r001 xxxx xxxx
// Test accumulator %acR.
//
// flags out: --xx xx00
void DSPEmitter::tst(const UDSPInstruction opc)
{
#ifdef _M_X64
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		u8 reg = (opc >> 11) & 0x1;
//		s64 acc = dsp_get_long_acc(reg);
		get_long_acc(reg);
//		Update_SR_Register64(acc);
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// TSTAXH $axR.h
// 1000 011r xxxx xxxx
// Test high part of secondary accumulator $axR.h.
//
// flags out: --x0 xx00
//void DSPEmitter::tstaxh(const UDSPInstruction opc)
//{
//	u8 reg  = (opc >> 8) & 0x1;

//	s16 val = dsp_get_ax_h(reg);
//	Update_SR_Register16(val);
//	zeroWriteBackLog();
//}

//----

// CMP
// 1000 0010 xxxx xxxx
// Compares accumulator $ac0 with accumulator $ac1.
//
// flags out: x-xx xxxx
//void DSPEmitter::cmp(const UDSPInstruction opc)
//{
//	s64 acc0 = dsp_get_long_acc(0);
//	s64 acc1 = dsp_get_long_acc(1);
//	s64 res = dsp_convert_long_acc(acc0 - acc1);
//	
//	Update_SR_Register64(res, isCarry2(acc0, res), isOverflow(acc0, -acc1, res)); // CF -> influence on ABS/0xa100
//	zeroWriteBackLog();
//}

// CMPAR $acS axR.h
// 1100 0001 xxxx xxxx
// Compares accumulator $acS with accumulator axR.h.
// Not described by Duddie's doc - at least not as a separate instruction.
//
// flags out: x-xx xxxx
//void DSPEmitter::cmpar(const UDSPInstruction opc)
//{
//	u8 rreg = ((opc >> 12) & 0x1) + DSP_REG_AXH0;
//	u8 sreg = (opc >> 11) & 0x1;

//	s64 sr = dsp_get_long_acc(sreg);
//	s64 rr = (s16)g_dsp.r[rreg];
//	rr <<= 16;
//	s64 res = dsp_convert_long_acc(sr - rr);
//	
//	Update_SR_Register64(res, isCarry2(sr, res), isOverflow(sr, -rr, res));
//	zeroWriteBackLog();
//}

// CMPI $amD, #I
// 0000 001r 1000 0000
// iiii iiii iiii iiii
// Compares mid accumulator $acD.hm ($amD) with sign extended immediate value I. 
// Although flags are being set regarding whole accumulator register.
//
// flags out: x-xx xxxx
//void DSPEmitter::cmpi(const UDSPInstruction opc)
//{
//	u8 reg  = (opc >> 8) & 0x1;

//	s64 val = dsp_get_long_acc(reg);
//	s64 imm = (s64)(s16)dsp_fetch_code() << 16; // Immediate is considered to be at M level in the 40-bit accumulator.
//	s64 res = dsp_convert_long_acc(val - imm);

//	Update_SR_Register64(res, isCarry2(val, res), isOverflow(val, -imm, res));
//}

// CMPIS $acD, #I
// 0000 011d iiii iiii
// Compares accumulator with short immediate. Comaprison is executed
// by subtracting short immediate (8bit sign extended) from mid accumulator
// $acD.hm and computing flags based on whole accumulator $acD.
//
// flags out: x-xx xxxx
//void DSPEmitter::cmpis(const UDSPInstruction opc)
//{
//	u8 areg = (opc >> 8) & 0x1;

//	s64 acc = dsp_get_long_acc(areg);
//	s64 val = (s8)opc;
//	val <<= 16; 
//	s64 res = dsp_convert_long_acc(acc - val);

//	Update_SR_Register64(res, isCarry2(acc, res), isOverflow(acc, -val, res));
//}

//----

// XORR $acD.m, $axS.h
// 0011 00sd 0xxx xxxx
// Logic XOR (exclusive or) middle part of accumulator $acD.m with
// high part of secondary accumulator $axS.h.
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::xorr(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u8 sreg = (opc >> 9) & 0x1;
//	u16 accm = g_dsp.r[DSP_REG_ACM0 + dreg] ^ g_dsp.r[DSP_REG_AXH0 + sreg];
//	
//	zeroWriteBackLogPreserveAcc(dreg);

//	g_dsp.r[DSP_REG_ACM0 + dreg] = accm;
//	Update_SR_Register16((s16)accm, false, false, isOverS32(dsp_get_long_acc(dreg)));
//}

// ANDR $acD.m, $axS.h
// 0011 01sd 0xxx xxxx
// Logic AND middle part of accumulator $acD.m with high part of
// secondary accumulator $axS.h.
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::andr(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u8 sreg = (opc >> 9) & 0x1;
//	u16 accm = g_dsp.r[DSP_REG_ACM0 + dreg] & g_dsp.r[DSP_REG_AXH0 + sreg];
//	
//	zeroWriteBackLogPreserveAcc(dreg);

//	g_dsp.r[DSP_REG_ACM0 + dreg] = accm;
//	Update_SR_Register16((s16)accm, false, false, isOverS32(dsp_get_long_acc(dreg)));
//}

// ORR $acD.m, $axS.h
// 0011 10sd 0xxx xxxx
// Logic OR middle part of accumulator $acD.m with high part of
// secondary accumulator $axS.h.
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::orr(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u8 sreg = (opc >> 9) & 0x1;
//	u16 accm = g_dsp.r[DSP_REG_ACM0 + dreg] | g_dsp.r[DSP_REG_AXH0 + sreg];
//	
//	zeroWriteBackLogPreserveAcc(dreg);

//	g_dsp.r[DSP_REG_ACM0 + dreg] = accm;
//	Update_SR_Register16((s16)accm, false, false, isOverS32(dsp_get_long_acc(dreg)));
//}

// ANDC $acD.m, $ac(1-D).m
// 0011 110d 0xxx xxxx
// Logic AND middle part of accumulator $acD.m with middle part of
// accumulator $ac(1-D).m
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::andc(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u16 accm = g_dsp.r[DSP_REG_ACM0 + dreg] & g_dsp.r[DSP_REG_ACM0 + (1 - dreg)];
//	
//	zeroWriteBackLogPreserveAcc(dreg);

//	g_dsp.r[DSP_REG_ACM0 + dreg] = accm;
//	Update_SR_Register16((s16)accm, false, false, isOverS32(dsp_get_long_acc(dreg)));
//}

// ORC $acD.m, $ac(1-D).m
// 0011 111d 0xxx xxxx
// Logic OR middle part of accumulator $acD.m with middle part of
// accumulator $ac(1-D).m.
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::orc(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u16 accm = g_dsp.r[DSP_REG_ACM0 + dreg] | g_dsp.r[DSP_REG_ACM0 + (1 - dreg)];
//	
//	zeroWriteBackLogPreserveAcc(dreg);

//	g_dsp.r[DSP_REG_ACM0 + dreg] = accm;
//	Update_SR_Register16((s16)accm, false, false, isOverS32(dsp_get_long_acc(dreg)));
//}

// XORC $acD.m
// 0011 000d 1xxx xxxx
// Logic XOR (exclusive or) middle part of accumulator $acD.m with $ac(1-D).m
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::xorc(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u16 accm = g_dsp.r[DSP_REG_ACM0 + dreg] ^ g_dsp.r[DSP_REG_ACM0 + (1 - dreg)];

//	zeroWriteBackLogPreserveAcc(dreg);

//	g_dsp.r[DSP_REG_ACM0 + dreg] = accm;
//	Update_SR_Register16((s16)accm, false, false, isOverS32(dsp_get_long_acc(dreg)));
//}

// NOT $acD.m
// 0011 001d 1xxx xxxx
// Invert all bits in dest reg, aka xor with 0xffff
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::notc(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u16 accm = g_dsp.r[DSP_REG_ACM0 + dreg] ^ 0xffff;

//	zeroWriteBackLogPreserveAcc(dreg);

//	g_dsp.r[DSP_REG_ACM0 + dreg] = accm;
//	Update_SR_Register16((s16)accm, false, false, isOverS32(dsp_get_long_acc(dreg)));
//}

// XORI $acD.m, #I
// 0000 001r 0010 0000
// iiii iiii iiii iiii
// Logic exclusive or (XOR) of accumulator mid part $acD.m with
// immediate value I.
//
// flags out: --xx xx00
//void DSPEmitter::xori(const UDSPInstruction opc)
//{
//	u8 reg  = (opc >> 8) & 0x1;
//	u16 imm = dsp_fetch_code();
//	g_dsp.r[DSP_REG_ACM0 + reg] ^= imm;

//	Update_SR_Register16((s16)g_dsp.r[DSP_REG_ACM0 + reg], false, false, isOverS32(dsp_get_long_acc(reg)));
//}

// ANDI $acD.m, #I
// 0000 001r 0100 0000
// iiii iiii iiii iiii
// Logic AND of accumulator mid part $acD.m with immediate value I.
//
// flags out: --xx xx00
//void DSPEmitter::andi(const UDSPInstruction opc)
//{
//	u8 reg  = (opc >> 8) & 0x1;
//	u16 imm = dsp_fetch_code();
//	g_dsp.r[DSP_REG_ACM0 + reg] &= imm;

//	Update_SR_Register16((s16)g_dsp.r[DSP_REG_ACM0 + reg], false, false, isOverS32(dsp_get_long_acc(reg)));
//}

// ORI $acD.m, #I
// 0000 001r 0110 0000
// iiii iiii iiii iiii
// Logic OR of accumulator mid part $acD.m with immediate value I.
//
// flags out: --xx xx00
//void DSPEmitter::ori(const UDSPInstruction opc)
//{
//	u8 reg  = (opc >> 8) & 0x1;
//	u16 imm = dsp_fetch_code();
//	g_dsp.r[DSP_REG_ACM0 + reg] |= imm;

//	Update_SR_Register16((s16)g_dsp.r[DSP_REG_ACM0 + reg], false, false, isOverS32(dsp_get_long_acc(reg)));
//}

//----

// ADDR $acD.M, $axS.L
// 0100 0ssd xxxx xxxx
// Adds register $axS.L to accumulator $acD.M register.
//
// flags out: x-xx xxxx
void DSPEmitter::addr(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
	u8 sreg = ((opc >> 9) & 0x3) + DSP_REG_AXL0;

	MOV(64, R(R11), ImmPtr(&g_dsp.r));
//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 ax = (s16)g_dsp.r[sreg];
	MOVSX(64, 16, RDX, MDisp(R11, sreg * 2));
//	ax <<= 16;
	SHL(64, R(RDX), Imm8(16));
//	s64 res = acc + ax;
	ADD(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	Update_SR_Register64(res, isCarry(acc, res), isOverflow(acc, ax, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
	
#else
	Default(opc);
#endif
}

// ADDAX $acD, $axS
// 0100 10sd xxxx xxxx
// Adds secondary accumulator $axS to accumulator register $acD.
//
// flags out: x-xx xxxx
void DSPEmitter::addax(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
	u8 sreg = (opc >> 9) & 0x1;

//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 ax  = dsp_get_long_acx(sreg);
	get_long_acx(sreg, RDX);
//	s64 res = acc + ax;
	ADD(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry(acc, res), isOverflow(acc, ax, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// ADD $acD, $ac(1-D)
// 0100 110d xxxx xxxx
// Adds accumulator $ac(1-D) to accumulator register $acD.
//
// flags out: x-xx xxxx
void DSPEmitter::add(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg  = (opc >> 8) & 0x1;

//	s64 acc0 = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 acc1 = dsp_get_long_acc(1 - dreg);
	get_long_acc(1 - dreg, RDX);
//	s64 res = acc0 + acc1;
	ADD(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry(acc0, res), isOverflow(acc0, acc1, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// ADDP $acD
// 0100 111d xxxx xxxx
// Adds product register to accumulator register.
//
// flags out: x-xx xxxx
void DSPEmitter::addp(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;

//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 prod = dsp_get_long_prod();
	get_long_prod(RDX);
//	s64 res = acc + prod;
	ADD(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry2(acc, res), isOverflow(acc, prod, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry2();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// ADDAXL $acD, $axS.l
// 0111 00sd xxxx xxxx
// Adds secondary accumulator $axS.l to accumulator register $acD.
// should be unsigned values!!
//
// flags out: x-xx xxxx
//void DSPEmitter::addaxl(const UDSPInstruction opc)
//{
//	u8 sreg = (opc >> 9) & 0x1;
//	u8 dreg = (opc >> 8) & 0x1;

//	u64 acc = dsp_get_long_acc(dreg);
//	u16 acx = (u16)dsp_get_ax_l(sreg);

//	u64 res = acc + acx;

//	zeroWriteBackLog();

//	dsp_set_long_acc(dreg, (s64)res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64((s64)res, isCarry(acc, res), isOverflow((s64)acc, (s64)acx, (s64)res));
//}

// ADDI $amR, #I 
// 0000 001r 0000 0000
// iiii iiii iiii iiii
// Adds immediate (16-bit sign extended) to mid accumulator $acD.hm.
//
// flags out: x-xx xxxx
void DSPEmitter::addi(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 areg = (opc >> 8) & 0x1;
//	s64 acc = dsp_get_long_acc(areg);
	get_long_acc(areg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 imm = (s16)dsp_fetch_code();
	s16 imm = dsp_imem_read(compilePC+1);
	//imm <<= 16;
	MOV(16, R(RDX), Imm16(imm));
	MOVSX(64, 16, RDX, R(RDX));
	SHL(64, R(RDX), Imm8(16));
//	s64 res = acc + imm;
	ADD(64, R(RAX), R(RDX));
//	dsp_set_long_acc(areg, res);
//	res = dsp_get_long_acc(areg);
//	Update_SR_Register64(res, isCarry(acc, res), isOverflow(acc, imm, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RSI), R(RAX));
		set_long_acc(areg, RSI);
		Update_SR_Register64_Carry();
	}
	else
	{
		set_long_acc(areg, RAX);
	}
#else
	Default(opc);
#endif
}

// ADDIS $acD, #I
// 0000 010d iiii iiii
// Adds short immediate (8-bit sign extended) to mid accumulator $acD.hm.
//
// flags out: x-xx xxxx
void DSPEmitter::addis(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;

//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 imm = (s8)(u8)opc;
//	imm <<= 16;
	MOV(8, R(RDX), Imm8((u8)opc));
	MOVSX(64, 8, RDX, R(RDX));
	SHL(64, R(RDX), Imm8(16));
//	s64 res = acc + imm;
	ADD(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry(acc, res), isOverflow(acc, imm, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// INCM $acsD
// 0111 010d xxxx xxxx
// Increment 24-bit mid-accumulator $acsD.
//
// flags out: x-xx xxxx
void DSPEmitter::incm(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
	s64 sub = 0x10000;
//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 res = acc + sub;
	ADD(64, R(RAX), Imm32((u32)sub));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry(acc, res), isOverflow(acc, sub, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RDX), Imm32((u32)sub));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry();
	}
	else
	{
		set_long_acc(dreg);
		Update_SR_Register64_Carry();
	}
#else
	Default(opc);
#endif
}

// INC $acD
// 0111 011d xxxx xxxx
// Increment accumulator $acD.
//
// flags out: x-xx xxxx
void DSPEmitter::inc(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 res = acc + 1;
	ADD(64, R(RAX), Imm8(1));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry(acc, res), isOverflow(acc, 1, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RDX), Imm64(1));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry();
	}
	else
	{
		set_long_acc(dreg);
		Update_SR_Register64_Carry();
	}
#else
	Default(opc);
#endif
}

//----

// SUBR $acD.M, $axS.L
// 0101 0ssd xxxx xxxx
// Subtracts register $axS.L from accumulator $acD.M register.
//
// flags out: x-xx xxxx
void DSPEmitter::subr(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
	u8 sreg = ((opc >> 9) & 0x3) + DSP_REG_AXL0;

//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 ax = (s16)g_dsp.r[sreg];
	MOV(64, R(R11), ImmPtr(&g_dsp.r));
	MOVSX(64, 16, RDX, MDisp(R11, sreg * 2));
//	ax <<= 16;
	SHL(64, R(RDX), Imm8(16));
//	s64 res = acc - ax;
	SUB(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry2(acc, res), isOverflow(acc, -ax, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		NEG(64, R(RDX));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry2();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// SUBAX $acD, $axS
// 0101 10sd xxxx xxxx
// Subtracts secondary accumulator $axS from accumulator register $acD.
//
// flags out: x-xx xxxx
void DSPEmitter::subax(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
	u8 sreg = (opc >> 9) & 0x1;

//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 acx = dsp_get_long_acx(sreg);
	get_long_acx(sreg, RDX);
//	s64 res = acc - acx;
	SUB(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry2(acc, res), isOverflow(acc, -acx, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		NEG(64, R(RDX));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry2();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// SUB $acD, $ac(1-D)
// 0101 110d xxxx xxxx
// Subtracts accumulator $ac(1-D) from accumulator register $acD. 
//
// flags out: x-xx xxxx
void DSPEmitter::sub(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
//	s64 acc1 = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 acc2 = dsp_get_long_acc(1 - dreg);
	get_long_acc(1 - dreg, RDX);
//	s64 res = acc1 - acc2;
	SUB(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry2(acc1, res), isOverflow(acc1, -acc2, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		NEG(64, R(RDX));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry2();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// SUBP $acD
// 0101 111d xxxx xxxx
// Subtracts product register from accumulator register.
//
// flags out: x-xx xxxx
void DSPEmitter::subp(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 prod = dsp_get_long_prod();
	get_long_prod(RDX);
//	s64 res = acc - prod;
	SUB(64, R(RAX), R(RDX));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry2(acc, res), isOverflow(acc, -prod, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		NEG(64, R(RDX));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry2();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// DECM $acsD
// 0111 100d xxxx xxxx
// Decrement 24-bit mid-accumulator $acsD.
//
// flags out: x-xx xxxx
void DSPEmitter::decm(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x01;
	s64 sub = 0x10000;
//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 res = acc - sub;
	SUB(64, R(RAX), Imm32((u32)sub));
//	dsp_set_long_acc(dreg, res);
//	res = dsp_get_long_acc(dreg);	
//	Update_SR_Register64(res, isCarry2(acc, res), isOverflow(acc, -sub, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(64, R(RDX), Imm64(-sub));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry2();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

// DEC $acD
// 0111 101d xxxx xxxx
// Decrement accumulator $acD.
//
// flags out: x-xx xxxx
void DSPEmitter::dec(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x01;
//	s64 acc = dsp_get_long_acc(dreg); 
	get_long_acc(dreg, RCX);
	MOV(64, R(RAX), R(RCX));
//	s64 res = acc - 1;
	SUB(64, R(RAX), Imm8(1));
//	dsp_set_long_acc(dreg, res);
	set_long_acc(dreg);
//	res = dsp_get_long_acc(dreg);
//	Update_SR_Register64(res, isCarry2(acc, res), isOverflow(acc, -1, res));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		MOV(8, R(RDX), Imm8(1));
		NEG(8, R(RDX));
		MOV(64, R(RSI), R(RAX));
		set_long_acc(dreg, RSI);
		Update_SR_Register64_Carry2();
	}
	else
	{
		set_long_acc(dreg, RAX);
	}
#else
	Default(opc);
#endif
}

//----

// NEG $acD
// 0111 110d xxxx xxxx
// Negate accumulator $acD.
//
// flags out: --xx xx00
void DSPEmitter::neg(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
//	s64 acc = dsp_get_long_acc(dreg);
	get_long_acc(dreg);
//	acc = 0 - acc;
	NEG(64, R(RAX));
//	dsp_set_long_acc(dreg, acc);
	set_long_acc(dreg);
//	Update_SR_Register64(dsp_get_long_acc(dreg));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// ABS  $acD
// 1010 d001 xxxx xxxx 
// absolute value of $acD
//
// flags out: --xx xx00
void DSPEmitter::abs(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 11) & 0x1;

//	s64 acc = dsp_get_long_acc(dreg); 	
	get_long_acc(dreg);
//	if (acc < 0) acc = 0 - acc;
	CMP(64, R(RAX), Imm8(0));
	FixupBranch lessThan = J_CC(CC_L);
	NEG(64, R(RAX));
	set_long_acc(dreg);
	SetJumpTarget(lessThan);
//	dsp_set_long_acc(dreg, acc);
	set_long_acc(dreg);
//	Update_SR_Register64(dsp_get_long_acc(dreg));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}
//----

// MOVR $acD, $axS.R
// 0110 0srd xxxx xxxx
// Moves register $axS.R (sign extended) to middle accumulator $acD.hm.
// Sets $acD.l to 0.
// TODO: Check what happens to acD.h.
//
// flags out: --xx xx00
void DSPEmitter::movr(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 areg = (opc >> 8) & 0x1;
	u8 sreg = ((opc >> 9) & 0x3) + DSP_REG_AXL0;
 		
//	s64 acc = (s16)g_dsp.r[sreg];
	MOV(64, R(R11), ImmPtr(&g_dsp.r));
	MOVSX(64, 16, RAX, MDisp(R11, sreg * 2));
//	acc <<= 16;
	SHL(64, R(RAX), Imm8(16));
//	acc &= ~0xffff;
//	dsp_set_long_acc(areg, acc);
	set_long_acc(areg);
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// MOVAX $acD, $axS
// 0110 10sd xxxx xxxx
// Moves secondary accumulator $axS to accumulator $axD. 
//
// flags out: --xx xx00
void DSPEmitter::movax(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
	u8 sreg = (opc >> 9) & 0x1;

//	s64 acx = dsp_get_long_acx(sreg);
	get_long_acx(sreg);
//	dsp_set_long_acc(dreg, acx);
	set_long_acc(dreg);
//	Update_SR_Register64(acx);
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// MOV $acD, $ac(1-D)
// 0110 110d xxxx xxxx
// Moves accumulator $ax(1-D) to accumulator $axD.
//
// flags out: --x0 xx00
void DSPEmitter::mov(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 dreg = (opc >> 8) & 0x1;
//	u64 acc = dsp_get_long_acc(1 - dreg);
	get_long_acc(1 - dreg);
//	dsp_set_long_acc(dreg, acc);
	set_long_acc(dreg);
//	Update_SR_Register64(acc);
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

//----

// LSL16 $acR
// 1111 000r xxxx xxxx
// Logically shifts left accumulator $acR by 16.
//
// flags out: --xx xx00
void DSPEmitter::lsl16(const UDSPInstruction opc)
{
	Default(opc); return; // TODO: Breaks ZTP Wii
#ifdef _M_X64
	u8 areg = (opc >> 8) & 0x1;
//	s64 acc = dsp_get_long_acc(areg);
	get_long_acc(areg);
//	acc <<= 16;
	SHL(64, R(RAX), Imm8(16));
//	dsp_set_long_acc(areg, acc);
	set_long_acc(areg);
//	Update_SR_Register64(dsp_get_long_acc(areg));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// LSR16 $acR
// 1111 010r xxxx xxxx
// Logically shifts right accumulator $acR by 16.
//
// flags out: --xx xx00
void DSPEmitter::lsr16(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 areg = (opc >> 8) & 0x1;

//	u64 acc = dsp_get_long_acc(areg);
	get_long_acc(areg);
//	acc &= 0x000000FFFFFFFFFFULL; 	// Lop off the extraneous sign extension our 64-bit fake accum causes
//	acc >>= 16; 
	SHR(64, R(RAX), Imm8(16));
	AND(64, R(RAX), Imm32(0xffffff));
//	dsp_set_long_acc(areg, (s64)acc);
	set_long_acc(areg);
//	Update_SR_Register64(dsp_get_long_acc(areg));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// ASR16 $acR
// 1001 r001 xxxx xxxx
// Arithmetically shifts right accumulator $acR by 16.
//
// flags out: --xx xx00
void DSPEmitter::asr16(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 areg = (opc >> 11) & 0x1;

//	s64 acc = dsp_get_long_acc(areg);
	get_long_acc(areg);
//	acc >>= 16;
	SHR(64, R(RAX), Imm8(16));
//	dsp_set_long_acc(areg, acc);
	set_long_acc(areg);
//	Update_SR_Register64(dsp_get_long_acc(areg));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// LSL $acR, #I
// 0001 010r 00ii iiii
// Logically shifts left accumulator $acR by number specified by value I.
//
// flags out: --xx xx00
void DSPEmitter::lsl(const UDSPInstruction opc) 
{
#ifdef _M_X64
	u8 rreg = (opc >> 8) & 0x01;
	u16 shift = opc & 0x3f; 
//	u64 acc = dsp_get_long_acc(rreg);
	get_long_acc(rreg);

//	acc <<= shift;
	SHL(64, R(RAX), Imm8((u8)shift));

//	dsp_set_long_acc(rreg, acc);
	set_long_acc(rreg);
//	Update_SR_Register64(dsp_get_long_acc(rreg));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// LSR $acR, #I
// 0001 010r 01ii iiii
// Logically shifts right accumulator $acR by number specified by value
// calculated by negating sign extended bits 0-6.
//
// flags out: --xx xx00
//void DSPEmitter::lsr(const UDSPInstruction opc)
//{
//	u8 rreg = (opc >> 8) & 0x01;
//	u16 shift;
//	u64 acc = dsp_get_long_acc(rreg);
//	acc &= 0x000000FFFFFFFFFFULL; 	// Lop off the extraneous sign extension our 64-bit fake accum causes

//	if ((opc & 0x3f) == 0)
//		shift = 0;
//	else
//		shift = 0x40 - (opc & 0x3f);

//	acc >>= shift;
//	
//	dsp_set_long_acc(rreg, (s64)acc);
//	Update_SR_Register64(dsp_get_long_acc(rreg));
//}

// ASL $acR, #I
// 0001 010r 10ii iiii
// Logically shifts left accumulator $acR by number specified by value I.
//
// flags out: --xx xx00
void DSPEmitter::asl(const UDSPInstruction opc)
{
#ifdef _M_X64
	u8 rreg = (opc >> 8) & 0x01;
	u16 shift = opc & 0x3f;
//	u64 acc = dsp_get_long_acc(rreg);
	get_long_acc(rreg);
//	acc <<= shift;
	SHL(64, R(RAX), Imm8((u8)shift));
//	dsp_set_long_acc(rreg, acc);
	set_long_acc(rreg);
//	Update_SR_Register64(dsp_get_long_acc(rreg));
	if (!(DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_START_OF_INST) || (DSPAnalyzer::code_flags[compilePC] & DSPAnalyzer::CODE_UPDATE_SR))
	{
		Update_SR_Register64();
	}
#else
	Default(opc);
#endif
}

// ASR $acR, #I
// 0001 010r 11ii iiii
// Arithmetically shifts right accumulator $acR by number specified by
// value calculated by negating sign extended bits 0-6.
//
// flags out: --xx xx00
//void DSPEmitter::asr(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x01;
//	u16 shift;

//	if ((opc & 0x3f) == 0)
//		shift = 0;
//	else
//		shift = 0x40 - (opc & 0x3f);

//	// arithmetic shift
//	s64 acc = dsp_get_long_acc(dreg);
//	acc >>= shift;

//	dsp_set_long_acc(dreg, acc);
//	Update_SR_Register64(dsp_get_long_acc(dreg));
//}

// LSRN  (fixed parameters)
// 0000 0010 1100 1010
// Logically shifts right accumulator $ACC0 by lower 7-bit (signed) value in $AC1.M
// (if value negative, becomes left shift).
//
// flags out: --xx xx00
//void DSPEmitter::lsrn(const UDSPInstruction opc)
//{
//	s16 shift; 
//	u16 accm = (u16)dsp_get_acc_m(1);
//	u64 acc = dsp_get_long_acc(0);
//	acc &= 0x000000FFFFFFFFFFULL;

//	if ((accm & 0x3f) == 0)
//		shift = 0;
//	else if (accm & 0x40)
//		shift = -0x40 + (accm & 0x3f);
//	else
//		shift = accm & 0x3f;

//	if (shift > 0) {
//		acc >>= shift;
//	} else if (shift < 0) {
//		acc <<= -shift;
//	}

//	dsp_set_long_acc(0, (s64)acc);
//	Update_SR_Register64(dsp_get_long_acc(0));
//}

// ASRN  (fixed parameters)
// 0000 0010 1100 1011
// Arithmetically shifts right accumulator $ACC0 by lower 7-bit (signed) value in $AC1.M
// (if value negative, becomes left shift).
//
// flags out: --xx xx00
//void DSPEmitter::asrn(const UDSPInstruction opc)
//{
//	s16 shift;
//	u16 accm = (u16)dsp_get_acc_m(1);
//	s64 acc = dsp_get_long_acc(0);

//	if ((accm & 0x3f) == 0)
//		shift = 0;
//	else if (accm & 0x40)
//		shift = -0x40 + (accm & 0x3f);
//	else
//		shift = accm & 0x3f;

//	if (shift > 0) {
//		acc >>= shift;
//	} else if (shift < 0) {
//		acc <<= -shift;
//	}

//	dsp_set_long_acc(0, acc);
//	Update_SR_Register64(dsp_get_long_acc(0));
//}

// LSRNRX $acD, $axS.h
// 0011 01sd 1xxx xxxx
// Logically shifts left/right accumulator $ACC[D] by lower 7-bit (signed) value in $AX[S].H
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::lsrnrx(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u8 sreg = (opc >> 9) & 0x1;

//	s16 shift;
//	u16 axh = g_dsp.r[DSP_REG_AXH0 + sreg];
//	u64 acc = dsp_get_long_acc(dreg);
//	acc &= 0x000000FFFFFFFFFFULL;

//	if ((axh & 0x3f) == 0)
//		shift = 0;
//	else if (axh & 0x40)
//		shift = -0x40 + (axh & 0x3f);
//	else
//		shift = axh & 0x3f;

//	if (shift > 0) {
//		acc <<= shift;
//	} else if (shift < 0) {
//		acc >>= -shift;
//	}

//	zeroWriteBackLog();

//	dsp_set_long_acc(dreg, (s64)acc);
//	Update_SR_Register64(dsp_get_long_acc(dreg));
//}

// ASRNRX $acD, $axS.h
// 0011 10sd 1xxx xxxx
// Arithmetically shifts left/right accumulator $ACC[D] by lower 7-bit (signed) value in $AX[S].H
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::asrnrx(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;
//	u8 sreg = (opc >> 9) & 0x1;

//	s16 shift;
//	u16 axh = g_dsp.r[DSP_REG_AXH0 + sreg];
//	s64 acc = dsp_get_long_acc(dreg);

//	if ((axh & 0x3f) == 0)
//		shift = 0;
//	else if (axh & 0x40)
//		shift = -0x40 + (axh & 0x3f);
//	else
//		shift = axh & 0x3f;

//	if (shift > 0) {
//		acc <<= shift;
//	} else if (shift < 0) {
//		acc >>= -shift;
//	}

//	zeroWriteBackLog();

//	dsp_set_long_acc(dreg, acc);
//	Update_SR_Register64(dsp_get_long_acc(dreg));
//}

// LSRNR  $acD
// 0011 110d 1xxx xxxx
// Logically shifts left/right accumulator $ACC[D] by lower 7-bit (signed) value in $AC[1-D].M
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::lsrnr(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;

//	s16 shift;
//	u16 accm = (u16)dsp_get_acc_m(1 - dreg);
//	u64 acc = dsp_get_long_acc(dreg);
//	acc &= 0x000000FFFFFFFFFFULL;

//	if ((accm & 0x3f) == 0)
//		shift = 0;
//	else if (accm & 0x40)
//		shift = -0x40 + (accm & 0x3f);
//	else
//		shift = accm & 0x3f;

//	if (shift > 0)
//		acc <<= shift;
//	else if (shift < 0)
//		acc >>= -shift;

//	zeroWriteBackLog();

//	dsp_set_long_acc(dreg, (s64)acc);
//	Update_SR_Register64(dsp_get_long_acc(dreg));
//}

// ASRNR  $acD
// 0011 111d 1xxx xxxx
// Arithmeticaly shift left/right accumulator $ACC[D] by lower 7-bit (signed) value in $AC[1-D].M
// x = extension (7 bits!!)
//
// flags out: --xx xx00
//void DSPEmitter::asrnr(const UDSPInstruction opc)
//{
//	u8 dreg = (opc >> 8) & 0x1;

//	s16 shift;
//	u16 accm = (u16)dsp_get_acc_m(1 - dreg);
//	s64 acc = dsp_get_long_acc(dreg);

//	if ((accm & 0x3f) == 0)
//		shift = 0;
//	else if (accm & 0x40)
//		shift = -0x40 + (accm & 0x3f);
//	else
//		shift = accm & 0x3f;

//	if (shift > 0)
//		acc <<= shift;
//	else if (shift < 0)
//		acc >>= -shift;

//	zeroWriteBackLog();

//	dsp_set_long_acc(dreg, acc);
//	Update_SR_Register64(dsp_get_long_acc(dreg));
//}


//}  // namespace

//
