#include "stdafx.h"

//	yaneParser.cpp

#include "yaneParser.h"

CParser::CParser(void){
	m_scanner.SetDebugFile(NULL);
}

void		CParser::DebugFile(LPSTR filename){
	m_scanner.SetDebugFile(m_generator.DebugFile(filename));
}

LRESULT		CParser::SetInstructionAreaSize(BYTE*start,DWORD dwSize){
	m_generator.SetInstructionAreaSize(start,dwSize);	// 横投げ:p
	return 0;
}

DWORD		CParser::GetInstructionAreaSize(void){
	return m_generator.GetInstructionAreaSize();
}

DWORD		CParser::GetInstructionCodeSize(void){
	return m_generator.GetInstructionCodeSize();
}

void		CParser::SetUserFunctionList(CUserFunctionList*p){
	m_generator.SetUserFunctionList(p);	// 横投げ:p
}

void		CParser::SetUserVariableList(CUserVariableList*p){
	m_generator.SetUserVariableList(p);	// 横投げ:p
}

void		CParser::SetDllFunction(CUserFunctionList*p,CImportDllList*q,int*lpn,DWORD dwScriptSDK){
	m_generator.SetDllFunction(p,q,lpn,dwScriptSDK);
}

void		CParser::Unimport(int nScriptLevel){
	m_generator.Unimport(nScriptLevel);
}

//////////////////////////////////////////////////////////////////////////////

LRESULT		CParser::Compile(const string& filename){
	if (m_scanner.ReadFile(filename)!=0) return -1;

	m_generator.m_ErrorFP = NULL;
	
	m_loop_label_counter = 0;
	m_statement_level = 0;
	m_generator.BeginGenerate();

	sym = m_scanner.PeekSym();	// 最初のひとつだけ先読みしておく:p

	// Parsing開始！
	program();

	m_generator.EndGenerate();
	if (m_generator.m_ErrorFP!=NULL) {
		fprintf(m_generator.m_ErrorFP,"\n\tスクリプトのコンパイルエラー内容は、以上です。");
		fclose(m_generator.m_ErrorFP);
		m_generator.m_ErrorFP=NULL;
	}

	return m_generator.m_nErrorCount;
}

void	CParser::Error(const string& str){		//	エラーメッセージ表示用
	m_generator.Error(str);	//	丸投げ:p
}

//////////////////////////////////////////////////////////////////////////////
//	scannerクラスのいるやつ:p

EToken	CParser::GetSym(void){
	EToken e = m_scanner.GetSym();
	sym	= m_scanner.PeekSym();	// 一命令だけ先読み
	return e;
}

//////////////////////////////////////////////////////////////////////////////
//	非終端記号のパージングルーチンの羅列から
//////////////////////////////////////////////////////////////////////////////

int		CParser::function_parameter(void){
// パラメータを順番にスタックに積む。積んだバイト数を返す
	if (sym!=TK_LPR) {
		Error("関数呼出しが(で始まっていない");
		return 0; // これちゃうやん...
	}
	GetSym();

	int size = 0;
	int size2 = 0;
	while (true){
		if (sym==TK_RPR) {	// )で終わり
			GetSym();
			break ;
		}
		CCodeItem x;
		x.SetEmpty();	//	ループになっているので２回目で困る(?)
		expression(x);

		// ローカル変数と、引数を同じスタックで共有するからこんなややこしい
		// ことになるのだが...
		m_generator.Op4(VCPU_PUSH,x);	// 便利な世の中になったのぉ:p
		size = 4; // 4byte固定
		m_generator.m_SPOffset += size; // こいつをずらしていく必要がある
		size2 += size;	// スタックを加算した分だけあとで戻す

		if (sym==TK_RPR) {
			// TK_RPRの場合一周して終わる
			m_generator.m_SPOffset -= size2;	// これで、正常やね？		
			continue;
		}
		if (sym==TK_COMMA) {
			GetSym();
			if (sym == TK_RPR) {
				Error("関数の呼び出しでカンマのあとの数字が省略されている");
			}
			continue;
		}

		Error("関数の呼出しパラメータがおかしい");
		break;
	}
	return size2;
}

	//////////////////////////////////////////////////////////////////////////////
	// Op1,Op2,Op3を利用すれば、すっきり書けるね！
	//////////////////////////////////////////////////////////////////////////////


