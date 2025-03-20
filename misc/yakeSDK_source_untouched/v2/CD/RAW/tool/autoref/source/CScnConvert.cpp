#include "stdafx.h"
#include <direct.h>	 // for _mkdir
#include <mbctype.h> // for _ismbblead
#include "CScnConvert.h"
#include "../../yaneSDK/yaneDebugWindow.h"
///////////////////////////////////////////////////////////
//	コンバータ本体↓
//	いじるときは、これを適当にいじってくらはい。
static const char* aReservedWordTable[] = {
	"void",
	"bool",
	"char",
	"short",
	"int",
	"long",
	"signed",
	"unsigned",
	"true",
	"false",
	"class",
	"struct",
	"enum",
	"const",
	"static",
	"const",
	"volatile",
	"virtual",
	"explicit",
};

LRESULT CScnConvert::Convert(string strSrcPath, vector<string> astrSrcFilename, string strDstPath)
{
	// "/"をつけましょー
	CStringScanner::Replace(strSrcPath,"\\","/");
	if(*strSrcPath.rbegin()!='/') { strSrcPath += '/'; }
	CStringScanner::Replace(strDstPath,"\\","/");
	if(*strDstPath.rbegin()!='/') { strDstPath += '/'; }

	// フォルダを掘りましょー
	_mkdir(strDstPath.c_str());

	// 初期化
	m_apClassDesc.clear();

	{
		string line;
		CFile src_file;
		vector<string>::iterator itFile = astrSrcFilename.begin();
		while (itFile!=astrSrcFilename.end()){
			// ファイル名を設定
			CStringScanner::Replace(*itFile,"\\","/");
			m_strFilename.assign(*itFile, strSrcPath.size(), itFile->size()-strSrcPath.size());
			CDbg().Out("解析中："+m_strFilename);

			src_file.Close();
			if (src_file.Read(strSrcPath + m_strFilename)!=0) continue;
			// ファイルを全て読み込む
			m_strSrc.erase();
			while (src_file.ReadLine(line)==0) {
				m_strSrc += line + "\n";
			}
			m_lpSrcPos = m_strSrc.c_str();
			// 解析する
			while (*m_lpSrcPos!='\0'){
				if (ParseClass()) continue;
				SkipTo(m_lpSrcPos, "\n");
			}
			itFile++;
		}// end while (itFile!=astrSrcFilename.end())
	}

	// 書き込むファイル
	CFile dst_file;
	smart_vector_ptr<CClassDesc>::const_iterator it = m_apClassDesc.begin();
	string header_filename;
	while (it != m_apClassDesc.end())
	{
		const CClassDesc& desc = *(*it);	// iterator->smart_ptr->reference
		if (header_filename != desc.strHeaderFilename) {
			//	前の開いてたら閉じる
			if (dst_file.GetFilePtr()!=NULL){
				dst_file.Write("</BODY></HTML>");
				dst_file.Close();
			}
			// 新しいヘッダファイル名
			header_filename = (*it)->strHeaderFilename;
			CDbg().Out("出力中："+header_filename);
			string temp = header_filename;
			CStringScanner::Replace(temp,"/","!");
			CStringScanner::Replace(temp,".","_");
			dst_file.Open(strDstPath + temp + ".html", "w");
			// HTMLヘッダ
			dst_file.Write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n");
			dst_file.Write("<HTML><HEAD>\n");
			dst_file.Write("<META http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_jis\">\n");
			dst_file.Write("<META http-equiv=\"Content-Language\" content=\"ja\">\n");
			dst_file.Write("<META name=\"GENERATOR\" content=\"autoref version 1.0.0\">\n");
			dst_file.Write("<LINK REL=STYLESHEET TYPE=\"text/css\" HREF=\"document.css\">\n");
			dst_file.Write("<SCRIPT LANGUAGE=\"JavaScript\" SRC=\"document.js\"></SCRIPT>\n");
			dst_file.Write("<TITLE>" + header_filename + " のドキュメント</TITLE>\n");
			dst_file.Write("</HEAD>\n");
			dst_file.Write("<BODY>\n");
			dst_file.Write("<A name=\"top\"></A>\n");
		}
		{	// クラス名
			dst_file.Write("<A name=\"" + desc.strName + "\"></A>\n");
			// テンプレートクラスの場合
			if (desc.pTemplateArg!=NULL){
				string line;
				const int size = desc.pTemplateArg->astrType.size();
				for (int i=0; i<size; i++){
					if (i!=0) line += ", ";
					line += desc.pTemplateArg->astrType[i];
					line += " ";
					line += desc.pTemplateArg->astrName[i];
				}
				CStringScanner::Replace(line,"&","&amp;");
				CStringScanner::Replace(line,"<","&lt;");
				CStringScanner::Replace(line,">","&gt;");
				CStringScanner::Replace(line,"\n","<BR>");
				CStringScanner::Replace(line," ","&nbsp;");
				dst_file.Write("<SPAN class=\"reserved_word\">template</SPAN>&nbsp;");
				dst_file.Write("&lt;<SPAN class=\"template_arg\">"+line+"</SPAN>&gt;<BR>");
			}
			dst_file.Write("<SPAN class=\"reserved_word\">"+desc.GetKindOf()+"</SPAN>&nbsp;");
			dst_file.Write("<SPAN class=\"class_name\">"+desc.GetFullName()+"</SPAN>\n");
			dst_file.Write("<HR>\n");
		}
		{	// クラスコメント
			dst_file.Write("<DIV class=\"block_title\">"
							+ desc.GetKindOfJ() + "の説明</DIV>\n");
			string comment = desc.strComment;
			if (comment=="") { comment = "なし"; }
			CStringScanner::Replace(comment,"&","&amp;");
			CStringScanner::Replace(comment,"<","&lt;");
			CStringScanner::Replace(comment,">","&gt;");
			CStringScanner::Replace(comment, "\n", "<BR>");
			dst_file.Write("<DIV class=\"comment_block\">");
			dst_file.Write(comment + "</DIV>\n");
		}
		{	// 継承の指定
			dst_file.Write("<DIV class=\"block_title\">継承元</DIV>\n");
			dst_file.Write("<DIV class=\"super_class_block\">");
			const int size = desc.apSuperClass.size();
			if(size==0) dst_file.Write("なし");
			for(int i=0; i<size; i++){
				const smart_ptr<CSuperClassDesc> p = desc.apSuperClass[i];
				if (p->bVirtualDerive) {
					dst_file.Write("<SPAN class=\"reserved_word\">virtual</SPAN>&nbsp;");
				}
				dst_file.Write("<SPAN class=\"reserved_word\">");
				dst_file.Write(p->GetDeriveKindOf() + "</SPAN>&nbsp;");
				// そのクラスがリストに存在すればリンクを張る
				string name = p->strName;
				LinkRegisteredClass(name);	// ここで文末に" "が追加される
				dst_file.Write(name + "<BR>\n");
			}
			dst_file.Write("</DIV>\n");
		}
		{	// クラスメンバ
			string line;
			string comment;
			const int size = desc.m_apMember.size();
			dst_file.Write("<DIV class=\"block_title\">クラスメンバ</DIV>\n");
			if(size==0)	dst_file.Write("<DIV class=\"class_member_block\">なし</DIV>\n");
			for(int i=0; i<size; i++){
				dst_file.Write("<DIV class=\"class_member_block\">");
				smart_vector_ptr<IMemberDesc>::const_iterator itMember = desc.m_apMember[i]->apMemberDesc.begin();
				smart_vector_ptr<IMemberDesc>::const_iterator itMemberEnd = desc.m_apMember[i]->apMemberDesc.end();
				while (itMember!=itMemberEnd){
					const IMemberDesc* pMember = (*itMember);
					dst_file.Write("<DIV class=\""+pMember->GetAccessType()+"_member\">");
					switch (pMember->GetType()){
					case 0:
						{	// CMemberFunctionDesc
							CMemberFunctionDesc* p = (CMemberFunctionDesc*)pMember;
							// テンプレート関数の場合
							if (p->pTemplateArg!=NULL){
								string line;
								const int size = p->pTemplateArg->astrType.size();
								dst_file.Write("<SPAN class=\"reserved_word\">template</SPAN>&nbsp;");
								dst_file.Write("&lt;<SPAN class=\"template_arg\">");
								for (int i=0; i<size; i++){
									if (i!=0) dst_file.Write(",&nbsp;");
									line = p->pTemplateArg->astrType[i];
									LinkRegisteredClass(line);	// ここでHTML用に整形される
									MarkReservedWord(line);	// ここで文末に" "が追加される
									dst_file.Write(line);
									line = p->pTemplateArg->astrName[i];
									CStringScanner::Replace(line,"&","&amp;");
									CStringScanner::Replace(line,"<","&lt;");
									CStringScanner::Replace(line,">","&gt;");
									CStringScanner::Replace(line,"\n","<BR>");
									dst_file.Write(line);
								}
								dst_file.Write("</SPAN>&gt;<BR>");
							}
							// 戻り値
							line = p->strRetvalType;
							LinkRegisteredClass(line);	// ここでHTML用に整形される
							MarkReservedWord(line);	// ここで文末に" "が追加される
							dst_file.Write(line);
							// 関数名
							line = p->strName;
							CStringScanner::Replace(line,"&","&amp;");
							CStringScanner::Replace(line,"<","&lt;");
							CStringScanner::Replace(line,">","&gt;");
							CStringScanner::Replace(line,"\n","<BR>");
							dst_file.Write(line);
							// 引数
							{
								string line;
								const int size = p->pArg->astrType.size();
								dst_file.Write("(");
								for (int i=0; i<size; i++){
									if (i!=0) dst_file.Write(", ");
									line = p->pArg->astrType[i];
									LinkRegisteredClass(line);	// ここでHTML用に整形される
									MarkReservedWord(line);	// ここで文末に" "が追加される
									dst_file.Write(line);
									line = p->pArg->astrName[i];
									CStringScanner::Replace(line,"&","&amp;");
									CStringScanner::Replace(line,"<","&lt;");
									CStringScanner::Replace(line,">","&gt;");
									CStringScanner::Replace(line,"\n","<BR>");
									dst_file.Write(line);
								}
								dst_file.Write(")");
							}
							if (p->bPureVirtual) dst_file.Write("&nbsp;=&nbsp;0");
						} break;
					case 1:
						{	// CMemberVariableDesc
							CMemberVariableDesc* p = (CMemberVariableDesc*)pMember;
							line = p->strType;
							LinkRegisteredClass(line);	// ここでHTML用に整形される
							MarkReservedWord(line);	// ここで文末に" "が追加される
							dst_file.Write(line);
							line = p->strName;
							CStringScanner::Replace(line,"&","&amp;");
							CStringScanner::Replace(line,"<","&lt;");
							CStringScanner::Replace(line,">","&gt;");
							CStringScanner::Replace(line,"\n","<BR>");
							dst_file.Write(line);
						} break;
					case 2:
						{	// CEnumDesc
							CEnumDesc* p = (CEnumDesc*)pMember;
							const int size = p->astrMember.size();
							string line = "enum ";
							MarkReservedWord(line);
							dst_file.Write(line + p->strName + "<BR>{<BR>");
							for (int i=0; i<size; i++){
								if (i!=0) dst_file.Write(",<BR>");
								line = p->astrMember[i];
								CStringScanner::Replace(line,"&","&amp;");
								CStringScanner::Replace(line,"<","&lt;");
								CStringScanner::Replace(line,">","&gt;");
								CStringScanner::Replace(line,"\n","<BR>");
								CStringScanner::Replace(line,"="," = ");
								dst_file.Write("　　"+line);
							}
							dst_file.Write("<BR>}");
						} break;
					default:; // ありえないはず
					}
					dst_file.Write("</DIV>\n");
					itMember++;
				}
				comment = desc.m_apMember[i]->strComment;
				CStringScanner::Replace(comment,"&","&amp;");
				CStringScanner::Replace(comment,"<","&lt;");
				CStringScanner::Replace(comment,">","&gt;");
				CStringScanner::Replace(comment,"\n","<BR>");
				dst_file.Write("<DIV class=\"comment_block\">");
				dst_file.Write(comment + "</DIV>\n");
				dst_file.Write("</DIV>\n");
			}
		}
		dst_file.Write("<HR><BR>\n");
		it++;
	}// end while (it != m_apClassDesc.end())

	//	前の開いてたら閉じる
	if (dst_file.GetFilePtr()!=NULL){
		dst_file.Write("</BODY></HTML>");
		dst_file.Close();
	}

	// CSSをコピーする
	if(!CFile::PathFileExists(strDstPath + "document.css"))
	{
		CFile file;
		if (!file.Read(CFile::GetCurrentDir()+"template/document.css")){
			file.WriteBack(strDstPath + "document.css");
		}
	}
	// JSをコピーする
	if (!CFile::PathFileExists(strDstPath + "document.js"))
	{
		CFile file;
		if (!file.Read(CFile::GetCurrentDir()+"template/document.js")){
			file.WriteBack(strDstPath + "document.js");
		}
	}

	return 0;
}

