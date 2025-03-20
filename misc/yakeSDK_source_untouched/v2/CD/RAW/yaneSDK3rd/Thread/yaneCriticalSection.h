//	yaneCriticalSection.h :
//		programmed by yaneurao	'00/03/02

#ifndef __yaneCriticalSection_h__
#define __yaneCriticalSection_h__

class ICriticalSection {
public:
	virtual void	Enter()=0;
	virtual void	Leave()=0;
	virtual ~ICriticalSection(){}
};

class CCriticalSection : public ICriticalSection {
/**
　スレッド間の排他を制御します。
　あるスレッドでEnter関数を呼び出したあと、Leave関数を呼び出すまでの間は、
　クリティカルセクションとなります。
　クリティカルセクションを、同時に２つ以上のスレッドが実行することは出来ません。
　同一スレッドからならば、多重にEnterすることが出来ます。
　（詳しくは、マルチスレッド関係の文献を参照すること）

  class CMutex に比較して、非常に高速なのが特徴です。

  注意：
  　CCriticalSectionの実体は、グローバルスコープに置くか、
  　staticなメンバ変数にするかしてください。
*/
public:
	virtual void	Enter();	///	CriticalSectionに入る
	virtual void	Leave();	///	CriticalSectionから出る

	CCriticalSection();
	virtual ~CCriticalSection();
protected:
	CRITICAL_SECTION	m_csCriticalSection;
};

class CCriticalLock {
/**
	クリティカルセクションの使用補助クラス
		CCriticalSectionでEnterに入ったときに例外が発生すると
		その関数から抜けてしまうので、Leaveされなくて困ることがある。
	そこで、
		{
			CCriticalLock guard(GetCriticalSection());
			//	ここにその処理を書く
		}
	というように使えば、例外が発生したとしても、このクラスの
	デストラクタで、CCriticalSection::Leaveが呼び出されることが
	保証されます。
*/
public:
	CCriticalLock(CCriticalSection* cs) : m_cs(cs) { m_cs->Enter(); }
	~CCriticalLock() { m_cs->Leave(); }
protected:
	CCriticalSection* m_cs;
};

#endif
