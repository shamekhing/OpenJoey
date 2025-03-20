//
//	yaneVM2i386 :
//												2000/01/19
//
//		convert from Virtual Machine Code into i386 native Instruction
//							programmed by yaneurao(M.Isozaki)
//
//		This converting algorithms was developed by yaneurao '97.
//

#ifndef __yaneVM2i386_h__
#define __yaneVM2i386_h__

#include <windows.h>

class CVM2i386 {
public:
	LRESULT Convert(DWORD* lpCode,DWORD dwSize);
	LRESULT	Execute(void){return ((LRESULT(*)(void))m_lpNativeCode)();}

	void	Free(void);

	CVM2i386(DWORD pc,DWORD sp,DWORD ret);
	~CVM2i386();
protected:
	BYTE*	m_lpNativeCode;		//	ネイティブコード領域
	DWORD	m_dwCodeSize;		//	そのサイズ
	DWORD	m_flOldProtect;		//	その領域の旧メモリ属性
	DWORD	m_PC,m_SP,m_RET;	//	コード生成に際して使用するアドレス

	static bool	m_bInitialize;	//	テーブルを初期化したかのフラグ
	static int	m_nInitialCode;	//	イニシャルコードナンバー
};

#endif
