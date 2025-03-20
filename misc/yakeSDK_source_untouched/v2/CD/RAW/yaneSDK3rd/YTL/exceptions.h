/*
	ありがちな例外を定義
*/

#ifndef __YTLExceptions_h__
#define __YTLExceptions_h__

#ifdef USE_EXCEPTION

class CException {
/**
	すべての例外は、こいつから派生させる
*/
public:
	CException(){
#ifdef USE_STOP_EXCEPTION
	MessageBox(NULL,"例外が発生したので停止します！","例外発生",MB_OK);
	/**	このダイアログが出た直後に、VSから
		プロセスのアタッチをして、ビルド⇒ブレーク
		すればコールスタックを見てどこで発生した例外か特定できる
	*/

	///	ついでにメモリエラーをわざと発生させるコードを仕掛ける
	* LPLONG(0xcdcdcdcd) = 0;
	///	WinNT/2000系ならば、わざとこのメモリエラーを発生させて、
	///	デバッグしたほうが手っ取り早い
#endif
	}
	virtual string getError() const { return ""; };
};

class CIndexOutOfBoundsException : public CException {
///		配列範囲外へのメモリへのアクセス例外
///		class smart_ptr を使用しているときにチェック機構により発生
	virtual string getError() const {
		return "配列範囲外へのメモリへのアクセス";
	};
};

class CNullPointerException : public CException {
///		Null pointerからのアクセス例外
///		class smart_ptr を使用しているときにチェック機構により発生
	virtual string getError() const {
		return "Null pointerからのアクセス";
	};
};

class CRuntimeException : public CException {
///		範囲外の数字が指定されているときのアクセス例外
public:
	CRuntimeException(const string& strErrorName="") : m_str(strErrorName) {
		if (m_str.empty()){
			m_str = "実行時例外（範囲外の数値が指定されている)";
		}
	}
protected:
	virtual string getError() const {
		return m_str;
	};
	string m_str;
};

class CSyntaxException : public CException {
///		一般的な文法エラーに対するアクセス例外
public:
	CSyntaxException(const string& strErrorName) : m_str(strErrorName) {}	
protected:
	virtual string getError() const {
		return m_str;
	}
	string m_str;
};

#endif	//	USE_EXCEPTION

#endif
