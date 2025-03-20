//	yaneEvent.h :
//		programmed by yaneurao	'02/03/05

#ifndef __yaneEvent_h__
#define __yaneEvent_h__

class IEvent {
public:
	virtual	void	SetEvent()=0;
	virtual void	ResetEvent()=0;
	virtual LRESULT	Wait(int nTime=0)=0;
	virtual ~IEvent(){}
};

class CEvent : public IEvent {
/**
	WIN32のイベントのwrapper

	何かのイベントが発生したことをあるスレッドから
	別のスレッドに通知できるようにする同期オブジェクトです。

	イベントは、スレッドがいつその作業を行うかを
	知る必要があるときに有用です。

*/
public:
	virtual	void	SetEvent();
	///	オブジェクトをシグナル状態（利用できる状態）にする

	virtual void	ResetEvent();
	///	オブジェクトをノンシグナル状態（利用できない状態）にする

	virtual LRESULT	Wait(int nTime=0);
	/**
		オブジェクトがシグナル状態（利用できる状態）になるまで待つ
		nTime時間だけ待つ　　nTime==0 ならば無限に待つ

		返し値	0 : オブジェクトがシグナル状態になった
				1 : 待ち時間タイムアウトになった
				2 : それ以外（ありえないはず）
	*/

	CEvent();
	virtual ~CEvent();
protected:
	HANDLE	m_hHandle;
};

#endif
