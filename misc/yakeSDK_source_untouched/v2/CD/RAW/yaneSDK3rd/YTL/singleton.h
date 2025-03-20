/*
	singleton	: singleton template
		programmed & desinged by yaneurao(M.Isozaki) '02/03/13
*/

#ifndef __YTLSingleton_h__
#define __YTLSingleton_h__

#ifdef USE_MEMORY_STATE
	#include "../Auxiliary/yaneMemory.h"
#endif

template <class T>
class singleton {
/**
	このポインタは、
	必要に迫られたときに始めてobjectをnewするようなポインタ

	非ローカルなstaticオブジェクトとして、

	class CHoge{
	public:
		static singleton<CHoge> m_obj;
		static CHoge* GetObj() { return m_obj->get(); }
	};
	のように使って大丈夫！オブジェクトの解体についても保証される

	というか、このクラスは、ひとつのクラスにつき、一つしか存在できない。
	おまけに、上記のようにstaticとしてしか使えない(;´Д`)
*/
public:
	singleton() {
	//	初期化は何も行なわない
	//	というか非ローカルなstaticオブジェクトの
	//	初期化の問題があるので、このオブジェクトの初期かについては
	//	何も行なってはいけない
	}

	~singleton() { Release(); }

	///	ポインタのふりをするための仕掛け
	T& operator*() { return *get(); }
	T* operator->() { return get();	}
	T* get() { CheckInstance(); return m_lpObj; }

	///	オブジェクトがNullかどうかを検証する
	bool	isNull() const { return m_lpObj == NULL; }

	///	オブジェクトを強制的に解体する
	///	（デストラクタでも解放している）
	void Release() {
		if (m_lpObj!=NULL) {
			delete m_lpObj;
			m_lpObj = NULL;
		}
	}

	void CheckInstance(){
		if (m_lpObj==NULL) {
			m_lpObj = new T;
#ifdef USE_MEMORY_STATE
		if (CMemoryState::IsActive()){
		/**	Log記録中(BeginSnap～EndSnapの最中)
			このときに生成されたオブジェクトは、EndSnapのあと、
			operator new/deleteのオーバーロードが解除されるまでに
			オブジェクトをコールバックによって解体してやる必要がある
		*/
			//	callbackオブジェクトを用意
			smart_ptr<function_callback> fn(
				function_callback_v::Create((void(singleton<T>::*)())Release,this)
			);
			CMemoryState::RegistCallBack(fn);
			//	callbackを依頼する
		}
#endif
		}
	}
	/**
		インスタンスが存在するのかそのチェックを行ない、
		存在しなければ生成する。本来、明示的に行なう必要は無いが、
		このオブジェクト自体が非常に初期化の時間のかかり、
		リアルタイムにその初期化を行なわれては困るときには、
		（必要なのがわかっていれば）事前に生成を行なっておく
		ことで、それを回避できる。
	*/
protected:
	static T*	m_lpObj;
	//	staticで用意することで、
	//	リンク時(非ローカルなstaticオブジェクトの初期化前)にNULL
	//	であることを保証する

};

//	static なオブジェクト
template <class T> T* singleton<T>::m_lpObj = 0;

#endif
