#include "stdafx.h"

// yaneVM2i386.cpp
//
//	This convert algorithms was thought by yaneurao(M.Isozaki)
//	See the road to a SuperProgrammer
//		http://www.sun-inet.or.jp/~yaneurao/rsp/
//

#include "yaneVM2i386.h"
#include "yaneVM2i386table.h"


LRESULT CVM2i386::Convert(DWORD* lpCode,DWORD dwSize){
	Free();

	//	確保すべきメモリサイズを算出する
	DWORD*	lpdw = lpCode;

	//	確保すべきサイズ
	m_dwCodeSize = VM_to_i386[m_nInitialCode].dwBytes;	//	イニシャルコード分加算

	//	仮想マシンコード１命令に対し、数バイト～数十バイトのi386ネイティブ
	//	コードとが直接的に対応する。よって、命令をスキャンして生成オブジェ
	//	クトサイズを求めることは簡単である。
	int size;
	while((BYTE*)lpdw<(BYTE*)lpCode+dwSize){
		if ((int)*lpdw > m_nInitialCode) return 1; // perhaps invalid instruction..
		m_dwCodeSize	+= VM_to_i386[*lpdw].dwBytes;
		size			=  VM_to_i386[*lpdw].dwSize;
		lpdw			+= (size & 0xff) + 1;
		if (size>=256) {	//	可変長命令。-1が来るまでスキップする。あまり良い実装ではないが…。
			while ((int)*(lpdw++) != -1)
									;
		}
	}
	//	なんと、これだけで確保すべきネイティブオブジェクトコード格納エリアの
	//	サイズが確定したことになる

	//	Windows95ではVirtualProtectではなくVirtualAllocを使ったほうが無難…
	m_lpNativeCode = new BYTE[m_dwCodeSize];
	::VirtualAlloc((LPVOID)m_lpNativeCode,m_dwCodeSize,MEM_COMMIT,PAGE_EXECUTE_READWRITE);

	lpdw = lpCode;	//	再スキャン開始
	BYTE*	lpby = (BYTE*)m_lpNativeCode;

	bool bFirst = true;
	while((BYTE*)lpdw<(BYTE*)lpCode+dwSize || bFirst){
		int	inst;
		DWORD*	lpdw2;	//	次の命令位置
		if (bFirst) {
			bFirst	= false;
			inst	= m_nInitialCode;
			//	一度目はイニシャルコードをフェッチしたとして処理する
			lpdw2	= lpdw;
		} else {
			inst = *lpdw;	//	命令フェッチ
			//	make address translation table on original memory(VM memory)
			*lpdw	= (DWORD)lpby;
			size	=  VM_to_i386[inst].dwSize;
			lpdw2	= lpdw + (size & 0xff) + 1;
			if (size>=256) {	//	可変長命令。-1が来るまでスキップする。
				while ((int)*(lpdw2++) != -1)
									;
			}
		}

		WORD* wdTable;
		wdTable = VM_to_i386[inst].lpwdInstruction;
		for(;;){
			WORD wd;
			wd = *(wdTable++);
			if(wd < 0x100) {
				*(lpby++) = (BYTE)wd;	//	通常の命令
			} else if(V_THIS(0) <= wd && wd < V_THIS(256)){
				//	address of this Instruction
				*((DWORD*)lpby) = (DWORD)(lpdw + wd-V_THIS(0));
				lpby += 4;
			} else if(V_NEXT(0) <= wd && wd < V_NEXT(256)){
				//	address of next Instruction
				*((DWORD*)lpby) = (DWORD)(lpdw2 + wd-V_NEXT(0));
				lpby += 4;
			} else if (wd == V_PC) {
				*((DWORD*)lpby) = m_PC;	//	address of m_PC
				lpby += 4;
			} else if (wd == V_SP) {
				*((DWORD*)lpby) = m_SP;	//	address of m_SP
				lpby += 4;
			} else if (wd == V_RNUM) {
				*((DWORD*)lpby) = m_RET;	//	address of m_ReturnNum;
				lpby += 4;
			} else if (wd == V_EOF) {
				break;
			} else {
				//	may be a unknown instruction.(inner bug)
				delete [] m_lpNativeCode;
				m_lpNativeCode = NULL;
				return 1;	// Error Termination!
			}
		}
		lpdw	= lpdw2;
	}
	//	なんとたったこれだけで仮想CPUコードから
	//	i386ネイティブコードに変換が終了したことになる

	//	メモリ属性を実行可能に変更する
//	VirtualProtect(m_lpNativeCode,m_dwCodeSize,PAGE_EXECUTE & PAGE_EXECUTE_READ,&m_flOldProtect);
	//	これで実行できまっしぇー！
	//	Windows95ではVirtualAllocを使ったほうが無難…

	return 0;
}

bool CVM2i386::m_bInitialize = false;
int	 CVM2i386::m_nInitialCode;
CVM2i386::CVM2i386(DWORD pc,DWORD sp,DWORD ret) {
	m_lpNativeCode = NULL;

	//	Initialize pointer
	m_PC = pc;
	m_SP = sp;
	m_RET= ret;

	// Initialize the translation table(size)
	if (!m_bInitialize) {
		m_bInitialize = true;
		int i;
		for(i=0;;i++) {
			if (VM_to_i386[i].lpwdInstruction == NULL) break;
			DWORD size;
			size = 0;
			for(int j=0;;j++){
				if (VM_to_i386[i].lpwdInstruction[j] == V_EOF) break;
				if (VM_to_i386[i].lpwdInstruction[j] >= V_DWORD) size+=4; else size++;
			}
			VM_to_i386[i].dwBytes = size;
			// 仮想CPUコードに対応するi386命令群のサイズを格納しておく。
		}
		m_nInitialCode = i-1;	//	これがイニシャルコード
	}
}

CVM2i386::~CVM2i386(void) {
	Free();
}

void CVM2i386::Free(void) {
	if (m_lpNativeCode!=NULL) {
		//	メモリ属性を元に戻してから解放する必要がある
		//	VirtualProtect(m_lpNativeCode,m_dwCodeSize,m_flOldProtect,NULL);
		VirtualFree((LPVOID)m_lpNativeCode,m_dwCodeSize,MEM_RELEASE);
		delete [] m_lpNativeCode;
		m_lpNativeCode = NULL;
	}
}