// factorでなければ、ひとつもGetSymせずに帰る
//	(ただし、ジャンプラベルは例外)
bool	CParser::factor(CCodeItem &x){
	//	debuged '99/12/31
	if (sym==TK_MINUS) {	// 単項マイナス
		GetSym();
		if (!factor(x)) {
			Error("単項マイナスのあとがおかしい");
		} else {
			CCodeItem y;
			y.SetValue((LONG)0);
			m_generator.Op1(VCPU_SUB,y,x);	// しもたなー。結果はyに入ってきよる
			x = y;	// とりゃー！！
		}
		return true;
	}
	if (sym==TK_PLUS) {
		GetSym();	// 単項プラスには意味がない。
		return factor(x);
	}
	if (sym==TK_MUL) {	// ポインタ演算子
		GetSym();
		if (!factor(x)) {
			Error("単項マイナスのあとがおかしい");
		} else {
			if (x.IsAddress()){	// まだアドレスかー！
				m_generator.Op2(x);	// スタックに積んで
			}
			x.SetAddress(true);	// ポインタ演算子よーん
			if (!m_bLeft) m_generator.Op2(x);	//	右辺値のアドレス参照スタックは禁止！
		}
		return true;
	}
	if (sym==TK_AND) {	// アドレス演算子
		GetSym();
		if (!factor(x)) {
			Error("単項マイナスのあとがおかしい");
		} else {
			if (x.IsReg()) {	//	変数
				m_generator.Put(VCPU_LEAS,x.GetLabel(),"$");	// これで実効アドレスが入るはず...
				x.SetType(3);	// OnStack!			
			} else if (x.IsAddress()) {
				x.SetAddress(false);	//	非アドレスにしておく。
			} else {
				Error("アドレス演算子&が使えない");
			}
		}
		return true;
	}
	if (sym==TK_CPL) {
		GetSym();
		factor(x);
		m_generator.Op3(VCPU_CPL,x);
		return true;
	}
	if (sym==TK_NOT) {	// 単項NOT演算子
		GetSym();
		factor(x);
		m_generator.Op3(VCPU_NOT,x);
		return true;
	}
	if (sym==TK_LPR) {
		GetSym();
		expression(x);
		if (sym!=TK_RPR) {
			Error("左括弧を忘れてるよ");
		} else {
			GetSym();
		}
		return true;
	}
	if (sym==TK_LABEL) { //
		x.SetLabel(m_scanner.GetLabel());	// Reg!?
		GetSym();
		selector(x);	// セレクタ

		if (sym==TK_LPR) {	// えー！！関数呼出しーだったの?
			// xは無視しちゃる！
			DWORD ADR;
			if (m_generator.IsGrobalLabel(x.GetLabel()) || m_generator.IsLocalLabel(x.GetLabel())){
			// 変数に()をつけて関数呼び出しスタイルをとっているのか？
				m_generator.Op2(x);	// xをスタックに積んだった:p
				m_generator.Put(VCPU_LEAS,(DWORD)0,"$");	// いまから積むねんから、いまのspを入れておけば十分
				int n = function_parameter();	// パラメータを順番にスタックに積む
				// スタックに積んだバイト数を返す
				m_generator.Put3(VCPU_CALLUF);	//	(_2,_1)逆順評価
				m_generator.PutPara("$");
				m_generator.Put(VCPU_SUBSP,n);	// スタックに積んだ分、戻す
				x.SetType(3);	// On the stack...		
				// いらんなら捨ててよ。＞返し値
			} else if (ADR = m_generator.IsUserFunc(x.GetLabel())){
				m_generator.Put(VCPU_LEAS,(DWORD)0,"$");	// いまから積むねんから、いまのspを入れておけば十分
				int n = function_parameter();	// パラメータを順番にスタックに積む
				// スタックに積んだバイト数を返す
				m_generator.Put(VCPU_CALLUF,ADR,"_","$");
				m_generator.Put(VCPU_SUBSP,n);	// スタックに積んだ分、戻す
				x.SetType(3);	// On the stack...		
				// いらんなら捨ててよ。＞返し値
			} else { // なら内部関数かよ！
				int n = function_parameter();	// パラメータを順番にスタックに積む
				m_generator.Put(VCPU_CALL);
				m_generator.PutGrobalLabelAddress(x.GetLabel());	// あとでアドレス算出していれてね
				m_generator.Put(VCPU_SUBSP,n);

				m_generator.Put(VCPU_MOVRETNUM,"$");
				// 左辺値がなければ捨てるべきのような気もするが
				// その最適化は、先読みなしには不可能である

				x.SetType(3);
			}
			
		} else if (sym == TK_COLON) {	// えー！ジャンプラベルだったのー！！
			m_generator.SetLabel(x.GetLabel());	// そのラベルはジャンプラベル
			x.SetEmpty();

			GetSym();
			return false;
		}
		return true;
	}
	if (sym==TK_NUM) {
		x.SetValue(m_scanner.GetNum());
		
		GetSym();
		return true;
	}
	if (sym==TK_QUOTE) {
		x.SetValue((DWORD)*m_scanner.GetLabel());	// 1文字
		GetSym();
		return true;
	}
	if (sym==TK_WQUOTE) {
		x.SetValue(m_generator.PutString(m_scanner.GetLabel()));
		GetSym();

		while (sym==TK_PLUS){	// 連結String!
			GetSym(); // TK_PLUS
			if (sym != TK_WQUOTE) {
				Error("文字列の連結記号＋のあとに文字列が来ていない");
			} else {
				x.SetValue(m_generator.AppendString(m_scanner.GetLabel()));
				GetSym();
			}
		}
		
		// 文字列をコード生成して、そこへのアドレスを得る。
		return true;
	}

	// factorではないっちゅーことやね:p
	return false;
}

