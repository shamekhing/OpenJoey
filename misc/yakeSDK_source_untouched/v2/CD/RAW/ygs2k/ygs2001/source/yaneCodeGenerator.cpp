#include "stdafx.h"

//	yaneCodeGenerator.cpp

#include "yaneCodeGenerator.h"
#include "../../yaneSDK/YTL/yaneMacro.h"

//////////////////////////////////////////////////////////////////////////////

LONG	CCodeGenerator::m_dummy;	// 捨てコード
//////////////////////////////////////////////////////////////////////////////
CCodeGenerator::CCodeGenerator(void) {
	m_lpFile=NULL;
	m_bDebug = false;
	m_dwLabelNo=0;

	m_dwInstAreaSize=0;	// 0KB
	m_lpInstArea=NULL;
	
	m_lpUserFuncList=NULL;
	m_ErrorFP = NULL;
}

CCodeGenerator::~CCodeGenerator() {
	if (m_lpFile!=NULL) fclose(m_lpFile);
};

LRESULT CCodeGenerator::SetInstructionAreaSize(BYTE*start,DWORD dwSize) {
	m_lpInstArea=start;
	m_dwInstAreaSize=dwSize;

	return 0;
};

DWORD CCodeGenerator::GetInstructionAreaSize(void) {
	return m_dwCode;
}

DWORD CCodeGenerator::GetInstructionCodeSize(void) {
	return (DWORD)((BYTE*)m_lpDS-(BYTE*)m_lpInstArea);
}

void	CCodeGenerator::SetUserFunctionList(CUserFunctionList*p){
	m_lpUserFuncList=p;
}

void	CCodeGenerator::SetUserVariableList(CUserVariableList*p){
	m_lpUserVariList=p;
}

void	CCodeGenerator::SetDllFunction(CUserFunctionList*p,CImportDllList*q,int*lpn,DWORD dwScriptSDK){
	m_lpDllUserFunc	= p;
	m_lpDllUserList	= q;
	m_lpnScriptLevel= lpn;
	m_dwScriptSDK	= dwScriptSDK;
}

void	CCodeGenerator::Error(const string& str){		//	エラーメッセージ表示用
	// このファイルのオープンは、Parser側で行なわれていると仮定して良い
	m_nErrorCount++;

	if (m_ErrorFP==NULL) {
		// エラーファイルは、起動ディレクトリに出力
		CFile f;
		m_ErrorFP = fopen(f.MakeFullName("CompileError.txt").c_str(),"w");
	}

	if (m_ErrorFP!=NULL) {
		fprintf(m_ErrorFP,"%s(%d) : %s\n"
			,CScanner::GetFileName().c_str()
			,CScanner::GetScanline(),str.c_str());

		//	画面にも出してやる．．
		//	CDbg dbg;
		//	dbg.Out(str);
	}	// 邪道?
}
//////////////////////////////////////////////////////////////////////////////
	
FILE*	CCodeGenerator::DebugFile(LPSTR filename){
	if (m_lpFile!=NULL) fclose(m_lpFile);
	CFile f;
	m_lpFile = fopen(f.MakeFullName(filename).c_str(),"w");
	m_bDebug=true;
	return m_lpFile;
}

void	CCodeGenerator::BeginGenerate(void){
	//	ラベルのクリア
	m_grobal_jump_label_list.clear();
	m_local_jump_label_list.clear();
	m_local_inner_label_list.clear();
	m_local_push.clear();

	m_SPOffset			= 0;	// PUSH中のスタック調整のため
	m_dwLabelNo			= 0;

	m_lpCS				= (DWORD*)m_lpInstArea;
	m_lpDS				= (BYTE*)m_lpInstArea + m_dwInstAreaSize;
	m_dwCode			=	0;

	m_nErrorCount = 0;
}

void	CCodeGenerator::EndGenerate(void){
	if (m_lpCS!=NULL && m_lpDS<(BYTE*)m_lpCS) {
		Error("プログラム格納の為のデータ領域があふれているのでプログラムは正常に動かないと思われます");
	}

	if (m_lpFile!=NULL) fclose(m_lpFile);
}

//////////////////////////////////////////////////////////////////////////////
//	命令コードの生成
//////////////////////////////////////////////////////////////////////////////
//
//	メモリを参照する命令は、そこを覚えておいてあとでロードされたアドレスに対する
//	オフセット値を加算する必要がある

void	CCodeGenerator::Put(VCPU inst){
	if	(m_bDebug) { OutFile(); OutFile(inst); OutFile("\n"); }
	if (m_lpCS!=NULL) { *(m_lpCS++) = inst; } else { m_dwCode+=4; }
}

void	CCodeGenerator::Put(VCPU inst,VCPUPara p1){
	if (inst==VCPU_ADDSP || inst==VCPU_SUBSP){	// ０演算は無意味
		if (p1.IsNum() && p1.GetNum()==0) return ;
	}

	if (Put2(inst,p1)) return ; // 特殊な命令か？
	if (m_bDebug) { OutFile(); OutFile(inst); OutFile("\t"); OutFile(p1); OutFile("\n"); }
	if (m_lpCS!=NULL) { *(m_lpCS++) = inst; } else { m_dwCode+=4; }
	PutPara(p1);
}

void	CCodeGenerator::Put(VCPU inst,VCPUPara p1,VCPUPara p2){
	if	(Put2(inst,p1,p2)) return ; // 特殊な命令か？
	if	(m_bDebug) { OutFile(); OutFile(inst); OutFile("\t"); OutFile(p1); OutFile("\t,\t"); OutFile(p2); OutFile("\n"); }
	
	if (m_lpCS!=NULL) { *(m_lpCS++) = inst; } else { m_dwCode+=4; }
	PutPara(p1);
	PutPara(p2);
}

void	CCodeGenerator::Put(VCPU inst,VCPUPara p1,VCPUPara p2,VCPUPara p3){
	if	(m_bDebug) { OutFile(); OutFile(inst); OutFile("\t"); OutFile(p1); OutFile("\t,\t"); OutFile(p2); OutFile("\t,\t"); OutFile(p3);OutFile("\n"); }
	if (m_lpCS!=NULL) { *(m_lpCS++) = inst; } else { m_dwCode+=4; }
	PutPara(p1);
	PutPara(p2);
	PutPara(p3);
}

void	CCodeGenerator::PutPara(VCPUPara p){
	if (m_lpCS==NULL) { m_dwCode+=4; return ; }
	if (p.IsNum()) {
		*(m_lpCS++) = p.GetNum();
 		return ;
	}
	if (p.GetStr()=="$") {
		// このメモリ位置をPushしやな...
		m_local_push.push_back(m_lpCS);
		m_lpCS++;
	} else if (p.GetStr()=="_") {
		// 一番前の$のアドレスがここを指すようにする。
		if (m_local_push.empty()){
			Error("CCodeGenerator::PutParaで_に対応する$が存在しない");
		} else {
			*(m_local_push.back()) = (DWORD)m_lpCS;
			m_local_push.pop_back();
		}
		*(m_lpCS++) = 0;
	} else {
		// これはエラーでないの？
		m_lpCS++;
		Error("宣言されていない変数が使用されている");
	}
}