void CScnConvert::LinkRegisteredClass(string& line)
{
	LPCSTR lpStr = line.c_str();
	string dst;
	string word;
	while (true){
		while (true){
			if (*lpStr==' '||*lpStr=='<'||*lpStr=='>'||*lpStr==','||*lpStr=='*'||*lpStr=='&'||*lpStr=='\0'){
				break;
			}
			word += *lpStr++;
		}
		const CClassDesc* p = FindClass(word);
		if (p){
			dst += "$link_start_1$";
			dst += GetAddress(p);
			dst += "\" target=_self$link_start_2$";
			dst += word;
			dst += "$link_end$";
		} else {
			dst += word;
		}
		if (*lpStr=='\0') break;
		word.erase();
		dst += *lpStr++;
	}
	CStringScanner::Replace(dst,"&","&amp;");
	CStringScanner::Replace(dst,"<","&lt;");
	CStringScanner::Replace(dst,">","&gt;");
	CStringScanner::Replace(dst,"$link_start_1$","<A class=\"class_link\" href=\"");
	CStringScanner::Replace(dst,"$link_start_2$",">");
	CStringScanner::Replace(dst,"$link_end$","</A>");

	line = dst;
}

void CScnConvert::MarkReservedWord(string& word)
{
	word += " ";
	const int size = NELEMS(aReservedWordTable);
	string line;
	for (int i=0; i<size; i++){
		line = aReservedWordTable[i];
		CStringScanner::Replace(word, (line+" ").c_str(), ("<SPAN class=\"reserved_word\">"+line+"</SPAN> ").c_str());
		CStringScanner::Replace(word, (line+"*").c_str(), ("<SPAN class=\"reserved_word\">"+line+"</SPAN>*").c_str());
		CStringScanner::Replace(word, (line+"&amp;").c_str(), ("<SPAN class=\"reserved_word\">"+line+"</SPAN>&amp;").c_str());
	}
}

