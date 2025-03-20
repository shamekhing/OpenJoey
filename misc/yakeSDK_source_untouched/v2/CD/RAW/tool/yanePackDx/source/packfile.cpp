//	packfile.cpp
#include "Warning.h"
#include "NameEditDlg.h"
#include "ProgDlg.h"
#include "packfile.h"
#include "yaneLZSS.h"
#include "yaneMacro.h"

#include <string>
#include <vector>
using namespace std;

//	yanePackDx形式
struct CFileInfo {
	CHAR	filename[256];
	DWORD	startpos;	//	seek pos
	size_t	filesize;	//	file size(圧縮サイズ)
	size_t	arcsize;	//	arc size (展開後のファイルサイズ)
};

typedef vector<CFileInfo> CFileInfoArray;

//	yaneSDK2ndのCFileからとってきた＾＾；
string	GetPureFileNameOf(const string filename){
	string purefilename;
	string::iterator pos1;
	// pos1 : 最後に見つかった\か/の位置
	purefilename = filename;
	string::iterator it = purefilename.begin();
	pos1 = NULL;	// not found
	bool bKanji=false;
	while (true) {
		if (*it == '\0') break;
		if (bKanji) { bKanji=false; goto skip; }
//		if (((BYTE)*it>=0x80 && (BYTE)*it<=0xa0) || ((BYTE)*it>=0xe0 && (BYTE)*it<=0xff)) {
		if (_ismbblead(*it)){
			bKanji = true;	//	２バイトコードの１バイト目であった
		} else {
			bKanji = false;	//	２バイトコードの１バイト目ではなかった
			if (*it == '\\' || *it=='/') {
				pos1 = it;
			}
		}
skip:;
		it++;
	}
	if (pos1!=NULL){
		purefilename.erase(purefilename.begin(),pos1+1);
	}
	return purefilename;
}

LRESULT	GetParentDir(string& filename){						//	親フォルダを返す
// e.g. "c:\test1\test2\test3.exe" -> "c:\test1\test2\" -> "c:\test1\" -> "c:\" -> "c:\"
	//	漢字の１バイト目を避ける必要あり...
	//
	//	1. もし、ネットワークドライブで有れば、コンピュータ名をスキップする。
	//	　　そうすることで、ローカルのパスと同じ扱いになる。
	//　　　（\\HOST\PATH までが、ドライブレターであるとする ＞ 仕様
	//	2. \と/を探す。その際に最後に見つかった位置と最後から２番目に見つかった位置を記録しておく
	//	3. 末尾が\でも/でもない場合、ルートならば￥を付加する。ルートでなければ、最後　　　　　に見つかったところまでを親ディレクトリとして返す
	//	4. 末尾が\ か /である場合　、ルートならばそのまま返す。ルートでなければ、最後から２番目に見つかったところまでを親ディレクトリとして返す
	//
	//	2001/07/29 sohei
	//	  ルートに有るファイル名を渡されたら\や/は一つしかないが、ルートを返さないといけない
	//	2001/08/06 sohei
	//	  string が \ を付加することで拡張された場合、バグるのを修正
	//	　ねこ☆あるふぁ～　さん報告有り難うございます。

	if (filename.empty()) return 1;	//	こんなん渡すな^^;

	string::iterator it = filename.begin();
	if (filename.substr(0,2)=="\\\\") {
		//	ネットワークパス
		it += 2;	if (*it == '\0')	return 1;	//	\\ でおわりかいな^^;
		//	このときは、コンピュータ名をスキップする
		bool bKanji=false;
		for (;true; it++) {
			if (*it == '\0') break;
			if (bKanji) { bKanji=false; continue; }
//			if (((BYTE)*it>=0x80 && (BYTE)*it<=0xa0) || ((BYTE)*it>=0xe0 && (BYTE)*it<=0xff)) {
			if (_ismbblead(*it)){
				bKanji = true;	//	２バイトコードの１バイト目であった
			} else {
				bKanji = false;	//	２バイトコードの１バイト目ではなかった
				if (*it == '\\' || *it=='/') {	//	区切り文字
					it++;	//	\\HOST\ の次からとすればローカルと同じ扱いになる
					break;
				}
			}
		}
	} else {
	}
	string::iterator pos1,pos2;	
	// pos1 : 最後に見つかった\か/の位置
	// pos2 : 最後から２番目に見つかった\か/の位置
	pos1 = pos2 = NULL;	// not found

	bool bKanji=false;
	
	while (true) {
		if (*it == '\0') break;
		if (bKanji) { bKanji=false; goto skip; }
//		if (((BYTE)*it>=0x80 && (BYTE)*it<=0xa0) || ((BYTE)*it>=0xe0 && (BYTE)*it<=0xff)) {
		if (_ismbblead(*it)){
			bKanji = true;	//	２バイトコードの１バイト目であった
		} else {
			bKanji = false;	//	２バイトコードの１バイト目ではなかった
			if (*it == '\\' || *it=='/') {	//	区切り文字
				pos2 = pos1;
				pos1 = it;
			}
		}
skip:;
		it++;
	}
	//	2001/08/06	sohei
	//		string が伸張したときに、イテレータが無効になって、
	//		不正な処理を引き起こしていたのを修正。
	//		ねこ☆あるふぁ～　さん有り難うございます。

	//	末尾が\でも/でもないやん
	if ( !bKanji && (*(it-1)!='\\' && *(it-1)!='/')) {
		//	この場合は最後に見つかった所までが必要。
		if (pos1 == NULL) {	//	一つも\が無いならルートなので\を付加する
			filename+='\\';
		} else {
			filename.erase(pos1+1,filename.end());	//	ルートではないので、最後に見つかったところまでを返す
		}
	} else {
		//	この場合は２番目に見つかった所までが必要。
		if (pos2 == NULL) return 1;	//	ルートなんで取り除く必要はない
		filename.erase(pos2+1,filename.end());		//	最後から２番目までを返せばよい
	}
	return 0;
}

//////////////////////////////////////////////////////////////
#include "yaneDir.h"

