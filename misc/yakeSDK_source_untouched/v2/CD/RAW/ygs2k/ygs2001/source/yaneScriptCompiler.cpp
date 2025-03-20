#include "stdafx.h"

//	yaneScriptCompiler.cpp :

#include "yaneScriptCompiler.h"
#include "../../yaneSDK/YTL/yaneMacro.h"
#include "../../yaneSDK/yaneMsgDlg.h"

EScriptMessage	CScriptCompiler::m_Message;			//	メッセージがあるか？
bool	CScriptCompiler::m_bErrorRestrain;			//	エラー抑制オプション

//////////////////////////////////////////////////////////////////////////////
CScriptCompiler::CScriptCompiler(void){

	SetStackSize(16*1024);
	// スタックのディフォルトは16KB

	SetInstructionAreaSize(0);
	//　モジュール当たりの命令実行エリアのサイズは自由拡張
	//	どうもいろいろいじったとき固定サイズはバグを残してしまった＾＾

	m_UserFunc.clear();
	m_UserVari.clear();
	
	m_bDebug = false;
	m_Message	= ESM_Nothing;
	m_nScriptCallLevel = 0;
	m_bErrorResult = 0; // スクリプト側のエラーハンドラのため
	m_bErrorRestrain = false;

	//	他スクリプトの呼出しのための関数を提供
	RegistUserFunction("CallScript",CallScript);
	RegistUserFunction("JumpScript",JumpScript);
	RegistUserFunction("Quit",Quit);
	RegistUserFunction("SetErrorRestrain",SetErrorRestrain);
	RegistUserVariable("bErrorResult",m_bErrorResult);
}

CScriptCompiler::~CScriptCompiler(){
	for(int i=0;i<m_aCodeArea.size();i++){
		DELETE_SAFE(m_aCodeArea[i]);	// 確保していた領域の解放
	}
}

LRESULT	CScriptCompiler::SetStackSize(DWORD dw){
	m_VCPU.SetStackSize(m_nStackSize=dw);
	return 0;
}

LRESULT	CScriptCompiler::SetInstructionAreaSize(DWORD dw){
	m_nInstructionAreaSize = dw;
	return 0;
}

LRESULT	CScriptCompiler::Execute(void*exe_adr){
	return m_VCPU.ExecuteFrom(exe_adr);
}
//////////////////////////////////////////////////////////////////////////////

