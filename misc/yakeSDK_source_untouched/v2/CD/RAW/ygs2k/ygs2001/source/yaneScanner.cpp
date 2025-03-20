#include "stdafx.h"

// yaneScanner.cpp
#include "yaneScanner.h"
#include "../../yaneSDK/YTL/yaneMacro.h"

	/////////////////////////////////////////////////////////////////
	//　エラー表示のために泣く泣くstaticにした。
	//	そのうち変更するかも

string		CScanner::m_static_filename;	//	エラー表示のため
int			CScanner::m_static_nLine;
FILE*		CScanner::m_lpFP = NULL;			//	デバッグ用ファイル出力

	/////////////////////////////////////////////////////////////////

CScanner::CScanner(void){
	m_lpScanner	= NULL;
	Init();
}

CScanner::~CScanner(){
	DELETE_SAFE(m_lpScanner);
}

void CScanner::Init(void){
	m_nLine = 0;
	m_static_nLine = m_nLine;
	m_lpPos	= m_line;
	m_bPeeked = false;
	strcpy(m_line,"");

	m_nNestLevel = 0;
	m_nComment	= 0;
	DELETE_SAFE(m_lpScanner);
}

	/////////////////////////////////////////////////////////////////

LRESULT CScanner::ReadFile(const string& filename){
	Init();
	m_static_filename = filename;
	return m_file.Read(filename);
}

	/////////////////////////////////////////////////////////////////

EToken CScanner::GetSym(void){
	if(m_lpScanner!=NULL) {	// include中！
		EToken e;
		e = m_lpScanner->GetSym();
		if (e!=TK_EOF) return e;
		DELETE_SAFE(m_lpScanner);	// include終了！
		m_static_filename=m_file.GetName();
	}
	if (m_bPeeked) {
		m_bPeeked = false;
		return m_sym;
	}
	EToken e = PeekSym();	// 何とゆー投げやりな...:p
	m_bPeeked = false;
	return e;
}

EToken CScanner::PeekSym(void){	// 呼んだついでにコピーする
	m_sym = PeekSym2();
	return m_sym;
}