void CPackFile::Pack() {
	char buf[1024*256];						//	文字バッファ無限大:p
	buf[0] = NULL;							//	これないとおかしなりよる


	//-----------------------------------Meteox---------//
	// 02'04'20	最初に表示されるフォルダのバグ修正		//
	CHAR initialdir[_MAX_PATH];
	::GetCurrentDirectory(_MAX_PATH,initialdir);
	//--------------------------------------------------//


	CFileDialog fdlg(TRUE,"*.*",NULL,OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|
		OFN_OVERWRITEPROMPT,"*.* (すべてのファイル) |*.*||");
	fdlg.m_ofn.lpstrInitialDir = initialdir;	// こいつで起動してね by Meteox
	fdlg.m_ofn.lpstrFile	= buf;
	fdlg.m_ofn.nMaxFile		= 1024*256-1;	//	for WinAPI's Bug
	if (fdlg.DoModal()!=IDOK) return ;

	size_t filepos = 0;
	CFileInfoArray fa;


	//-----------------------------------Meteox---------//
	// 02'04'14	プログレスバーの追加  					//
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetRange(0,100);
	dlg.SetStatus("ファイルを読み込んでいます。");

	// 02'04'18 ダイアログボックスの追加				
	NameEditDlg namedlg;
	namedlg.DoModal();
	if (namedlg.CheckCancelButton()) return;
	string sFileName;	// 作成するファイル名
	sFileName = namedlg.NewFileName();
	if (sFileName.empty()) sFileName = "target";
	int n = sFileName.length();
	if (n>4) n -= 4;
	if (n != sFileName.find(".dat")) sFileName += ".dat";

	// 02'04'22 同名ファイルの上書き警告				
	CDir dir;
	{
		string src_path;
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		src_path = buf;
		if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		dir.SetPath(src_path); // 検索パス設定

	}

	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);

	string SourceFile;
	while (dir.FindFile(SourceFile)==0) {
		SourceFile = GetPureFileNameOf(SourceFile);

		if (!strcmp(sFileName.c_str(),SourceFile.c_str())){	
			Warning warn;
			warn.DoModal();
			if (warn.CheckCancelButton()) return;
		}

	}

	// 02'04'21 プログレスバー関連の追加				
	int nCount = 0;
	double	dData = 0;
	double  fullfilesize = 0;
	string sDispFile;

	POSITION pos;
	pos = fdlg.GetStartPosition( );
	while (true) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		string SourceFile;
		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		SourceFile = GetPureFileNameOf(SourceFile);

		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	32文字以上のファイル名は無視
//		if (SourceFile.size()<32) {

		if (fp1.GetLength() !=0) {
				BYTE* lpBuf = new BYTE[fp1.GetLength()];
				fread(lpBuf,fp1.GetLength(),1,fp1.m_pStream);
				fullfilesize += fp1.GetLength();		// bug fixed '00/09/04
				DELETEPTR_SAFE(lpBuf);
		}

		fp1.Close();

		if (pos==NULL) break;
	}
	//													//
	//--------------------------------------------------//


	pos = fdlg.GetStartPosition( );
	while (true) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		string SourceFile;
		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		SourceFile = GetPureFileNameOf(SourceFile);

		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	32文字以上のファイル名は無視
//		if (SourceFile.size()<32) {
			CFileInfo fi;
			ZeroMemory(fi.filename,sizeof(fi.filename));
			strcpy(fi.filename,SourceFile.c_str());
			fi.startpos	= filepos;

		//-------------------------------------------Meteox-----//
		//   02'04'21	プログレスバー関連の追加				//
		sDispFile = fi.filename;
		sDispFile += "を読み込んでいます";
		dlg.SetStatus(sDispFile.data());
		dData += fi.filesize;
		nCount = (int)((100.0*dData/fullfilesize));
		dlg.SetPos(nCount);
		//														//
		//------------------------------------------------------//

			if (fp1.GetLength() !=0) {
				BYTE* lpBuf = new BYTE[fp1.GetLength()];
				fread(lpBuf,fp1.GetLength(),1,fp1.m_pStream);
				// ファイルサイズを取得するためには、実際に圧縮しなければわからない:p
				LRESULT lr;
				CLZSS lzss;
				DWORD dwDstSize; // 圧縮後サイズ
				lr = lzss.Encode(lpBuf,*(BYTE**)NULL,fp1.GetLength(),dwDstSize,false);
				DELETEPTR_SAFE(lpBuf);
				fi.filesize	= fp1.GetLength();
				if (lr==0) {
					fi.arcsize	= dwDstSize;	//	圧縮サイズ
				} else {
					fi.arcsize  = fp1.GetLength();
				}
//				filepos		+= fp1.GetLength();
				filepos		+= fi.arcsize;		// bug fixed '00/09/04
			} else {
				fi.filesize	= fi.arcsize	= 0;
			}
			fa.push_back(fi);					//	add vector

//		}



		fp1.Close();

		if (pos==NULL) break;
	}

	//	ファイル名は取得できたので、次は、これを元にファイルを生成する
	//	まずは、SeekPosと一致させるためにポジション計算をしなくてはならない。


	//---------------------------------------Meteox-----//
	// 02'04'14	プログレスバー関連の追加				//
	nCount = 0;
	dData = 0;
	fullfilesize = 0;

	vector<CFileInfo>::iterator it;

	// 02'04'19 同名ファイルを含むときのエラーを例外処理
	for(it=fa.begin();it!=fa.end();it++){
		if (!sFileName.compare((*it).filename)){
			AfxMessageBox("同名のファイルを含める事が出来ません。");
			return;
		}
	}
	//													//
	//--------------------------------------------------//


	CFile fp2(sFileName.c_str(),CFile::modeWrite | CFile::modeCreate);
	fp2.Write("yanepkDx",8);	// header
	DWORD filenum = fa.size();
	fp2.Write(&filenum,sizeof(filenum));

	for(it=fa.begin();it!=fa.end();it++){
		(*it).startpos += fa.size()*sizeof(CFileInfo) + 12;		// 8==sizeof('yanepack') + sizeof(DWORD)
		fp2.Write(it,sizeof(CFileInfo));	//	header
	}
	for(it=fa.begin();it!=fa.end();it++){


		//-------------------------------------------Meteox-----//
		//   02'04'19	プログレスバー関連の追加				//
		if (dlg.CheckCancelButton()){
			fp2.Close();
			CFile::Remove(sFileName.c_str());
			return;
		}
		//   02'04'14	プログレスバー関連の追加				//
		sDispFile = (*it).filename;
		sDispFile += "を圧縮しています";
		dlg.SetStatus(sDispFile.data());
		dData += (*it).arcsize;
		nCount = (int)((100.0*dData/(double)filepos));
		dlg.SetPos(nCount);
		//														//
		//------------------------------------------------------//


		CFile fp1((*it).filename,CFile::modeRead);
		if ((*it).filesize!=0){
			BYTE *buf = new BYTE[(*it).filesize];
			fp1.Read(buf,(*it).filesize);		//	丸ごと読み込む
			if ((*it).filesize == (*it).arcsize){
				fp2.Write(buf,(*it).filesize);		//	丸ごと書き込む
			} else {
			//	圧縮して渡す:p
				BYTE* lpDstBuf;
				CLZSS lzss;
				DWORD dwDstSize; // 圧縮後サイズ
				lzss.Encode(buf,lpDstBuf,(*it).filesize,dwDstSize,true);
				fp2.Write(lpDstBuf,(*it).arcsize);		//	丸ごと書き込む
				DELETEPTR_SAFE(lpDstBuf);
			}

			DELETEPTR_SAFE(buf);
		}
		fp1.Close();
	}

	if (fa.size()!=0) {
		AfxMessageBox("圧縮処理が終了しました。");
	}

	fp2.Close();
}