string CScnConvert::GetAddress(const CClassDesc* pClass)
{
	string temp = pClass->strHeaderFilename;
	CStringScanner::Replace(temp,"/","!");
	CStringScanner::Replace(temp,".","_");
	return temp + ".html#" + pClass->strName;
}

const CClassDesc* CScnConvert::FindClass(const string& strClassName)
{
	map<string,CClassDesc*>::const_iterator it = m_mapNameToClass.find(strClassName);
	if (it!=m_mapNameToClass.end()) return it->second;
	return NULL;
}

bool CScnConvert::SkipComment(LPCSTR& pSrcStr, bool bSkipSpecialComment/*=true*/, string* lpComment/*=NULL*/)
{
	// 運命共同体(笑)
	LPCSTR& lpSrcPos = pSrcStr;
	LPCSTR lpSrcPosBackup = lpSrcPos;	// 元に戻せるように

	// これらはスキップしないように
	if (!bSkipSpecialComment&&IsToken(lpSrcPos, "/**")){ lpSrcPos = lpSrcPosBackup;  return false; }
	ef (!bSkipSpecialComment&&IsToken(lpSrcPos, "///")){ lpSrcPos = lpSrcPosBackup;  return false; }

	// こいつらはスキップ対象
	if (IsToken(lpSrcPos, "/*")){
		SkipTo(lpSrcPos, "*/", lpComment);
		return true;
	}
	ef (IsToken(lpSrcPos, "//")){
		if (SkipTo(lpSrcPos, "\n", lpComment)==0){	// 一行取りだし
			lpSrcPos--;
		}
		ef (SkipTo(lpSrcPos, "\0", lpComment)==0){	// 一行取りだし
			lpSrcPos--;
		}
		return true;
	}

	return false;
}