LRESULT	CScriptCompiler::ReExecute(void){
	//	他スクリプトのCallとJumpもここで管理する必要がある
	//	よって、擬似的なメッセージループを持つ (C) yaneurao

	LRESULT hr;
	for(;;) {
		hr = m_VCPU.ReExecute();	//	

		switch (m_Message){
		case ESM_Nothing: {
			if (hr==0 && m_nScriptCallLevel) {
				//	スクリプト呼出しからのリターン
				CParser parser;	// parserはローカルで持っていたほうが安全でしょ？
				parser.SetDllFunction(&m_DllUserFunc,&m_DllUserList,&m_nScriptCallLevel,m_dwScriptSDK);
				parser.Unimport(m_nScriptCallLevel);

				m_nScriptCallLevel--;
				Unload();				//	現在のスクリプトをUnLoad
				m_VCPU.PopPC();		//	これで復帰！
				break;
			} else {
				return hr;					//	もうスクリプトは終わりらしい:p
			}
						  }
		case ESM_JumpScript: {
			if (hr!=0) {
				CMsgDlg msg;
				msg.Out("スクリプトエラー","JumpScript実行後は、haltでなくreturnしてください");
				return 0;	//	正常終了した顔して戻ろっと:p
			}
			//	いまあるやつ解放するんじゃ
			Unload();	//	これで最後のんがUnloadされる
			if (Load(m_loadfile)!=0){
				CMsgDlg msg;
				msg.Out("スクリプト","JumpScriptでスクリプトのロードに失敗しました。");
				return 0;	//	正常終了した顔して戻ろっと:p
			}
			break;
							 }
		case ESM_CallScript: {
			if (hr==0) {
				CMsgDlg msg;
				msg.Out("スクリプトエラー","CallScript実行後は、returnでなくhaltしてください");
				return 0;	//	正常終了した顔して戻ろっと:p
			}
			//	いまあるやつは解放せんのじゃ
			m_VCPU.PushPC();		//	戻り先を積む

			//	Loadする瞬間にScriptのCallLevelを見てUnimportするので...
			m_nScriptCallLevel++;	//	スクリプトのコールレベルを設定
			if ((m_bErrorResult=Load(m_loadfile))!=0){
				m_nScriptCallLevel--;
				// Error end...
				if (!m_bErrorRestrain) {
					CMsgDlg msg;
					msg.Out("スクリプトエラー","CallScriptでスクリプトのロードに失敗しました。");
					return 0;	//	正常終了した顔して戻ろっと:p
				} else {
					m_VCPU.PopPC();		//	戻り先を降ろす
					break;
				}
			}
			break;
							 }
		case ESM_Quit : {
			CParser parser;	// parserはローカルで持っていたほうが安全でしょ？
			parser.SetDllFunction(&m_DllUserFunc,&m_DllUserList,&m_nScriptCallLevel,m_dwScriptSDK);
			parser.Unimport(0);			//	すべてのimport lib.のunimport
			return 0;	//	強制終了にゃりん:p
						}
		// default : //	UNDEFINED MESSAGE
		} // switch
		m_Message = ESM_Nothing;	//	メッセージを処理したよん
	}

	return hr;		//	上のは永久ループだからいらんねんけどブロックコードってやつよ
}

//////////////////////////////////////////////////////////////////////////////

LRESULT	CScriptCompiler::PushReturnAddress(void){
	return m_VCPU.PushReturnAddress();
}

//////////////////////////////////////////////////////////////////////////////
void	CScriptCompiler::SetDebugMode(bool b){
	// デバッグ用に中間ファイルを出力する
	m_bDebug = b;
}

//////////////////////////////////////////////////////////////////////////////
LRESULT	CScriptCompiler::RegistUserFunction(const string& funcname,LONG (*func)(LONG *)){
	//	登録
	CUserFunction user;
	user.m_label = funcname;
	user.m_func	 = func;
	m_UserFunc.push_back(user);
	return 0;
}

LRESULT	CScriptCompiler::RegistUserVariable(const string& variname,LONG &l){
	//	重複定義に対する処理が必要
	for(int i=0;i<m_UserVari.size();i++){
		if (m_UserVari[i].m_label == variname) {
			m_UserVari[i].m_vari = &l;
			return 0;
		}
	}

	//	登録
	CUserVariable user;
	user.m_label = variname;
	user.m_vari	 = &l;
	m_UserVari.push_back(user);
	return 0;
}
//////////////////////////////////////////////////////////////////////////////