// ------ static function ------
string	GetSuffixOf(const string filename){
	string suffix;
	suffix = GetPureFileNameOf(filename);
	// 　　　　↑間にディレクトリ記号があるとまずいのでパス名除去
	string::size_type pos;
	pos = suffix.rfind('.');
	if (pos==string::npos) {
		suffix.erase();
	} else {
		suffix.erase(0,pos+1);
	}
	return suffix;
}	

#pragma comment(lib,"imagehlp.lib")

#include "imagehlp.h"
void CPackFile::Unpack() {
	//-----------------------------------Meteox---------//
	// 02'04'20	最初に表示されるフォルダのバグ修正		//
	CHAR initialdir[_MAX_PATH];
	::GetCurrentDirectory(_MAX_PATH,initialdir);
	//--------------------------------------------------//

	CFileDialog fdlg(TRUE,"*.*",NULL,OFN_HIDEREADONLY|OFN_FILEMUSTEXIST
		,"*.* (すべてのファイル) |*.*||");
	fdlg.m_ofn.lpstrInitialDir = initialdir;	// こいつで起動してね by Meteox
	if (fdlg.DoModal()!=IDOK) return ;


	//-----------------------------------Meteox---------//
	// 02'04'20	プログレスバーの追加  					//
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetRange(0,100);
	dlg.SetStatus("ファイルを読み込んでいます。");
	//													//
	//--------------------------------------------------//


	CFile fp1(fdlg.GetPathName(),CFile::modeRead | CFile::typeBinary);

	string currentdir = fdlg.GetPathName();
	{	//	target.datならばtargetという文字を抽出
		string purefile = GetPureFileNameOf(currentdir);
		string suffix = GetSuffixOf(purefile);
		GetParentDir(currentdir);
		currentdir = currentdir + purefile.substr(0,purefile.length()-suffix.length()-1) + "\\";
	}
//	CHAR buf[_MAX_PATH];
//	::GetCurrentDirectory(_MAX_PATH,buf);
//	currentdir = buf + currentdir;


	// 02'04'22 同名ファイルの上書き警告				
	CDir dir;
	{
		string src_path;
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		src_path = buf;
		if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		dir.SetPath(src_path); // 検索パス設定

	}

	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(true);
	dir.EnablePackFile(false);
	dir.EnableSubdir(false);

	string SourceFile;
	string Check = fdlg.GetPathName();
	{	//	target.datならばtargetという文字を抽出
		string purefile = GetPureFileNameOf(Check);
		string suffix = GetSuffixOf(purefile);
		Check = purefile.substr(0,purefile.length()-suffix.length()-1);
	}
	while (dir.FindFile(SourceFile)==0) {
		SourceFile = GetPureFileNameOf(SourceFile);
		if (!strcmp(Check.c_str(),SourceFile.c_str())){
			Warning warn;
			warn.DoModal();
			if (warn.CheckCancelButton()) return;
		}

	}

	char theID[9];
	fp1.Read(theID,8);	// identifier
	theID[8]='\0';		// delimiter
	if (strcmp("yanepkDx",theID)) return ; // 一致せず

	DWORD filenum;
	fp1.Read(&filenum,sizeof(DWORD));

	CFileInfoArray fa;

	// ファイルinfoの読み込み
	for(int i=0;i<filenum;i++){
		CFileInfo fi;
		fp1.Read(&fi,sizeof(fi));
		fa.push_back(fi);
	}


	//---------------------------------------Meteox-----//
	// 02'04'20	プログレスバー関連の追加				//
	int nCount = 0;
	double	dData = 0;
	string sDispFile;

	// 02'04'20 ファイル全体の大きさを指す、filepos追加	
	size_t filepos = 0;

	vector<CFileInfo>::iterator it;

	for(it=fa.begin();it!=fa.end();it++){
		filepos += (*it).filesize;
	}
	//													//
	//--------------------------------------------------//

	
	for(it=fa.begin();it!=fa.end();it++){

		//-------------------------------------------Meteox-----//
		//   02'04'20	プログレスバー関連の追加				//
		sDispFile = (*it).filename;
		sDispFile += "を解凍しています";
		dlg.SetStatus(sDispFile.data());
		dData += (*it).filesize;
		nCount = (int)((100.0*dData/(double)filepos));
		dlg.SetPos(nCount);
		//														//
		//------------------------------------------------------//


		string filename;
		filename = currentdir + (*it).filename;
		::MakeSureDirectoryPathExists(filename.c_str());
		CFile fp2(filename.c_str(),CFile::typeBinary | CFile::modeWrite | CFile::modeCreate);
		if ((*it).filesize!=0){

			//-------------------------------------------Meteox-----//
			//   02'04'21	プログレスバー関連の追加				//
			if (dlg.CheckCancelButton()){
				fp2.Close();
				return;
			}
			//														//
			//------------------------------------------------------//


			BYTE *buf = new BYTE[(*it).arcsize];
			fp1.Read(buf,(*it).arcsize);		//	丸ごと読み込む

			if ((*it).filesize == (*it).arcsize){
				//	非圧縮ファイル
				fp2.Write(buf,(*it).filesize);		//	丸ごと書き込む
			} else {
				//	圧縮ファイル
				CLZSS lzss;
				BYTE* lpDst = new BYTE[(*it).filesize];
				lzss.Decode(buf,lpDst,(*it).filesize,false);
				fp2.Write(lpDst,(*it).filesize);	//	丸ごと書き込む
				DELETEPTR_SAFE(lpDst);
			}
			DELETEPTR_SAFE(buf);
		}
		fp2.Close();

	}


	if (fa.size()!=0) {
		AfxMessageBox("解凍処理が終了しました。");
	}
	fp1.Close();
}

