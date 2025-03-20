//	yaneThread.h :
//		programmed by yaneurao	'00/02/26
//		同期オブジェクトの導入 '01/11/19

#ifndef __yaneThread_h__
#define __yaneThread_h__

#include "yaneEvent.h"

class IThread {
public:
	virtual LRESULT CreateThread()=0;
	virtual LRESULT CreateThread(const smart_ptr<function_callback>& fn)=0;
	virtual LRESULT StopThread()=0;
	virtual LRESULT JumpToThread()=0;
	virtual bool IsThreadExecute()const=0;
	virtual bool IsThreadValid()const=0;
	virtual void InvalidateThread()=0;
	virtual	LRESULT ThreadSleep(int nTime)=0;

	virtual ~IThread(){}
};

class CThread : public IThread {
/**
	スレッドを生成するときの補助として使います。
	同期オブジェクトを用いているので、
	結構レスポンスが良い．．かも知れません（笑）
*/
public:
	virtual void ThreadProc() {}
	/**
		ワーカースレッドの処理を記述するために、
		この関数をオーバーライドします。
		（このクラスを派生させて使うとき）
	*/
	virtual LRESULT CreateThread();
	virtual LRESULT CreateThread(const smart_ptr<function_callback>& fn);
	/**
		ワーカースレッドが発生し、ThreadProcを実行しはじめます。

		引数として、コールバックする関数を指定しない場合は、
		ThreadProcが呼び出されるのでThreadProcをオーバーライドしておくこと。
	*/

	virtual LRESULT StopThread();
	/**
		スレッドのストップ。停止するまで待ちます
		返し値：
			０：正常終了
			１：すでにスレッドが存在していなかった（正常終了）
	*/

	virtual LRESULT JumpToThread();	///	スレッドを作らずにジャンプ

	virtual bool IsThreadExecute()const /// スレッドは実行中か？
	{ return m_bThreadExecute; }

	virtual bool IsThreadValid()const { return m_bThreadValid;	 }
	/**
		スレッドのStopはかかっていないか
		この関数がfalseになったときに、ThreadProcから脱出するように
		コーディングします。
	*/

	virtual void InvalidateThread();
	/**
		スレッドに停止信号を送る
		（その停止を待つわけではない）
		ワーカースレッドは次にIsThreadValidをチェックしたときに
		falseが返ってきますので、そのときにThreadProcから抜け出るでしょう。
		あるいはThreadSleepしたときに非０が返るので、そこでスレッドを
		抜けるコードを書いても良いです。
	*/

	virtual	LRESULT ThreadSleep(int nTime);
	/**
		nTime [ms]待つ。
	返し値：
		0：正常終了　
		1 : 待っている最中に、別スレッドからStopThreadが呼び出されれば
		同期オブジェクトがシグナル状態（スレッドを終了させなさい、
		という状態）になり、ただちに関数を抜ける。その場合、非0が返る。

	通例、ワーカースレッドは、
　　while ( IsThread( ) ) {
　　　	処理 ;
　　　	Sleep( 20 );
　　} 
	というようなコーディングによって、何かの処理を行なう処理を書きますが、
	このときにSleep(20)があるため、スレッドがInvalidateされて（終了の合図）
	から、この分だけレスポンスが低下します。そこで、ここをSleepではなく、
	if (ThreadSleep( 20 )) break; などとすれば、この分のレスポンスを
	向上させることが出来ます。ThreadSleepの実装は、同期オブジェクトを待つ
	(::WaitForSingleObject)ようになっているので、この部分においてCPUパワーを、
	あまり使用せず、効率的にSleepすることが出来ます。
	*/

	CThread();
	virtual ~CThread();

protected:
	volatile bool m_bThreadExecute;
	//	Threadは実行中なのか？(他スレッドからは読み取り専用)
	volatile bool m_bThreadValid;
	//	Threadを停止させたいときはfalseにする

	volatile DWORD	m_dwThreadID;		//	スレッドハンドル

	static void ThreadCallBack(LPVOID lpVoid);

	CEvent	m_vEvent;
	//	これは、スレッドが生きている間は、ノンシグナル状態
	//	シグナル状態になったときは、スレッドを終了させなさいという合図
	CEvent* GetEvent() { return &m_vEvent; }

	smart_ptr<function_callback> m_fn;
	smart_ptr<function_callback> GetCallBack() const { return m_fn; }
};

#endif
