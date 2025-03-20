// autoref "超手抜き" core  誰か一緒に作って^^;;
//		programmed by ENRA		'02/04/21
#pragma once

class CClassDesc;
class CSuperClassDesc
{
public:
	CSuperClassDesc() : bVirtualDerive(false), nDeriveKindOf(2),pSubClass(NULL) {};
	virtual ~CSuperClassDesc() {};

	string	strName;			// クラス名
	CClassDesc* pSubClass;		// 派生先クラス（smart_ptrでは相共有になって解体されなくなるので駄目！）
	bool	bVirtualDerive;		// virtual継承か？
	int		nDeriveKindOf;		// 継承タイプ 0: public / 1: protected / 2: private

	string	GetDeriveKindOf() const {
		switch(nDeriveKindOf){
		case 0: return "public";
		case 1: return "protected";
		case 2: return "private";
		}
		return "";	// dummy
	}
};

class IMemberDesc
{
public:
	IMemberDesc() : nAccessType(2) {};
	virtual ~IMemberDesc(){};
	int nAccessType;

	// RTTIもどき^^;
	// 0: CMemberFunctionDesc
	// 1: CMemberVariableDesc
	// 2: CEnumDesc
	virtual int GetType() const = 0;

	string	GetAccessType() const {
		switch(nAccessType){
		case 0: return "public";
		case 1: return "protected";
		case 2: return "private";
		}
		return "";	// dummy
	}
};

class CMemberDescBlock
{
public:
	CMemberDescBlock(){};
	virtual ~CMemberDescBlock(){};
	smart_vector_ptr<IMemberDesc> apMemberDesc;		// IMemberDesc派生クラスのリスト
	string strComment;								// コメント
};

class CArgDesc;
class CMemberFunctionDesc : public IMemberDesc
{
public:
	CMemberFunctionDesc() : bPureVirtual(false) {};
	virtual ~CMemberFunctionDesc(){};
	string	strName;					// 関数名
	string	strRetvalType;				// 返値の型
	smart_ptr<CArgDesc>	pArg;			// 引数
	smart_ptr<CArgDesc>	pTemplateArg;	// テンプレート引数
	bool	bPureVirtual;
	int GetType() const { return 0; };
};

class CMemberVariableDesc : public IMemberDesc
{
public:
	CMemberVariableDesc(){};
	virtual ~CMemberVariableDesc(){};
	string	strName;			// 変数名
	string	strType;			// 型
	int GetType() const { return 1; };
};

class CEnumDesc : public IMemberDesc
{
public:
	CEnumDesc(){};
	virtual ~CEnumDesc(){};

	string	strName;			// enum名
	vector<string>	astrMember;	// 列挙子
	int GetType() const { return 2; };
};

class CArgDesc
{
public:
	CArgDesc(){};
	virtual ~CArgDesc(){};
	vector<string>	astrType;	// 引数の型
	vector<string>	astrName;	// 引数名
};

class CClassDesc
{
public:
	CClassDesc() : bIsClass(false),nAccessType(2) {};
	virtual ~CClassDesc(){};

	bool	bIsClass;									// "Class" or "Struct"
	string	strName;									// クラス名
	string	strHeaderFilename;							// ヘッダファイル名
	string	strComment;									// クラスへのコメント
	smart_ptr<CArgDesc>	pTemplateArg;					// テンプレート引数
	int nAccessType;									// このクラスのアクセス属性
	smart_vector_ptr<CSuperClassDesc> apSuperClass;		// 継承元クラス
	smart_ptr<CClassDesc> pOuterClass;					// 外側のクラス

	string	GetKindOf() const { return (bIsClass) ? "class" : "struct"; }
	string	GetKindOfJ() const { return (bIsClass) ? "クラス" : "構造体"; }
	string	GetFullName(const string& separater="::") const
	{
		string name = this->strName;
		// クラス内クラスの場合、OuterClassを頭にくっつけていく
		smart_ptr<CClassDesc> p = this->pOuterClass;
		while(p!=NULL){
			name = p->strName + separater + name;
			p = p->pOuterClass;
		}
		return name;
	}

	void	AddMember(smart_ptr<CMemberDescBlock> pDesc) { m_apMember.insert(pDesc); }
	smart_vector_ptr<CMemberDescBlock> m_apMember;		// メンバ群
};

class CScnConvert {
/**
	autorefの本体
*/
public:
	CScnConvert() : m_nLineNo(0),m_nCurAccessType(2) {};
	virtual ~CScnConvert(){};
	/// あるファイルを、変換する。出力先pathは、こいつで設定しておく
	LRESULT	Convert(string strSrcPath, vector<string> astrSrcFilename, string strDstPath);

	void	Err(string ErrMes);

protected:
	// class、structを解析する
	bool	ParseClass(smart_ptr<CClassDesc> pOuterClass=NULL);
	// 関数や変数の宣言を解析する
	bool	ParseMember(CMemberDescBlock* pMemberBlock);
	// enumを解析する
	bool	ParseEnum(CMemberDescBlock* pMemberBlock);
	// テンプレート引数を解析する
	smart_ptr<CArgDesc> ParseTemplate();


	// クラス名からCClassDescを探す
	const	CClassDesc* FindClass(const string& strClassName);
	// 登録されたクラス名に一致した箇所にHTMLタグを付ける(<A>)
	void	LinkRegisteredClass(string& line);
	// 予約語にHTMLタグを付ける(<SPAN>)
	void	MarkReservedWord(string& word);
	// ドキュメントのアドレス（ファイル名）を得る
	string	GetAddress(const CClassDesc* pClass);


	// コメントをとばす
	// bSkipSpecialComment=trueなら"/***/","///"も対象
	// pCommentを指定すれば中身を得る事が出来る
	bool	SkipComment(LPCSTR &pSrcStr, bool bSkipSpecialComment=true, string* pComment=NULL);
	// autoref用のコメントを取り出す
	bool	ExtractSpecialComment(LPCSTR &pSrcStr, string& strComment);

	// CClassDescを追加する
	void	AddClassDesc(smart_ptr<CClassDesc> p) {
		m_apClassDesc.insert(p);
		m_mapNameToClass.insert(pair<string,CClassDesc*>(p->GetFullName(), p));
	}
	// 解析されたクラス
	smart_vector_ptr<CClassDesc> m_apClassDesc;
	map<string, CClassDesc*>	 m_mapNameToClass;

	// ファイル内容
	string	m_strSrc;
	// 現在の文字位置
	LPCSTR	m_lpSrcPos;
	// 現在処理中のファイル名
	string	m_strFilename;

	// helper
	// 現在のアクセスタイプ（クラス内解析時）
	int		m_nCurAccessType;
	// 現在のコメント（クラス内解析時）
	string	m_strCurComment;

	// 解析中のラインナンバー
	int		m_nLineNo;

protected:
	// ヘルパ関数群
	bool	IsToken(LPCSTR &lp, LPCSTR lp2);
	LRESULT	SkipSpace(LPCSTR &lp);
	LRESULT	SkipTo(LPCSTR &pSrcStr, LPCSTR pTokenStr, string* pSkipedStr=NULL);
};