#include <direct.h>
#include <shellapi.h>

//  フォルダ選択ダイアログの起動
UINT CPackFile::GetOpenFolderName(string& Buffer, string DefaultFolder, string Title)
{
    BROWSEINFO bi;
    ITEMIDLIST* pidl;
    char szSelectedFolder[MAX_PATH];

    ZeroMemory( &bi, sizeof(BROWSEINFO));
    bi.hwndOwner = NULL;
    //  コールバック関数を指定
    bi.lpfn   = CPackFile::SHBrowseProc;
    //  デフォルトで選択させておくフォルダを指定
    bi.lParam = (LPARAM)DefaultFolder.c_str();
    //  タイトルの指定
    bi.lpszTitle = Title.c_str();//"フォルダを選択してください";
    //  フォルダダイアログの起動
    pidl = SHBrowseForFolder( &bi );
    if( pidl )
    {
        //  選択されたフォルダ名を取得
        SHGetPathFromIDList( pidl, szSelectedFolder );
        _SHFree(pidl);
        if( (DWORD)lstrlen(szSelectedFolder) < MAX_PATH ){
            Buffer = szSelectedFolder;
		}else{
			Buffer = "";
		}
        //  フォルダが選択された
        return IDOK;
    }
    //  フォルダは選択されなかった
    return IDCANCEL;
}

/////////////////////////////////////////////////////////////////////////////
//  SHBrowseForFolder()用コールバック関数

int CALLBACK CPackFile::SHBrowseProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if( uMsg == BFFM_INITIALIZED && lpData )
    {
        //  デフォルトで選択させるパスの指定
        SendMessage( hWnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//  システムが確保したITEMIDLISTを開放しなければならない

void CPackFile::_SHFree(ITEMIDLIST* pidl)
{
    IMalloc*  pMalloc;
    SHGetMalloc( &pMalloc );
    if ( pMalloc )
    {
        pMalloc->Free( pidl );
        pMalloc->Release();
    }
}


void CPackFile::FolderPack() {

	string src_path;
	{
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		UINT ret = GetOpenFolderName(src_path,buf,
			"圧縮してまとめたいフォルダを選択して下さい.\n"
			);
		if(ret==IDOK){
			if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		}else{
			return;
		}
	}

	size_t filepos = 0;
	CFileInfoArray fa;


	//-----------------------------------Meteox---------//
	// 02'04'22 同名ファイルの上書き警告				//
	CDir dir;
	{
		string src_path;
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		src_path = buf;
		if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		dir.SetPath(src_path); // 検索パス設定

	}

	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);


	string target;
	target = src_path;
	GetParentDir(target);
	target =
		src_path.substr(target.length(),src_path.length()-target.length()-1) + ".dat";

	string SourceFile;
	while (dir.FindFile(SourceFile)==0) {
		SourceFile = GetPureFileNameOf(SourceFile);

		if (!strcmp(target.c_str(),SourceFile.c_str())){	
			Warning warn;
			warn.DoModal();
			if (warn.CheckCancelButton()) return;
		}

	}


	// 02'04'14	プログレスバーの追加  					
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetRange(0,100);
	dlg.SetStatus("ファイルを読み込んでいます。");
	
	// 02'04'21 プログレスバー関連の追加				
	int nCount = 0;
	double	dData = 0;
	double  fullfilesize = 0;
	string sDispFile;

	int nFiles = 0;
	while (dir.FindFile(SourceFile)==0) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		nFiles++;

		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	カレントフォルダからの差分をとる
		SourceFile = SourceFile.substr(src_path.length()
			,SourceFile.length()-src_path.length());
			//	GetPureFileNameOf(SourceFile);

			if (fp1.GetLength() !=0) {
				fullfilesize += fp1.GetLength();
			}
//		}
		fp1.Close();

//		if (pos==NULL) break;
	}

	if (nFiles == 0) {
		AfxMessageBox("ファイルが存在しませんでした");
		return ;
	}
	//													//
	//--------------------------------------------------//


	dir.SetPath(src_path); // 検索パス設定
	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);
	
	while (dir.FindFile(SourceFile)==0) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

//		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		
		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	カレントフォルダからの差分をとる
		SourceFile = SourceFile.substr(src_path.length()
			,SourceFile.length()-src_path.length());
			//	GetPureFileNameOf(SourceFile);

//		if (SourceFile.size()<32) {
			CFileInfo fi;
			ZeroMemory(fi.filename,sizeof(fi.filename));
			strcpy(fi.filename,SourceFile.c_str());
			fi.startpos	= filepos;
	
			//-------------------------------------------Meteox-----//
			//   02'04'21	プログレスバー関連の追加				//
			sDispFile = fi.filename;
			sDispFile += "を読み込んでいます";
			dlg.SetStatus(sDispFile.data());
			dData += fp1.GetLength();
			nCount = (int)((100.0*dData/fullfilesize));
			dlg.SetPos(nCount);
			//														//
			//------------------------------------------------------//
	
			if (fp1.GetLength() !=0) {
				BYTE* lpBuf = new BYTE[fp1.GetLength()];
				fread(lpBuf,fp1.GetLength(),1,fp1.m_pStream);
				// ファイルサイズを取得するためには、実際に圧縮しなければわからない:p
				LRESULT lr;
				CLZSS lzss;
				DWORD dwDstSize; // 圧縮後サイズ
				lr = lzss.Encode(lpBuf,*(BYTE**)NULL,fp1.GetLength(),dwDstSize,false);
				DELETEPTR_SAFE(lpBuf);
				fi.filesize	= fp1.GetLength();
				if (lr==0) {
					fi.arcsize	= dwDstSize;	//	圧縮サイズ
				} else {
					fi.arcsize  = fp1.GetLength();
				}
				filepos		+= fi.arcsize;		// bug fixed '00/09/04
			} else {
				fi.filesize	= fi.arcsize	= 0;
			}
			fa.push_back(fi);					//	add vector
