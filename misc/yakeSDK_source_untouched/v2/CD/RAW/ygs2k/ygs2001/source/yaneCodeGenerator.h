//	yaneCodeGenerator.h :
//		コードジェネレータ
//
//		Programmed by yaneurao(M.Isozaki)	'99/07/25
//

#ifndef __yaneCodeGenerator_h__
#define __yaneCodeGenerator_h__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>					//	STL
#include <string>					//	STL
using namespace std;

#include	"yaneScanner.h"			// Scanningするクラス
#include	"yaneVirtualCPU.h"		// VirtualCPUクラス

//////////////////////////////////////////////////////////////////////////////

//	命令生成時のパラメータ
class VCPUPara {
public:
	VCPUPara(DWORD dw)		{ m_dw = dw;		m_type = 1; }
	VCPUPara(LPSTR str)		{ m_str = str;		m_type = 2; }
	VCPUPara(string str)	{ m_str = str;		m_type = 2; }
	bool	IsNum(void)		{ return m_type == 1; }
	bool	IsStr(void)		{ return m_type == 2; }
	DWORD	GetNum(void)	{ return m_dw; }
	string	GetStr(void)	{ return m_str; }
	bool	IsLabel(void)	{ return m_type==2 && m_str!="$" && m_str!="_"; }
protected:
	DWORD	m_dw;
	string	m_str;
	int		m_type;	// 数字か文字かどっちやねん？
};

//////////////////////////////////////////////////////////////////////////////

//	読み込んだコードアイテムを入れる
//	本当は、これを拡張して、配列やさまざまな型に対応させたかったのだけど...
class CCodeItem {
public:
	bool	IsEmpty(void)		{ return m_type == 0; }
	bool	IsReg(void)			{ return m_type == 1; }
	bool	IsNum(void)			{ return m_type == 2; }
	bool	IsOnStack(void)		{ return m_type == 3; }
	bool	IsAddress(void)		{ return m_bAdr; }

	void	SetValue(DWORD dw)	{ m_type = 2; m_dwValue = dw; }
	DWORD	GetValue(void)		{ return m_dwValue; }
	
	void	SetLabel(string s)	{ m_type = 1; m_label = s; }
	string&	GetLabel(void)		{ return m_label; }

	void	SetAddress(bool b)	{ m_bAdr = b; }	// アドレス参照なのか？

	void	SetType(int n)		{ m_type = n; }
	int		GetType(void)		{ return m_type; }
	void	SetEmpty(void)		{ m_type = 0; }

	int		GetBaseSize(void)	{ return 4; } // LONG以外は、未実装

	//	ポインタ型の変数なのか？
	void	IncPointer(void)	{ m_nPointer++; }
	void	DecPointer(void)	{ m_nPointer--; }
	void	SetPointer(int n)	{ m_nPointer = n; }
	int		IsPointer(void)		{ return m_nPointer; }

public:
	CCodeItem(void)	{ m_bAdr=false; m_nPointer=0; }

	// CopyConstructor
	CCodeItem(CCodeItem&p) { m_type=p.m_type; m_dwValue=p.m_dwValue;
		m_label=p.m_label; m_bAdr=p.m_bAdr; m_nBaseSize=p.m_nBaseSize;
		m_nPointer=0; }

protected:
	int		m_type;			//	0:empty 1:レジスタ名 2:数値 3:仮想スタック上の数値
	DWORD	m_dwValue;		//	値
	string	m_label;		//	変数名
	bool	m_bAdr;			//	アドレスなのか？
	int		m_nBaseSize;	//	ベースサイズはいくらなのか？
	int		m_nPointer;		//	ポインタ型変数なのか？
};

//////////////////////////////////////////////////////////////////////////////

// ユーザー公開関数は、CScriptCompilerのほうから提供してもらうの！
class CUserFunction {
public:
	string		m_label;
	LONG		(*m_func)(LONG *);

	CUserFunction(void) {}
	CUserFunction(const CUserFunction& user) { m_label=user.m_label; m_func = user.m_func; }
};


// ユーザー公開変数は、CScriptCompilerのほうから提供してもらうの！
class CUserVariable {
public:
	string		m_label;
	LONG		*m_vari;