void	CParser::term(CCodeItem &x){
	factor(x);
	if (x.IsEmpty()) return ; // だめじゃん...

	while (true){
		if (sym == TK_MUL || sym==TK_DIV || sym==TK_AND || sym==TK_MOD || sym==TK_MOD2) {
			
			VCPU inst;
			switch (sym){
			case TK_MUL :	inst = VCPU_MUL; break;
			case TK_DIV :	inst = VCPU_DIV; break;
			case TK_AND :	inst = VCPU_AND; break;
			case TK_MOD :	inst = VCPU_MOD; break;
			case TK_MOD2 :	inst = VCPU_MOD2; break;
			}
			m_generator.Op2b(x);	//	痛いなぁ．．．こんなコード入れんとあかんかぁ...

			GetSym();
			CCodeItem y;
			y.SetEmpty();	//	ループになっているので２回目で困る。
			factor(y);

			m_generator.Op1(inst,x,y);
			continue;
		}
		// うーん。エラーではないのか...
		break;
	}
}

void	CParser::simple_expression(CCodeItem &x){
	term(x);
	if (x.IsEmpty()) return ; // だめじゃん...
	while (true){
		// 本来なら、この部分、命令を順番にならべておいて範囲比較で良いのだけど
		// 下手なことして、あとでエンバグるのが恐い．．．
		if (sym==TK_PLUS || sym==TK_MINUS || sym==TK_OR || sym==TK_XOR ||
			sym==TK_SHR || sym==TK_SHL) {

			VCPU inst;
			switch(sym){
			case TK_PLUS:	inst=VCPU_ADD;	break;
			case TK_MINUS:	inst=VCPU_SUB;	break;
			case TK_OR:		inst=VCPU_OR;	break;
			case TK_XOR:	inst=VCPU_XOR;	break;
			case TK_SHR:	inst=VCPU_SHR;	break;
			case TK_SHL:	inst=VCPU_SHL;	break;
			}

			m_generator.Op2b(x);	//	痛いなぁ．．．こんなコード入れんとあかんかぁ...

			GetSym();

			CCodeItem y;
			y.SetEmpty();	//	ループになっているので２回目で困る。
			term(y);
			
			m_generator.Op1(inst,x,y);
			continue;
		}
		break;
	}
}

void	CParser::expression(CCodeItem &x){
	simple_expression(x);
	if (sym>=TK_BE && sym<=TK_NE) {
		// 比較演算子。これは畳み込みは行なわない。
		// また、Tokenと命令は１対１に対応しているから...
		
		m_generator.Op2b(x);	//	痛いなぁ．．．こんなコード入れんとあかんかぁ...

		EToken e = GetSym();
		CCodeItem y;
		simple_expression(y);
		m_generator.Op1((VCPU)((int)e-(int)TK_BE+(int)VCPU_CMPBE),x,y);
	}
}

void	CParser::selector(CCodeItem &x){
	//	納期やばいので構造体はサポートせず。
	//	怒られるかなぁ...
	if (sym==TK_LSQ) {	// 配列ちゃん:p
		GetSym();

		if (x.IsNum()) {
			Error("配列名が数値？");
			return ;
		}

		//	もしラベルがポインタ型ならば、
		if (m_generator.IsPointer(x.GetLabel())) {
			//	何もしない(手抜き:p)
		} else {
			// x[y] := *(&x + y*x.BaseSize);
			m_generator.Put(VCPU_LEAS,x.GetLabel(),"$");	// これで実効アドレスが入るはず...
			x.SetType(3);	// これでいいのだ:p
		}

		CCodeItem y;
		expression(y);
	
		// 配列参照は、ポインタ演算だから、基底サイズ倍して加算...
		CCodeItem z;
		z.SetValue(x.GetBaseSize());
		m_generator.Op1(VCPU_MUL,y,z);	// 基底倍
		m_generator.Op1(VCPU_ADD,x,y);	// 加算
		x.SetAddress(true);	// これはポインタ的に扱ってね！
		if (!m_bLeft) m_generator.Op2(x);	//	右辺値のアドレス参照スタックは禁止！

		if (sym!=TK_RSQ) {
			Error("配列の]を忘れている");
		} else {
			GetSym();
		}
	}

}

	////////////////////////////////////////////////////////////////