//		}
		fp1.Close();

//		if (pos==NULL) break;
	}

	//	ファイル名は取得できたので、次は、これを元にファイルを生成する
	//	まずは、SeekPosと一致させるためにポジション計算をしなくてはならない。

	//---------------------------------------Meteox-----//
	// 02'04'14	プログレスバー関連の追加				//
	nCount = 0;
	dData = 0;
	//													//
	//--------------------------------------------------//


	CFile fp2(target.c_str(),CFile::modeWrite | CFile::modeCreate);
	fp2.Write("yanepkDx",8);	// header
	DWORD filenum = fa.size();
	fp2.Write(&filenum,sizeof(filenum));

	vector<CFileInfo>::iterator it;
	for(it=fa.begin();it!=fa.end();it++){
		(*it).startpos += fa.size()*sizeof(CFileInfo) + 12;		// 8==sizeof('yanepack') + sizeof(DWORD)
		fp2.Write(it,sizeof(CFileInfo));	//	header
	}
	for(it=fa.begin();it!=fa.end();it++){


		//-------------------------------------------Meteox-----//
		//   02'04'19	プログレスバー関連の追加				//
		if (dlg.CheckCancelButton()){
			fp2.Close();
			CFile::Remove(target.c_str());
			return;
		}
		//   02'04'14	プログレスバー関連の追加				
		sDispFile = (*it).filename;
		sDispFile += "を圧縮しています";
		dlg.SetStatus(sDispFile.data());
		dData += (*it).arcsize;
		nCount = (int)((100.0*dData/(double)filepos));
		dlg.SetPos(nCount);
		//														//
		//------------------------------------------------------//


		string name;
		name = (src_path + (*it).filename);
		CFile fp1(name.c_str(),CFile::modeRead);
		if ((*it).filesize!=0){
			BYTE *buf = new BYTE[(*it).filesize];
			fp1.Read(buf,(*it).filesize);		//	丸ごと読み込む
			if ((*it).filesize == (*it).arcsize){
				fp2.Write(buf,(*it).filesize);		//	丸ごと書き込む
			} else {
			//	圧縮して渡す:p
				BYTE* lpDstBuf;
				CLZSS lzss;
				DWORD dwDstSize; // 圧縮後サイズ
				lzss.Encode(buf,lpDstBuf,(*it).filesize,dwDstSize,true);
				fp2.Write(lpDstBuf,(*it).arcsize);		//	丸ごと書き込む
				DELETEPTR_SAFE(lpDstBuf);
			}
			DELETEPTR_SAFE(buf);
		}

		fp1.Close();
	}

	if (fa.size()!=0) {
		AfxMessageBox("圧縮処理が終了しました。");
	}
	fp2.Close();

}


// 02'04'13	コピペで圧縮処理だけ削ってます ---Meteox---//
void CPackFile::PurePack(void) {
	//-----------------------------------Meteox---------//
	// 02'04'20	最初に表示されるフォルダのバグ修正		//
	CHAR initialdir[_MAX_PATH];
	::GetCurrentDirectory(_MAX_PATH,initialdir);
	//--------------------------------------------------//

	char buf[1024*256];						//	文字バッファ無限大:p
	buf[0] = NULL;							//	これないとおかしなりよる
	
	CFileDialog fdlg(TRUE,"*.*",NULL,OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|
		OFN_OVERWRITEPROMPT,"*.* (すべてのファイル) |*.*||");
	fdlg.m_ofn.lpstrInitialDir = initialdir;	// こいつで起動してね by Meteox
	fdlg.m_ofn.lpstrFile	= buf;
	fdlg.m_ofn.nMaxFile		= 1024*256-1;	//	for WinAPI's Bug
	if (fdlg.DoModal()!=IDOK) return ;

	size_t filepos = 0;
	CFileInfoArray fa;
	

	//-----------------------------------Meteox---------//
	// 02'04'18 ダイアログボックスの追加				//				
	NameEditDlg namedlg;
	namedlg.DoModal();
	if (namedlg.CheckCancelButton()) return;
	string sFileName;
	sFileName = namedlg.NewFileName();
	if (sFileName.empty()) sFileName = "target";
	int n = sFileName.length();
	if (n>4) n -= 4;
	if (n != sFileName.find(".dat")) sFileName += ".dat";

	// 02'04'22 同名ファイルの上書き警告				
	CDir dir;
	{
		string src_path;
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		src_path = buf;
		if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		dir.SetPath(src_path); // 検索パス設定

	}

	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);

	string SourceFile;
	while (dir.FindFile(SourceFile)==0) {
		SourceFile = GetPureFileNameOf(SourceFile);

		if (!strcmp(sFileName.c_str(),SourceFile.c_str())){	
			Warning warn;
			warn.DoModal();
			if (warn.CheckCancelButton()) return;
		}

	}

	// 02'04'14	プログレスバーの追加  					
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetRange(0,100);
	dlg.SetStatus("ファイルを読み込んでいます。");

	// 02'04'21 プログレスバー関連の追加				
	int nCount = 0;
	double	dData = 0;
	double  fullfilesize = 0;
	string sDispFile;

	POSITION pos;
	pos = fdlg.GetStartPosition( );
	while (true) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		string SourceFile;
		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		SourceFile = GetPureFileNameOf(SourceFile);

		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	32文字以上のファイル名は無視
//		if (SourceFile.size()<32) {

		if (fp1.GetLength() !=0) {
				BYTE* lpBuf = new BYTE[fp1.GetLength()];
				fread(lpBuf,fp1.GetLength(),1,fp1.m_pStream);
				fullfilesize += fp1.GetLength();		// bug fixed '00/09/04
				DELETEPTR_SAFE(lpBuf);
		}

		fp1.Close();

		if (pos==NULL) break;
	}
	//													//
	//--------------------------------------------------//


	pos = fdlg.GetStartPosition( );
	while (true) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		string SourceFile;
		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		SourceFile = GetPureFileNameOf(SourceFile);

		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	32文字以上のファイル名は無視
