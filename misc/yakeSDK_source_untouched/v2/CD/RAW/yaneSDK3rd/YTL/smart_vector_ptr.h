/*
	smart_vector_ptr	: a vector of smart_ptr
		programmed & desinged by yaneurao(M.Isozaki) '02/03/01
*/

#ifndef __YTLSmartVectorPtr_h__
#define __YTLSmartVectorPtr_h__

// The debugger can't handle symbols more than 255 characters long.
// STL often creates symbols longer than that.
// When symbols are longer than 255 characters, the warning is disabled.

#pragma warning(disable:4786)
#include <vector>
using namespace std ;

#include "smart_ptr.h"

template<class T> class smart_vector_ptr : public vector<smart_ptr<T> > {
public:
	/// newしたポインタの所有権を移し、追加登録する構文
	template <class S>
	void	insert(S* ptr){
		//	所有権を持ったsmart_ptrを生成し、
		//	smart_ptr間のコピーで、所有権が移動する
		smart_ptr<T> t(ptr);
		vector<smart_ptr<T> >::push_back(t);
	}
	template <class S>
	void	insert(const smart_ptr<S>& s){
		smart_ptr<T> t;
		t.UpCast(s);	//	アップキャスト構文
		vector<smart_ptr<T> >::push_back(t);
	}
	void	insert(){
		///	生成して追加する。
		smart_ptr<T> t(new T);
		vector<smart_ptr<T> >::push_back(t);
	}

	/**
		このチェインのすべてのメンバの関数を呼び出す
		例)
			GetList()->for_each(ISound::Play,5);
			// これならば、全てのインスタンスに対して
			Play(5)を呼び出したことになる
	*/
	template <class R>
	void	for_each(R (T::*fn)()){
		iterator it = begin();
		while (it!=end()){ ((**it).*fn)(); it++; }
	}
	template <class R,class Arg1>
	void	for_each(R (T::*fn)(Arg1),const Arg1& a1){
		iterator it = begin();
		while (it!=end()){ ((**it).*fn)(a1); it++; }
	}
	template <class R,class Arg1,class Arg2>
	void	for_each(R (T::*fn)(Arg1,Arg2),const Arg1& a1,const Arg2& a2){
		iterator it = begin();
		while (it!=end()){ ((**it).*fn)(a1,a2); it++; }
	}
	template <class R,class Arg1,class Arg2,class Arg3>
	void	for_each(R (T::*fn)(Arg1,Arg2,Arg3),const Arg1& a1
		,const Arg2& a2,const Arg3& a3){
		iterator it = begin();
		while (it!=end()){ ((**it).*fn)(a1,a2,a3); it++; }
	}
	template <class R,class Arg1,class Arg2,class Arg3,class Arg4>
	void	for_each(R (T::*fn)(Arg1,Arg2,Arg3,Arg4),const Arg1& a1
		,const Arg2& a2,const Arg3& a3,const Arg4& a4){
		iterator it = begin();
		while (it!=end()){ ((**it).*fn)(a1,a2,a3,a4); it++; }
	}
	template <class R,class Arg1,class Arg2,class Arg3,class Arg4,
		class Arg5>
	void	for_each(R (T::*fn)(Arg1,Arg2,Arg3,Arg4,Arg5),const Arg1& a1,
		const Arg2& a2,const Arg3& a3,const Arg4& a4,const Arg5& a5){
		iterator it = begin();
		while (it!=end()){ ((**it).*fn)(a1,a2,a3,a4,a5); it++; }
	}
	template <class R,class Arg1,class Arg2,class Arg3,class Arg4,
		class Arg5,class Arg6>
	void	for_each(R (T::*fn)(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6),const Arg1& a1,
		const Arg2& a2,const Arg3& a3,const Arg4& a4,class Arg5& a5,
		const Arg6& a6){
		iterator it = begin();
		while (it!=end()){ ((**it).*fn)(a1,a2,a3,a4,a5,a6); it++; }
	}

	/**
		bool isValid() constというメンバを持つクラスに対して、
		これがtrueが返ってくるインスタンスに対してのみ、
		指定された関数を呼び出し、
		非Validな奴は消していくイテレータ抽象

		ただし、vectorのeraseなのでやや遅い．．
		このメソッドが必要ならば　class smart_list_ptr を使うべし
	*/
	template <class R>
	void	for_each_valid(R (T::*fn)()){
		iterator it = begin();
		while (it!=end()){
			if ((*it)->isValid()){
				((**it).*fn)(); it++;
			} else {
				it = erase(it);
			}
		}
	}
	template <class R,class Arg1>
	void	for_each_valid(R (T::*fn)(Arg1),const Arg1& a1){
		iterator it = begin();
		while (it!=end()){
			if ((*it)->isValid()){
				((**it).*fn)(a1); it++;
			} else {
				it = erase(it);
			}
		}
	}
	template <class R,class Arg1,class Arg2>
	void	for_each_valid(R (T::*fn)(Arg1,Arg2),const Arg1& a1,const Arg2& a2){
		iterator it = begin();
		while (it!=end()){
			if ((*it)->isValid()){
				((**it).*fn)(a1,a2); it++;
			} else {
				it = erase(it);
			}
		}
	}
	template <class R,class Arg1,class Arg2,class Arg3>
	void	for_each_valid(R (T::*fn)(Arg1,Arg2,Arg3),const Arg1& a1
		,const Arg2& a2,const Arg3& a3){
		iterator it = begin();
		while (it!=end()){
			if ((*it)->isValid()){
				((**it).*fn)(a1,a2,a3); it++;
			} else {
				it = erase(it);
			}
		}
	}
	template <class R,class Arg1,class Arg2,class Arg3,class Arg4>
	void	for_each_valid(R (T::*fn)(Arg1,Arg2,Arg3,Arg4),const Arg1& a1
		,const Arg2& a2,const Arg3& a3,const Arg4& a4){
		iterator it = begin();
		while (it!=end()){
			if ((*it)->isValid()){
				((**it).*fn)(a1,a2,a3,a4); it++;
			} else {
				it = erase(it);
			}
		}
	}
	template <class R,class Arg1,class Arg2,class Arg3,class Arg4,
		class Arg5>
	void	for_each_valid(R (T::*fn)(Arg1,Arg2,Arg3,Arg4,Arg5),
		const Arg1& a1,const Arg2& a2,const Arg3& a3,
		const Arg4& a4,const Arg5& a5){
		iterator it = begin();
		while (it!=end()){
			if ((*it)->isValid()){
				((**it).*fn)(a1,a2,a3,a4,a5); it++;
			} else {
				it = erase(it);
			}
		}
	}
	template <class R,class Arg1,class Arg2,class Arg3,class Arg4,
		class Arg5,class Arg6>
	void	for_each_valid(R (T::*fn)(Arg1,Arg2,Arg3,Arg4,Arg5,Arg6),
		const Arg1& a1,const Arg2& a2,const Arg3& a3,
		const Arg4& a4,class Arg5& a5,const Arg6& a6){
		iterator it = begin();
		while (it!=end()){
			if ((*it)->isValid()){
				((**it).*fn)(a1,a2,a3,a4,a5,a6); it++;
			} else {
				it = erase(it);
			}
		}
	}

};

#endif
