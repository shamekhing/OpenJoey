// yaneVirtualCPU.h :
//		virtual CPU Kernel class
//
//		Programmed by yaneurao(M.Isozaki) '99/07/18-'99/07/25
//

//
//	ある命令の演算結果をその先にある、即値をとる命令のところに
//	直接パッチを当てに行くことによって、スタックアーキテクチャと
//	同じことが実現できる。
//	仮想ＣＰＵ向きのインストラクションセットである。
//

#ifndef __yaneVirtualCPU_h__
#define __yaneVirtualCPU_h__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "yaneVM2i386.h"	//	native code converter

/////////////////////////////////////////////////////////////////////
// This is a emulating Kernel of a Virtual CPU.
// This instruction set is created by yaneurao(M.Isozaki) '99/07/19
/////////////////////////////////////////////////////////////////////


enum VCPU {	// Instruction Code
	VCPU_HALT,		// halt
	VCPU_RETURN,	// return
	VCPU_RETURNNUM,	// return with num
	VCPU_MOVRETNUM,	// move return num to memory
	VCPU_CALL,		// call inner proc
	VCPU_CALLUF,	// call user function	LONG user_func(LONG*&)
	VCPU_JMP,		// jump
	VCPU_XALT,		// xalt(tablized jump)

	VCPU_PUSH,		// push
	VCPU_PUSHM,		// push from Memory
	VCPU_POPM,		// pop to Memory

	// SPもどこか固定メモリ空間が割り当たっていればこんな命令は不要なのだが
	VCPU_ADDSP,		// add sp to LONG
	VCPU_SUBSP,		// sub sp to LONG

	VCPU_ADD,		// add
	VCPU_SUB,		// sub
	VCPU_MUL,		// mul
	VCPU_DIV,		// div
	VCPU_MOD,		// mod
	VCPU_MOD2,		// mod2
	VCPU_AND,		// and
	VCPU_OR,		// or
	VCPU_XOR,		// xor
	VCPU_CPL,		// compliment
	VCPU_NOT,		// not

	VCPU_SHR,		// shift right
	VCPU_SHL,		// shift left

	VCPU_INC,		// ++
	VCPU_DEC,		// --
	VCPU_INCS,		// ++ on stack
	VCPU_DECS,		// -- on stack

	VCPU_IF,		// if

	// 直交性も大切。ビールのお客様も大切。（なんのこっちゃ）
	VCPU_MOV,		// move immediate_long	to memory						mov L1		,[L2]
	VCPU_MOVM,		// move memory to memory								mov [L1]	,[L2]
	VCPU_MOVMM,		// move immediate_long	to memory of morory pointing	mov L1		,[[L2]]

	VCPU_MOVS,		// move immediate_long to stack memory					mov L1		,[SP-k]
	VCPU_MOVSM,		// move stack memory to memory							mov [SP-k]	,[L2]
	VCPU_MOVMS,		// move memory to stack memory
	VCPU_MOVSS,		// move stack memory to stack memory
	VCPU_MOVMSM,	// move memory of stack memory pointing to memory		mov [[SP-k]],[L2]
	VCPU_MOVMMM,	// move memory of memory pointing to memory

	VCPU_LEAS,		// load effective address of stack memory				lea	[sp-k]	,[L2]
	
	// 比較も、こんなにいらないが直交性も大切
	VCPU_CMPBE,		// compare below			(<)
	VCPU_CMPLE,		// compare less or equal	(<=)
	VCPU_CMPAB,		// compare above			(>)
	VCPU_CMPME,		// compare more or equal	(>=)
	VCPU_CMPEQ,		// compare equal			(==)
	VCPU_CMPNE,		// compare not equal		(!=)

//	VCPU_MOVTM,		// move immediate_long to memory(virtual)
};
	
class CVirtualCPU {
	/////////////////////////////////////////////////////////////////
	// 命令の実行には、それに先行してスタックとプログラムエリアの確保が必要だよーん
	// 命令エリアは、外部で確保してねん。
public:
	LRESULT	ResetPC(LONG *);			// PC（プログラムカウンタ）を設定する
	LRESULT ReExecute(void);			// 実行を再開する（スタックには何もしない）
										// 正常に戻ってくるためにはPushReturnAddressが必要

	LRESULT PushReturnAddress(void);	// NULLをスタックに積む(end code)
	LRESULT ExecuteFrom(void *);		// 特定アドレスから実行（関数など）
										//	スタックには、thisが積まれているので正常に戻ってくる

	LRESULT PushPC();					//	スタックには、PCを積む
	LRESULT PopPC();					//	スタックから、PCを降ろす

	/////////////////////////////////////////////////////////////////
	void SetStackSize(DWORD dw);		// スタックサイズ(DWORD単位で指定）

	/////////////////////////////////////////////////////////////////

	CVirtualCPU(void);
	~CVirtualCPU();

static bool	m_bNative;			//	use Native Code Compiler?

	/////////////////////////////////////////////////////////////////
public:	//	面倒なんで:p
	LONG*	m_StackFrame;		// Stack Frame
	LONG*	m_SP;				// Stack Pointer
	LONG*	m_PC;				// Program Counter
	LONG	m_ReturnNum;		// 返し値

	//	start up codeのためにポインタが一つ必要。（なんでも良い）
	CVM2i386*	m_lpNativeCode;		//	NativeCode Converter
};

// ユーザーデータとのやりとりをするメモリは、このクラスには含まれないので、
// ユーザー側で関数を用意して、やりとりするよろし:p

#endif