//		if (SourceFile.size()<32) {
			CFileInfo fi;
			ZeroMemory(fi.filename,sizeof(fi.filename));
			strcpy(fi.filename,SourceFile.c_str());
			fi.startpos	= filepos;


		//-------------------------------------------Meteox-----//
		//   02'04'21	プログレスバー関連の追加				//
		sDispFile = fi.filename;
		sDispFile += "を読み込んでいます";
		dlg.SetStatus(sDispFile.data());
		dData += fi.filesize;
		nCount = (int)((100.0*dData/fullfilesize));
		dlg.SetPos(nCount);
		//														//
		//------------------------------------------------------//
	
	
			if (fp1.GetLength() !=0) {
				fi.filesize	= fp1.GetLength();
				fi.arcsize  = fp1.GetLength();
//				filepos		+= fp1.GetLength();
				filepos		+= fi.arcsize;		// bug fixed '00/09/04
			} else {
				fi.filesize	= fi.arcsize	= 0;
			}
			fa.push_back(fi);					//	add vector
//		}
		fp1.Close();

		if (pos==NULL) break;
	}

	//	ファイル名は取得できたので、次は、これを元にファイルを生成する
	//	まずは、SeekPosと一致させるためにポジション計算をしなくてはならない。


	//---------------------------------------Meteox-----//
	// 02'04'14	プログレスバー関連の追加				//
	nCount = 0;
	dData = 0;

	vector<CFileInfo>::iterator it;

	// 02'04'19 同名ファイルを含むときのエラーを例外処理
	for(it=fa.begin();it!=fa.end();it++){
		if (!sFileName.compare((*it).filename)){
			AfxMessageBox("同名のファイルを含める事が出来ません。");
			return;
		}
	}
	//													//
	//--------------------------------------------------//


	CFile fp2(sFileName.c_str(),CFile::modeWrite | CFile::modeCreate);
	fp2.Write("yanepkDx",8);	// header
	DWORD filenum = fa.size();
	fp2.Write(&filenum,sizeof(filenum));

	for(it=fa.begin();it!=fa.end();it++){
		(*it).startpos += fa.size()*sizeof(CFileInfo) + 12;		// 8==sizeof('yanepack') + sizeof(DWORD)
		fp2.Write(it,sizeof(CFileInfo));	//	header
	}
	for(it=fa.begin();it!=fa.end();it++){


		//-------------------------------------------Meteox-----//
		//   02'04'19	プログレスバー関連の追加				//
		if (dlg.CheckCancelButton()){
			fp2.Close();
			CFile::Remove(sFileName.c_str());
			return;
		}
		//   02'04'14	プログレスバー関連の追加				
		sDispFile = (*it).filename;
		sDispFile += "を統合しています";
		dlg.SetStatus(sDispFile.data());
		dData += (*it).arcsize;
		nCount = (int)((100.0*dData/(double)filepos));
		dlg.SetPos(nCount);
		//														//
		//------------------------------------------------------//


		CFile fp1((*it).filename,CFile::modeRead);
		if ((*it).filesize!=0){
			BYTE *buf = new BYTE[(*it).filesize];
			fp1.Read(buf,(*it).filesize);		//	丸ごと読み込む
			fp2.Write(buf,(*it).filesize);		//	丸ごと書き込む
			DELETEPTR_SAFE(buf);
		}

		fp1.Close();
	}

	if (fa.size()!=0) {
		AfxMessageBox("ファイルの統合が終了しました。");
	}
	fp2.Close();
}


// 02'04'13 コピペで圧縮処理だけ削ってます ---Meteox---//
void CPackFile::FolderPurePack() {

	string src_path;
	{
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		UINT ret = GetOpenFolderName(src_path,buf,
			"非圧縮でまとめたいフォルダを選択して下さい.\n"
			);
		if(ret==IDOK){
			if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		}else{
			return;
		}
	}

	size_t filepos = 0;
	CFileInfoArray fa;

	//-----------------------------------Meteox---------//
	// 02'04'22 同名ファイルの上書き警告				//
	CDir dir;
	{
		string src_path;
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		src_path = buf;
		if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		dir.SetPath(src_path); // 検索パス設定

	}

	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);

	string target;
	target = src_path;
	GetParentDir(target);
	target =
		src_path.substr(target.length(),src_path.length()-target.length()-1) + ".dat";

	string SourceFile;
	while (dir.FindFile(SourceFile)==0) {
		SourceFile = GetPureFileNameOf(SourceFile);

		if (!strcmp(target.c_str(),SourceFile.c_str())){	
			Warning warn;
			warn.DoModal();
			if (warn.CheckCancelButton()) return;
		}

	}

	// 02'04'14	プログレスバーの追加  					
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetRange(0,100);
	dlg.SetStatus("ファイルを読み込んでいます。");

	// 02'04'21 プログレスバー関連の追加				
	int nCount = 0;
	double	dData = 0;
	double  fullfilesize = 0;
	string sDispFile;

	dir.SetPath(src_path); // 検索パス設定
	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);

	int nFiles = 0;
	while (dir.FindFile(SourceFile)==0) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		nFiles++;

		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	カレントフォルダからの差分をとる
		SourceFile = SourceFile.substr(src_path.length()
			,SourceFile.length()-src_path.length());
			//	GetPureFileNameOf(SourceFile);

			if (fp1.GetLength() !=0) {
				fullfilesize += fp1.GetLength();
			}
//		}
		fp1.Close();

//		if (pos==NULL) break;
	}

	if (nFiles == 0) {
		AfxMessageBox("ファイルが存在しませんでした");
		return ;
	}
	//													//
	//--------------------------------------------------//


	dir.SetPath(src_path); // 検索パス設定
	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);

	while (dir.FindFile(SourceFile)==0) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

//		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		
		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	カレントフォルダからの差分をとる
		SourceFile = SourceFile.substr(src_path.length()
			,SourceFile.length()-src_path.length());
			//	GetPureFileNameOf(SourceFile);