//////////////////////////////////////////////////////////////////////////////
// MOV命令直交性向上委員会（意味不明）
//////////////////////////////////////////////////////////////////////////////
/*
	MOV X,Y
		X == [memory?1]
		Y == [memory?2]
			=> MOVM		?1		, ?2
	MOV X,Y
		X == [memory?1]
		Y == [sp-?2]
			=> MOVMS	?1,?2

	MOV X,Y
		X == [sp-?1]
		Y == [memory?2]
			=> MOVSM	?1,?2

	MOV X,Y
		X == [sp-?1]
		Y == [sp-?2]
			=> MOVSS	?1,?2
	/////////////////////////////////////////////////////////////////
	
	MOV X,$
		X == [sp-??]		: MOVSM		??		,$
		X == [memory??]		: MOVM		memory??,$
	MOV _,X
		X == [sp-??]		: MOVS		$,??
		X == [memory??]		: MOV		$,memory??

	/////////////////////////////////////////////////////////////////

	MOVM X,Y
		X == [memory?1]
		Y == [memory?2]
			=> MOVM		?1		, ?2

	MOVM X,Y
		X == [memory?1]
		Y == [sp-?2]
			=> MOVM		?1		, $
			   MOVS		_		, ?2

	MOVM X,Y
		X == [sp-?1]
		Y == [memory?2]
			=> MOVMSM	?1		,?2


	MOVM X,Y
		X == [sp-?1]
		Y == [sp-?2]
			=> MOVMSM	?1		,$
			   MOVS		_		,Y

	/////////////////////////////////////////////////////////////////
	
	MOVM X,$
		X == [sp-??]		: MOVMSM	??		,$
		X == [memory??]		: MOVMMM	memory??,$
	MOVM _,X
		X == [sp-??]		: MOVMS		$,??
		X == [memory??]		: MOVM		$,memory??

	/////////////////////////////////////////////////////////////////

	LEAS X,$
		X == [sp-??]		: LEAS		??		,$
		X == [memory??]		: MOV		memory??,$
*/

//	インクリメント＆デクリメントは特殊
bool	CCodeGenerator::Put2(VCPU inst,VCPUPara p1){
	if	(inst==VCPU_INC || inst==VCPU_DEC) {
		int k1,k2;
		k1 = IsLocalLabel(p1);
		k2 = IsGrobalLabel(p1);

		VCPU inst2;
		if (inst==VCPU_INC) inst2=VCPU_INCS; else inst2=VCPU_DECS;

		// 再帰的呼出しによって解決する
		if (k1!=0)	{ Put(inst2,-k1); return true; } 
		if (k2!=0)	{ Put(inst , k2); return true; }
		if (p1.IsLabel())	{ // それは未宣言の変数代入ではないか？
			Error("変数 "+p1.GetStr()+"が宣言せずに使用されています。");
			return true;
		}
	}
	return false;
}

bool	CCodeGenerator::Put2(VCPU inst,VCPUPara p1,VCPUPara p2){
	if	(inst==VCPU_MOV) {
		int k1,k2,k3,k4;

		k1 = IsLocalLabel(p1);
		k2 = IsGrobalLabel(p1);
		k3 = IsLocalLabel(p2);
		k4 = IsGrobalLabel(p2);

		if (k1!=0 && k3!=0) { Put(VCPU_MOVSS,-k1,-k3);	return true; }
		if (k1!=0 && k4!=0) { Put(VCPU_MOVSM,-k1,k4);	return true; }
		if (k2!=0 && k3!=0) { Put(VCPU_MOVMS,k2,-k3);	return true; }
		if (k2!=0 && k4!=0) { Put(VCPU_MOVM,k2,k4);		return true; }

		// 再帰的呼出しによって解決する
		if (k1!=0)	{ Put(VCPU_MOVSM,-k1,p2); return true; } 
		if (k2!=0)	{ Put(VCPU_MOVM , k2,p2); return true; }
		if (p1.IsLabel())	{ // それは未宣言の変数代入ではないか？
			Error("変数 "+p1.GetStr()+"が宣言せずに使用されています。");
			return true;
		}

		if (k3!=0)	{ Put(VCPU_MOVS,p1,-k3);	 return true; }
		if (k4!=0)	{ Put(VCPU_MOV,p1,k4);		 return true; }
		if (p2.IsLabel())	{ // それは未宣言の変数代入ではないか？
			Error("変数 "+p2.GetStr()+"が宣言せずに使用されています。");
			return true;
		}
	} else if (inst==VCPU_LEAS) {

		int k;
		if ((k = IsLocalLabel(p1))!=0)	{ Put(VCPU_LEAS,-k,p2);	return true; }
		if ((k = IsGrobalLabel(p1))!=0)	{ Put(VCPU_MOV,k,p2);	return true; }

	} else if (inst==VCPU_MOVM) {
		int k1,k2,k3,k4;

		k1 = IsLocalLabel(p1);
		k2 = IsGrobalLabel(p1);
		k3 = IsLocalLabel(p2);
		k4 = IsGrobalLabel(p2);

		if (k1!=0 && k3!=0) { Put(VCPU_MOVMSM,-k1,"$");	Put(VCPU_MOVS,"_",-k3);	return true; }
		if (k1!=0 && k4!=0) { Put(VCPU_MOVMSM,-k1,k4);	return true; }
		if (k2!=0 && k3!=0) { Put(VCPU_MOVM,k2,"$"); Put(VCPU_MOVS,"_",-k3); return true; }
		if (k2!=0 && k4!=0) { Put(VCPU_MOVM,k2,k4); return true; }

		if (k1!=0)	{ Put(VCPU_MOVMSM,-k1,p2);	return true; }
		if (k2!=0)	{ Put(VCPU_MOVMMM,k2,p2);	return true; }

		if (p1.IsLabel())	{ // それは未宣言の変数代入ではないか？
			Error("変数 "+p1.GetStr()+" が宣言せずに使用されています。");
			return true;
		}

		if (k3!=0)	{ Put(VCPU_MOVMS,p1,-k3);	return true; }
		if (k4!=0)	{ Put(VCPU_MOVM,p1,k4);		return true; }

		if (p2.IsLabel())	{ // それは未宣言の変数代入ではないか？
			Error("変数 "+p2.GetStr()+" が宣言せずに使用されています。");
			return true;
		}
	}
		
	return false;	// 未処理
}

//////////////////////////////////////////////////////////////////////////////