	CUserVariable(void) { }
	CUserVariable(const CUserVariable& user) { m_label=user.m_label; m_vari = user.m_vari; }
};

typedef vector<CUserFunction > CUserFunctionList;
typedef vector<CUserVariable > CUserVariableList;

//////////////////////////////////////////////////////////////////////////////
//	importしたDLLリスト

class CImportDll {
public:
	//	DLLを読み込んで、その関数を関数テーブルに登録する
	LRESULT		Load(string filename,CUserFunctionList*,DWORD dwScriptSDK);

	//	読み込んだDLLを解放し、関数テーブルから削除する
	LRESULT		Unload(void);

	//	自分のスクリプトレベルを返す
	int*		GetScriptLevel(void) { return& m_nScriptLevel; }
	string		GetFileName(void) { return m_filename; }

	CImportDll(void);
//	CImportDll(const CImportDll& impdll);
	~CImportDll();

protected:
	string		m_filename;		//	DLLファイル名
	HINSTANCE	m_hInstance;	//	DLLハンドル
	int			m_nScriptLevel;	//	どのLevelにおいてImportしたか

	CUserFunctionList*	m_lpDllUserList;	//	ユーザー関数テーブル
	int			m_nFunctionSize;
};

typedef vector<CImportDll*> CImportDllList;

//////////////////////////////////////////////////////////////////////////////

// 変数名等の管理。こいつも拡張する予定だったのに...
class CLabelStruct {
public:
	string		label;		//	ラベル名
	EToken		type;		//	変数タイプ
	bool		bArray;		//	配列なのか？
	int			size;		//	配列である場合、そのサイズ
	DWORD		offset;		//	オフセット位置
	int			nPointer;	//	ポインタ型なのか？

	CLabelStruct(void) { }
	CLabelStruct(const CLabelStruct&ls){
		label	=	ls.label;
		type	=	ls.type;
		bArray	=	ls.bArray;
		size	=	ls.size;
		offset	=	ls.offset;
		nPointer=	ls.nPointer;
	}

};

class CJumpLabelStruct {
public:
	string		m_label;
	bool		m_bExist;	// すでに存在するのか？
	DWORD*		m_adr;		// もし存在するのであれば、そのアドレス
							// 未出であれば、それを使用している１つ目のアドレス
							// (そのアドレスからチェインで使用しているアドレスをたどっていく)

	CJumpLabelStruct(void) { }
	CJumpLabelStruct(const CJumpLabelStruct& js){
		m_label		=	js.m_label;
		m_bExist	=	js.m_bExist;
		m_adr		=	js.m_adr;
	}
};

class CLocalJumpLabelStruct {	// こちらは番号管理なので無駄がない
public:
	bool		m_bExist;	// 既出なのか？
	DWORD*		m_adr;		// もし既出であれば、そのアドレス
							// 未出であれば、それを使用している１つ目のアドレス
							// (そのアドレスからチェインで使用しているアドレスをたどっていく)
};

//	ベクター化
typedef vector<CLabelStruct >			CLabelList;
typedef vector<CJumpLabelStruct >		CJumpLabelList;
typedef vector<CLocalJumpLabelStruct >	CLocalJumpLabelList;
typedef vector<DWORD* >					CLocalPushList;

//////////////////////////////////////////////////////////////////////////////
class CCodeGenerator {
public:
	FILE*	DebugFile(LPSTR filename);		// デバッグ用ファイル出力
	
	LRESULT	SetInstructionAreaSize(BYTE*start,DWORD dwSize);
	//	命令エリアを指定する
	//	必ず最初に、これで確保したメモリを知らせてあげて！
	//	start==NULLならばダミーコンパイル

	DWORD	GetInstructionAreaSize(void);	//	命令+staticデータ領域
	//	ダミーコンパイルでわかった必要コードエリアサイズを返す
	DWORD	GetInstructionCodeSize(void);	//	命令サイズ
	//	実コンパイルでわかった必要コードエリアサイズを返す

	void	SetUserFunctionList(CUserFunctionList*p);
	//	CScriptCompilerで、ユーザー関数登録されたやつを
	//	どかーんとCParser経由で引き渡す