EToken CScanner::PeekSym2(void){
	if(m_lpScanner!=NULL) {	// include中！
		EToken e;
		e = m_lpScanner->PeekSym();
		if (e!=TK_EOF) return e;
		DELETE_SAFE(m_lpScanner);	// include終了！
		m_static_filename=m_file.GetName();
	}
//	次のバッファを一つ見るのでありんす（バッファから取り除きはしない）
	if (m_bPeeked) {
		return m_sym;	// これは、もうええんちゃうん？
	}

	m_bPeeked = true;

	while (true) {
		BYTE c = *m_lpPos;
		//	charはsignedなのでBYTEかintにしないとハマル

		if (IsToken(m_lpPos,"/*")) {
			m_nComment++;	//	コメント行の開始
			continue;
		}
		if (m_nComment) {
			//	コメント文中の処理
			if (c == '\0' ) { // 行末やん...？
				if (ReadLine()) return TK_EOF;	// EOFだよーん
				//	このEOFの出現は予期せぬものであるはずだが…
				continue;
			}
			if (IsToken(m_lpPos,"*/")) {
				m_nComment--;
				continue;
			}
			//	漢字の１バイト目か？
			if (((c>=0x80)&&(c<=0x9f))||
				((c>=0xe0)&&(c<=0xff))){
				m_lpPos+=2;	//	漢字まるごとスキップ
			} else {
				m_lpPos++;	//	そうでなければ一つスキップ
			}
			continue;
		}

		// スペース or TAB
		if (c == ' ' || c == '\t') { m_lpPos++; continue; }

		// 全角スペースのスキップ
		if ((c == 0x81) && (*(m_lpPos+1) == 0x40)) {
			m_lpPos+=2; continue;
		}

		if (c == '\0' ) { // 行末やん...？
			if (ReadLine()) return TK_EOF;	// EOFだよーん
			continue;
		}
			
		if (IsToken(m_lpPos,"//")) {	// コメント行
			if (ReadLine()) return TK_EOF;	// EOFだよーん
			continue;	
		}

		// 本当は、ハッシュ化したいのだが、手間の割には...
		if (IsToken2(m_lpPos,"halt"))	return TK_HALT;
		if (IsToken2(m_lpPos,"if"))		return TK_IF;
		if (IsToken2(m_lpPos,"else"))	return TK_ELSE;
		if (IsToken2(m_lpPos,"loop"))	return TK_LOOP;
		if (IsToken2(m_lpPos,"break"))	return TK_BREAK;
		if (IsToken2(m_lpPos,"alt"))	return TK_ALT;
		if (IsToken2(m_lpPos,"case"))	return TK_CASE;
		if (IsToken2(m_lpPos,"default"))return TK_DEFAULT;
		if (IsToken2(m_lpPos,"goto"))	return TK_GOTO;
		if (IsToken2(m_lpPos,"return"))	return TK_RETURN;
		if (IsToken2(m_lpPos,"for"))	return TK_FOR;
		if (IsToken2(m_lpPos,"while"))	return TK_WHILE;
		if (IsToken2(m_lpPos,"do"))		return TK_DO;
		if (IsToken2(m_lpPos,"jump"))	return TK_JUMP;
		if (IsToken2(m_lpPos,"import"))	return TK_IMPORT;

		// type name
		if (IsToken2(m_lpPos,"long"))	return TK_LONG;
		if (IsToken2(m_lpPos,"int"))	return TK_INT;
		if (IsToken2(m_lpPos,"short"))	return TK_SHORT;
		if (IsToken2(m_lpPos,"byte"))	return TK_BYTE;
		if (IsToken2(m_lpPos,"str"))	return TK_STR;
		if (IsToken2(m_lpPos,"function")) return TK_FUNC;
		if (IsToken2(m_lpPos,"void"))	return TK_VOID;

		if (IsToken(m_lpPos,"<="))		return TK_LE;
		if (IsToken(m_lpPos,">="))		return TK_ME;
		if (IsToken(m_lpPos,"=="))		return TK_EQ;
		if (IsToken(m_lpPos,"!="))		return TK_NE;

		if (IsToken(m_lpPos,">>"))		return TK_SHR;
		if (IsToken(m_lpPos,"<<"))		return TK_SHL;
		if (IsToken(m_lpPos,"++"))		return TK_INC;
		if (IsToken(m_lpPos,"--"))		return TK_DEC;

		if (IsToken(m_lpPos,"->"))		return TK_MEMBER;

		if (IsToken(m_lpPos,"||"))		return TK_OR;
		if (IsToken(m_lpPos,"&&"))		return TK_AND;
		if (IsToken(m_lpPos,"%%"))		return TK_MOD2;

		if (IsToken(m_lpPos,"include"))	{
			if (m_nNestLevel>=16)		return TK_ERROR; // Too Many Nest!!
			m_lpScanner = new CScanner;
			m_lpScanner->m_nNestLevel = m_nNestLevel+1;
			while (*m_lpPos==' ' || *m_lpPos=='\t') m_lpPos++;
			GetQuotedLabel(m_lpPos);
			if (m_lpScanner ->	ReadFile(m_label)) {
				DELETE_SAFE(m_lpScanner);
				return TK_ERROR;
			}
			EToken e;
			e = m_lpScanner->PeekSym();
			if (e!=TK_EOF) {
				m_bPeeked = false; // その場合、先読み分をフラッシュ
				return e;
			}
			DELETE_SAFE(m_lpScanner);	// いきなりinclude終了！
			m_static_filename=m_file.GetName();
			c = '\0';
		}

		// 一文字トークン

		if (c=='<')	{ m_lpPos++;	return TK_BE;	}
		if (c=='>')	{ m_lpPos++;	return TK_AB;	}

		if (c=='.')	{ m_lpPos++;	return TK_DOT;	}
		if (c==',')	{ m_lpPos++;	return TK_COMMA;}

		if (c==';')	{ m_lpPos++;	return TK_DLM;	}
		if (c==':')	{ m_lpPos++;	return TK_COLON;	}

		if (c=='+')	{ m_lpPos++;	return TK_PLUS;	}
		if (c=='-')	{ m_lpPos++;	return TK_MINUS;}
		if (c=='*')	{ m_lpPos++;	return TK_MUL;	}
		if (c=='/')	{ m_lpPos++;	return TK_DIV;	}
		if (c=='%') { m_lpPos++;	return TK_MOD;	}

		if (c=='|')	{ m_lpPos++;	return TK_OR;	}
		if (c=='&')	{ m_lpPos++;	return TK_AND;	}
		if (c=='^')	{ m_lpPos++;	return TK_XOR;	}
		if (c=='~')	{ m_lpPos++;	return TK_CPL;	}
		if (c=='!')	{ m_lpPos++;	return TK_NOT;	}
		if (c=='.')	{ m_lpPos++;	return TK_DOT;	}

		if (c=='{')	{ m_lpPos++;	return TK_LBR;	}
		if (c=='}')	{ m_lpPos++;	return TK_RBR;	}
		if (c=='(')	{ m_lpPos++;	return TK_LPR;	}
		if (c==')')	{ m_lpPos++;	return TK_RPR;	}
		if (c=='[')	{ m_lpPos++;	return TK_LSQ;	}
		if (c==']')	{ m_lpPos++;	return TK_RSQ;	}
		if (c=='=') { m_lpPos++;	return TK_BECOME;	}

		// quote & double quote
		if (c=='\'')	{ GetQuotedLabel(m_lpPos);	return TK_QUOTE;	}
		if (c=='"')		{ GetQuotedLabel(m_lpPos);	return TK_WQUOTE;	}

		if (!NumCheck(m_lpPos)) {	// 調べて数値ならばm_numに。そうでなければ返し値、非0
			return TK_NUM;
		}

		CopyLabel(m_lpPos);			// m_lpPosから次のトークンまでコピー
		if (*m_label=='\0') return TK_ERROR; // 不明トークン
		return TK_LABEL;			// ラベルのコピー

	}	// GetToken...
}