// Put命令で逆スタックで実行
//	Put(inst,"_2","_1");
void	CCodeGenerator::Put3(VCPU inst){
	if	(m_bDebug) { OutFile(); OutFile(inst); OutFile("\t_2\t,\t_1\t,\t($)\n"); }
	if  (m_lpCS==NULL) {
		m_dwCode+=12;	// 3DWORD命令なのだ
		return ;
	}

	*(m_lpCS++) = inst;

	// 一番前の$のアドレスがここを指すようにする。
	int size;
	if ((size = m_local_push.size())<=1) {
		Error("CCodeGenerator::CCodeGeneratorで_2,_1に対応する$が存在しない");
		return ;
	}

	*m_local_push[size-2] = (DWORD)m_lpCS;		// 逆順でPop
	*(m_lpCS++) = 0;
	*m_local_push[size-1] = (DWORD)m_lpCS;
	*(m_lpCS++) = 0;
 	m_local_push.pop_back();
 	m_local_push.pop_back();
}

//////////////////////////////////////////////////////////////////////////////
void	CCodeGenerator::Op1(VCPU inst,CCodeItem &p1,CCodeItem &p2){
	// ２項間演算

/*
	// 新しい演算子の追加方法
	//	1.CVirtualCPUに命令追加
	//	2.CCodeGenerator::OutFileにデバッグ用命令表示部追加
	//	3.ETokenにトークン追加
	//	4.CParserの適切な部分に、それを追加。
	//	5.この関数の定数畳み込み部分にその最適化コードを追加
*/
	if (p1.IsEmpty()) {
		Error("２項演算子の左の値がおかしい");
		return ;
	}
	if (p2.IsEmpty()) {
		Error("２項演算子の右の値がおかしい");
		return ;
	}

	// 可能な限りわかり見通し良くしておかないと、とんでもないバグを出しそう...
	if (!p1.IsNum()) {
		Op2(p1);
	}
	if (!p2.IsNum()) {
		Op2(p2);		// p2 -> p1の順でスタックに積まないと...
	}
	
	if (p1.IsNum() && p2.IsOnStack()){
		Put(inst,p1.GetValue(),"_","$");
	} else if (p1.IsOnStack() && p2.IsNum()) {
		Put(inst,"_",p2.GetValue(),"$");
	} else if (p1.IsOnStack() && p2.IsOnStack()) {
		Put3(inst);		// これだけは、逆順で評価されなくてはならない。
		PutPara("$");	// あとはこれでええじゃろ...
	} else if (p1.IsNum() && p2.IsNum()) {
		// 定数同士なので、定数畳み込みの最適化が出来る。

		// 本来は、ここから仮想ＣＰＵを実行させて、この返し値を利用する
		// べきかも知れないが、仮想ＣＰＵは速度を考慮した命令セットにしたため、
		// そこまでうまく出来ていない（笑）
		switch (inst) {
			//	定数の畳み込み最適化は、ここに登録していけばＯＫ！

		case VCPU_ADD:
			p1.SetValue((LONG)p1.GetValue() + (LONG)p2.GetValue());
			break;
		case VCPU_SUB:
			p1.SetValue((LONG)p1.GetValue() - (LONG)p2.GetValue());
			break;
		case VCPU_MUL :
			p1.SetValue((LONG)p1.GetValue() * (LONG)p2.GetValue());
			break;
		case VCPU_DIV :
			if (p2.GetValue()==0) {
				Error("０で除算しています");	// 0割りは検出すべきでしょう...
			} else {
				p1.SetValue((LONG)p1.GetValue() / (LONG)p2.GetValue());
			}
			break;

		case VCPU_MOD:
			p1.SetValue(p1.GetValue() % p2.GetValue());
			break;
		case VCPU_MOD2: {
			LONG lw = p1.GetValue() % p2.GetValue();
			if (lw<0) lw += p2.GetValue();
			p1.SetValue(lw);
			break;
						}
		case VCPU_AND:
			p1.SetValue(p1.GetValue() & p2.GetValue());
			break;
		case VCPU_OR:
			p1.SetValue(p1.GetValue() | p2.GetValue());
			break;
		case VCPU_XOR:
			p1.SetValue(p1.GetValue() ^ p2.GetValue());
			break;
		case VCPU_SHR:
			p1.SetValue((LONG)p1.GetValue() >> (LONG)p2.GetValue());
			break;
		case VCPU_SHL:
			p1.SetValue((LONG)p1.GetValue() << (LONG)p2.GetValue());
			break;

		default:
			Put(inst,p1.GetValue(),p2.GetValue(),"$");
			p1.SetType(3);	// the result is on the stack!!
		}
		return ;
	}

	p1.SetType(3);	// the result is on the stack!!
}

	/////////////////////////////////////////////////////////////////
void	CCodeGenerator::Op2(CCodeItem &p1){
	// 単項をスタックに積む
	if (p1.IsEmpty()) {
		Error("文法エラー");
		return ;
	}
	
	if (p1.IsReg()) {
		if (p1.IsAddress()) {
			Put(VCPU_MOVM,p1.GetLabel(),"$");
			p1.SetType(3);	// On the Stack
			p1.SetAddress(false);
		} else {
			Put(VCPU_MOV,p1.GetLabel(),"$");
			p1.SetType(3);	// On the Stack
		}
	} else if (p1.IsOnStack() && p1.IsAddress()) {
		Put(VCPU_MOVM,"_","$");
		p1.SetAddress(false);
	} else if (p1.IsNum()) {
		// 即値ポインタは、禁止なんだけどなー...
		if (p1.IsAddress()) {
			Put(VCPU_MOVM,p1.GetValue(),"$");
			p1.SetAddress(false);
		} else {
			Put(VCPU_MOV,p1.GetValue(),"$");	// 即値だが、評価のために積む
		}
	}

	p1.SetType(3);	// the result is on the stack!!
}
	/////////////////////////////////////////////////////////////////
void	CCodeGenerator::Op2b(CCodeItem &p1){
	// 単項をスタックに積む（即値だけは積まない）
	if (p1.IsEmpty()) {
		Error("文法エラー");
		return ;
	}
	
	if (p1.IsReg()) {
		if (p1.IsAddress()) {
			Put(VCPU_MOVM,p1.GetLabel(),"$");
			p1.SetType(3);	// On the Stack
			p1.SetAddress(false);
		} else {
			Put(VCPU_MOV,p1.GetLabel(),"$");
			p1.SetType(3);	// On the Stack
		}
	} else if (p1.IsOnStack() && p1.IsAddress()) {
		Put(VCPU_MOVM,"_","$");
		p1.SetAddress(false);
	} else if (p1.IsNum()) {
		// 即値ポインタは、禁止なんだけどなー...
		if (p1.IsAddress()) {
			Put(VCPU_MOVM,p1.GetValue(),"$");
			p1.SetAddress(false);
		} else {
			return ;	// 即値だから積まない
		}
	}

	p1.SetType(3);	// the result is on the stack!!
}
	/////////////////////////////////////////////////////////////////

