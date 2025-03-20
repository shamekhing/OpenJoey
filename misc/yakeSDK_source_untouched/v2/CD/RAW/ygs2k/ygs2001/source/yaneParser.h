//	yaneParser.h :
//		構文パーザ
//
//		Programmed by yaneurao(M.Isozaki)	'99/07/24
//
//		文をパージングしながら、並行してコードジェネレイトも行なう。
//

#ifndef __yaneParser_h__
#define __yaneParser_h__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include	"yaneVirtualCPU.h"		//	VirtualCPU
#include	"yaneScanner.h"			//	トークン解析
#include	"yaneCodeGenerator.h"	//	コード生成

class CParser {
public:
	LRESULT		SetInstructionAreaSize(BYTE*start,DWORD dwSize);
	//	コード生成エリアは、CScriptCompiler側で確保するのだ
	//	start==NULLならば、ダミーコンパイル

	DWORD		GetInstructionAreaSize(void);	//	命令+staticデータ領域
	//	ダミーコンパイルで必要だとわかったコードエリアサイズを返す
	DWORD		GetInstructionCodeSize(void);	//	命令
	//	実コンパイルで必要だとわかったコードエリアサイズを返す

	void		SetUserFunctionList(CUserFunctionList*);
	// ユーザー定義関数リストは、CScriptCompiler側で用意するのだ
	
	void		SetUserVariableList(CUserVariableList*);
	// ユーザー定義関数リストは、CScriptCompiler側で用意するのだ

	void		SetDllFunction(CUserFunctionList*p,CImportDllList*q,int*lpn,DWORD dwScriptSDK);
	//	DLLのimportリストもCScriptCompiler側から渡すのだ

	void		Unimport(int nScriptLevel);
	//	importしていたDLLの解放

	LRESULT		Compile(const string& filename);

	void		DebugFile(LPSTR filename);	// デバッグ用にファイルにコード出力

	/////////////////////////////////////////////////////////////////
public:
	CParser(void);
//	~CParser();

	/////////////////////////////////////////////////////////////////
protected:		//	まずは、非終端記号のパージングルーチンの羅列から
	// bool型をとる関数は、それでなければfalseを返す
	bool	factor(CCodeItem&);
	void	term(CCodeItem&);
	void	simple_expression(CCodeItem&);
	void	expression(CCodeItem&);
	bool	assignment(void);
	void	selector(CCodeItem&);
	void	alt_statement(void);
	void	for_statement(void);
	void	while_statement(void);
	void	do_statement(void);
	void	jump_statement(void);
	bool	case_block(DWORD dw); // ラベルID

	void	declare_parameters(void);
	bool	statement(void);
	void	statement_sequence(void);

	int		function_parameter(void);

	void	local_declaration(void);	// 忙しときに、もー！！
	void	grobal_declaration(EToken e,const string& label);	// 忙しときに、もー！！
	void	function(void);
	void	program(void);

	/////////////////////////////////////////////////////////////////

protected:
	void	Error(const string& str);		//	エラーメッセージ表示用

	/////////////////////////////////////////////////////////////////
protected:
	CScanner		m_scanner;		// スキャンクラス
	EToken			sym;			//	面倒なので、先読み分
	EToken			GetSym(void);	//	面倒なので作った:p
	
	CCodeGenerator	m_generator;	// コードジェネレータ

	DWORD			m_loop_label[32];	// loop文のラベル位置32ケ
	int				m_loop_label_counter;

	// not use...
	int				m_statement_level;	// statementのレベル

	int				m_stackpoint;
	// 関数が呼び出されたときのstackpointからaddしている量

	bool			m_bLeft;		//	左辺値なのか？
	//	右辺におけるアドレス参照スタックは廃止する。
	//	右辺におけるラベルは廃止する。
};


#endif