bool CScnConvert::ExtractSpecialComment(LPCSTR& pSrcStr, string& strComment)
{
	// 運命共同体(笑)
	LPCSTR& lpSrcPos = pSrcStr;

	if (IsToken(lpSrcPos, "/**")){
		SkipTo(lpSrcPos, "*/", &strComment);
		while (*strComment.begin()==' '||*strComment.begin()=='\t'||*strComment.begin()=='\n'){ strComment.erase(0,1); }
		return true;
	}
	ef (IsToken(lpSrcPos, "///")){
		if (SkipTo(lpSrcPos, "\n", &strComment)==0){	// 一行取りだし
			lpSrcPos--;
		}
		ef (SkipTo(lpSrcPos, "\0", &strComment)==0){	// 一行取りだし
			lpSrcPos--;
		}
		while (*strComment.begin()==' '||*strComment.begin()=='\t'||*strComment.begin()=='\n'){ strComment.erase(0,1); }
		return true;
	}

	return false;
}

bool CScnConvert::ParseClass(smart_ptr<CClassDesc> pOuterClass/*=NULL*/)
{
	// 運命共同体(笑)
	LPCSTR& lpSrcPos = m_lpSrcPos;
	LPCSTR lpSrcPosBackup = m_lpSrcPos;	// 元に戻せるように

	// テンプレート指定がないか探す
	smart_ptr<CArgDesc> pTemplateArg = ParseTemplate();
	// "class"か"struct"があるかどうかを調べる
	while (SkipComment(lpSrcPos,false)){};	// 普通のコメントをとばす
	bool bIsClass = false;
	if (IsToken(lpSrcPos, "class")&&(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ bIsClass = true; }
	ef (IsToken(lpSrcPos, "struct")&&(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ bIsClass = false; }
	else { m_lpSrcPos = lpSrcPosBackup;  return false; }	// 違ったみたい

	// 解析していって、こいつに放り込む
	smart_ptr<CClassDesc> pCurClass;
	pCurClass.Add();
	pCurClass->bIsClass = bIsClass;
	pCurClass->pTemplateArg = pTemplateArg;
	// ヘッダファイル名
	pCurClass->strHeaderFilename = m_strFilename;
	// 外部クラスの設定
	pCurClass->pOuterClass = pOuterClass;

	// クラス名を得る
	bool bExistSuperClass = false;	// 継承指定があるかどうか
	while (true)
	{
		SkipSpace(lpSrcPos);
		if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
		// スコープ指定子
		if (IsToken(lpSrcPos, "::")) { pCurClass->strName += "::";  continue; }
		// 継承指定だ
		if (IsToken(lpSrcPos, ":")){ bExistSuperClass = true;  break; }
		// 終わりかい
		if (IsToken(lpSrcPos, "{")) break;
		// なんや空宣言かいな…
		if (IsToken(lpSrcPos, ";")){ m_lpSrcPos = lpSrcPosBackup;  return false; }	
		// 関数の戻り値のようだ
		if (IsToken(lpSrcPos, "*")||IsToken(lpSrcPos, "&")){ m_lpSrcPos = lpSrcPosBackup;  return false; }	
		// 文字列終了？？そんなアホな…
		if (*lpSrcPos=='\0'){ m_lpSrcPos = lpSrcPosBackup;  return false; }
		// 名前だー
		pCurClass->strName += *lpSrcPos++;
	}
	// 継承指定があるなら解析
	while (bExistSuperClass)
	{
		SkipSpace(lpSrcPos);
		if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
		smart_ptr<CSuperClassDesc> pSuperClass;  pSuperClass.Add();
		// 派生先を代入
		pSuperClass->pSubClass = pCurClass;
		// 継承タイプを取得 virtualとpublic/protected/privateの指定に順番は無いからこう並べる^^;
		int nType = 2;
		bool bVirtual = false;
		if (IsToken(lpSrcPos, "virtual")&&(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){
			lpSrcPos++;
			while (SkipComment(lpSrcPos)){};	// 普通のコメントをとばす
			bVirtual = true;
			if (IsToken(lpSrcPos, "public")&&
				(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ nType = 0; }
			ef (IsToken(lpSrcPos, "protected")&&
				(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ nType = 1; }
			ef (IsToken(lpSrcPos, "private")&&
				(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ nType = 2; }
		}
		ef (IsToken(lpSrcPos, "public")&&(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){
			lpSrcPos++;
			while (SkipComment(lpSrcPos)){};	// 普通のコメントをとばす
			nType = 0;
			if (IsToken(lpSrcPos, "virtual")
				&&(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ bVirtual = true; }
		}
		ef (IsToken(lpSrcPos, "protected")&&(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){
			lpSrcPos++;
			while (SkipComment(lpSrcPos)){};	// 普通のコメントをとばす
			nType = 1;
			if (IsToken(lpSrcPos, "virtual")&&
				(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ bVirtual = true; }
		}
		ef (IsToken(lpSrcPos, "private")&&(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){
			lpSrcPos++;
			while (SkipComment(lpSrcPos)){};	// 普通のコメントをとばす
			nType = 2;
			if (IsToken(lpSrcPos, "virtual")&&
				(*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){ bVirtual = true; }
		}
		pSuperClass->nDeriveKindOf = nType;
		pSuperClass->bVirtualDerive = bVirtual;
		// 派生元クラス名を得る
		bool bExit = false;
		while (true){
			SkipSpace(lpSrcPos);
			if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
			// 区切りだ
			if (IsToken(lpSrcPos, ",")) { break; }
			// 終わりかい
			if (IsToken(lpSrcPos, "{")) { bExit = true;  break; }
			// 文字列終了？？そんなアホな…
			if (*lpSrcPos=='\0') { m_lpSrcPos = lpSrcPosBackup;  return false; }
			// 派生元クラス名だー
			pSuperClass->strName += *lpSrcPos++;
		}
		// 登録
		pCurClass->apSuperClass.insert(pSuperClass);
		if (bExit) break;
	}
	// このクラスの宣言終了位置を探す
	LPCSTR lpSrcClassEndPos = lpSrcPos;
	{
		int braket_count = 1;	// "{"の直後のはずだから
		bool bExit = false;
		string dummy;
		while (!bExit){
			// コメントをとばす
			SkipSpace(lpSrcClassEndPos);
			if (SkipComment(lpSrcClassEndPos)) continue;	// 普通のコメントをとばす
			// 文字列終了？？そんなアホな…
			if (*lpSrcClassEndPos=='\0') { m_lpSrcPos = lpSrcPosBackup;  return false; }
			// ブラケットのネストをカウントしながら0になる所を探す
			if (*lpSrcClassEndPos=='{') { braket_count++; }
			ef (*lpSrcClassEndPos=='}' && --braket_count==0) { bExit = true; }
			lpSrcClassEndPos++;
		}	// これでlpClassEndPosは"}"の直後に来ているはず		
	}

	// クラスコメントを探す
	while (SkipComment(lpSrcPos,false)){};
	ExtractSpecialComment(lpSrcPos, pCurClass->strComment);

	// もういいでしょうヽ(´▽｀)ノ
	AddClassDesc(pCurClass);

	// あとはメンバを探していく
	string line, line2;
	m_strCurComment.erase();
	m_nCurAccessType = (pCurClass->bIsClass)?2:0;	// classならprivate structならpublic
	smart_ptr<CMemberDescBlock> pCurMemberBlock(new CMemberDescBlock,true);
	while (lpSrcPos < lpSrcClassEndPos)
	{
		// 空行に来たらコメントをflush
		{
			// この下にある解析関数は、解析後"\n"を残すようにしている（あるいはそうなってしまう）
			// だから、２行取得してSkipSpaceして、両方"\0"なら空行と判断する
			LPCSTR temp1 = lpSrcPos;
			SkipTo(temp1, "\n", &line);		// 現在の行
			SkipTo(temp1, "\n", &line2);	// 次の行
			LPCSTR temp2 = line.c_str();
			LPCSTR temp3 = line2.c_str();
			SkipComment(temp2,false);
			SkipSpace(temp2);
			SkipSpace(temp3);
			if (*temp2=='\0'&&*temp3=='\0'){
				pCurMemberBlock->strComment += m_strCurComment;
				m_strCurComment.erase();
				if (pCurMemberBlock->apMemberDesc.size()!=0){
					pCurClass->AddMember(pCurMemberBlock);
					pCurMemberBlock.Add();
				}
				lpSrcPos = temp1;
				continue;
			}
		}
		// 普通のコメントをとばす
		SkipSpace(lpSrcPos);
		if (SkipComment(lpSrcPos,false)) continue;
		// メンバコメントの探索
		if (ExtractSpecialComment(lpSrcPos, line)){
			m_strCurComment += line + "\n";
			continue;
		}
		// アクセスタイプの探索
		if (IsToken(lpSrcPos, "public:"))    { m_nCurAccessType = 0;  continue; }
		if (IsToken(lpSrcPos, "protected:")) { m_nCurAccessType = 1;  continue; }
		if (IsToken(lpSrcPos, "private:"))   { m_nCurAccessType = 2;  continue; }

		// クラス内クラスの探索
		const int nAccessTypeBackup = m_nCurAccessType;	// 探索の時に上書きされるため
		if (ParseClass(pCurClass)){
			SkipSpace(lpSrcPos);
			while(SkipComment(lpSrcPos)){};	// 普通のコメントをとばす
			// "}"の直後のはずだから";"を探そう。
			if (!IsToken(lpSrcPos, ";")){	// あぁ、メンバ指定があるんだ…
				SkipTo(lpSrcPos, ";", &line);
				CMemberVariableDesc* p = new CMemberVariableDesc;
				(*m_apClassDesc.rbegin())->nAccessType = m_nCurAccessType;
				p->nAccessType = m_nCurAccessType;
				p->strType = (*m_apClassDesc.rbegin())->strName;
				p->strName = line;
				pCurMemberBlock->apMemberDesc.insert(smart_ptr<IMemberDesc>(p,true));
			}
			m_nCurAccessType = nAccessTypeBackup;	// 元に戻す
			continue;
		}
		// enumの探索
		if (ParseEnum(pCurMemberBlock)){
			SkipSpace(lpSrcPos);
			while(SkipComment(lpSrcPos)){};	// 普通のコメントをとばす
			// "}"の直後のはずだから";"を探そう。
			if (!IsToken(lpSrcPos, ";")){	// あぁ、メンバ指定があるんだ…
				const CEnumDesc* pEnumDesc = (CEnumDesc*)(pCurMemberBlock->apMemberDesc.rbegin()->get());
				SkipTo(lpSrcPos, ";", &line);
				CMemberVariableDesc* p = new CMemberVariableDesc;
				p->nAccessType = m_nCurAccessType;
				p->strType = (pEnumDesc->strName=="") ? "enum" : pEnumDesc->strName;
				p->strName = line;
				pCurMemberBlock->apMemberDesc.insert(smart_ptr<IMemberDesc>(p,true));
			}
			continue;
		}

		// メンバ変数・関数の探索
		if (ParseMember(pCurMemberBlock)) continue;

		SkipTo(lpSrcPos, "\n", &line);  lpSrcPos--;	// back to "\n"
	}

	// flushできなかった最後のブロックをflush
	if (pCurMemberBlock->apMemberDesc.size()!=0){
		pCurMemberBlock->strComment += m_strCurComment;
		pCurClass->AddMember(pCurMemberBlock);
	}

	// 終了
	m_lpSrcPos = lpSrcClassEndPos;
	return true;
}

bool CScnConvert::ParseEnum(CMemberDescBlock* pMemberBlock)
{
	LPCSTR lpSrcPos = m_lpSrcPos;
	if (IsToken(lpSrcPos, "enum")&&(*lpSrcPos=='{'||*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){
	} else {
		return false;
	}

	// 列挙子開始を探す
	smart_ptr<CEnumDesc> pEnumDesc;  pEnumDesc.Add();
	pEnumDesc->nAccessType = m_nCurAccessType;
	SkipSpace(lpSrcPos);
	while (*lpSrcPos!='{'){
		if (SkipComment(lpSrcPos)) continue;
		pEnumDesc->strName += *lpSrcPos++;
		SkipSpace(lpSrcPos);
	}
	lpSrcPos++;	// skip '{'

	{
		string enum_member;
		int nStatus = 0;
		while (true){
			SkipSpace(lpSrcPos);
			if (SkipComment(lpSrcPos)) continue;
			// 列挙子１つ終了
			if (*lpSrcPos==',') {
				if (enum_member!="") pEnumDesc->astrMember.push_back(enum_member);
				enum_member.erase();
				lpSrcPos++;
				continue;
			}
			// 列挙子全て終了
			if (*lpSrcPos=='}') {
				if (enum_member!="") pEnumDesc->astrMember.push_back(enum_member);
				lpSrcPos++;
				m_lpSrcPos = lpSrcPos;
				pMemberBlock->apMemberDesc.insert(smart_ptr<IMemberDesc>(pEnumDesc.release(),true));
				return true;
			}
			enum_member += *lpSrcPos++;
		}
	}
	return false;
}

smart_ptr<CArgDesc> CScnConvert::ParseTemplate()
{
	LPCSTR lpSrcPos = m_lpSrcPos;
	if (IsToken(lpSrcPos, "template")&&
		(*lpSrcPos=='<'||*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n')){
	} else {
		return NULL;
	}
	SkipSpace(lpSrcPos);
	if (*lpSrcPos!='<') return NULL;
	lpSrcPos++;	// skip '<'

	smart_ptr<CArgDesc> pArgDesc;  pArgDesc.Add();
	bool bTypeSearch = true;
	bool bSkipSpace = true;
	string line;
	while (true)
	{
		if (bSkipSpace||!bTypeSearch) { SkipSpace(lpSrcPos); }
		// コメントをとばす
		if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
		// 文字列終了？？そんなアホな…
		if (*lpSrcPos=='\0') { return NULL; }
		// ブラケットのネストをカウントしながら0になる所を探す
		if (*lpSrcPos=='<') {
			line += *lpSrcPos++;
			int braket_count = 1;
			while(braket_count>0){	// これ以上は分解しない、めんどいし^^;
				if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
				if (*lpSrcPos=='<') { ++braket_count; }
				if (*lpSrcPos=='>') { --braket_count; }
				line += *lpSrcPos++;
			}
			continue;
		}
		if (*lpSrcPos=='(') {
			line += *lpSrcPos++;
			int braket_count = 1;
			while(braket_count>0){
				if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
				if (*lpSrcPos=='(') { ++braket_count; }
				if (*lpSrcPos==')') { --braket_count; }
				line += *lpSrcPos++;
			}
			continue;
		}
		// 型指定or変数の終わり
		if ((bTypeSearch&&(*lpSrcPos==' '||*lpSrcPos=='\t'))||(!bTypeSearch&&*lpSrcPos==',')){
			// 空白を詰める
			CStringScanner::Replace(line, " ","");
			CStringScanner::Replace(line, "\t","");
			if (bTypeSearch) pArgDesc->astrType.push_back(line);
			else			 pArgDesc->astrName.push_back(line);
			line.erase();
			lpSrcPos++;
			bTypeSearch = !bTypeSearch;
			bSkipSpace = true;
			continue;
		}
		// 引数の終わり
		if (*lpSrcPos=='>') {
			if (line!="") pArgDesc->astrName.push_back(line);
			line.erase();
			lpSrcPos++;
			m_lpSrcPos = lpSrcPos;
			return pArgDesc;
		}
		line += *lpSrcPos++;
		bSkipSpace = false;
	}
	return NULL;	// dummy
}

bool CScnConvert::ParseMember(CMemberDescBlock* pMemberBlock)
{
	// テンプレート関数の可能性がある
	smart_ptr<CArgDesc> pTemplateArg = ParseTemplate();

	LPCSTR lpSrcPos = m_lpSrcPos;
	bool bVariable = true;
	string line;
	string str1;
	string str2;
	string str3;
	// 変数の型 or 関数の戻り値の型 を探索する
	SkipSpace(lpSrcPos);
	while (true)
	{
		// コメントをとばす
		if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
		// 文字列終了？？そんなアホな…
		if (*lpSrcPos=='\0') { return false; }
		// コンストラクタかデストラクタやな
		if (*lpSrcPos=='(') {
			str1 = "";
			str2 = line;
			lpSrcPos++;
			goto parse_arg;
		}
		// ここでこいつらが出るのはおかしい
		if (*lpSrcPos=='{'||*lpSrcPos=='}'||*lpSrcPos==';') { return false; }
		// ブラケットのネストをカウントしながら0になる所を探す
		if (*lpSrcPos=='<') {
			line += *lpSrcPos++;
			int braket_count = 1;
			while(braket_count>0){	// これ以上は分解しない、めんどいし^^;
				SkipSpace(lpSrcPos);
				if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
				if (*lpSrcPos=='(') {
					line += *lpSrcPos++;
					int braket_count2 = 1;
					while(braket_count2>0){
						SkipSpace(lpSrcPos);
						if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
						if (*lpSrcPos=='(') { ++braket_count2; }
						if (*lpSrcPos==')') { --braket_count2; }
						line += *lpSrcPos++;
					}
					continue;
				}
				if (*lpSrcPos=='<') { ++braket_count; }
				if (*lpSrcPos=='>') { --braket_count; }
				line += *lpSrcPos++;
			}
			continue;
		}
		// こいつらは型の属性
		if ((IsToken(lpSrcPos,"const ")||IsToken(lpSrcPos,"const\t")||IsToken(lpSrcPos,"const\n"))) {
			line += "const ";
			continue;
		}
		if ((IsToken(lpSrcPos,"static ")||IsToken(lpSrcPos,"static\t")||IsToken(lpSrcPos,"static\n"))) {
			line += "static ";
			continue;
		}
		if ((IsToken(lpSrcPos,"inline ")||IsToken(lpSrcPos,"inline\t")||IsToken(lpSrcPos,"inline\n"))) {
			line += "inline ";
			continue;
		}
		if ((IsToken(lpSrcPos,"volatile ")||IsToken(lpSrcPos,"volatile\t")||IsToken(lpSrcPos,"volatile\n"))) {
			line += "volatile ";
			continue;
		}
		if ((IsToken(lpSrcPos,"virtual ")||IsToken(lpSrcPos,"virtual\t")||IsToken(lpSrcPos,"virtual\n"))) {
			line += "virtual ";
			continue;
		}
		if ((IsToken(lpSrcPos,"explicit ")||IsToken(lpSrcPos,"explicit\t")||IsToken(lpSrcPos,"explicit\n"))) {
			line += "explicit ";
			continue;
		}
		// 型指定の終わり
		if (*lpSrcPos==' '||*lpSrcPos=='\t'||*lpSrcPos=='\n'){
			lpSrcPos++;
			break;
		}
		line += *lpSrcPos++;
	}
	str1 = line;

	// 変数or関数の名前を探索する
	// 変数or関数かの判断もここで行う
	{
		line.erase();
		while (true)
		{
			SkipSpace(lpSrcPos);
			// コメントをとばす
			if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
			// 文字列終了？？そんなアホな…
			if (*lpSrcPos=='\0') { return false; }
			// 関数やね
			if (IsToken(lpSrcPos,"(")){
				bVariable = false;
				break;
			}
			// "("が来ずに";"が来たから変数やね 
			if (IsToken(lpSrcPos,";")){
				bVariable = true;
				break;
			}
			line += *lpSrcPos++;
		}
	}
	str2 = line;

	// 変数の場合はCMemberVariableDescを追加して帰る
	if (bVariable){
		CMemberVariableDesc* p = new CMemberVariableDesc;
		p->nAccessType = m_nCurAccessType;
		p->strType = str1;
		p->strName = str2;
		pMemberBlock->apMemberDesc.insert(smart_ptr<IMemberDesc>(p,true));
		m_lpSrcPos = lpSrcPos;
		return true;
	}

parse_arg:;
	// 関数だから準備
	smart_ptr<CMemberFunctionDesc> pFuncDesc;  pFuncDesc.Add();
	pFuncDesc->nAccessType		= m_nCurAccessType;
	pFuncDesc->strRetvalType	= str1;
	pFuncDesc->strName			= str2;
	pFuncDesc->pTemplateArg		= pTemplateArg;
	pFuncDesc->pArg.Add();
	CArgDesc* pArgDesc = pFuncDesc->pArg;
	// 関数の引数を解析する --- ParseTemplate関数からコピペして一部改編^^;
	bool bTypeSearch = true;
	bool bSkipSpace = true;
	line.erase();
	while (true)
	{
		if (bSkipSpace||!bTypeSearch) { SkipSpace(lpSrcPos); }
		// コメントをとばす
		if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
		// 文字列終了？？そんなアホな…
		if (*lpSrcPos=='\0') { return false; }
		// ブラケットのネストをカウントしながら0になる所を探す
		if (*lpSrcPos=='<') {
			line += *lpSrcPos++;
			int braket_count = 1;
			while(braket_count>0){	// これ以上は分解しない、めんどいし^^;
				SkipSpace(lpSrcPos);
				if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
				if (*lpSrcPos=='(') {
					line += *lpSrcPos++;
					int braket_count2 = 1;
					while(braket_count2>0){
						SkipSpace(lpSrcPos);
						if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
						if (*lpSrcPos=='(') { ++braket_count2; }
						if (*lpSrcPos==')') { --braket_count2; }
						line += *lpSrcPos++;
					}
					continue;
				}
				if (*lpSrcPos=='<') { ++braket_count; }
				if (*lpSrcPos=='>') { --braket_count; }
				line += *lpSrcPos++;
			}
			continue;
		}
		if (*lpSrcPos=='(') {
			line += *lpSrcPos++;
			int braket_count = 1;
			while(braket_count>0){
				SkipSpace(lpSrcPos);
				if (SkipComment(lpSrcPos)) continue;	// 普通のコメントをとばす
				if (*lpSrcPos=='(') { ++braket_count; }
				if (*lpSrcPos==')') { --braket_count; }
				line += *lpSrcPos++;
			}
			continue;
		}
		// こいつらは引数の属性
		if ((IsToken(lpSrcPos,"const ")||IsToken(lpSrcPos,"const\t")||IsToken(lpSrcPos,"const\n"))) {
			line += "const ";
			continue;
		}
		// 型指定or変数の終わり
		if ((bTypeSearch&&(*lpSrcPos==' '||*lpSrcPos=='\t'))||(!bTypeSearch&&*lpSrcPos==',')){
			if (bTypeSearch&&line!="void") pArgDesc->astrType.push_back(line);
			else			 pArgDesc->astrName.push_back(line);
			line.erase();
			lpSrcPos++;
			bTypeSearch = !bTypeSearch;
			bSkipSpace = true;
			continue;
		}
		// 引数の終わり
		if (*lpSrcPos==')') {
			if (line!="") pArgDesc->astrName.push_back(line);
			line.erase();
			lpSrcPos++;
			break;
		}
		if (!bTypeSearch&&(*lpSrcPos=='&'||*lpSrcPos=='*')) {	// ポインタや参照
			line += *lpSrcPos++;
			line += " ";
			continue;
		}
		line += *lpSrcPos++;
		bSkipSpace = false;
	}

	// プロトタイプ宣言ならpushして帰る
	line.erase();
	string comment;
	while (true){
		SkipSpace(lpSrcPos);
		// コメントをとばす
		if (SkipComment(lpSrcPos,false)) continue;	// 普通のコメントをとばす
		// ここにもスペシャルコメントがある
		if (ExtractSpecialComment(lpSrcPos,comment)){
			m_strCurComment += comment + "\n";
			continue;
		}
		if (IsToken(lpSrcPos,";")){
			// お疲れ～
			pFuncDesc->bPureVirtual = (line=="=0");
			pMemberBlock->apMemberDesc.insert(smart_ptr<IMemberDesc>(pFuncDesc.release(),true));
			m_lpSrcPos = lpSrcPos;
			return true;
		}
		if (IsToken(lpSrcPos,"{")){
			// inline関数でっか……
			break;
		}
		line += *lpSrcPos++;
	}

	// inline関数ならば、関数内のコメントを拾う
	int braket_count = 1;	// "{"の直後のはずだから
	bool bExit = false;
	while (!bExit){
		// コメントをとばす
		SkipSpace(lpSrcPos);
		if (SkipComment(lpSrcPos,false)) continue;	// 普通のコメントをとばす
		if (ExtractSpecialComment(lpSrcPos,comment)){	// 関数内のスペシャルコメント
			m_strCurComment += comment + "\n";
			continue;
		}
		// 文字列終了？？そんなアホな…
		if (*lpSrcPos=='\0') { return false; }
		// ブラケットのネストをカウントしながら0になる所を探す
		if (*lpSrcPos=='{') { braket_count++; }
		ef (*lpSrcPos=='}' && --braket_count==0) { bExit = true; }
		lpSrcPos++;
	}	// これで"}"の直後に来ているはず	

	// お疲れ～？
	pMemberBlock->apMemberDesc.insert(smart_ptr<IMemberDesc>(pFuncDesc.release(),true));
	m_lpSrcPos = lpSrcPos;
	// ……なのだが、この直後に";"が来る事があり得る
	SkipSpace(lpSrcPos);
	while (SkipComment(lpSrcPos)){};
	if (IsToken(lpSrcPos, ";")) m_lpSrcPos = lpSrcPos;

	// お疲れ～
	return true;
}

void CScnConvert::Err(string ErrMes){
	CDbg().Out("Error>"+CStringScanner::NumToString(m_nLineNo) + "行目:" + ErrMes);
}

// 文字列系ヘルパ関数(CStringScanner改^^;)
bool CScnConvert::IsToken(LPCSTR &lp, LPCSTR lp2){
// マルチバイトを意識する必要はない
	LPCSTR lpx = lp;
	//	ここで、スペース、タブを飛ばして良いと思われる
	SkipSpace(lpx);
	while (true){
		if (*lp2=='\0') {
			//	すべて一致した！
			lp = lpx;	// ポインタを進める！
			return true;
		}
		if (toupper(*(lpx++))!=toupper(*(lp2++))) return false;	//	ダメじゃん...
	}
}
LRESULT CScnConvert::SkipSpace(LPCSTR &lp){
// マルチバイトを意識する必要はない
	//	スペース、タブ、改行を切り詰める
	while (*lp==0x0a || *lp==0x0d || *lp==' ' || *lp=='\t'){
		lp++;
	}
	if (*lp=='\0') return 1;
	return 0;
}
LRESULT	CScnConvert::SkipTo(LPCSTR &pSrcStr, LPCSTR pTokenStr, string* pSkipedStr/*=NULL*/){
// マルチバイトを意識する必要がある
	LPCSTR pSrcStr_org = pSrcStr;
	bool bKanji = false;
	while (true){
		if (bKanji) {	// ２バイト文字の２バイト目や
			bKanji = false;
			pSrcStr++;
		}
		if (_ismbblead(*pSrcStr)) {	// ２バイト文字の１バイト目や
			bKanji = true;
		}
		if (*pSrcStr=='\0') { //	EOFに出くわした！
			pSrcStr = pSrcStr_org;
			return 1;
		}

		//	if (IsToken(lp,lp2)) break;	//	これ、インライン展開したほうがいいなぁ...
		//	遅そうなので↑を手動でインライン展開する＾＾；
		{
			LPCSTR lp1 = pSrcStr;
			LPCSTR lp2 = pTokenStr;
			while (true){
				if (*lp2=='\0') {
					if (pSkipedStr){	//	文字バッファにコピーしておく。
						const int nSize = pSrcStr - pSrcStr_org;	// トークンの前まで
						pSkipedStr->assign(pSrcStr_org, nSize);
					}
					pSrcStr = lp1;	// ポインタを進める！
					return 0;	//	一致したんで終了
				}
				if (*(lp1++)!=*(lp2++)) break;	//	ダメじゃん...
			}
		}
		pSrcStr++;
	}
	return 0;
}