LRESULT	CScriptCompiler::Load(const string& filename,bool bOutError){
	m_loadfile = filename;	//	保存:p

	DWORD size=m_nInstructionAreaSize;
	if (size==0) {	//	sizeの確定をさせるためのダミーコンパイル
		CParser parser;	// parserはローカルで持っていたほうが安全でしょ？
		parser.SetUserFunctionList(&m_UserFunc);
		parser.SetUserVariableList(&m_UserVari);
		parser.SetInstructionAreaSize(NULL,0);
		parser.SetDllFunction(&m_DllUserFunc,&m_DllUserList,&m_nScriptCallLevel,m_dwScriptSDK);
//		parser.Unimport(m_nScriptCallLevel);
		LRESULT hr;
		hr = parser.Compile(filename);
		if (hr<0) {
			if (bOutError&& !m_bErrorRestrain) {
				CMsgDlg msg;
				msg.Out("スクリプト読み込みエラー","スクリプトファイル"+filename+"がありませんでした。");
			}
			return hr;
		} else if (hr>0) {
			if (!m_bErrorRestrain) {
				CMsgDlg msg;
				msg.Out("スクリプト","コンパイルエラーが発生しました。(エラー内容はCompileError.txtを参照のこと)");
			}
			return hr;
		}
		//	正常終了。よって、必要コードエリアsize確定。
		size = parser.GetInstructionAreaSize();	//	必要コードエリア
	}

	//	前のゴミが残っていると嫌なので、再確保
	CParser parser;
	if (m_bDebug) parser.DebugFile("debug_asm.txt");	// デバッグ出力ね！
	parser.SetUserFunctionList(&m_UserFunc);
	parser.SetUserVariableList(&m_UserVari);
	parser.SetDllFunction(&m_DllUserFunc,&m_DllUserList,&m_nScriptCallLevel,m_dwScriptSDK);

	//	もしparseするのが一回目ならば、Unimportしておく。
//	if (m_nInstructionAreaSize!=0) parser.Unimport(m_nScriptCallLevel);

	CCodeArea *code = new CCodeArea;					// エリアを確保して知らせる
	parser.SetInstructionAreaSize(code->AllocCodeArea(size),size);
	code->SetFileName(filename);
	m_aCodeArea.push_back(code); // ポインタを追加

	LRESULT hr;
	hr = parser.Compile(filename);

	//	２回目でエラーになることってあんのか？
	if (hr<0) {	
		if (bOutError&& !m_bErrorRestrain) {
			CMsgDlg msg;
			msg.Out("スクリプト読み込みエラー","スクリプトファイル"+filename+"がありませんでした。");
		}
	} else if (hr>0) {
		if (!m_bErrorRestrain) {
			CMsgDlg msg;
			msg.Out("スクリプト","コンパイルエラーが発生しました。(エラー内容はCompileError.txtを参照のこと)");
		}
	}
	if (hr) {
		DELETE_SAFE(code);
		m_aCodeArea.pop_back();	// 削除しとこっと:p
		return hr;
	}

	m_exe_adr = (LONG*)code->GetCodeArea();
	m_VCPU.ResetPC(m_exe_adr);	//	自動的に先頭アドレスを指すように！
	m_VCPU.PushReturnAddress();	//	さらに帰り先をPushする
	if (CVirtualCPU::m_bNative) {
		code->NativeCodeSetup(parser.GetInstructionCodeSize(),m_VCPU);
	}
	return 0;
}

LRESULT	CScriptCompiler::Unload(const string& filename){
	// 指定のファイル名のものをUnloadする（危険極まりないが...）
	for(int i=0;i<m_aCodeArea.size();i++){
		if (m_aCodeArea[i]->IsFile(filename)) {
			DELETE_SAFE(m_aCodeArea[i]);
		}
	}
	return 0;
}

LRESULT	CScriptCompiler::Unload(void){	//	最後のものをUnLoadする
	//	最後に読み込んだものをUnLoadするのは、安全
	DELETE_SAFE(m_aCodeArea.back());
	m_aCodeArea.pop_back();
	return 0;
}

LONG*	CScriptCompiler::GetEntryPoint(void){
	return m_exe_adr;
}

/////////////////////////////////////////////////////////////////////
//	他スクリプトの呼出しのための関数をユーザーに提供する

int	CScriptCompiler::m_nScriptCallLevel;
string CScriptCompiler::m_loadfile;			//	こいつにスクリプトの名前入れれ:p
TUserFunc(CScriptCompiler::JumpScript)			{	m_Message=ESM_JumpScript; m_loadfile = (LPSTR)*p; return 0; }
TUserFunc(CScriptCompiler::CallScript)			{	m_Message=ESM_CallScript; m_loadfile = (LPSTR)*p; return 0; }
TUserFunc(CScriptCompiler::Quit)				{	m_Message=ESM_Quit; return 0; }
TUserFunc(CScriptCompiler::SetErrorRestrain)	{	m_bErrorRestrain = *p!=0; return 0; }

/////////////////////////////////////////////////////////////////////
