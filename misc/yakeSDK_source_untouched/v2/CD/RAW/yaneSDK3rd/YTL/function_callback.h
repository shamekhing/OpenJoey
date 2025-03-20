/*
	function_callback	: versatile function callbacker
		reprogrammed & desinged by yaneurao(M.Isozaki) '02/03/08

	参考記事：
		『InsideWindows』1999年8月号
		επιστημη氏「任意の関数をスレッドに」
*/

#ifndef __YTLFunctionCallback_h__
#define __YTLFunctionCallback_h__

class function_callback {
/**
	関数のコールバック用基底クラス
	こいつの、smart_ptrを相手に渡して、
	相手からは、run()を呼び出してもらえればそれで良い

	例)
		smart_ptr<function_callback_r>
			s(function_callback_r<int>::Create(&test,1,10));
		//	コールバックオブジェクトが完成
		int nResult = s->run();
		//	int test(int n1,int n2)という関数をtest(1,10)と呼び出す

		function_callback_rg2のrg2とは、
			r : result有り	vならばresult無し
			g : グローバルな関数 mならばmember関数
			2 : 関数の引数は２つ
		の意味です。

		引数は5つのテンプレートまで用意してあります。
		それ以上については、自前で用意してください。

		例：
		smart_ptr<function_callback_v>
			s(function_callback_v::Create(&CHoge::test,this,1,10));
		//	コールバックオブジェクトが完成
		s->run();
		//	void CHoge::test(int n1,int n2)という関数をtest(1,10)と呼び出す

		function_callback_vもfunction_callback_rも、
		このfunction_callback派生クラスなので、戻り値はどうでも良いのならば
		受け取るのは、smart_ptr<function_callback>でも構いません。

		void test(int n)という関数をtest(m)とコールバックするならば、
		smart_ptr<function_callback> f(function_callback_v::Create(test,m));
		f->run();

		//	int CHoge::test(int n1,int n2,int n3)という関数をtest(1,10,100)と呼び出す
		smart_ptr<function_callback_r<int> >
			s(function_callback_r<int>::Create(CHoge::test,this,1,10,100));
		s->run();
		int r = s->GetResult();	//	これで返し値を取得

		//	グローバルな関数int	SukiSukiSakura(int n1,int n2)を呼び出す場合
		smart_ptr<function_callback_r<int> >
			s(function_callback_r<int>::Create(&SukiSukiSakura,2,3));
		s->run();
		int r = s->GetResult();	//	これで返し値を取得

*/
public:
	virtual	void run() = 0;
	virtual ~function_callback(){}
};

class function_callback_v : public function_callback {
///		返し値を持たないバージョン
public:
	//	ヘルパ関数
	static function_callback_v* Create(void (*f)());