	void	SetUserVariableList(CUserVariableList*p);
	//	CScriptCompilerで、ユーザー変数登録されたやつを
	//	どかーんとCParser経由で引き渡す

	void	SetDllFunction(CUserFunctionList*p,CImportDllList*q,int* lpn,DWORD dwScriptSDK);
	//	ScriptCompilerからどかーんとCParser経由で頼んます:p

	void	BeginGenerate(void);			// コード生成開始
	void	EndGenerate(void);				// コード生成の終了

	/////////////////////////////////////////////////////////////////
public:
	// 命令コードの生成
	//		'$' : 次の'_'の出現したアドレスを入れる
	//		'_' : '$'と対応。
	//
	//	例)
	//		Put(VCPU_ADD,2,3,'$');		// ADD 2,3,$
	//		Put(VCPU_ADD,'_',4,100);	// ADD _,4,100
	//	という命令を生成すれば、100番地に2+3+4を入れることが出来る。
	//

	void	Put(VCPU inst);
	void	Put(VCPU inst,VCPUPara p1);
	void	Put(VCPU inst,VCPUPara p1,VCPUPara p2);
	void	Put(VCPU inst,VCPUPara p1,VCPUPara p2,VCPUPara p3);

	//
	void	PutPara(VCPUPara p);						// パラメータのPut
	bool	Put2(VCPU inst,VCPUPara p1);				// inc or dec
	bool	Put2(VCPU inst,VCPUPara p1,VCPUPara p2);	// 直交性をあげるためのPut
	void	Put3(VCPU inst);
	// Put(inst,"_","_","$");では、スタックが逆になって困る場合にどうぞ:p

	/////////////////////////////////////////////////////////////////

	void	Op1(VCPU inst,CCodeItem &p1,CCodeItem &p2);		//	２項演算子
	void	Op2(CCodeItem &p1);								//	スタックに積む
	void	Op2b(CCodeItem &p1);							//	ラベルはスタックに積む。即値は積まない。
	void	Op3(VCPU inst,CCodeItem &p1);					//	単項演算子の処理(inst Im1 Lp1型命令)
	void	Op4(VCPU inst,CCodeItem &p1);					//	(inst Im1型命令用)
	void	Op5(CCodeItem &p1,CCodeItem &p2);				//	代入用オペコード p1 = p2を生成する
//	void	Op6(VCPU inst,CCodeItem &p1,CCodeItem &p2);		//	(inst Im1,Im2型命令用)
	
	/////////////////////////////////////////////////////////////////

	//	文字列型データ
	DWORD	PutString(LPSTR p);		//  そのアドレスを返す
	DWORD	AppendString(LPSTR p);	//　連結文字列定数の為
	//	DWORD型データ
	DWORD	PutDWORD(DWORD d);		//	そのアドレスを返す

	int		GetLocalLabelArea(void);
	// ローカルラベル（パラメータ引数含まず）のサイズを返す

	DWORD	IsUserFunc(string);
	// それはユーザー関数(CScriptCompilerでRegisterUserFunctionされたもの）なのか？
	// もしそうであれば、その関数アドレスを返す
	int		IsLocalLabel(VCPUPara);
	//	ローカルラベルであれば、[sp+k]のk(<0)を返す。さもなくば0。
	DWORD	IsGrobalLabel(VCPUPara);
	//	グローバルラベルであれば、その配置アドレスを返す。さもなくば0。
	int		IsPointer(const string &p);	//	labelはポインタ型の変数か？

	void	SequencePoint(void);		//	シーケンスポイント
	void	FunctionExitPoint(void);	//	関数終了ポイント
	void	FileExitPoint(void);		//	ファイル終了ポイント
	//	ここがシーケンスポイント。この時点で副作用の持っている式を削除。
	// （実際には面倒なので削除していないが:p）

	/////////////////////////////////////////////////////////////////
	// 変数の登録／確保
	LRESULT	vari(bool bGrobal,EToken type,string label,
		bool bArray,int nArraySize,int nPointer);
	void	ResetLocalLabel(void);	// Localラベルのクリア
	void	ParameterEnd(void);		// 引数リストの終了