//		if (SourceFile.size()<32) {
			CFileInfo fi;
			ZeroMemory(fi.filename,sizeof(fi.filename));
			strcpy(fi.filename,SourceFile.c_str());
			fi.startpos	= filepos;

			//-------------------------------------------Meteox-----//
			//   02'04'21	プログレスバー関連の追加				//
			sDispFile = fi.filename;
			sDispFile += "を読み込んでいます";
			dlg.SetStatus(sDispFile.data());
			dData += fp1.GetLength();
			nCount = (int)((100.0*dData/fullfilesize));
			dlg.SetPos(nCount);
			//														//
			//------------------------------------------------------//
	
			if (fp1.GetLength() !=0) {
				fi.filesize	= fp1.GetLength();
				fi.arcsize	= fp1.GetLength();
				filepos		+= fi.arcsize;		// bug fixed '00/09/04
			} else {
				fi.filesize	= fi.arcsize	= 0;
			}
			fa.push_back(fi);					//	add vector
//		}
		fp1.Close();

//		if (pos==NULL) break;
	}

	//	ファイル名は取得できたので、次は、これを元にファイルを生成する
	//	まずは、SeekPosと一致させるためにポジション計算をしなくてはならない。

	//---------------------------------------Meteox-----//
	// 02'04'14	プログレスバー関連の追加				//
	nCount = 0;
	dData = 0;
	//													//
	//--------------------------------------------------//


	CFile fp2(target.c_str(),CFile::modeWrite | CFile::modeCreate);
	fp2.Write("yanepkDx",8);	// header
	DWORD filenum = fa.size();
	fp2.Write(&filenum,sizeof(filenum));

	vector<CFileInfo>::iterator it;
	for(it=fa.begin();it!=fa.end();it++){
		(*it).startpos += fa.size()*sizeof(CFileInfo) + 12;		// 8==sizeof('yanepack') + sizeof(DWORD)
		fp2.Write(it,sizeof(CFileInfo));	//	header
	}
	for(it=fa.begin();it!=fa.end();it++){

		//-------------------------------------------Meteox-----//
		//   02'04'19	プログレスバー関連の追加				//
		if (dlg.CheckCancelButton()){
			fp2.Close();
			CFile::Remove(target.c_str());
			return;
		}
		//   02'04'14	プログレスバー関連の追加
		sDispFile = (*it).filename;
		sDispFile += "を統合しています";
		dlg.SetStatus(sDispFile.data());
		dData += (*it).arcsize;
		nCount = (int)((100.0*dData/(double)filepos));
		dlg.SetPos(nCount);
		//														//
		//------------------------------------------------------//

	
		string name;
		name = (src_path + (*it).filename);
		CFile fp1(name.c_str(),CFile::modeRead);
		if ((*it).filesize!=0){
			BYTE *buf = new BYTE[(*it).filesize];
			fp1.Read(buf,(*it).filesize);		//	丸ごと読み込む
			fp2.Write(buf,(*it).filesize);		//	丸ごと書き込む
			DELETEPTR_SAFE(buf);
		}

		fp1.Close();
	}

	if (fa.size()!=0) {
		AfxMessageBox("ファイルの統合が終了しました。");
	}
	fp2.Close();

}