	template <class Arg1,class T>
		static function_callback_v* Create(void (T::*f)(Arg1),const Arg1& a1){
			return new function_callback_vg1<Arg1>(f,a1);
	}
	template <class Arg1>
	static function_callback_v* Create(void (*f)(Arg1),const Arg1& a1){
			return new function_callback_vg1<Arg1>(f,a1);
	}
	template <class Arg1,class Arg2>
	static function_callback_v* Create(void (*f)(Arg1,Arg2),const Arg1& a1,const Arg2& a2){
			return new function_callback_vg2<Arg1,Arg2>(f,a1,a2);
	}
	template <class Arg1,class Arg2,class Arg3>
	static function_callback_v* Create(void (*f)(Arg1,Arg2,Arg3),
		const Arg1& a1,const Arg2& a2,const Arg3& a3){
			return new function_callback_vg3<Arg1,Arg2,Arg3>(f,a1,a2);
	}
	template <class Arg1,class Arg2,class Arg3,class Arg4>
	static function_callback_v* Create(void (*f)(Arg1,Arg2,Arg3,Arg4),
		const Arg1& a1,const Arg2& a2,const Arg3& a3,const Arg4& a4){
			return new function_callback_vg4<Arg1,Arg2,Arg3,Arg4>(f,a1,a2,a3,a4);
	}
	template <class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
	static function_callback_v* Create(void (*f)(Arg1,Arg2,Arg3,Arg4,Arg5),
		const Arg1& a1,const Arg2& a2,const Arg3& a3,
		const Arg4& a4,const Arg5& a5){
			return new function_callback_vg5<Arg1,Arg2,Arg3,Arg4,Arg5>(func f,Arg1 a1,Arg2 a2,Arg3 a3,Arg4 a4,Arg5 a5);
	}
	template <class obj>
	static function_callback_v* Create(void (obj::*f)(),obj* o){
			return new function_callback_vm0<obj>(f,o);
	}
	template <class obj,class Arg1>
	static function_callback_v* Create(void (obj::*f)(Arg1),obj* o,const Arg1& a1){
			return new function_callback_vm1<obj,Arg1>(f,o,a1);
	}
	template <class obj,class Arg1,class Arg2>
	static function_callback_v* Create(void (obj::*f)(Arg1,Arg2),obj* o,
		const Arg1& a1,const Arg2& a2){
			return new function_callback_vm2<obj,Arg1,Arg2>(f,o,a1,a2);
	}
	template <class obj,class Arg1,class Arg2,class Arg3>
	static function_callback_v* Create(void (obj::*f)(Arg1,Arg2,Arg3),obj* o,
		const Arg1& a1,const Arg2& a2,const Arg3& a3){
			return new function_callback_vm3<obj,Arg1,Arg2,Arg3>(f,o,a1,a2,a3);
	}
	template <class obj,class Arg1,class Arg2,class Arg3,class Arg4>
	static function_callback_v* Create(void (obj::*f)(Arg1,Arg2,Arg3,Arg4),
		obj* o,const Arg1& a1,const Arg2& a2,const Arg3& a3,const Arg4& a4){
			return new function_callback_vm4<obj,Arg1,Arg2,Arg3,Arg4>(f,o,a1,a2,a3,a4);
	}
	template <class obj,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
	static function_callback_v* Create(void (obj::*f)(Arg1,Arg2,Arg3,Arg4,Arg5),
		obj* o,const Arg1& a1,const Arg2& a2,const Arg3& a3,const Arg4& a4,const Arg5& a5){
			return new function_callback_vm5<obj,Arg1,Arg2,Arg3,Arg4,Arg5>(f,o,a1,a2,a3,a4,a5);
	}
};

template <class Result>
class function_callback_r : public function_callback {
///		返し値をテンプレート化されたバージョン
public:
	Result GetResult() const { return _r; }

	//	ヘルパ関数
	static function_callback_r* Create(Result (*f)());
	template <class Arg1>
	static function_callback_r* Create(Result (*f)(Arg1),const Arg1& a1){
			return new function_callback_rg1<Result,Arg1>(f,a1);
	}
	template <class Arg1,class Arg2>
	static function_callback_r* Create(Result (*f)(Arg1,Arg2),
		const Arg1& a1,const Arg2& a2){
			return new function_callback_rg2<Result,Arg1,Arg2>(f,a1,a2);
	}
	template <class Arg1,class Arg2,class Arg3>
	static function_callback_r* Create(Result (*f)(Arg1,Arg2,Arg3),
		const Arg1& a1,const Arg2& a2,const Arg3& a3){
			return new function_callback_rg3<Result,Arg1,Arg2,Arg3>(f,a1,a2);
	}
	template <class Arg1,class Arg2,class Arg3,class Arg4>
	static function_callback_r* Create(Result (*f)(Arg1,Arg2,Arg3,Arg4),
		const Arg1& a1,const Arg2& a2,const Arg3& a3,const Arg4& a4){
			return new function_callback_rg4<Result,Arg1,Arg2,Arg3,Arg4>(f,a1,a2,a3,a4);
	}
	template <class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
	static function_callback_r* Create(Result (*f)(Arg1,Arg2,Arg3,Arg4),
		const Arg1& a1,const Arg2& a2,const Arg3& a3,const Arg4& a4,const Arg5& a5){
			return new function_callback_rg5<Result,Arg1,Arg2,Arg3,Arg4,Arg5>(func f,Arg1 a1,Arg2 a2,Arg3 a3,Arg4 a4,Arg5 a5);
	}
	template <class obj>
	static function_callback_r* Create(Result (obj::*f)(),obj* o){
			return new function_callback_rm0<Result,obj>(f,o);
	}
	template <class obj,class Arg1>
	static function_callback_r* Create(Result (obj::*f)(Arg1),obj* o,const Arg1& a1){
			return new function_callback_rm1<Result,obj,Arg1>(f,o,a1);
	}
	template <class obj,class Arg1,class Arg2>
	static function_callback_r* Create(Result (obj::*f)(Arg1,Arg2),obj* o,
		const Arg1& a1,const Arg2& a2){
			return new function_callback_rm2<Result,obj,Arg1,Arg2>(f,o,a1,a2);
	}
	template <class obj,class Arg1,class Arg2,class Arg3>
	static function_callback_r* Create(Result (obj::*f)(Arg1,Arg2,Arg3),obj* o,
		const Arg1& a1,const Arg2& a2,const Arg3& a3){
			return new function_callback_rm3<Result,obj,Arg1,Arg2,Arg3>(f,o,a1,a2,a3);
	}
	template <class obj,class Arg1,class Arg2,class Arg3,class Arg4>
	static function_callback_r* Create(Result (obj::*f)(Arg1,Arg2,Arg3,Arg4),
		obj* o,const Arg1& a1,const Arg2& a2,const Arg3& a3,const Arg4& a4){
			return new function_callback_rm4<Result,obj,Arg1,Arg2,Arg3,Arg4>(f,o,a1,a2,a3,a4);
	}
	template <class obj,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
	static function_callback_r* Create(Result (obj::*f)(Arg1,Arg2,Arg3,Arg4,Arg5),
		obj* o,const Arg1& a1,const Arg2& a2,const Arg3& a3,const Arg4& a4,const Arg5& a5){
			return new function_callback_rm5<Result,obj,Arg1,Arg2,Arg3,Arg4,Arg5>(f,o,a1,a2,a3,a4,a5);
	}

protected:
	Result _r;
};

