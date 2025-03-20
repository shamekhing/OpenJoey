#include "stdafx.h"

#include "yaneVirtualCPU.h"
#include "../../yaneSDK/YTL/yaneMacro.h"

//////////////////////////////////////////////////////////////////////////////
//	ネイティブコードで実行するか？
bool CVirtualCPU::m_bNative = true;
//bool CVirtualCPU::m_bNative = false;

//	これは、オプションでの変更は利かない。
//	また、これをtrueにしていると、VirtualProtectを使うのでデバッガで追えない。

//	スクリプトのプログラムをデバッグするには、
//	ここをfalseにして、仮想CPUカーネルで実行すること。
//////////////////////////////////////////////////////////////////////////////

// virtual CPU Emulation by yaneurao(M.Isozaki)

LRESULT CVirtualCPU::ResetPC(LONG *pc){
	m_PC = pc;
	return 0;
}

LRESULT CVirtualCPU::ExecuteFrom(void *p){
	m_PC = (LONG*)p;
	PushReturnAddress();	//	いるやろー？
	return ReExecute();
}

LRESULT CVirtualCPU::PushPC(void){
	*(m_SP++) = (LONG)m_PC;		// 戻り先を積め～
	return 0;
}

LRESULT CVirtualCPU::PopPC(void){
	m_PC = (LONG*)*(--m_SP);	// 戻り先を降ろせ～
	return 0;
}

/////////////////////////////////////////////////////////////////////
// This is a emulating Kernel of a Virtual CPU.
// This instruction set is created by yaneurao(M.Isozaki) '99/07/19
/////////////////////////////////////////////////////////////////////

LRESULT CVirtualCPU::PushReturnAddress(void){
	*(m_SP++) = (LONG)NULL;	//	NULLアドレスへのリターンならば、帰ってくるのだ。
	return 0;
}

