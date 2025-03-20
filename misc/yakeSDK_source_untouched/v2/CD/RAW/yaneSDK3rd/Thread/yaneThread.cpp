#include "stdafx.h"
#include "yaneThread.h"
#include <process.h> 

//////////////////////////////////////////////////////////////////////////////

CThread::CThread(){
	m_dwThreadID	 = -1;
	//	-1はエラー値。(スレッドハンドルとしてはありえない値)
	//	（_beginthreadexは理論上は０が返ってくることがある）

	m_bThreadExecute = false;
	m_bThreadValid	 = false;
}

CThread::~CThread(){
	StopThread();
}

//////////////////////////////////////////////////////////////////////////////

LRESULT CThread::CreateThread(const smart_ptr<function_callback>& fn) {
	m_fn = fn;
	return CreateThread();
}

LRESULT CThread::CreateThread() {
	StopThread();
	m_bThreadValid	  = true;
//	return !::CreateThread(NULL,0,ThreadCallBack,this,NULL,&m_dwThreadID);
	m_dwThreadID = ::_beginthread(ThreadCallBack,0,(void*)this);
	/*
					↑ここでコンパイルエラーが出るとしたら、
		リンクするライブラリがシングルスレッドになっているからです。
		プロジェクトの設定⇒「C/C++」⇒「コード生成」で、
		「使用するランタイムライブラリ」は、
		マルチスレッド用のものを使用してください。
	*/

	if (m_dwThreadID==-1) {
		return 1;
	}
	return 0;
		//	⇒_beginthreadexは、エラー時、-1が返ってくる
	/*
		CreateThreadではなく_beginthreadでスレッドを生成しないと、
		C言語のランタイムを呼び出している場合、小規模なメモリリークが発生する。
		(80バイト程度)
		あと、コンパイルするときは、マルチスレッド用のライブラリを
		リンクするようにしてください。
	*/
}

LRESULT CThread::StopThread() {
	InvalidateThread();			//	止まって欲しいねん！

	if (m_dwThreadID==-1) return 0;
	//	スレッドもとから動いてへんで？

	//	強制的に、このスレッドを削除するコードは危険なので投入しない
	DWORD dwResult = ::WaitForSingleObject((HANDLE)m_dwThreadID,INFINITE);
	m_dwThreadID = -1;
	if (dwResult != WAIT_OBJECT_0) {
		//	きっと、このスレッドハンドルの所有権がすでに
		//	破棄されていたのだと思う..
		return 1;
	}
	//	終了したので．．
	return 0;
}

LRESULT CThread::JumpToThread() {
	GetEvent()->ResetEvent();	//	ノンシグナル状態にする
	m_bThreadValid	  = true;
	ThreadProc();				//	これを実行するのだが
	m_bThreadValid	  = false;
	return 0;
}

void CThread::ThreadCallBack(LPVOID lpVoid){
	CThread* pThread = reinterpret_cast<CThread*>(lpVoid);
	pThread->m_bThreadExecute = true;
	//	このフラグは読み取り専用なのでここで書き換えて良い
	if (pThread->GetCallBack().isNull()){
		pThread->ThreadProc();
	} else {
		pThread->GetCallBack()->run();	//	コールバックする
		pThread->m_fn.Delete();			//	function_callbackを解体する
	}
	pThread->m_bThreadExecute = false;
	//	このフラグは読み取り専用なのでここで書き換えて良い
}

void	CThread::InvalidateThread(){
	m_bThreadValid = false;
	//	同期オブジェクトもシグナル状態にしてやる
	GetEvent()->SetEvent();
}

LRESULT CThread::ThreadSleep(int nTime){
	//	nTime [ms]待つ。待っている最中に、同期オブジェクトがシグナル状態
	//	（スレッドを終了させなさい、という状態）になれば、非0が返る。
	return !GetEvent()->Wait(nTime);
}
