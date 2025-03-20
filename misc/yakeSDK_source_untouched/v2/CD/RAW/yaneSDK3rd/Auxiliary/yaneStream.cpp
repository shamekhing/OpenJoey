#include "stdafx.h"
#include "yaneStream.h"
#include "yaneFile.h"

////////////////////////////////////////////////////////////////////////////

void	CTextOutputStreamFile::Clear() {
	if (!m_szFileName.empty()){
		::DeleteFile(m_szFileName.c_str());
	}
}

void __cdecl CTextOutputStreamFile::Out(LPSTR fmt, ... ) {
	if (m_szFileName.empty()) return ;
	CHAR	buf[512];
	::wvsprintf(buf,fmt,(LPSTR)(&fmt+1) );

	int n = ::lstrlen(buf) - 1;
	if (buf[n] == '\n') buf[n] = '\0'; // 改行を潰す

	string filename(CFile::MakeFullName(m_szFileName));
	FILE* file = fopen(filename.c_str(),"aw");
	if (file!=NULL){
		fprintf(file,"%s\n",buf);	//	改行自動追記
		fclose(file);
	}
}	///	文字列を表示する

void	ITextOutputStream::Out(const string& str){
	Out((LPSTR)str.c_str());
}

void	ITextOutputStream::Out(int i){
	CHAR buf[16];
	::wsprintf(buf,"%d",i);
	Out(buf);
};

void __cdecl CTextOutputStreamVersatile::Out(LPSTR fmt, ... )
//	↑可変引数の委譲は不可か？
{
	CHAR	buf[512];
	::wvsprintf(buf,fmt,(LPSTR)(&fmt+1) );
	m_vDevice->Out(buf);
}

//	エラー出力デバイスをひとつ用意
#ifdef USE_ErrorLog		//	CErrorLogクラスを有効にする。
//	エラー出力デバイスをひとつ用意
CTextOutputStreamVersatile Err;
#else
CTextOutputNullDevice Err;
//	↑こないしとけば最適化で消えるでしょう．．
#endif

////////////////////////////////////////////////////////////////////////////