LRESULT CVirtualCPU::ReExecute(void){
// リターンコード
//	0 : Returnで終了→正常終了
//	1 : HALTで終了→またReExecuteで、次の命令から実行することができる。
  if (m_bNative) {
		return m_lpNativeCode->Execute();	//	これだけ:p
  }

  //	実は、ネイティブコードで実行するようにしたので、
  //	以下のルーチンは不要なのだが...

  for(;;){
	// switch ～ caseにしとくから、ちゃんとジャンプテーブルにしろよ>VC++
	switch (*(m_PC++)) {
	case VCPU_HALT: // HALT
		return 1;						//	終了～

	case VCPU_RETURN: // RETURN
		--m_SP;
		if (*m_SP == (LONG)NULL) return 0;	// 終了～
		m_PC = (LONG*)*m_SP;	// リターン
		break;

	case VCPU_RETURNNUM: // RETURN num
		--m_SP;
		if (*m_SP == (LONG)NULL) return 0;	// 終了～
		m_ReturnNum = (DWORD)*m_PC;	// これが返し値
		m_PC = (LONG*)*m_SP;	// リターン
		break;

	case VCPU_MOVRETNUM:	// MOVRETNUM lp	 move return num to memory
		*(LONG*)*(m_PC++) = m_ReturnNum;
		break;

	case VCPU_CALL: // Call lp
		*(m_SP++) = (LONG) (m_PC + 1);	// リターンアドレス
		m_PC = (LONG*)*m_PC;
		break;

	case VCPU_CALLUF: { // Call func lp1 lp2 ; lp1引数アドレス lp2返し値アドレス（返し値つきユーザー関数を呼び出す！！）
		LONG (*user_func)(LONG*); // 可変引数。ユーザー側は、SPを引数の数の分だけ進める必要がある。
		user_func = (LONG (*)(LONG*))*(m_PC++); // うひょー！！
		LONG l = user_func((LONG*)*(m_PC++));	// ユーザー関数呼出し。
		*((LONG*)*(m_PC++)) = l;	// 返し値を反映させる
	}
		break;

	case VCPU_JMP :	// Jmp lp  ジャンプする
		m_PC = (LONG*)*m_PC;
		break;

	case VCPU_XALT :	// xalt Im(,label1,label2,...) テーブルジャンプする
		m_PC = (LONG*)*(LONG*)(m_PC+(long)*m_PC+1);
		break;

	/////////////////////////////////////////////////////////////////

	case VCPU_PUSH: // Push Im（即値のPush）
		*(m_SP++) = (LONG) *(m_PC++);
		break;

	case VCPU_PUSHM: // Pushm lp（特定メモリの値をPush）
		*(m_SP++) = *(LONG*)*(m_PC++);
		break;

	case VCPU_POPM: // Pop lp（Popして、その値をlpに書き込む）
		*(LONG*)*(m_PC++) = *(--m_SP);
		break;

	case VCPU_ADDSP: // ADDSP Im（SPに即値加算。SPは上位メモリに向かって伸びることに注意すること）
		m_SP = (LONG*)((BYTE*)m_SP + *(m_PC++));
		break;

	case VCPU_SUBSP: // SUBSP Im（SPに即値減算。SPは上位メモリに向かって伸びることに注意すること）
		m_SP = (LONG*)((BYTE*)m_SP - *(m_PC++));
		break;

	/////////////////////////////////////////////////////////////////

	case VCPU_ADD: // ADD Im Im lp (加算命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC + *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_SUB: // SUB Im Im lp (減算命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC - *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_MUL: // MUL Im Im lp (乗算命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC * *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_DIV: // DIV Im Im lp (除算命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC / *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_MOD: // MOD Im Im lp (剰余命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC % *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_MOD2: // MOD2 Im Im lp (剰余命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC % *(m_PC+1);
		if (*(LONG*)*(m_PC+2) < 0) {
			*(LONG*)*(m_PC+2) += *(m_PC+1);
		}
		m_PC += 3;
		break;

	case VCPU_AND: // AND Im Im lp (ビットAND命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC & *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_OR: // OR Im Im lp (ビットOR命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC | *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_XOR: // XOR Im Im lp (ビットXOR命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *(m_PC) ^ *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_CPL: // CPL Im lp (補数命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+1) = ~(*m_PC);	// これもあれば便利じゃろ？
		m_PC += 2;
		break;

	case VCPU_NOT: // NOT Im lp (NOT命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+1) = !(*m_PC);
		m_PC += 2;
		break;

	case VCPU_SHR: // SHR Im1 Im2 lp (シフト命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC >> *(m_PC+1);
		m_PC += 3;
		break;

	case VCPU_SHL: // SHL Im1 Im2 lp (シフト命令 LONG,LONG,値をストアするポインタ)
		*(LONG*)*(m_PC+2) = *m_PC << *(m_PC+1);
		m_PC += 3;
		break;

	/////////////////////////////////////////////////////////////////
	case VCPU_INC: // INC lp
		(*(LONG*)*(m_PC))++;
		m_PC ++;
		break;

	case VCPU_DEC: // DEC lp
		(*(LONG*)*(m_PC))--;
		m_PC ++;
		break;

	case VCPU_INCS: // INCS Im
		(*(LONG*)((BYTE*)m_SP - *m_PC))++;
		m_PC ++;
		break;

	case VCPU_DECS: // INCS Im
		(*(LONG*)((BYTE*)m_SP - *m_PC))--;
		m_PC ++;
		break;

	/////////////////////////////////////////////////////////////////

	case VCPU_IF: // IF Im Jump1 (分岐命令 LONG,値が0のときのジャンプ先)
		if (!*m_PC) {
			m_PC = (LONG*)*(++m_PC);		//	条件不成立ジャンプ
		} else {
			m_PC+=2;						//	条件成立（よってジャンプしない）
		}
		break;

	/////////////////////////////////////////////////////////////////

	case VCPU_MOV: // MOV Im lp (移動命令。LONGを、特定のメモリに書き込む)
		*((LONG*)*(m_PC+1)) = *m_PC;
		m_PC += 2;
		break;

	case VCPU_MOVM: // MOVM lp1 lp2（メモリtoメモリのMOV命令。lp1からlp2へ）
		*(LONG*)*(m_PC+1) = *(LONG*)*m_PC;
		m_PC += 2;
		break;
	
	case VCPU_MOVMM: // MOVMM Im lp (移動命令。LONGを、メモリ間接メモリに書き込む)
		*(LONG*)*(LONG*)*(m_PC+1) = *m_PC;
		m_PC += 2;
		break;

	case VCPU_MOVS: // MOVS Im1 Im2 (スタック上のメモリの移動命令。[SP-Im2]にIm1の内容をストアする)
			// スタックは上位メモリに向かって伸びていることに注意。
			// 前回pushしたメモリにアクセスするならば、MOVS 4,LABEL ; MOVS [SP-4],LABELの意味
			// メモリはBYTE単位であることにも注意すること。
		*(LONG*)((BYTE*)m_SP - *(m_PC+1)) =	*m_PC;
		m_PC += 2;
		break;

	case VCPU_MOVMS:		// move memory to stack memory
		*(LONG*)((BYTE*)m_SP - *(m_PC+1)) = *(LONG*)*m_PC;
		m_PC += 2;
		break;

	case VCPU_MOVSS:		// move stack memory to stack memory
		*(LONG*)((BYTE*)m_SP - *(m_PC+1)) = *(LONG*)((BYTE*)m_SP - *m_PC);
		m_PC += 2;
		break;

	case VCPU_MOVSM: // MOVSM Im lp (スタック上のメモリの移動命令。[SP-Im]の内容をlpにストアする)
		*(LONG*)*(m_PC+1) = *(LONG*)((BYTE*)m_SP - *m_PC);
		m_PC += 2;
		break;

	case VCPU_MOVMSM: // MOVMSM Im lp (スタック上のメモリ間接メモリの移動命令。[[SP-Im]]の内容をlpにストアする)
		*(LONG*)*(m_PC+1) = *(LONG*)*(LONG*)((BYTE*)m_SP - *m_PC);
		m_PC += 2;
		break;

	case VCPU_MOVMMM: // MOVMMM lp lp  メモリ間接メモリ to メモリ
		*(LONG*)*(m_PC+1) = *(LONG*)*(LONG*)*m_PC;
		m_PC += 2;
		break;

	case VCPU_LEAS:	  // LEAS Im lp （[Sp-Im]の実効アドレスを入れるlp=sp-Im）
		*(LONG*)*(m_PC+1) = (LONG)((BYTE*)m_SP - *m_PC);
		m_PC += 2;
		break;
	/////////////////////////////////////////////////////////////////

	case VCPU_CMPBE: // CMP(<) Im1 Im2 lp （比較命令 符号比較でIm1 < Im2ならばlpのメモリに非0をストア）
		if (*m_PC < *(m_PC+1)) {
			*(LONG*)*(m_PC+2) = 1;
		} else {
			*(LONG*)*(m_PC+2) = 0;
		}
		m_PC += 3;
		break;

	case VCPU_CMPLE: // CMP(<=) Im1 Im2 lp （比較命令 符号比較でIm1 <= Im2ならばlpのメモリに非0をストア）
		if (*m_PC <= *(m_PC+1)) {
			*(LONG*)*(m_PC+2) = 1;
		} else {
			*(LONG*)*(m_PC+2) = 0;
		}
		m_PC += 3;
		break;

	case VCPU_CMPAB: // CMP(>) Im1 Im2 lp （比較命令 符号比較でIm1 > Im2ならばlpのメモリに非0をストア）
		if (*m_PC > *(m_PC+1)) {	// 不要なようでもあれば便利！
			*(LONG*)*(m_PC+2) = 1;
		} else {
			*(LONG*)*(m_PC+2) = 0;
		}
		m_PC += 3;
		break;

	case VCPU_CMPME: // CMP(>=) Im1 Im2 lp （比較命令 符号比較でIm1 > Im2ならばlpのメモリに非0をストア）
		if (*m_PC >= *(m_PC+1)) {	// 不要なようでもあれば便利！
			*(LONG*)*(m_PC+2) = 1;
		} else {
			*(LONG*)*(m_PC+2) = 0;
		}
		m_PC += 3;
		break;

	case VCPU_CMPEQ: // CMP(==) Im1 Im2 lp （比較命令 Im1==Im2ならばlpのメモリに非0をストア）
		if (*m_PC == *(m_PC+1)) {
			*(LONG*)*(m_PC+2) = 1;
		} else {
			*(LONG*)*(m_PC+2) = 0;
		}
		m_PC += 3;
		break;

	case VCPU_CMPNE: // CMP(!=) Im1 Im2 lp （比較命令 Im1!=Im2ならばlpのメモリに非0をストア）
		if (*m_PC != *(m_PC+1)) {
			*(LONG*)*(m_PC+2) = 1;
		} else {
			*(LONG*)*(m_PC+2) = 0;
		}
		m_PC += 3;
		break;

#if _MSC_VER >= 1200	// VC++6.0upper
	// 到達しないことをコンパイラに通知することにより、上限・下限チェックを省略させる！
	default:
		__assume(0);
			// This tells the optimizer that the default
			// cannot be reached. As so it does not have to generate
			// the extra code to check that '*(m_PC++)' has a value 
			// not represented by a case arm.  This makes the switch 
			// run faster.
#endif // _MSC_VER >= 1200

	} // end switch
	// たったこれだけだけど、ユーザー関数があるから拡張自在だし
	// これくらいシンプルなほうが良いと思う....

  } // end for
}

/////////////////////////////////////////////////////////////////////

void CVirtualCPU::SetStackSize(DWORD dw){
	DELETE_SAFE(m_StackFrame);
	m_StackFrame = new LONG[dw];
	m_SP		= m_StackFrame;
}

/////////////////////////////////////////////////////////////////////
CVirtualCPU::CVirtualCPU(void){
	m_StackFrame		= NULL;
	m_SP				= NULL;
	m_PC				= NULL;

	m_lpNativeCode = new CVM2i386((DWORD)&m_PC,(DWORD)&m_SP,(DWORD)&m_ReturnNum);
	m_lpNativeCode->Convert(NULL,0);	//	ネイティブコード用スタートアップルーチンの生成
}

CVirtualCPU::~CVirtualCPU(){
	DELETE_SAFE(m_lpNativeCode);
	DELETEPTR_SAFE(m_StackFrame);
}