bool	CParser::assignment(void){
	m_bLeft = true;	//	左辺値は、特殊扱い

	CCodeItem x;
	expression(x);

	if (x.IsEmpty()) {
		m_bLeft = false;
		return false;
	}

	// 左辺値かどうかのチェックできひんなぁ...
	if (sym==TK_BECOME) {
		GetSym();

		m_bLeft = false;
		CCodeItem y;
		expression(y);
		m_generator.Op5(x,y);	//	代入用オペコード:p

		return true;	// TK_DLMを読み込みなちゃい
	}
	if (sym==TK_INC) {
		GetSym();
		m_generator.Op3(VCPU_INC,x);
		return true;	// TK_DLMを読み込みなちゃい
	}
	if (sym==TK_DEC) {
		GetSym();
		m_generator.Op3(VCPU_DEC,x);
		return true;	// TK_DLMを読み込みなちゃい
	}

	//	関数();の形なので
	if (m_bLeft) return true; // TK_DLMを読み込みなちゃい
	
	return false;
}

	////////////////////////////////////////////////////////////////

bool	CParser::statement(void){
	m_generator.SequencePoint();
	//	ここがシーケンスポイントになるので、
	//	この時点で副作用を持っている式は無駄ということになる。

	if (sym==TK_GOTO){ // goto文
		GetSym();
		if (sym!=TK_LABEL) {
			Error("Gotoのあとがラベルでない");
			return true;
		}
		m_generator.Put(VCPU_JMP);
		m_generator.PutLabelAddress(m_scanner.GetLabel());	

		GetSym();
		if (sym!=TK_DLM) {
			Error(";を忘れている");
		} else {
			GetSym();
		}
		return true;
	}
	if (sym==TK_DLM) { // ";" 空文
		GetSym();
		return true;
	}
	if (sym==TK_LBR) { // "{"
		GetSym();
		statement_sequence();
		if (sym!=TK_RBR) {
			Error("}を忘れている");
		}
		GetSym();
		return true;
	}
	if (sym==TK_IF) {
		GetSym();
		if(sym!=TK_LPR){
			Error("ifで(を忘れている");
		} else GetSym();
		
		CCodeItem x;
		expression(x);		// 式が来て...
		
		if(sym!=TK_RPR){
			Error("ifで)を忘れている");
		} else GetSym();

		m_generator.Op4(VCPU_IF,x); // このあとL1が来る...
		DWORD dwID = m_generator.GetLabelAddress();	// ラベル作成
		m_generator.PutLabelAddress(dwID);
		statement();
		if (sym==TK_ELSE){	// 次がelseならば、それも処理しなければならない
			GetSym();
			m_generator.Put(VCPU_JMP);
			DWORD dwID2 = m_generator.GetLabelAddress();
			m_generator.PutLabelAddress(dwID2);

			m_generator.SetLabel(dwID);
			statement();
			m_generator.SetLabel(dwID2);
		} else {
			m_generator.SetLabel(dwID);
		}
		return true;
	}
	if (sym==TK_LOOP) {	// LOOP文だよーん
		GetSym();
		DWORD dwID = m_generator.GetLabelAddress();
		m_generator.SetLabel(dwID);

		m_loop_label[m_loop_label_counter] = m_generator.GetLabelAddress();
		m_loop_label_counter++;
		// breakで抜けるためのラベルを用意
		
		statement();
		
		// LOOPの終了部にLOOP先頭に戻るコードを配置
		m_generator.Put(VCPU_JMP);
		m_generator.PutLabelAddress(dwID);
		
		m_generator.SetLabel(m_loop_label[--m_loop_label_counter]);

		return true;
	}
	if (sym==TK_BREAK){
		GetSym();

		int nest=1;
		if (sym==TK_NUM) { // break数指定
			nest = m_scanner.GetNum();		
			GetSym();
		}
		nest = m_loop_label_counter - nest;
		if (nest<0) {
			Error("不正なbreak");
			return false;
		}
		m_generator.Put(VCPU_JMP);
		m_generator.PutLabelAddress(m_loop_label[nest]);
		
		if (sym!=TK_DLM) {
			Error(";を忘れている");
			return false;
		}
		GetSym();
		return true;
	}
	if (sym==TK_ALT) {
		alt_statement();
		return true;
	}
	if (sym==TK_JUMP) {
		jump_statement();
		return true;
	}
	if (sym==TK_RETURN) {
		GetSym();
		if (sym==TK_DLM) {
			GetSym();
			int ofs = m_generator.GetLocalLabelArea();
			m_generator.Put(VCPU_SUBSP,ofs);	// スタック位置を戻す
			m_generator.Put(VCPU_RETURN);
			return true;
		}
		CCodeItem x;
		expression(x);
		int ofs = m_generator.GetLocalLabelArea();
		if (x.IsNum()) {
			//	こっちにしないと、RETURNNUMの生成するときにスタックが狂うことになる
			m_generator.Put(VCPU_SUBSP,ofs);	// スタック位置を戻す
			m_generator.Put(VCPU_RETURNNUM,x.GetValue());	// NUMを返す
		} else {
			m_generator.Op2(x);
			//	こっちにしないと、RETURNNUMの生成するときにスタックが狂うことになる
			m_generator.Put(VCPU_SUBSP,ofs);	// スタック位置を戻す
			m_generator.Put(VCPU_RETURNNUM,"_");	// NUMを返す
		}
		if (sym!=TK_DLM){
			Error("returnで;を忘れている");
		} else {
			GetSym();
		}
		return true;
	}
	if (sym==TK_HALT) {
		GetSym();
		m_generator.Put(VCPU_HALT);
		if (sym!=TK_DLM) {
			Error("haltのあとに;を忘れている");
		} else {
			GetSym();
		}
		return true;
	}
	if (sym==TK_FOR) {
		for_statement();
		return true;
	}
	if (sym==TK_DO) {
		do_statement();
		return true;
	}
	if (sym==TK_WHILE) {
		while_statement();
		return true;
	}
	if (sym==TK_LABEL || sym==TK_PLUS || sym==TK_MINUS || sym==TK_MUL) { // こりゃ恐らくは関数呼出しか何か
	// でも、ジャンプラベルの可能性もある...しくしく
		if (assignment()) {
			if (sym!=TK_DLM) {
				Error(";が無いのよーん");
			} else {
				GetSym();
			}
		}
		return true;
	}

	return false;	// だめじゃん...
}

