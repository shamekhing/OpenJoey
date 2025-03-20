// yaneScaner.h :
//	Tokenを切り出すルーチン
//
//			Programmed by yaneurao(M.Isozaki) '99/07/20
//
//	備考：Tokenは次の行にまたがないことが保証されている
//

#ifndef __yaneScanner_h__
#define __yaneScanner_h__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
using namespace std;

#include "../../yaneSDK/yaneFile.h"

enum EToken {
	TK_EOF,		//	ファイル終わっとるで:p
	TK_ERROR,	//	読みこみエラー発生
	TK_LABEL,	//	ラベル＝変数＝関数名＝配列名＝ジャンプアドレスである
	TK_NUM,		//	数値 このあとGetNum()すれば、その数値が得られる

	TK_HALT,	//	停止命令（本来は）

	TK_DLM,		//	;
	TK_COLON,	//	:

	TK_PLUS,	//	+
	TK_MINUS,	//	-
	TK_MUL,		//	*
	TK_DIV,		//	/
	TK_OR,		//	|
	TK_AND,		//	&
	TK_XOR,		//	^
	TK_CPL,		//	~
	TK_NOT,		//	!
	TK_DOT,		//	.
	TK_COMMA,	//	,

	TK_LBR,		//	{
	TK_RBR,		//	}
	TK_LPR,		//	(
	TK_RPR,		//	)
	TK_LSQ,		//	[
	TK_RSQ,		//	]
	TK_QUOTE,	//	'...'	// 中身はGetLabelで得られる
	TK_WQUOTE,	//	"..."	// 中身はGetLabelで得られる

	TK_VAR,		//	var
	TK_IF,		//	if
	TK_ELSE,	//	else
	TK_LOOP,	//	loop	// 制御構文はloop～breakで十分
	TK_BREAK,	//	break
	TK_ALT,		//	alt		// alt { case a : st , case b: st , default:xxx }
	TK_CASE,	//	case
	TK_DEFAULT,	//	default
	TK_GOTO,	//	goto	// gotoでラベルジャンプ
	TK_RETURN,	//	return
	TK_FOR,		//	for
	TK_WHILE,	//	while
	TK_DO,		//	do
	TK_JUMP,	//	jump
	TK_IMPORT,	//	import
	TK_MOD,		//	%
	TK_MOD2,	//	%%

	TK_BE,		//	<
	TK_LE,		//	<=
	TK_AB,		//	>
	TK_ME,		//	>=
	TK_EQ,		//	==
	TK_NE,		//	!=

	TK_SHR,		//	>>
	TK_SHL,		//	<<
	TK_INC,		//	++
	TK_DEC,		//	--

	TK_BECOME,	//	=
	TK_MEMBER,	//	->

	TK_LONG,	//	long
	TK_INT,		//	int
	TK_SHORT,	//	short
	TK_BYTE,	//	byte
	TK_STR,		//	str
	TK_FUNC,	//	function
	TK_VOID,	//	void

};

class CScanner {
public:
	LRESULT ReadFile(const string& filename);	// ファイルを読み込む
	
	EToken GetSym(void);			// 次のtokenを得る（読み込みポインタ進める）
	EToken PeekSym(void);			// 次のtokenを得る（読み込みポインタ進めない）
	LPSTR	GetLabel(void);			// GetSym/PeekSymした結果tokenがTK_LABEL等であった場合、
									// これでそのラベルを得ることが出来る。
	LONG	GetNum(void);			// GetSym/PeekSymした結果tokenがTK_NUMであった場合、
									// これでその数値を得ることが出来る。

	/////////////////////////////////////////////////////////////////
	// エラーメッセージ用
	static int			GetScanline(void)	{ return m_static_nLine; }
	static const string	GetFileName(void)	{ return m_static_filename.c_str(); }
	void	SetDebugFile(FILE*fp) { m_lpFP = fp; } // デバッグ用ファイル
	/////////////////////////////////////////////////////////////////
public:
	CScanner(void);
	~CScanner();

	/////////////////////////////////////////////////////////////////
protected:
	EToken PeekSym2(void);
	
protected:
	CScanner	*m_lpScanner;	//	for nesting include
	int			m_nNestLevel;	//

	void	Init(void);		//	バッファ初期化
	LRESULT	ReadLine(void);	//	一行読み込み

	bool	IsToken(LPSTR &lp,LPSTR lp2);
							// トークンの一致を調べる。一致していればlp1をトークンの終わりまで進める	void	CopyLabel(LPSTR& m_lpPos);		// m_lpPosから次のトークンまでコピー
	bool	IsToken2(LPSTR &lp,LPSTR lp2);	//	こちらは、後ろに文字が続いていればマッチしない
							// トークンの一致を調べる。一致していればlp1をトークンの終わりまで進める	void	CopyLabel(LPSTR& m_lpPos);		// m_lpPosから次のトークンまでコピー
	void	CopyLabel(LPSTR& m_lpPos);		// m_lpPosから次のトークンまでコピー
	LRESULT	NumCheck(LPSTR& m_lpPos);		// 調べて数値ならばm_numに。そうでなければ返し値、非0
	void	GetQuotedLabel(LPSTR &m_lpPos); // 引用ラベルを得る

	CFile	m_file;			//	読み込んでいるファイル
	int		m_nLine;		//	現在読み込み中のラインナンバー

	static	string		m_static_filename;	//	エラー表示のため
	static	int			m_static_nLine;

	char	m_line[256];	//	ラインバッファ

	char	m_label[256];	//	ラベルバッファ	(for GetLabel)
	LONG	m_num;			//	数字バッファ	(for GetNum)

	LPSTR	m_lpPos;		//	m_lineの解析位置

	bool	m_bPeeked;		//	前回PeekSymしていれば、それをそのまま渡すことが出来る
	EToken	m_sym;			//	次のシンボルを先読みした場合、いったんここに保管する

	int		m_nComment;		//	/* */によるコメント

	static FILE*	m_lpFP;			//	デバッグ用ファイル出力
};

#endif