////////////////////////////////////////////////////////
//	あとは、上記の、
//		リザルトの有無×引数の数(0-5)×メンバ関数or非メンバ関数
//	のバリエーション　＝　２４個　(;´Д`) 

//---------	リザルト無し	グローバル関数
class function_callback_vg0 : public function_callback_v {
//	void グローバル関数　引数0
public:
	typedef void (*function_type) ();
	function_callback_vg0(function_type f) : _f(f) {}
	virtual void run(){ _f(); }
protected:
	function_type _f;
};

template <class Arg1>
class function_callback_vg1 : public function_callback_v {
//	void グローバル関数　引数1
public:
	typedef void (*function_type) (Arg1);
	function_callback_vg1(function_type f,Arg1 a1) : _f(f),_a1(a1) {}
	virtual void run(){ _f(_a1); }
protected:
	function_type _f;
	Arg1 _a1;
};

template <class Arg1,class Arg2>
class function_callback_vg2 : public function_callback_v {
//	void グローバル関数　引数2
public:
	typedef void (*function_type) (Arg1,Arg2);
	function_callback_vg2(function_type f,Arg1 a1,Arg2 a2)
		: _f(f),_a1(a1),_a2(a2) {}
	virtual void run(){ _f(_a1,_a2); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
};

template <class Arg1,class Arg2,class Arg3>
class function_callback_vg3 : public function_callback_v {
//	void グローバル関数　引数3
public:
	typedef void (*function_type) (Arg1,Arg2,Arg3);
	function_callback_vg3(function_type f,Arg1 a1,Arg2 a2,Arg3 a3)
		: _f(f),_a1(a1),_a2(a2),_a3(a3) {}
	virtual void run(){ _f(_a1,_a2,_a3); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
};

template <class Arg1,class Arg2,class Arg3,class Arg4>
class function_callback_vg4 : public function_callback_v {
//	void グローバル関数　引数4
public:
	typedef void (*function_type) (Arg1,Arg2,Arg3,Arg4);
	function_callback_vg4(function_type f,Arg1 a1,Arg2 a2,Arg3 a3,Arg4 a4)
		: _f(f),_a1(a1),_a2(a2),_a3(a3),_a4(a4) {}
	virtual void run(){ _f(_a1,_a2,_a3,_a4); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
};

template <class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
class function_callback_vg5 : public function_callback_v {
//	void グローバル関数　引数5
public:
	typedef void (*function_type) (Arg1,Arg2,Arg3,Arg4,Arg5);
	function_callback_vg5(function_type f,Arg1 a1,Arg2 a2,Arg3 a3
		,Arg4 a4,Arg5 a5)
		: _f(f),_a1(a1),_a2(a2),_a3(a3),_a4(a4),_a5(a5) {}
	virtual void run(){ _f(_a1,_a2,_a3,_a4,_a5); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
	Arg5 _a5;
};

//---------	リザルト有り	グローバル関数
template <class Result>
class function_callback_rg0 : public function_callback_r<Result> {
//	void グローバル関数　引数無し
public:
	typedef Result (*function_type) ();
	function_callback_rg0(function_type f) : _f(f) {}
	virtual void run(){ _r = _f(); }
protected:
	function_type _f;
};

template <class Result,class Arg1>
class function_callback_rg1 : public function_callback_r<Result> {
//	void グローバル関数　引数1
public:
	typedef Result (*function_type) (Arg1);
	function_callback_rg1(function_type f,Arg1 a1) : _f(f),_a1(a1) {}
	virtual void run(){ _r = _f(_a1); }
protected:
	function_type _f;
	Arg1 _a1;
};

template <class Result,class Arg1,class Arg2>
class function_callback_rg2 : public function_callback_r<Result> {
//	void グローバル関数　引数2
public:
	typedef Result (*function_type) (Arg1,Arg2);
	function_callback_rg2(function_type f,Arg1 a1,Arg2 a2)
		: _f(f),_a1(a1),_a2(a2) {}
	virtual void run(){ _r = _f(_a1,_a2); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
};

template <class Result,class Arg1,class Arg2,class Arg3>
class function_callback_rg3 : public function_callback_r<Result> {
//	void グローバル関数　引数3
public:
	typedef Result (*function_type) (Arg1,Arg2,Arg3);
	function_callback_rg3(function_type f,Arg1 a1,Arg2 a2,Arg3 a3)
		: _f(f),_a1(a1),_a2(a2),_a3(a3) {}
	virtual void run(){ _r=_f(_a1,_a2,_a3); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
};

template <class Result,class Arg1,class Arg2,class Arg3,class Arg4>
class function_callback_rg4 : public function_callback_r<Result> {
//	void グローバル関数　引数4
public:
	typedef Result (*function_type) (Arg1,Arg2,Arg3,Arg4);
	function_callback_rg4(function_type f,Arg1 a1,Arg2 a2,Arg3 a3,Arg4 a4)
		: _f(f),_a1(a1),_a2(a2),_a3(a3),_a4(a4) {}
	virtual void run(){ _r=_f(_a1,_a2,_a3,_a4); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
};

template <class Result,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
class function_callback_rg5 : public function_callback_r<Result> {
//	void グローバル関数　引数5
public:
	typedef Result (*function_type) (Arg1,Arg2,Arg3,Arg4,Arg5);
	function_callback_rg5(function_type f,Arg1 a1,Arg2 a2,Arg3 a3
		,Arg4 a4,Arg5 a5)
		: _f(f),_a1(a1),_a2(a2),_a3(a3),_a4(a4),_a5(a5) {}
	virtual void run(){ _r=_f(_a1,_a2,_a3,_a4,_a5); }
protected:
	function_type _f;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
	Arg5 _a5;
};

//---------	リザルト無し	メンバ関数
template <class ObjectType>
class function_callback_vm0 : public function_callback_v {
//	void メンバ関数　引数0
public:
	typedef void (ObjectType::*function_type) ();
	function_callback_vm0(function_type f,ObjectType*o) : _f(f),_o(o) {}
	virtual void run(){ (_o->*_f)(); }
protected:
	function_type _f;
	ObjectType* _o;
};

template <class ObjectType,class Arg1>
class function_callback_vm1 : public function_callback_v {
//	void メンバ関数　引数1
public:
	typedef void (ObjectType::*function_type) (Arg1);
	function_callback_vm1(function_type f,ObjectType*o,Arg1 a1)
		: _f(f),_o(o),_a1(a1) {}
	virtual void run(){ (_o->*_f)(_a1); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
};

template <class ObjectType,class Arg1,class Arg2>
class function_callback_vm2 : public function_callback_v {
//	void メンバ関数　引数2
public:
	typedef void (ObjectType::*function_type) (Arg1,Arg2);
	function_callback_vm2(function_type f,ObjectType*o,Arg1 a1,Arg2 a2)
		: _f(f),_o(o),_a1(a1),_a2(a2) {}
	virtual void run(){ (_o->*_f)(_a1,_a2); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
};

template <class ObjectType,class Arg1,class Arg2,class Arg3>
class function_callback_vm3 : public function_callback_v {
//	void メンバ関数　引数3
public:
	typedef void (ObjectType::*function_type) (Arg1,Arg2,Arg3);
	function_callback_vm3(function_type f,ObjectType*o,Arg1 a1,Arg2 a2,Arg3 a3)
		: _f(f),_o(o),_a1(a1),_a2(a2),_a3(a3) {}
	virtual void run(){ (_o->*_f)(_a1,_a2,_a3); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
};

template <class ObjectType,class Arg1,class Arg2,class Arg3,class Arg4>
class function_callback_vm4 : public function_callback_v {
//	void メンバ関数　引数4
public:
	typedef void (ObjectType::*function_type) (Arg1,Arg2,Arg3,Arg4);
	function_callback_vm4(function_type f,ObjectType*o,Arg1 a1,Arg2 a2
		,Arg3 a3,Arg4 a4)
		: _f(f),_o(o),_a1(a1),_a2(a2),_a3(a3),_a4(a4) {}
	virtual void run(){ (_o->*_f)(_a1,_a2,_a3,_a4); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
};

template <class ObjectType,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
class function_callback_vm5 : public function_callback_v {
//	void メンバ関数　引数5
public:
	typedef void (ObjectType::*function_type) (Arg1,Arg2,Arg3,Arg4,Arg5);
	function_callback_vm5(function_type f,ObjectType*o,Arg1 a1,Arg2 a2
		,Arg3 a3,Arg4 a4,Arg5 a5)
		: _f(f),_o(o),_a1(a1),_a2(a2),_a3(a3),_a4(a4),_a5(a5) {}
	virtual void run(){ (_o->*_f)(_a1,_a2,_a3,_a4,_a5); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
	Arg5 _a5;
};

//---------	リザルト有り	メンバ関数
template <class Result,class ObjectType>
class function_callback_rm0 : public function_callback_r<Result> {
//	void グローバル関数　引数無し
public:
	typedef Result (ObjectType::*function_type) ();
	function_callback_rm0(function_type f,ObjectType*o) : _f(f),_o(o) {}
	virtual void run(){ _r = (_o->*_f)(); }
protected:
	function_type _f;
	ObjectType* _o;
};

template <class Result,class ObjectType,class Arg1>
class function_callback_rm1 : public function_callback_r<Result> {
//	void メンバ関数　引数1
public:
	typedef Result (ObjectType::*function_type) (Arg1);
	function_callback_rm1(function_type f,ObjectType*o,Arg1 a1)
		: _f(f),_o(o),_a1(a1) {}
	virtual void run(){ _r = (_o->*_f)(_a1); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
};

template <class Result,class ObjectType,class Arg1,class Arg2>
class function_callback_rm2 : public function_callback_r<Result> {
//	void メンバ関数　引数2
public:
	typedef Result (ObjectType::*function_type) (Arg1,Arg2);
	function_callback_rm2(function_type f,ObjectType*o,Arg1 a1,Arg2 a2)
		: _f(f),_o(o),_a1(a1),_a2(a2) {}
	virtual void run(){ _r = (_o->*_f)(_a1,_a2); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
};

template <class Result,class ObjectType,class Arg1,class Arg2,class Arg3>
class function_callback_rm3 : public function_callback_r<Result> {
//	void メンバ関数　引数3
public:
	typedef Result (ObjectType::*function_type) (Arg1,Arg2,Arg3);
	function_callback_rm3(function_type f,ObjectType*o,Arg1 a1,Arg2 a2,Arg3 a3)
		: _f(f),_o(o),_a1(a1),_a2(a2),_a3(a3) {}
	virtual void run(){ _r=(_o->*_f)(_a1,_a2,_a3); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
};

template <class Result,class ObjectType,class Arg1,class Arg2,class Arg3,class Arg4>
class function_callback_rm4 : public function_callback_r<Result> {
//	void メンバ関数　引数4
public:
	typedef Result (ObjectType::*function_type) (Arg1,Arg2,Arg3,Arg4);
	function_callback_rm4(function_type f,ObjectType*o,
		Arg1 a1,Arg2 a2,Arg3 a3,Arg4 a4)
		: _f(f),_o(o),_a1(a1),_a2(a2),_a3(a3),_a4(a4) {}
	virtual void run(){ _r=(_o->*_f)(_a1,_a2,_a3,_a4); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
};

template <class Result,class ObjectType,class Arg1,class Arg2,class Arg3,class Arg4,class Arg5>
class function_callback_rm5 : public function_callback_r<Result> {
//	void メンバ関数　引数5
public:
	typedef Result (ObjectType::*function_type) (Arg1,Arg2,Arg3,Arg4,Arg5);
	function_callback_rm5(function_type f,ObjectType*o,
		Arg1 a1,Arg2 a2,Arg3 a3,Arg4 a4,Arg5 a5)
		: _f(f),_o(o),_a1(a1),_a2(a2),_a3(a3),_a4(a4),_a5(a5) {}
	virtual void run(){ _r=(_o->*_f)(_a1,_a2,_a3,_a4,_a5); }
protected:
	function_type _f;
	ObjectType* _o;
	Arg1 _a1;
	Arg2 _a2;
	Arg3 _a3;
	Arg4 _a4;
	Arg5 _a5;
};

////////////////////////////////////////////////////////

#endif