void	CParser::statement_sequence(void){
	while (statement())
		
		;//	こんなんでええんかにゃー:p
}

	/////////////////////////////////////////////////////////////////
void	CParser::for_statement(void){
	//	for(statement; expression ; assignment) statement
	//	for(  ;LoopCheck  ; LoopStart	) { LoopStartOn ... } LoopEnd
	DWORD dwLoopStart	= m_generator.GetLabelAddress();
	DWORD dwLoopStartOn	= m_generator.GetLabelAddress();
	DWORD dwLoopCheck	= m_generator.GetLabelAddress();

	DWORD dwLoopEnd		= m_loop_label[m_loop_label_counter++] = m_generator.GetLabelAddress();
	// breakで抜けるためのラベルを用意

	GetSym();		// for
	if (sym!=TK_LPR) {
		Error("forで(を忘れている");
		return ;
	}
	GetSym();		// (

	statement();	//	初期化子

	m_generator.SetLabel(dwLoopCheck);

	CCodeItem x;					//	終了条件
	expression(x);
	m_generator.Op4(VCPU_IF,x);		//	このあと終了ラベル
	m_generator.PutLabelAddress(dwLoopEnd);

	m_generator.Put(VCPU_JMP);
	m_generator.PutLabelAddress(dwLoopStartOn);	// ループ開始ポイントへ

	if (sym!=TK_DLM) {
		Error("forで;を忘れている");
		return ;
	}
	GetSym();

	m_generator.SetLabel(dwLoopStart);

	assignment();
	if (sym!=TK_RPR) {
		Error("forで)を忘れている");
		return ;
	}
	GetSym();

	m_generator.Put(VCPU_JMP);
	m_generator.PutLabelAddress(dwLoopCheck);
	m_generator.SetLabel(dwLoopStartOn);

	statement();
	m_generator.Put(VCPU_JMP);
	m_generator.PutLabelAddress(dwLoopStart);

	m_generator.SetLabel(dwLoopEnd);		// 次のCase文にラベルを仕掛ける
	--m_loop_label_counter;
}
	/////////////////////////////////////////////////////////////////