void	CCodeGenerator::Op3(VCPU inst,CCodeItem &p1){
	// 単項演算子
	
	if (p1.IsEmpty()) {
		Error("単項演算子の後ろの値がおかしい");
		return ;
	}

	//	インクメンタ、デクリメンタは、特殊な扱い...
	if (inst == VCPU_INC || inst==VCPU_DEC) { // ++ or --
		if (!p1.IsReg() && !p1.IsAddress()) {
			Error("不正な++もしくは--");
			return ;
		}
		if (p1.IsReg()) {
			Put(inst,p1.GetLabel());
			return ;
		}
		Put(inst,"_");
		return ;
	}

	if (p1.IsNum() && !p1.IsAddress()) {
		// ものによっては、定数畳み込みが可能である。
		switch(inst){
		case VCPU_CPL :
			p1.SetValue(~p1.GetValue());
			break ;
		case VCPU_NOT :
			p1.SetValue(!p1.GetValue());
			break ;

		default:
			Put(inst,p1.GetValue(),"$");	// これでええんかな...
			p1.SetType(3);	// the result is on the stack!!
		}

		return ;
	}

	Op2(p1);	// スタックに積む
	// 結果は、スタック上にあるはずだから...

	Put(inst,"_","$");	// これでええんかな...
}

//////////////////////////////////////////////////////////////////////////////

void	CCodeGenerator::Op4(VCPU inst,CCodeItem &p1){
	// inst Im1型命令(push命令等)
	
	if (p1.IsEmpty()) {
		Error("演算子の後ろの値がおかしい");
		return ;
	}

	if (p1.IsNum() && !p1.IsAddress()) {
		// 定数即値利用最適化
		Put(inst,p1.GetValue());
	} else {
		Op2(p1);
		// 結果は、スタック上にあるはずだから...
		Put(inst,"_");
	}
	p1.SetEmpty();	// これで、もう放出したとしよう。
}

void	CCodeGenerator::Op5(CCodeItem &x,CCodeItem &y){
	// 代入用オペコード生成
	//	x = y;

	if (x.IsNum() || (x.IsOnStack() && !x.IsAddress())) {
	// 左辺値は、label, *label , *(xx+yy)のいずれかの形でなければならない
		Error("左辺値がおかしいです");
		return ;
	}

	if (y.IsEmpty()) {
		Error("右辺値が空です");
		return ;
	}

	int nType;
	nType = (y.GetType()-1) << 1;
	if (y.IsAddress()) nType++;
	// yの種類に応じて、0～5の番号を振り当て

	// 代入最適化の効果は大きい
	if (x.IsReg() && !x.IsAddress()) {
		// 左辺値がラベル
		switch (nType){
		case 0: Put(VCPU_MOV,y.GetLabel(),x.GetLabel()); break;
		case 1: Put(VCPU_MOVM,y.GetLabel(),x.GetLabel()); break;
		case 2: Put(VCPU_MOV,y.GetValue(),x.GetLabel()); break;
		case 3: Put(VCPU_MOVM,y.GetValue(),x.GetLabel()); break;
		case 4: Put(VCPU_MOV,"_",x.GetLabel()); break;
		case 5: Put(VCPU_MOVM,"_",x.GetLabel()); break;
		}
	} else if (x.IsReg() && x.IsAddress()) {
		// 左辺値がラベルポインタ
		// 確かに、最適化の余地はあるのだが、場合分けが複雑なわりには効果が薄いので...
		switch (nType){
		case 0: Put(VCPU_MOV,x.GetLabel(),"$");
				Put(VCPU_MOV,y.GetLabel(),"_");
				break;
		case 1: Put(VCPU_MOV,x.GetLabel(),"$");
				Put(VCPU_MOVM,y.GetLabel(),"_");
				break;
		case 2: Put(VCPU_MOV,x.GetLabel(),"$");
				Put(VCPU_MOV,y.GetValue(),"_");
				break;
		case 3: Put(VCPU_MOV,x.GetLabel(),"$");
				Put(VCPU_MOVM,y.GetValue(),"_");
				break;
		case 4:	Put(VCPU_MOV,x.GetLabel(),"$");
				Put3(VCPU_MOV);
				break;
		case 5: Put(VCPU_MOV,x.GetLabel(),"$");
				Put3(VCPU_MOVM);
				break;
		}
	} else if (x.IsOnStack() && x.IsAddress()) {
	//	左辺値が計算ポインタ
		switch (nType){
		case 0: Put(VCPU_MOV,y.GetLabel(),"_"); break;
		case 1: Put(VCPU_MOVM,y.GetLabel(),"_"); break;
		case 2: Put(VCPU_MOV,y.GetValue(),"_"); break;
		case 3: Put(VCPU_MOVM,y.GetValue(),"_"); break;
		case 4: Put(VCPU_MOV,"_","_"); break;	// これは正しい
		case 5: Put(VCPU_MOVM,"_","_"); break;	// これは正しい
		}
	}

	//	代入後、左辺式のCCodeItemは、その値をポイントする、メモリレジスタ扱いすれば、
	//	連続的な式の代入や、代入のあとの副作用を持つことが出来るのだが、面倒だし、
	//	プログラムが汚くなる原因なので、ここでは対応しない。

}
	
//////////////////////////////////////////////////////////////////////////////
int		CCodeGenerator::IsPointer(const string& p){	//	labelはポインタ型の変数か？

	//	やっぱ反復子使ったほうがすっきり書ける...
	CLabelList::iterator it;
	for(it = m_local_paralist.begin();it!=m_local_paralist.end();it++){
		if ((*it).label==p){
			return (*it).nPointer;
		}
	}
	for(it = m_local_list.begin();it!=m_local_list.end();it++){
		if ((*it).label==p){
			return (*it).nPointer;
		}
	}
	for(it = m_grobal_list.begin();it!=m_grobal_list.end();it++){
		if ((*it).label==p){
			return (*it).nPointer;
		}
	}

	return 0;	//	謎:p
}
//////////////////////////////////////////////////////////////////////////////

DWORD	CCodeGenerator::PutString(LPSTR p){		// 文字列型データ
	// *(m_lpDS-1)から下位メモリ方向に伸ばして領域を確保する。
	int len = strlen(p)+1;
	if (m_lpDS==NULL) {
		m_dwCode += len;
		return NULL;
	}

	m_lpDS -= len;
	strcpy((LPSTR)m_lpDS,p);	// 簡単やね:p

	if (m_bDebug) {
		char buf[256];
		wsprintf(buf,"(%.8X :\" ",m_lpDS);
		OutFile(buf);
		OutFile((LPSTR)m_lpDS);
		OutFile("\")\n");
	}

	return (DWORD)m_lpDS;
}