	BYTE*	GetDS(void) { return m_lpDS; }	//	variで変数確保した後とか、その確保されたメモリを
											//	指しているので初期化子を書くのに便利がええの。

	/////////////////////////////////////////////////////////////////
	// ラベル処理

	void	SetLabel(const string&);				// ラベルをそのアドレスに置く
	void	PutLabelAddress(const string&);			// ラベルのアドレスをコードに埋める
	void	SetGrobalLabel(const string&);			// グローバルラベルをそのアドレスに置く
	void	PutGrobalLabelAddress(const string&);	// グローバルラベルのアドレスをコードに埋める

	// 数字で管理されているラベル（ローカルラベル）
	DWORD	GetLabelAddress(void);	// ID識別子だけを取得する
	void	PutLabelAddress(DWORD);	// ID識別子で指定したラベルのアドレスをそこに埋める
	void	SetLabel(DWORD);		// ラベルをそのアドレスに置く

	/////////////////////////////////////////////////////////////////
	//	DLLのImport

	LRESULT	Import(LPCSTR filename);		//	DLLをImportする
	void	Unimport(int nScriptLevel);	//	nScriptLevelで読み込んだDLLを解放する

	/////////////////////////////////////////////////////////////////
public:
	int		m_SPOffset;				// PUSH中のローカル変数のためのスタック調整

	/////////////////////////////////////////////////////////////////
public:
	// エラー処理用ファイルポインタ
	FILE*	m_ErrorFP;				// 実際は、Parser側から操作される
	void	Error(const string&);	// こちらもエラー処理用関数を用意
	int		m_nErrorCount;			// エラー個数

	/////////////////////////////////////////////////////////////////

public:
	CCodeGenerator(void);
	~CCodeGenerator();
	
protected:
	/////////////////////////////////////////////////////////////////
	//	Debug用ファイル出力
	void	OutFile(VCPU);		//　デバッグ用命令コード出力。
	void	OutFile(VCPUPara);	//	CParaを出力する。
	void	OutFile(LPSTR);		//	文字列を出力する。
	void	OutFile(void);		//	そこのアドレスを出力する。
	
	/////////////////////////////////////////////////////////////////
protected:
	FILE*	m_lpFile;			//	デバッグ用、生成コード出力
	bool	m_bDebug;			//	デバッグモード用コード出力

	static	LONG m_dummy;		//	データを捨てるときは、こいつに入れて！
	/////////////////////////////////////////////////////////////////
protected:
	BYTE*	m_lpInstArea;		//	インストラクションエリア
	DWORD	m_dwInstAreaSize;	//	そのサイズ

	DWORD*	m_lpCS;				//	生成中のコード位置（先頭から）
	BYTE*	m_lpDS;				//	生成中のデータ位置（こちらは逆から）
protected:
	DWORD	m_dwCode;			//	ダミーコンパイル時のコードエリア測定用

	/////////////////////////////////////////////////////////////////
protected:
	bool	m_bParameterlist;						//	パラメータリスト受け取り中か？
	CLabelList		m_local_paralist;				//	パラメータリスト
	CLabelList		m_local_list;					//	関数内部で使用しているローカル変数
	CLabelList		m_grobal_list;					//	グローバル変数リストもっといるんか？
	CLocalPushList	m_local_push;					//	仮想ローカルスタックポインタ

	CUserFunctionList	*m_lpUserFuncList;			//	ユーザー登録関数一覧
	CUserVariableList	*m_lpUserVariList;			//	ユーザー登録変数一覧
	CUserFunctionList	*m_lpDllUserFunc;			//	DLL登録関数一覧
	CImportDllList		*m_lpDllUserList;
	int*				m_lpnScriptLevel;
	DWORD				m_dwScriptSDK;

	// 実は、関数の型チェックとかは一切していない。
	CJumpLabelList		m_grobal_jump_label_list;	// グローバル関数リスト
	CJumpLabelList		m_local_jump_label_list;
	CLocalJumpLabelList	m_local_inner_label_list;
	DWORD				m_dwLabelNo;	//	ラベルナンバー（連番）
};

#endif