void	CParser::while_statement(void){
	//	while (expression) statement
	//	while (LoopCheck ) statement LoopEnd
	DWORD dwLoopEnd		= m_loop_label[m_loop_label_counter++] = m_generator.GetLabelAddress();
	DWORD dwLoopCheck	= m_generator.GetLabelAddress();

	GetSym();		// for
	if (sym!=TK_LPR) {
		Error("whileで(を忘れている");
		return ;
	}
	GetSym();		// (
	m_generator.SetLabel(dwLoopCheck);
	CCodeItem x;					//	終了条件
	expression(x);
	m_generator.Op4(VCPU_IF,x);		//	このあと終了ラベル
	m_generator.PutLabelAddress(dwLoopEnd);
	if (sym!=TK_RPR) {
		Error("whileで)を忘れている");
		return ;
	}
	GetSym();
	statement();
	m_generator.Put(VCPU_JMP);
	m_generator.PutLabelAddress(dwLoopCheck);

	m_generator.SetLabel(dwLoopEnd);
	--m_loop_label_counter;
}
	/////////////////////////////////////////////////////////////////
void	CParser::do_statement(void){
	//	do statement while (expression);
	//	do LoopStart: statement expression LoopEnd

	DWORD dwLoopStart	= m_generator.GetLabelAddress();
	DWORD dwLoopEnd		= m_loop_label[m_loop_label_counter++] = m_generator.GetLabelAddress();

	GetSym();		// do
	m_generator.SetLabel(dwLoopStart);
	statement();

	if (sym!=TK_WHILE) {
		Error("doに対応するwhileを忘れている");
		return ;
	}
	GetSym();
	if (sym!=TK_LPR) {
		Error("whileで(を忘れている");
	}
	GetSym();
	
	CCodeItem x;					//	終了条件
	expression(x);
	m_generator.Op3(VCPU_NOT,x);	//	終了条件は逆条件
	m_generator.Op4(VCPU_IF,x);		//	このあとループラベル
	m_generator.PutLabelAddress(dwLoopStart);
	if (sym!=TK_RPR) {
		Error("whileで)を忘れている");
		return ;
	}
	GetSym();

	m_generator.SetLabel(dwLoopEnd);
	--m_loop_label_counter;
}

	/////////////////////////////////////////////////////////////////
void	CParser::jump_statement(void){	// added 99/12/01
	GetSym();	//	jump
	if (sym!=TK_LPR) {
		Error("jumpで(を忘れている");
		return ;
	}
	GetSym();
	CCodeItem x;
	expression(x);		// 式が来て...

	m_generator.Op4(VCPU_XALT,x);

	int n = 2;
	while (sym==TK_COMMA){
		GetSym();
		if (sym==TK_LABEL) {
			m_generator.PutLabelAddress(m_scanner.GetLabel());
			GetSym();
		} else {
			m_generator.PutPara((DWORD)0);	// dummy
		}
		n++;	//	,のごとに加算してゆく。
	}
	
	//	ネイティブコード化のため仕様変更'00/01/30
	m_generator.PutPara((DWORD)-1);		// end of the jump table
	//	これを埋めて、これをストップコードとしてネイティブコード化する。

	if (sym!=TK_RPR) {
		Error("jumpで)を忘れている");
		return ;
	}
	GetSym();
	if (sym!=TK_DLM) {
		Error("jumpで;を忘れている");
		return ;
	}
	GetSym();
}
	/////////////////////////////////////////////////////////////////
void	CParser::alt_statement(void){
	GetSym();	//	alt
	if (sym!=TK_LBR) {
		Error("altで{を忘れている");
		return ;
	}
	GetSym();
	DWORD dwID = m_generator.GetLabelAddress();
	while (case_block(dwID))
					;		//	こんなんでええんかにゃー:p
	if (sym!=TK_RBR) {
		Error("altで}を忘れている");
		return ;
	}
	GetSym();
	m_generator.SetLabel(dwID);
}

bool	CParser::case_block(DWORD dwQuitAlt){
	if (sym==TK_CASE){
		// IF文のところからぶっこ抜き:p
		GetSym();
		CCodeItem x;
		expression(x);		// 式が来て...
		m_generator.Op4(VCPU_IF,x); // このあとL1が来る...
		DWORD dwID = m_generator.GetLabelAddress();	// ラベル作成
		m_generator.PutLabelAddress(dwID);
		if (sym!=TK_COLON) {
			Error("caseで:を忘れている");
		} else {
			GetSym();
		}
		statement();
		m_generator.Put(VCPU_JMP);
		m_generator.PutLabelAddress(dwQuitAlt);	// alt文の終わりに

		m_generator.SetLabel(dwID);	// 次のCase文にラベルを仕掛ける
		return true;
	}
	if (sym==TK_DEFAULT) {
		GetSym();
		if (sym!=TK_COLON) {
			Error("defaultで:を忘れている");
		} else {
			GetSym();
		}
		statement();	// もうジャンプも何もしない
		return false; // これでcaseブロックは終わりのはず
	}
	return false;
}