LRESULT	CScanner::ReadLine(void){
	m_lpPos = m_line;
	m_nLine++;
	m_static_nLine = m_nLine;
	LRESULT hr;
	hr = m_file.ReadLine(m_line);
	// デバッグファイル出力中ならば、そちらにソースを織り込む
	if (m_lpFP!=NULL) fprintf(m_lpFP,"// %s\n",m_line);
	return hr;
}

	//////////////////////////////////////////////////////////////////

bool	CScanner::IsToken(LPSTR &lp,LPSTR lp2){
	LPSTR lpx = lp;
	while (true){
		if (*lp2=='\0') {
			// すべて一致した！
			lp = lpx;	// ポインタを進める！
			return true;
		}
		if (*(lpx++)!=*(lp2++)) return false;	//	ダメじゃん...
	}
}

bool	CScanner::IsToken2(LPSTR &lp,LPSTR lp2){
	LPSTR lpx = lp;
	while (true){
		if (*lp2=='\0') {
			// すべて一致した！
			if ((*lpx>='A' && *lpx<='Z')||(*lpx>='a' && *lpx<='z')||
				(*lpx>='0' && *lpx<='9')) {
				return false;	// 後ろに文字列が続いているということはラベルではないのか？
			}
			lp = lpx;	// ポインタを進める！
			return true;
		}
		if (*(lpx++)!=*(lp2++)) return false;	//	ダメじゃん...
	}
}

void	CScanner::CopyLabel(LPSTR& m_lpPos){		// m_lpPosから次のトークンまでコピー
	// A-Z,a-z,_,0-9,漢字が続いている間は、ラベル扱い

	LPSTR lp = m_label;
	while (true){
		BYTE c;
		c = *m_lpPos;
		if (((c>='A')&&(c<='Z'))||
			((c>='a')&&(c<='z'))||
			((c>='0')&&(c<='9'))||
			(c=='_')) {
			*(lp++) = c;
		} else if (	//	漢字の１バイト目か？
			((c>=0x80)&&(c<=0x9f))||
			((c>=0xe0)&&(c<=0xff))){
			*(lp++) = c;
			*(lp++) = *(++m_lpPos);	//	おまけでもう１バイトコピー:p
		} else {
			break;
		}
		m_lpPos++;
	}
	*lp = '\0';	// 終了コード
}

LRESULT	CScanner::NumCheck(LPSTR& m_lpPos){
// 調べて数値ならばm_numに。そうでなければ返し値、非0
	if (*m_lpPos <'0' || *m_lpPos >'9') return 1;	// 数値ちゃいますー！
	int nm;
	if ((*m_lpPos == '0') && (*(m_lpPos+1) == 'x')) {
		nm = 16;
		m_lpPos+=2;	// 進める:p
	} else {
		nm = 10;
	}

	m_num = 0;
	while (true){
		char c;
		c = *m_lpPos;
		switch (nm){
		case 10:	{	// 10進法
			if ((c>='0') && (c<='9')) {
				m_num *= nm;
				m_num += (LONG)(c-'0');
			} else {
				return 0;
			}
		}	break;
		case 16:	{	// 16進法
			if ((c>='0') && (c<='9')) {
				m_num *= nm;
				m_num += (LONG)(c-'0');
			} else if ((c>='A') && (c<='F')) {
				m_num *= nm;
				m_num += (LONG)(c-'A') + 10;
			} else if ((c>='a') && (c<='f')) {
				m_num *= nm;
				m_num += (LONG)(c-'a') + 10;
			} else {
				return 0;
			}
		}	break;
		} // end switch
		m_lpPos++;
	}
}

void	CScanner::GetQuotedLabel(LPSTR &p){
	char c = *(p++);
	// cで囲まれた部分をラベルにストア

	//	\nは、改行コードに変換する
	//	\\は、\に変換する

	LPSTR x = m_label;
	bool  bM=false;	// 2バイトコードの2バイト目か？
	while (true) {
		if (*p==c) { p++; break; }	// そこを抜けたところをポイントすべき
		if (*p=='\0') break;
		if (!bM && *p=='\\') {
			p++;
			if (*p=='n')  { *(x++) = '\n'; p++; continue; }
			if (*p=='\\') { *(x++) = '\\'; p++; continue; }
			//	Perhaps ScanError...
			//	break down ...
		} else if (!bM && (((BYTE)*p>=0x80 &&
				(BYTE)*p<=0xa0) || ((BYTE)*p>=0xe0 && (BYTE)*p<=0xff))) {
			bM = true;	//	２バイトコードの１バイト目であった
		} else {
			bM = false;	//	２バイトコードの１バイト目ではなかった
		}
		*(x++) = *(p++);
	}
	*x = '\0';
}

	//////////////////////////////////////////////////////////////////

LPSTR CScanner::GetLabel(void){
	if(m_lpScanner!=NULL) {	// include中ならば再帰的に呼び出して...
		return m_lpScanner->GetLabel();
	}
	return m_label;
}

LONG CScanner::GetNum(void){
	if(m_lpScanner!=NULL) {	// include中ならば再帰的に呼び出して...
		return m_lpScanner->GetNum();
	}
	return m_num;
}