DWORD	CCodeGenerator::AppendString(LPSTR p){		// 文字列型データ
	// *(m_lpDS-1)から下位メモリ方向に伸ばして領域を確保する。
	//	（前回PutStringされていると仮定して、前回のやつの領域移動も行なう）
	if (m_lpDS==NULL) {
		int len = strlen(p);
		m_dwCode += len;
		return NULL;
	}

	int len2 = strlen((LPSTR)m_lpDS);	// いま何文字あるねん？
	int len = strlen(p);
	m_lpDS -= len;
	CopyMemory(m_lpDS,m_lpDS+len,len2);	// こいつ賢いんやろな？
	strcpy((LPSTR)m_lpDS+len2,p);		// これでええんちゃうん？

	if (m_bDebug) {
		char buf[256];
		wsprintf(buf,"(%.8X : \"",m_lpDS);
		OutFile(buf);
		OutFile((LPSTR)m_lpDS);
		OutFile("\")\n");
	}

	return (DWORD)m_lpDS;
}

//////////////////////////////////////////////////////////////////////////////

DWORD	CCodeGenerator::IsUserFunc(string name){
	int i;
	for(i=0;i<m_lpUserFuncList->size();i++) {
		if ((*m_lpUserFuncList)[i].m_label==name)
			return (DWORD)((*m_lpUserFuncList)[i].m_func);
	}
	for(i=0;i<m_lpDllUserFunc->size();i++) {
		if ((*m_lpDllUserFunc)[i].m_label==name)
			return (DWORD)((*m_lpDllUserFunc)[i].m_func);
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////
void	CCodeGenerator::SequencePoint(void){
	// 余っている仮想スタック$を削除する
/*
	if (m_nlocal_push!=0) {
		fprintf(m_lpFile,"//  (delete $ × %d)\n",m_nlocal_push);
	}
*/
	for(int i=0;i<m_local_push.size();i++){
		* (m_local_push[i]) = (DWORD) &m_dummy;
		// そのアドレスに、捨てアドレスを入れて潰す
	}
	m_local_push.clear();		// ないないぽん:p
}

void	CCodeGenerator::FunctionExitPoint(void){
	//	未出のローカルラベルは、ここでエラーにする。
	//	ただし、内部的に使用しているラベルは未出である可能性はないので
	//	チェックしない。

	for(int i=0;i<m_local_jump_label_list.size();i++){
		if (!m_local_jump_label_list[i].m_bExist){
			// おかしいやんけー
			Error("ローカルジャンプラベル "+m_local_jump_label_list[i].m_label+" が未解決です");
		}
	}

	// Stringバッファもクリアすべきかもしれないが、まあ、それは良い。
	m_local_jump_label_list.clear();
	m_dwLabelNo = 0; // ローカルラベルは、これで終了のはず
}

void	CCodeGenerator::FileExitPoint(void){
	// 未出のグローバルラベルは、エラーにする。
	for(int i=0;i<m_grobal_jump_label_list.size();i++){
		if (!m_grobal_jump_label_list[i].m_bExist){

			//	もし、これがterminate関数ならば、ディフォルトのものを生成する
			string s;
			s = m_grobal_jump_label_list[i].m_label;
		/*
			if (s == "terminate") {
				//	ダミーのterminate関数登録
				SetGrobalLabel(s);
				Put(VCPU_RETURN);
			} else */ {
				// 無いっちゅーのは、おかしいやんけー
				Error("関数 "+s+" が未解決です（実体が定義されていません）");
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////

int		CCodeGenerator::IsLocalLabel(VCPUPara p1){
//	ローカルラベルであれば、[sp+k]のk(<0)を返す。さもなくば0。
	if (!p1.IsStr()) return 0;
	for(int i=0;i<m_local_paralist.size();i++){
		if (m_local_paralist[i].label==p1.GetStr()){
		//	これやんかー
			return	(int)(m_local_paralist[i].offset
				- m_local_paralist.back().offset
				- m_local_paralist.back().size
				- GetLocalLabelArea()
				- m_SPOffset	// Push中は、この分も考慮
				- 4);	// call stack分、さらに引く必要あり
		}
	}

	for(i=0;i<m_local_list.size();i++){
		if (m_local_list[i].label==p1.GetStr()){
		//	これやんかー
			return	(int)(m_local_list[i].offset
				- m_local_list.back().offset
				- m_local_list.back().size
				- m_SPOffset);
		}
	}

	return 0;
}

DWORD	CCodeGenerator::IsGrobalLabel(VCPUPara p1){
//	グローバルラベルであれば、その配置アドレスを返す。さもなくば0。
	if (!p1.IsStr()) return 0;
	for(int i=0;i<m_grobal_list.size();i++){
		if (m_grobal_list[i].label==p1.GetStr()){
		//	これやんかー
			return	(DWORD)(m_grobal_list[i].offset);
		}
	}
	// 外部変数もここで解決する。
	for(i=0;i < m_lpUserVariList->size();i++){
		if ((*m_lpUserVariList)[i].m_label==p1.GetStr()){
		//	これやんかー
			return	(DWORD)((*m_lpUserVariList)[i].m_vari);
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

LRESULT	CCodeGenerator::vari(bool bGrobal,EToken type,string label,
							 bool bArray,int nArraySize,int nPointer){
	CLabelList*	p;
	if (bGrobal) {
		p	= &m_grobal_list;
		// こいつだとしたら、領域確保が必要である...
	} else {
		if (m_bParameterlist) {
			p	= &m_local_paralist;
		} else {
			p	= &m_local_list;
		}
	}
	
	for(int i=0;i<p->size();i++) {
		if ((*p)[i].label==label) {
			Error("変数はすでに宣言されています。");
			return 1; // 登録されている
		}
	}

	CLabelStruct ls;
	ls.label	= label;
	ls.type		= type;
	ls.bArray	= bArray;			//	配列なのか？
	ls.nPointer = nPointer;		//	ポインタ型なのか？

	// ４バイト固定にしておこう...
	if (bArray) {
		ls.size	= nArraySize * 4;	// 配列サイズ
	} else {
		ls.size	= 4;
	}
	// そこまでのオフセットも入れておくと良いかも:p
	if (!bGrobal) {
		if (p->size()==0) {
			ls.offset = 0;
		} else {
			ls.offset = (p->back().offset) + (p->back().size);
		}
	} else {
		// 領域確保をしなくっちゃ！
		if (m_lpDS!=NULL) {
			m_lpDS -= ls.size;
			ls.offset = (DWORD)m_lpDS;

			//	ラベル名とそのアドレスをデバッグダンプに出力
			if (m_bDebug) {
				char buf[256];
				wsprintf(buf,"(%.8X : %s )\n",m_lpDS,label.c_str());
				OutFile(buf);
			}
		} else {
			ls.offset = ~0;
			//	これが０だとグローバル変数未出と扱われるのでダミーで数字を入れる
			m_dwCode += ls.size;
		}
		if (m_lpCS!=NULL && m_lpDS<(BYTE*)m_lpCS) {
			Error("プログラム格納の為のデータ領域があふれてしまいました");
		}
	}

	p->push_back(ls);
	return 0;
}

int		CCodeGenerator::GetLocalLabelArea(void){
	if (m_local_list.size()==0) return 0;
	
	return m_local_list.back().offset+
			m_local_list.back().size;
}

//////////////////////////////////////////////////////////////////////////////

void	CCodeGenerator::ResetLocalLabel(void){	// Localラベルのクリア
	m_local_list.clear();
	m_local_paralist.clear();
	m_bParameterlist	= true;
}
	
void	CCodeGenerator::ParameterEnd(void){		// 引数リストの終了
	m_bParameterlist	= false;
}

//////////////////////////////////////////////////////////////////////////////
//	ラベル処理
//////////////////////////////////////////////////////////////////////////////

void	CCodeGenerator::SetGrobalLabel(const string &p){		// ラベルをそのアドレスに置く
	// 置くだけ。コードは生成してはいけない。

	// グローバルラベルとは、これすなわち、関数なり！
	if	(m_bDebug) {
		char buf[60];
		wsprintf(buf,"\t<<%s>>:\n",p.c_str());
		OutFile(buf);
	}

	int j=-1;
	for(int i=0;i<m_grobal_jump_label_list.size();i++){
		if (m_grobal_jump_label_list[i].m_label==p) {
			if (m_grobal_jump_label_list[i].m_bExist) {
				Error("このラベルは既に存在しています");
			}
			j = i;
			break;
		}
	}
	if (j==-1) { // まだ新鮮:p
		CJumpLabelStruct ls;
		ls.m_adr	= (DWORD*)m_lpCS;
		ls.m_bExist = true;
		ls.m_label	= p;
		m_grobal_jump_label_list.push_back(ls);
	} else {
		//	このラベルの値を使用しているのであれば、そこからチェインをたどる
		LONG *p,*q;
		p = (LONG*)&(m_grobal_jump_label_list[j].m_adr);
		while(true){
			q	=	(LONG*)*p;
			*p	=	(LONG) m_lpCS;		//	現在のコードアドレスを入れて潰していく。
			p	=	q;
			if (p==NULL) break;
		}
		m_grobal_jump_label_list[j].m_bExist	= true;		// 既出である
	}
}

void	CCodeGenerator::PutGrobalLabelAddress(const string &p){ // ラベルのアドレスをコードに埋める
	// 未出でも可能。
	if	(m_bDebug) {
		char buf[50];
		wsprintf(buf,"\t\t\t<<%s>>\n",p.c_str());
		OutFile(buf);
	}

	int j=-1;
	for(int i=0;i<m_grobal_jump_label_list.size();i++){
		if (m_grobal_jump_label_list[i].m_label==p) {
			j = i;
			break;
		}
	}
	if (j!=-1) {
		if (m_grobal_jump_label_list[j].m_bExist){
		//	既出であれば（アドレスが求まっていれば）そいつを直にうめるだけ
			if (m_lpCS!=NULL) {
				*(m_lpCS++) = (LONG)m_grobal_jump_label_list[j].m_adr;
			} else {
				m_dwCode+=4;
			}
		} else {
		//	出てきてはいるが、残念なことにアドレスは求まっていない。
		//	仕方ないので、登録リスト（チェイン）に追加
			if (m_lpCS!=NULL) {
				*m_lpCS = (LONG)m_grobal_jump_label_list[j].m_adr;
				m_grobal_jump_label_list[j].m_adr = m_lpCS ++;
			} else {
				m_dwCode+=4;
			}
		}
	} else {
		// まったく出てきていないラベル
		if (m_lpCS!=NULL) {
			*m_lpCS = NULL;	// デリミタ
		}
		CJumpLabelStruct js;
		js.m_adr	=	m_lpCS;
		if (m_lpCS!=NULL) {
			m_lpCS++;
		} else {
			m_dwCode+=4;
		}
		js.m_bExist	=	false;
		js.m_label	=	p;
		m_grobal_jump_label_list.push_back(js);
	}
}

	//////////////////////////////////////////////////////////////////////
	// 以下のLocalバージョンは、上のGrobalを置換しただけ:p
	//////////////////////////////////////////////////////////////////////

void	CCodeGenerator::SetLabel(const string &p){		// ラベルをそのアドレスに置く
	// ローカルラベル
	if	(m_bDebug) {
		char buf[60];
		wsprintf(buf,"\t<%s>:\n",p.c_str());
		OutFile(buf);
	}

	int j=-1;
	for(int i=0;i<m_local_jump_label_list.size();i++){
		if (m_local_jump_label_list[i].m_label==p) {
			if (m_local_jump_label_list[i].m_bExist) {
				Error("このラベルは既に存在しています");
			}
			j = i;
			break;
		}
	}
	if (j==-1) { // まだ新鮮:p
		CJumpLabelStruct js;
		js.m_adr	= (DWORD*)m_lpCS;
		js.m_bExist = true;
		js.m_label	= p;
		m_local_jump_label_list.push_back(js);
	} else {
		//	このラベルの値を使用しているのであれば、そこからチェインをたどる
		LONG *p,*q;
		p = (LONG*)&(m_local_jump_label_list[j].m_adr);
		while(true){
			q	=	(LONG*)*p;
			*p	=	(LONG) m_lpCS;		//	現在のコードアドレスを入れて潰していく。
			p	=	q;
			if (p==NULL) break;
		}
		m_local_jump_label_list[j].m_bExist	= true;		// 既出である
	}
}

void	CCodeGenerator::PutLabelAddress(const string& p){ // ラベルのアドレスをコードに埋める
	// 未出でも可能。
	if	(m_bDebug) {
		char buf[50];
		wsprintf(buf,"\t\t\t<%s>\n",p.c_str());
		OutFile(buf);
	}

	int j=-1;
	for(int i=0;i<m_local_jump_label_list.size();i++){
		if (m_local_jump_label_list[i].m_label==p) {
			j = i;
			break;
		}
	}
	if (j!=-1) {
		if (m_local_jump_label_list[j].m_bExist){
		//	既出であれば（アドレスが求まっていれば）そいつを直にうめるだけ
			if (m_lpCS!=NULL) {
				*(m_lpCS++) = (LONG)m_local_jump_label_list[j].m_adr;
			} else {
				m_dwCode+=4;
			}
		} else {
		//	出てきてはいるが、残念なことにアドレスは求まっていない。
		//	仕方ないので、登録リスト（チェイン）に追加
			if (m_lpCS!=NULL) {
				*m_lpCS = (LONG)m_local_jump_label_list[j].m_adr;
				m_local_jump_label_list[j].m_adr = m_lpCS ++;
			} else {
				m_local_jump_label_list[j].m_adr = NULL;
				m_dwCode+=4;
			}
		}
	} else {
		// まったく出てきていないラベル
		if (m_lpCS!=NULL) {
			*m_lpCS = NULL;	// デリミタ
		}
		CJumpLabelStruct js;
		js.m_adr	=	m_lpCS;
		if (m_lpCS!=NULL) {
			m_lpCS++;
		} else {
			m_dwCode+=4;
		}
		js.m_bExist	=	false;
		js.m_label	=	p;
		m_local_jump_label_list.push_back(js);
	}
}

	/////////////////////////////////////////////////////////////////

void	CCodeGenerator::PutLabelAddress(DWORD dw){
	// ラベルのアドレスの埋め込み。未出でも可能。
	// ただし、ローカルラベルであること。
	if	(m_bDebug) {
		char buf[20];
		wsprintf(buf,"\t\t\t<%d>\n",dw);
		OutFile(buf);
	}
	//	ラベルの配置
	//	内部的に使用しているだけなのでチェックは甘め

	if (m_local_inner_label_list[dw].m_bExist) {
	//	既出であれば（アドレスが求まっていれば）そいつを直にうめるだけ
		if (m_lpCS!=NULL) {
			*(m_lpCS++) = (LONG)m_local_inner_label_list[dw].m_adr;
		} else {
			m_dwCode+=4;
		}
	} else {
		//	出てきてはいるが、残念なことにアドレスは求まっていない。
		//	仕方ないので、登録リスト（チェイン）に追加
		if (m_lpCS!=NULL){
			*m_lpCS = (LONG)m_local_inner_label_list[dw].m_adr;
			m_local_inner_label_list[dw].m_adr = m_lpCS ++;
		} else {
			m_local_inner_label_list[dw].m_adr = NULL;
			m_dwCode+=4;
		}
	}
}

void	CCodeGenerator::SetLabel(DWORD dw){		// ラベルをそのアドレスに置く
	if	(m_bDebug && m_lpCS!=NULL) {
		char buf[20];
		wsprintf(buf,"\t<%d>:\n",dw);
		OutFile(buf);
	}
	//	ラベルの配置
	//	内部的に使用しているだけなのでチェックは甘め

	// ２重呼出しはしないだろうからm_bExist==falseのはず...

	//	このラベルの値を使用しているのであれば、そこからチェインをたどる
	LONG *p,*q;
	p = (LONG*)&m_local_inner_label_list[dw].m_adr;
	while(true){
		q	=	(LONG*)*p;
		*p	=	(LONG) m_lpCS;		//	現在のコードアドレスを入れて潰していく。
		p	=	q;
		if (p==NULL) break;
	}

	m_local_inner_label_list[dw].m_bExist	= true;		// 既出である
}

DWORD	CCodeGenerator::GetLabelAddress(void){
	// ラベルを配置はしないが、予約はする。
	CLocalJumpLabelStruct ls;
	ls.m_bExist	= false;
	ls.m_adr	= NULL;
	m_local_inner_label_list.push_back(ls);
	return m_local_inner_label_list.size()-1;	// ラベルの生成
}

//////////////////////////////////////////////////////////////////////////////
//	デバッグ用命令名の出力
//////////////////////////////////////////////////////////////////////////////
void	CCodeGenerator::OutFile(VCPU inst){ //　デバッグ用命令コード出力
	if (m_lpCS==NULL) return ;

	OutFile("\t\t");
	switch(inst){
	case VCPU_HALT		:	fprintf(m_lpFile,"HALT   ");		break;
	case VCPU_RETURN	:	fprintf(m_lpFile,"RETURN ");		break;
	case VCPU_RETURNNUM	:	fprintf(m_lpFile,"RETURNNUM");		break;
	case VCPU_MOVRETNUM	:	fprintf(m_lpFile,"MOVRETNUM");		break;
	case VCPU_CALL		:	fprintf(m_lpFile,"CALL   ");		break;
	case VCPU_CALLUF	:	fprintf(m_lpFile,"CALLUF ");		break;
	case VCPU_JMP		:	fprintf(m_lpFile,"JMP");			break;
	case VCPU_XALT		:	fprintf(m_lpFile,"XALT");			break;
	case VCPU_PUSH		:	fprintf(m_lpFile,"PUSH   ");		break;
	case VCPU_PUSHM		:	fprintf(m_lpFile,"PUSHM  ");		break;
	case VCPU_POPM		:	fprintf(m_lpFile,"POPM   ");		break;
	case VCPU_ADDSP		:	fprintf(m_lpFile,"ADDSP  ");		break;
	case VCPU_SUBSP		:	fprintf(m_lpFile,"SUBSP  ");		break;

	case VCPU_ADD		:	fprintf(m_lpFile,"ADD    ");		break;
	case VCPU_SUB		:	fprintf(m_lpFile,"SUB    ");		break;
	case VCPU_MUL		:	fprintf(m_lpFile,"MUL    ");		break;
	case VCPU_DIV		:	fprintf(m_lpFile,"DIV    ");		break;
	case VCPU_MOD		:	fprintf(m_lpFile,"MOD    ");		break;
	case VCPU_MOD2		:	fprintf(m_lpFile,"MOD2   ");		break;
	case VCPU_AND		:	fprintf(m_lpFile,"AND    ");		break;
	case VCPU_OR		:	fprintf(m_lpFile,"OR     ");		break;
	case VCPU_XOR		:	fprintf(m_lpFile,"XOR    ");		break;
	case VCPU_CPL		:	fprintf(m_lpFile,"CPL    ");		break;
	case VCPU_NOT		:	fprintf(m_lpFile,"NOT    ");		break;
	case VCPU_SHR		:	fprintf(m_lpFile,"SHR    ");		break;
	case VCPU_SHL		:	fprintf(m_lpFile,"SHL    ");		break;
	case VCPU_INC		:	fprintf(m_lpFile,"INC    ");		break;
	case VCPU_DEC		:	fprintf(m_lpFile,"DEC    ");		break;
	case VCPU_INCS		:	fprintf(m_lpFile,"INCS   ");		break;
	case VCPU_DECS		:	fprintf(m_lpFile,"DECS   ");		break;

	case VCPU_IF		:	fprintf(m_lpFile,"IF     ");		break;
	case VCPU_MOV		:	fprintf(m_lpFile,"MOV    ");		break;
	case VCPU_MOVM		:	fprintf(m_lpFile,"MOVM   ");		break;
	case VCPU_MOVMM		:	fprintf(m_lpFile,"MOVMM  ");		break;
	case VCPU_MOVS		:	fprintf(m_lpFile,"MOVS   ");		break;
	case VCPU_MOVSM		:	fprintf(m_lpFile,"MOVSM  ");		break;
	case VCPU_MOVMS		:	fprintf(m_lpFile,"MOVMS  ");		break;
	case VCPU_MOVSS		:	fprintf(m_lpFile,"MOVSS  ");		break;
	case VCPU_MOVMSM	:	fprintf(m_lpFile,"MOVMSM ");		break;
	case VCPU_MOVMMM	:	fprintf(m_lpFile,"MOVMMM ");		break;
	case VCPU_LEAS		:	fprintf(m_lpFile,"LEAS   ");		break;
	case VCPU_CMPBE		:	fprintf(m_lpFile,"CMPBE  ");		break;
	case VCPU_CMPLE		:	fprintf(m_lpFile,"CMPLE  ");		break;
	case VCPU_CMPAB		:	fprintf(m_lpFile,"CMPAB  ");		break;
	case VCPU_CMPME		:	fprintf(m_lpFile,"CMPME  ");		break;
	case VCPU_CMPEQ		:	fprintf(m_lpFile,"CMPEQ  ");		break;
	case VCPU_CMPNE		:	fprintf(m_lpFile,"CMPNE  ");		break;
	default				:	{
								fprintf(m_lpFile,"Unknown");
								Error("スクリプトの内部エラー");
								break;
							}
	} // end switch
}

void	CCodeGenerator::OutFile(VCPUPara para){	//	CParaを出力する。
	if (m_lpCS==NULL) return ;

	if (para.IsNum()) {
		fprintf(m_lpFile,"%.8X",para.GetNum());
	} else {
		fprintf(m_lpFile,"%s",para.GetStr().c_str());
	}
}

void	CCodeGenerator::OutFile(LPSTR str){		//	文字列を出力する。
	if (m_lpCS==NULL) return ;

	fprintf(m_lpFile,str);
}

void	CCodeGenerator::OutFile(void){		//	アドレスを出力する。
	if (m_lpCS==NULL) return ;

	fprintf(m_lpFile,"%.8X",m_lpCS);
}

//////////////////////////////////////////////////////////////////////////////

LRESULT	CCodeGenerator::Import(LPCSTR filename){
	//	フルパス比較(/と\を混合されては困るけど)
	string	file;
	file		= CFile::MakeFullName(filename);

	CImportDllList::iterator it = m_lpDllUserList->begin();
	while (it!=m_lpDllUserList->end()){
		if ((*it)->GetFileName() == file) return 0; // 同名のは既に読み込まれているので何もしない
		it++;
	}

	//	サイズ拡張
	//	m_lpDllUserList->resize(m_lpDllUserList->size() + 1);
	//	これではデストラクタが呼び出されてしまうらしい...
	CImportDll* lp = new CImportDll;

	LRESULT l=lp->Load(file,m_lpDllUserFunc,m_dwScriptSDK);
	if (l!=0) {
		DELETE_SAFE(lp);
		return l;
	}
	*(lp->GetScriptLevel()) = *m_lpnScriptLevel;
	m_lpDllUserList->push_back(lp);
	return 0;
}

void	CCodeGenerator::Unimport(int nScriptLevel){
	CImportDll* lp;
	while (m_lpDllUserList->size()
		&& *((lp=*m_lpDllUserList->rbegin())->GetScriptLevel())>=nScriptLevel){
			DELETE_SAFE(lp);	//	lp->Unload();
			m_lpDllUserList->pop_back();
	}
}

//////////////////////////////////////////////////////////////////////////////
//	インポートライブラリの処理

CImportDll::CImportDll(void){
	m_hInstance		= NULL;
	m_lpDllUserList	= NULL;
	m_nScriptLevel	= -1;
}

/*
CImportDll::CImportDll(const CImportDll& impdll){
	//	なんでいちいちこんな分かりきったコピーコンストラクタ書かんとあかんかなー
	m_filename = impdll.m_filename;
	m_hInstance= impdll.m_hInstance;
	m_nScriptLevel=impdll.m_nScriptLevel;
	m_lpDllUserList=impdll.m_lpDllUserList;
	m_nFunctionSize=impdll.m_nFunctionSize;
}
*/

CImportDll::~CImportDll(){
	Unload();
}

LRESULT	CImportDll::Load(string filename,CUserFunctionList* lpDllUserList,DWORD dwScriptSDK){
	m_filename = "";

	m_hInstance	= ::LoadLibrary(filename.c_str());
	if (m_hInstance==NULL) return 1;

	FARPROC proc = GetProcAddress(m_hInstance,"InquireFunction");
	if (proc==NULL) return 2; // これimportライブラリちゃうやろ！

	struct S {
		LPSTR		lpFuncName;
		LONG		(*lpFunc)(LONG*);
	} *s;
	s = ((S* (*)(void))proc)();

	//	このサイズを覚えておく
	//	（スタック的にしか追加されないので）
	m_nFunctionSize = lpDllUserList->size();
	m_lpDllUserList = lpDllUserList;

	while (s->lpFunc !=NULL) {
		CUserFunction uf;
		uf.m_func	= s->lpFunc;
		uf.m_label	= s->lpFuncName;
		lpDllUserList->push_back(uf);
		s++;
	}

	//	各DLL用の個別の初期化

	proc = GetProcAddress(m_hInstance,"SetScriptSDK");
	if (proc!=NULL) {
		// ScriptSDKは使用らしい
		((void (*)(DWORD*))proc)(&dwScriptSDK);
	}

	proc = GetProcAddress(m_hInstance,"fSetCurrentDirectory");
	if (proc!=NULL) {
		// file関係らしい:p
		string s;
		s = CFile::GetCurrentDir();
		LPCSTR p = s.c_str();
		((void (*)(DWORD*))proc)((DWORD*)&p);
	}

	proc = GetProcAddress(m_hInstance,"fSetFindPath");
	if (proc!=NULL) {
		// dir関係らしい:p
		string s;
		s = CFile::GetCurrentDir();
		LPCSTR p = s.c_str();
		((void (*)(DWORD*))proc)((DWORD*)&p);
	}

	m_filename = filename;	//	成功したときにしかコピーしない
	return 0;
}

LRESULT	CImportDll::Unload(void){
	if (m_hInstance==NULL) return 0;

	if (m_lpDllUserList->size() > m_nFunctionSize) {
		m_lpDllUserList->resize(m_nFunctionSize);
	}

	LRESULT l = ::FreeLibrary(m_hInstance)?0:1;
	m_hInstance = NULL;
	return l;
}

//////////////////////////////////////////////////////////////////////////////