void CParser::declare_parameters(void){
	//	関数パラメータのタイプ名は、無視する:p
	if (sym>=TK_LONG && sym<=TK_STR) {
		GetSym();
	}
	int bPointer=0;
	while (sym==TK_MUL) {
		bPointer++;
		GetSym();
	}
	if (sym==TK_LABEL){
		m_generator.vari(false,TK_LONG,m_scanner.GetLabel(),false,0,bPointer);
		GetSym();
	} else if (sym==TK_VOID) {	// voidなら構わない
		GetSym();	// 無視:p
	}
	while (true){
		if (sym==TK_RPR) return ;
		if (sym==TK_COMMA){
			GetSym();		// ,
			//	関数パラメータのタイプ名は、無視する:p
			if (sym>=TK_LONG && sym<=TK_STR) {
				GetSym();
			}
			bPointer = 0;
			while (sym==TK_MUL) {
				bPointer++;
				GetSym();
			}
			if (sym==TK_LABEL){
				m_generator.vari(false,TK_LONG,m_scanner.GetLabel(),false,0,bPointer); // 変数の確保
				// local,TYPE e,label名,1==array,array数
				GetSym();
			} else {
				Error(",の後がおかしい");
			}
			continue;
		}

		Error("関数の引数がおかしい(型名は不要です)");
		break;
	}
}

void	CParser::local_declaration(void){
	while(true) { 
		if (sym>=TK_LONG && sym<= TK_VOID) {
		// 変数宣言...
			GetSym();

lp:		// 変数をカンマで区切って連続させてあるかも
			int bPointer = 0;

			while (sym==TK_MUL) {
				bPointer++;
				GetSym();
			}
			if (sym!=TK_LABEL) {
				Error("変数宣言がおかしい");
				return ;
			}
			string s;
			s = m_scanner.GetLabel();
			GetSym();
			
			bool	bArray = false;
			int		nArray = 0;
			if (sym==TK_LSQ){
				GetSym();
				CCodeItem x;
				x.SetEmpty();	//	ループになっているので２回目で困る。
				expression(x);
				if (!x.IsNum()) {
					Error("配列変数の添え字が定数でない");	
					return ;
				}
				nArray = x.GetValue();
				if (sym!=TK_RSQ) {
					Error("配列変数の宣言で ] を忘れている");	
					return ;
				} else {
					GetSym();
				}
				bArray = true;
			}
			m_generator.vari(false,sym,s,bArray,nArray,bPointer);
			// 変数領域の確保
			// 1==grobal,TYPE e,label名,0==not array,array数
			
			if (sym==TK_COMMA) {
				GetSym();
				goto lp;
			}
			if (sym!=TK_DLM) {
				Error(";を忘れている");		
			} else {
				GetSym();
			}
			continue;
		}

		break;
	}
}

// 上のんコピーしたれ... + 初期化子追加
void	CParser::grobal_declaration(EToken e,const string& label){
	string s;
	s = label;
	bool	bArray;
	int		nArray;
	while(true) { 
		int bPointer = 0;
		while (sym==TK_MUL) {
			bPointer++;
			GetSym();
		}
		// 変数をカンマで区切って連続させてあるかも...
		bArray = false;
		nArray = 0;

		if (sym==TK_LSQ){
			GetSym();
			CCodeItem x;
			x.SetEmpty();	//	ループになっているので２回目で困る。
			expression(x);
			if (!x.IsNum()) {
				Error("配列変数の添え字が定数でない");	
				return ;
			}
			nArray = x.GetValue();
			if (sym!=TK_RSQ) {
				Error("配列変数の宣言で ] を忘れている");	
				return ;
			} else {
				GetSym();
			}
			bArray = true;
		}
		m_generator.vari(true,e,s,bArray,nArray,bPointer);
		// 変数領域の確保
		// 1==grobal,TYPE e,label名,0==not array,array数

		if (sym==TK_BECOME) {
		//	初期化子付きらしい...
			GetSym();
			if (bArray) {
				if (sym!=TK_LBR) { // = { .. }
					Error("配列変数の初期化子が{で始まっていない");
					return ;
				}
				GetSym();
			}
			if (nArray==0) nArray=1;
			DWORD*	pds;
			pds = (DWORD*)m_generator.GetDS();
			//	ここで得ないと初期化子が文字列を確保すると
			//	配列のアドレスが狂う…
			for(int i=0;i<nArray;i++){
				CCodeItem x;
				x.SetEmpty();	//	ループになっているので２回目で困る。
				expression(x);
				if (!x.IsNum()) {
					Error("変数の初期化子が定数でない");	
					return ;
				}
				// s[i] = n;
				if (pds!=NULL) {
					*(pds + i) = x.GetValue();
				}
				if (sym==TK_RBR || sym==TK_DLM) break;
				if (sym==TK_COMMA) {
					if (bArray) GetSym();
					continue;
				}
				Error("カンマなしで配列初期化子が列挙されている");
			}
			if (bArray) {
				if (sym!=TK_RBR) {
					Error("配列変数の初期化子が}で終わっていない");
					return ;
				}
				GetSym();
			}
		} else {
			//	一応、グローバル変数は０で初期化することにする。
			DWORD*	pds = (DWORD*)m_generator.GetDS();
			if (pds!=NULL) {
				if (nArray==0) nArray=1;
				for(int i=0;i<nArray;i++){
					*(pds + i) = 0;
				}
			}
		}

		if (sym==TK_COMMA) {
			GetSym();
			if (sym==TK_LABEL) {
				s = m_scanner.GetLabel();
				GetSym();
				continue;
			}
			Error(",のあとがおかしい");
		}
		if (sym==TK_DLM) {
			GetSym();
			return ;
		}
		Error(";を忘れている");	
		break;
	}
}
//////////////////////////////////////////////////////////////////////////////

