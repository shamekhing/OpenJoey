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
	m_hEventObject	= NULL;
}

CThread::~CThread(){
	StopThread();
}

//////////////////////////////////////////////////////////////////////////////

LRESULT CThread::CreateThread() {
	StopThread();
//	return !::CreateThread(NULL,0,ThreadCallBack,this,NULL,&m_dwThreadID);
	m_dwThreadID = ::_beginthread(ThreadCallBack,0,(void*)this);
	/*
					↑ここでコンパイルエラーが出るとしたら、
				リンクするライブラリがシングルスレッドになっているからです。
		プロジェクトの設定⇒「C/C++」⇒「コード生成」で、
		「使用するランタイムライブラリ」は、マルチスレッド用のものを使用してください。
	*/

	if (m_dwThreadID==-1) {
		return 1;
	}
	return 0;
		//	⇒_beginthreadexは、エラー時、-1が返ってくる
	/*
		CreateThreadではなく_beginthreadでスレッドを生成しないと、
		C言語のランタイムを呼び出している場合、小規模なメモリリークが発生する。(80バイト程度)
		あと、コンパイルするときは、マルチスレッド用のライブラリをリンクするように。
	*/
}

LRESULT CThread::StopThread() {
	InvalidateThread();			//	止まって欲しいねん！

/*
	while (m_bThreadExecute)	//	まだかな、まだかな...
		::Sleep(10);
	↑この方法で、スレッドの終了を待つと、スレッドの後処理が終了していないのに
	このフラグが倒れた瞬間、スレッドが終了したものとして、メインスレッドが
	終了してしまい、このスレッドが残骸として残る可能性がある。
*/
	if (m_dwThreadID==-1) return 0;
	//	スレッド動いてへんで？

	//	これ、あまり良くないとは思うけど、スレッドハンドルが重複するほど長時間
	//	（おそらく、数年）電源を入れっぱなしで使用
	//	続けることは無いと思われるので、考えないことにする
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
	InnerCreateEvent();
	ThreadProc();				//	これを実行するのだが
	InnerDeleteEvent();
	return 0;
}

void CThread::ThreadCallBack(LPVOID lpVoid){
	CThread* pThread = reinterpret_cast<CThread*>(lpVoid);
	pThread->InnerCreateEvent();
	pThread->ThreadProc();
	pThread->InnerDeleteEvent();	// 同期オブジェクトを解体しとこっと＾＾；
}

void	CThread::InvalidateThread(){
	m_bThreadValid = false;
	//	同期オブジェクトもシグナル状態にしてやる

	::SetEvent(m_hEventObject);
}

///////////////////////////////////////////////////////////////////////////
//	同期オブジェクトの投入 '01/11/19


void	CThread::InnerCreateEvent(){
	InnerDeleteEvent();	//	前のやつを解体せねば！
	m_hEventObject = ::CreateEvent(
		NULL,	//	security descriptor
		FALSE,	//	auto reset
		FALSE,	//	not signal state
		NULL	//	event object name
	);
	//	NULLならば生成エラーなのだが．．
	m_bThreadExecute = true;
	m_bThreadValid	 = true;
}

void	CThread::InnerDeleteEvent(){
	if (m_hEventObject!=NULL){
		::CloseHandle(m_hEventObject);
		m_hEventObject = NULL;
	}
	m_bThreadExecute = false;
}

LRESULT CThread::ThreadSleep(int nTime){
//	nTime [ms]待つ。待っている最中に、同期オブジェクトがシグナル状態
//	（スレッドを終了させなさい、という状態）になれば、非0が返る。
	if (m_hEventObject == NULL) return -1;

	DWORD dwResult = ::WaitForSingleObject(m_hEventObject,nTime);
	if (dwResult == WAIT_TIMEOUT) {
		//	オッケー！　まだまだいける（笑）
		return 0;
	}

	return 1;
}