// 02'04'20	コピペでちょこちょこいじってます ---Meteox-----------//
// 圧縮データと非圧縮データを、いったん解凍して一つにまとめてます//
void CPackFile::PackOne() {
	//-----------------------------------Meteox---------//
	// 02'04'20	最初に表示されるフォルダのバグ修正		//
	CHAR initialdir[_MAX_PATH];
	::GetCurrentDirectory(_MAX_PATH,initialdir);
	//--------------------------------------------------//


	char buf[1024*256];						//	文字バッファ無限大:p
	buf[0] = NULL;							//	これないとおかしなりよる
	
	CFileDialog fdlg(TRUE,"*.*",NULL,OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|
		OFN_OVERWRITEPROMPT,"*.* (すべてのファイル) |*.*||");
	fdlg.m_ofn.lpstrInitialDir = initialdir;	// こいつで起動してね by Meteox
	fdlg.m_ofn.lpstrFile	= buf;
	fdlg.m_ofn.nMaxFile		= 1024*256-1;	//	for WinAPI's Bug
	if (fdlg.DoModal()!=IDOK) return ;


	//-----------------------------------Meteox---------//
	// 02'04'14	プログレスバーの追加  					//
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetRange(0,100);
	dlg.SetStatus("ファイルを読み込んでいます。");

	// 02'04'18 ダイアログボックスの追加				
	NameEditDlg namedlg;
	namedlg.DoModal();
	if (namedlg.CheckCancelButton()) return;
	string sFileName;	// 作成するファイル名
	sFileName = namedlg.NewFileName();
	if (sFileName.empty()) sFileName = "target";
	int n = sFileName.length();
	if (n>4) n -= 4;
	if (n != sFileName.find(".dat")) sFileName += ".dat";

	// 02'04'22 同名ファイルの上書き警告				
	CDir dir;
	{
		string src_path;
		CHAR buf[_MAX_PATH];
		::GetCurrentDirectory(_MAX_PATH,buf);
		src_path = buf;
		if(src_path[src_path.length()-1]!='\\') { src_path += "\\"; }
		dir.SetPath(src_path); // 検索パス設定

	}

	dir.SetFindFile("*"); // 検索ファイル名設定
	dir.EnableDirListup(false);
	dir.EnablePackFile(false);
	dir.EnableSubdir(true);

	string SourceFile;
	while (dir.FindFile(SourceFile)==0) {
		SourceFile = GetPureFileNameOf(SourceFile);

		if (!strcmp(sFileName.c_str(),SourceFile.c_str())){	
			Warning warn;
			warn.DoModal();
			if (warn.CheckCancelButton()) return;
		}

	}

	// 02'04'21 プログレスバー関連の追加				
	int nCount = 0;
	double	dData = 0;
	double  fullfilesize = 0;
	string sDispFile;

	POSITION pos;
	pos = fdlg.GetStartPosition( );
	int nFile = 0;	
	while (true) {
		nFile++;

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		string SourceFile;
		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		SourceFile = GetPureFileNameOf(SourceFile);
		
		//-----------------------------------Meteox---------//
		// 02'04'21 同名ファイルを含むときのエラーを例外処理//
		if (!strcmp(sFileName.c_str(),SourceFile.c_str())){
			AfxMessageBox("同名のファイルを含める事が出来ません。");
			return;
		}
		//													//
		//--------------------------------------------------//

		CStdioFile fp1(SourceFile.c_str(),CFile::modeRead|CFile::typeBinary);

		//	32文字以上のファイル名は無視
//		if (SourceFile.size()<32) {

		if (fp1.GetLength() !=0) {
				BYTE* lpBuf = new BYTE[fp1.GetLength()];
				fread(lpBuf,fp1.GetLength(),1,fp1.m_pStream);
				fullfilesize += fp1.GetLength();		// bug fixed '00/09/04
				DELETEPTR_SAFE(lpBuf);
		}

		fp1.Close();

		if (pos==NULL) break;
	}
	//													//
	//--------------------------------------------------//

		
	CFileInfoArray fa;
	CFileInfoArray *fp = new CFileInfoArray[nFile];
	DWORD filenum;

	pos = fdlg.GetStartPosition( );
	nFile = 0;
	while (true) {

		if (dlg.CheckCancelButton()) return; // 02'04'19 プログレスバー関連 by Meteox

		string SourceFile;
		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		SourceFile = GetPureFileNameOf(SourceFile);

		CFile fp1(SourceFile.c_str(),CFile::modeRead | CFile::typeBinary);
	
		string currentdir = SourceFile.c_str();
		{	//	target.datならばtargetという文字を抽出
			string purefile = GetPureFileNameOf(currentdir);
			string suffix = GetSuffixOf(purefile);
			GetParentDir(currentdir);
			currentdir = purefile.substr(0,purefile.length()-suffix.length()-1) + "\\";
		}
//		CHAR buf[_MAX_PATH];
//		::GetCurrentDirectory(_MAX_PATH,buf);
//		currentdir = buf + currentdir;

		char theID[9];
		fp1.Read(theID,8);	// identifier
		theID[8]='\0';		// delimiter
		if (strcmp("yanepkDx",theID)) return ; // 一致せず

		fp1.Read(&filenum,sizeof(DWORD));


		// ファイルinfoの読み込み
		for(int i=0;i<filenum;i++){
			CFileInfo fi;
			fp1.Read(&fi,sizeof(fi));
			
			//-------------------------------------------Meteox-----//
			//   02'04'21	プログレスバー関連の追加				//
			sDispFile = fi.filename;
			sDispFile += "を読み込んでいます";
			dlg.SetStatus(sDispFile.data());
			dData += fi.filesize;
			nCount = (int)((100.0*dData/fullfilesize));
			dlg.SetPos(nCount);
			//														//
			//------------------------------------------------------//
			
			string ss;
			ss = currentdir;
			ss += fi.filename;
			ZeroMemory(fi.filename,sizeof(fi.filename));
			strcpy(fi.filename,ss.c_str());
			fa.push_back(fi);
			fp[nFile].push_back(fi);
		}

//		}
		fp1.Close();

		if (pos==NULL) break;

		nFile++;
	}


	//---------------------------------------Meteox-----//
	// 02'04'20	プログレスバー関連の追加				//
	nCount = 0;
	dData = 0;

	// 02'04'20 プログレスバーに必要なのでfilepos追加	
	size_t filepos = 0;

	vector<CFileInfo>::iterator it;

	for(it=fa.begin();it!=fa.end();it++){
		filepos += (*it).arcsize;
	}
	//													//
	//--------------------------------------------------//


	//	ファイル名は取得できたので、次は、これを元にファイルを生成する
	//	まずは、SeekPosと一致させるためにポジション計算をしなくてはならない。

	CFile fp2(sFileName.c_str(),CFile::modeWrite | CFile::modeCreate);
	fp2.Write("yanepkDx",8);	// header
	filenum = fa.size();
	fp2.Write(&filenum,sizeof(filenum));

	for(it=fa.begin();it!=fa.end();it++){
		(*it).startpos += fa.size()*sizeof(CFileInfo) + 12;		// 8==sizeof('yanepack') + sizeof(DWORD)
		fp2.Write(it,sizeof(CFileInfo));	//	header
	}
	
	pos = fdlg.GetStartPosition( );
	for (n=0; n<=nFile ; n++){
		string SourceFile;
		SourceFile = fdlg.GetNextPathName(pos);
//		SourceFile.erase(0,SourceFile.rfind('\\')+1);
		//	↑これだと漢字の２バイト目に\が混じるとアウト
		SourceFile = GetPureFileNameOf(SourceFile);

		CFile fp1(SourceFile.c_str(),CFile::modeRead | CFile::typeBinary);
	
		char theID[9];
		fp1.Read(theID,8);	// identifier
		theID[8]='\0';		// delimiter
		if (strcmp("yanepkDx",theID)) return ; // 一致せず

		fp1.Read(&filenum,sizeof(DWORD));


		// ファイルinfoの読み込み
		for(int i=0;i<filenum;i++){
			CFileInfo fi;
			fp1.Read(&fi,sizeof(fi));
			fa.push_back(fi);
		}

		for(it=fp[n].begin();it!=fp[n].end();it++){

			//-------------------------------------------Meteox-----//
			//   02'04'19	プログレスバー関連の追加				//
			if (dlg.CheckCancelButton()){
				fp2.Close();
				CFile::Remove(sFileName.c_str());
				return;
			}
			//   02'04'14	プログレスバー関連の追加
			sDispFile = (*it).filename;
			sDispFile += "を結合しています";
			dlg.SetStatus(sDispFile.data());
			dData += (*it).arcsize;
			nCount = (int)((100.0*dData/(double)filepos));
			dlg.SetPos(nCount);
			//														//
			//------------------------------------------------------//
	
	
			BYTE *buf = new BYTE[(*it).arcsize];
			fp1.Read(buf,(*it).arcsize);		//	丸ごと読み込む
			fp2.Write(buf,(*it).arcsize);		//	丸ごと書き込む
			DELETEPTR_SAFE(buf);
		}
		fp1.Close();
	}

	if (fa.size()!=0) {
		AfxMessageBox("結合処理が終了しました。");
	}

	delete [] fp;

	fp2.Close();
}
