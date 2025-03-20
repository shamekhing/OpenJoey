//	yaneScriptCompiler.h :
//
//	楽しい楽しいスクリプトコンパイルクラス:p
//		programmed by yaneurao(M.Isozaki)	'99/07/25
//

//
//	ユーザーファンクションは、
//		LONG function(LONG*) {
//		}
//	の形であること。引数は、LONG*の指すところに並んで入っている
//

#ifndef __yaneScriptCompiler_h__
#define __yaneScriptCompiler_h__

/////////////////////////////////////////////////////////////////
// ユーザー公開関数
#define TUserFunc(FUNC) LONG FUNC(LONG *p)
/////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>				//	STL
#include <string>				//	STL
using namespace std;

#include "yaneParser.h"
#include "yaneVirtualCPU.h"
#include "yaneVM2i386.h"	//	native code converter
#include "../../yaneSDK/YTL/yaneMacro.h"

class CCodeArea;
typedef vector<CCodeArea*> CCodeAreaArray;

//	コードエリアの管理用（突貫工事:p）
class CCodeArea {
public:
	BYTE* AllocCodeArea(DWORD dw) { Release(); return m_lpCodeArea = new BYTE[dw]; }
	void* GetCodeArea(void) { return m_lpCodeArea; }
	void  SetFileName(string filename) { m_filename = filename; }
	void  GetFileName(string& filename) { filename = m_filename; }
	bool  IsFile(string filename) { return m_filename==filename; }
	LRESULT		NativeCodeSetup(DWORD dwInstructionSize,CVirtualCPU& cpu){
		DELETE_SAFE(m_lpNativeCode);
		m_lpNativeCode = new CVM2i386((DWORD)&cpu.m_PC,(DWORD)&cpu.m_SP,(DWORD)&cpu.m_ReturnNum);
		return  m_lpNativeCode->Convert((DWORD*)m_lpCodeArea,dwInstructionSize);
	}

public:
	CCodeArea(void) {
		m_lpCodeArea	=	NULL;
		m_lpNativeCode	=	NULL;
	}
	~CCodeArea()	{ Release(); } 
protected:
	void Release(void)	{
		DELETEPTR_SAFE(m_lpCodeArea);
		DELETE_SAFE(m_lpNativeCode);
	}

	string		m_filename;
	BYTE*		m_lpCodeArea;		//	仮想CPU用のコードエリア
	CVM2i386*	m_lpNativeCode;		//	NativeCode Converter
};

enum EScriptMessage {
	ESM_Nothing,
	ESM_JumpScript,
	ESM_CallScript,	
	ESM_Quit,
};

class CScriptCompiler {
public:
	LRESULT	SetStackSize(DWORD dw);				// スタックサイズ
	LRESULT	SetInstructionAreaSize(DWORD dw);	// 命令領域
	void	SetDebugMode(bool);					// デバッグ用に中間ファイルを出力する

	LRESULT	Load(const string& filename,bool bOutError=true);
												// スクリプトをLoadする（コンパイル開始！）
	LRESULT	Unload(const string& filename);		// スクリプトをUnloadする:p
	LRESULT	Unload(void);						// 最後に読み込んだスクリプトをUnloadする:p

	LRESULT	Execute(void*exe_adr);				// 実行する。
	LRESULT	ReExecute(void);					// HALTのあと、再実行する。
	LONG*	GetEntryPoint(void);				// Loadで読み込んだスクリプトの実行開始アドレス（読み込み直後に呼出してね）
	LRESULT	PushReturnAddress(void);			// 返り先(this)をスタックに積む(cf.CVirtualCPU)

	LRESULT	RegistUserFunction(const string& funcname,LONG (*)(LONG *));
	// ユーザー関数を登録する。
	LRESULT	RegistUserVariable(const string& variname,LONG &);
	// ユーザー変数を登録する。

	void	SetScriptSDK(LONG p) { m_dwScriptSDK = p; };
	//	スクリプトＳＤＫのポインタをもらう

public:
	CScriptCompiler(void);	
	virtual ~CScriptCompiler();

	/////////////////////////////////////////////////////////////////
protected:
	CVirtualCPU			m_VCPU;			// 仮想CPUを一つ持つ
	CCodeAreaArray		m_aCodeArea;	// 複数のコードエリアを持つ
	/////////////////////////////////////////////////////////////////
	DWORD				m_nStackSize;
	DWORD				m_nInstructionAreaSize;
	/////////////////////////////////////////////////////////////////
	bool				m_bDebug;
	LONG				m_bErrorResult;	  // for Error Handler of CallScript
	/////////////////////////////////////////////////////////////////
	CUserFunctionList	m_UserFunc;
	CUserVariableList	m_UserVari;
	/////////////////////////////////////////////////////////////////
	CUserFunctionList	m_DllUserFunc;		//	Dll側から登録されたFunction
	CImportDllList		m_DllUserList;
	DWORD				m_dwScriptSDK;
	/////////////////////////////////////////////////////////////////
	LONG*				m_exe_adr;
	/////////////////////////////////////////////////////////////////
protected:
	//	メッセージ
	static EScriptMessage	m_Message;			//	メッセージがあるか？

	// 他スクリプトの呼出し
	static	string m_loadfile;					//	こいつに名前入れれ:p
	static	int	m_nScriptCallLevel;				//	スクリプトの呼出しレベル

	static TUserFunc(JumpScript);				//	(LPSTR)
	static TUserFunc(CallScript);				//	(LPSTR)
	static TUserFunc(Quit);						//	(void)
	static TUserFunc(SetErrorRestrain);			//	(bool)
	static bool	m_bErrorRestrain;				// CallScriptのエラー抑制
};

#endif