void	CParser::function(void){ // 要は、statementよ。
	if (sym!=TK_LBR) {
		Error("関数が{で始まっていない");
	}
	GetSym();
	
	if (sym>=TK_LONG && sym<= TK_VOID) {
		// 変数宣言...
		local_declaration();
	}
	int ofs = m_generator.GetLocalLabelArea();
	m_generator.Put(VCPU_ADDSP,ofs);

	statement_sequence();	// ステートメント

	if (sym!=TK_RBR) {
		Error("関数が}で終わっていない");
		m_generator.Put(VCPU_SUBSP,ofs);
		m_generator.Put(VCPU_RETURN);
		m_generator.FunctionExitPoint();
	} else {
	//	GetSymの前に入れないと、デバッグダンプの順番がおかしくなる
		m_generator.Put(VCPU_SUBSP,ofs);
		m_generator.Put(VCPU_RETURN);
		m_generator.FunctionExitPoint();
		GetSym();
	}
	//　ローカルラベルは、きちんと解決されたか？
}

//////////////////////////////////////////////////////////////////////////////
void	CParser::program(void){
	m_generator.Put(VCPU_JMP);		// 頭で関数メインに飛ぶ！
//	m_generator.Put(VCPU_CALL);		// 頭で関数メインに飛ぶ！
	m_generator.PutGrobalLabelAddress("main"); // 

//	m_generator.Put(VCPU_JMP);
//	m_generator.PutGrobalLabelAddress("terminate"); // 終了処理関数

	m_bLeft = false;		//	通常は、右辺値

	while (true){
		if (sym==TK_IMPORT){
			GetSym();
			if (sym==TK_WQUOTE) {
				LRESULT l = m_generator.Import(m_scanner.GetLabel());
				if (l!=0) {
					l = m_generator.Import(((string)"lib/"+m_scanner.GetLabel()+".dll").c_str());
					if (l!=0) {
						Error("importで示しているファイルが存在しない");
					}
				}
				GetSym();
			} else {
				Error("import文のあとにファイル名が来ていない");
			}
			continue;
		}
		if (sym>=TK_LONG && sym<= TK_VOID) {
		// 関数定義か変数宣言か、プロトタイプ宣言
			GetSym();

			int bPointer = 0;
			while (sym==TK_MUL) {
				bPointer++;
				GetSym();
			}
			// *と&は、無視...
			if (sym!=TK_LABEL) {
				Error("ここは、変数名か関数名が来るべき");
			}
			string s;
			s = m_scanner.GetLabel();
			GetSym();
			if (sym==TK_DLM || sym==TK_LSQ || sym==TK_COMMA || sym==TK_BECOME) { // ";"なので変数宣言
				grobal_declaration(sym,s);	//	グローバル変数
				continue;				
			}
			if (sym==TK_LPR) { // "("なので関数実体
				GetSym();
				m_generator.SetGrobalLabel(s);
				//	ラベルの配置

				// (longしかないんで引数名だけで良い？)
				m_generator.ResetLocalLabel();	// ローカルラベルをクリア
				declare_parameters();
				m_generator.ParameterEnd();		// 引数リスト終了
				if (sym!=TK_RPR) {
					Error("関数定義で\")\"を忘れている");
				} else {
					GetSym();
				}
				function();	// 関数実体
				continue;
			}
			continue;
		}
		if (sym==TK_EOF) break;
		Error("ファイルスコープに意味不明の文字が...");
		break;
	}
	m_generator.FileExitPoint();	// 未処理ラベル等のチェック
}