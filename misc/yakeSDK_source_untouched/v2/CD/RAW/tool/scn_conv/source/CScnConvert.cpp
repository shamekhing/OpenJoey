
#include "stdafx.h"
#include "../../yaneSDK/yaneSDK.h"
#include "CScnConvert.h"

///////////////////////////////////////////////////////////
//	コンバータ本体↓
//	いじるときは、これを適当にいじってくらはい。

LRESULT CScnConvert::Convert(string filename,string outputpath){

	//	ソースファイル
	CFile src_file;
	src_file.Read(filename);

	//	書き込むファイル
	CFile dst_file;
	bool bWrite=false; // 書き込みCHU！

	m_nLineNo = 0;	//	解析中の行番号
	bool bRemoveBracket = false;

	while (true){
/////////////////////////////////////////////////////////
//	解析部　～ここから～
/////////////////////////////////////////////////////////
	string linebuf;
	if (src_file.ReadLine(linebuf)!=0) break;
	m_nLineNo++;

	LPSTR psz = (LPSTR)linebuf.c_str();
	string line = linebuf;	//	出力用のラインはコピっとく！
	//	こうしておけば、lineを潰しても、pszのポイント先には影響しない

	//	その行の最後に<HR>を放り込むのか？
	bool bOutHR = true;

	//	☆　もし先頭が // から始まるのであれば、これは特殊効果指定
	if (CStringScanner::IsToken(psz,"//")){

		//	特殊効果指定（タグの省略表記）なので、行末の<HR>は放り込まなくて良い
		bOutHR = false;
		
		//	－－－－　以下、コピペの嵐＾＾；

		/*
			// scene 数字は、出力するシナリオファイルナンバー
			例.
				//	scene 100
				とあれば、そこ以降はscn0100.htmlというファイルに出力
		*/
		if (CStringScanner::IsToken(psz,"scene")){
			int nSceneNo;
			if (CStringScanner::GetNumFromCsv(psz,nSceneNo)!=0){
				Err("Scene指定ナンバーの指定がおかしい。");
			} else {
				string obj_filename;
				obj_filename = outputpath + "/scn" + CStringScanner::NumToStringZ(nSceneNo,4)+".html";
				dst_file.Open(obj_filename.c_str(),"w");
				bWrite = true;
			}
			continue;
		}
		/*
			//ses 1
			⇒<SESTOP 1>と変換される
		*/
		if (CStringScanner::IsToken(psz,"ses")){
			int n1;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
				Err("ses指定ナンバーの指定がおかしい。");
			} else {
				line = "<SESTOP "+CStringScanner::NumToString(n1)+">";
				goto output;
			}
		}
		/*
			//se 1,2
			→<SEPLAY 1 2>と変換される
		*/
		if (CStringScanner::IsToken(psz,"se")){
			int n1;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
				Err("se指定ナンバーの指定がおかしい。");
			//	↑エラーは第１パラメータが存在しないとき。そこ以降３つ目までは任意
			} else {
				line = "<SEPLAY "+CStringScanner::NumToString(n1);
				if (CStringScanner::GetNumFromCsv(psz,n1)==0){
					line += ","+CStringScanner::NumToString(n1);
					if (CStringScanner::GetNumFromCsv(psz,n1)==0){
						line += ","+CStringScanner::NumToString(n1);
					}
				}
				line += ">";
				goto output;
			}
		}
		/*
			//bgms
			→<BGMSTOP>と変換される
		*/
		if (CStringScanner::IsToken(psz,"bgms")){
			line = "<BGMSTOP>";
			goto output;
		}
		/*
			//bgm 1,2
			→<BGMPLAY 1 2>と変換される
		*/
		if (CStringScanner::IsToken(psz,"bgm")){
			int n1;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
				Err("bgm指定ナンバーの指定がおかしい。");
			//	↑エラーは第１パラメータが存在しないとき。そこ以降２つ目までは任意
			} else {
				line = "<BGMPLAY "+CStringScanner::NumToString(n1);
				if (CStringScanner::GetNumFromCsv(psz,n1)==0){
					line += ","+CStringScanner::NumToString(n1);
				}
				line += ">";
				goto output;
			}
		}
		/*
			//bg 2,3
			→<BGCG 2 3>と変換される
		*/
		if (CStringScanner::IsToken(psz,"bg")){
			int n1;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
				Err("bg指定ナンバーの指定がおかしい。");
			//	↑エラーは第１パラメータが存在しないとき。そこ以降２つ目までは任意
			} else {
				line = "<BGCG "+CStringScanner::NumToString(n1);
				if (CStringScanner::GetNumFromCsv(psz,n1)==0){
					line += ","+CStringScanner::NumToString(n1);
				}
				line += ">";
				goto output;
			}
		}
		/*
			//sci 4,2,80
			→<StandCharaIn 4 2 80>と変換される
		*/
		if (CStringScanner::IsToken(psz,"sci")){
			int n1,n2;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0 ||
				CStringScanner::GetNumFromCsv(psz,n2)!=0){
				Err("sci指定ナンバーの指定がおかしい。");
			//	↑エラーは第１，２パラメータが存在しないとき。そこ以降３つ目までは任意
			} else {
				line = "<StandCharaIn "+CStringScanner::NumToString(n1);
				line += ","+CStringScanner::NumToString(n2);
				if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
					line += ","+CStringScanner::NumToString(n1);
				}
				line += ">";
				goto output;
			}
		}
		/*
			//sco 2,80
			→<StandCharaOut 2 80>と変換される
		*/
		if (CStringScanner::IsToken(psz,"sco")){
			int n1;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
				Err("sco指定ナンバーの指定がおかしい。");
			//	↑エラーは第１パラメータが存在しないとき。そこ以降２つ目までは任意
			} else {
				line = "<StandCharaOut "+CStringScanner::NumToString(n1);
				if (CStringScanner::GetNumFromCsv(psz,n1)==0){
					line += ","+CStringScanner::NumToString(n1);
				}
				line += ">";
				goto output;
			}
		}
		/*
			//fm 3
			→<FaceMark 3>と変換される
		*/
		if (CStringScanner::IsToken(psz,"fm")){
			int n1;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
				Err("fm指定ナンバーの指定がおかしい。");
			//	↑エラーは第１パラメータが存在しないとき。
			} else {
				line = "<FaceMark "+CStringScanner::NumToString(n1)+">";
				goto output;
			}
		}
		/*
			//nm 4
			→<NamePlate 4>と変換される
		*/
		if (CStringScanner::IsToken(psz,"nm")){
			int n1;
			if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
				Err("nm指定ナンバーの指定がおかしい。");
			//	↑エラーは第１パラメータが存在しないとき。
			} else {
				line = "<NamePlate "+CStringScanner::NumToString(n1)+">";
				goto output;
			}
		}
		/*
			//	auto play フレーム数
			→　<AutoPlay フレーム数>と変換されます
		*/
		LPSTR psz2 = psz;	//	ポインタずれるといかんので戻すために保存
		if (CStringScanner::IsToken(psz,"auto")){
			if (CStringScanner::IsToken(psz,"play")){
				int n1;
				if (CStringScanner::GetNumFromCsv(psz,n1)!=0){
					Err("auto play指定ナンバーの指定がおかしい。");
					//	↑エラーは第１パラメータが存在しないとき。
				} else {
					line = "<AutoPlay "+CStringScanner::NumToString(n1)+">";
					goto output;
				}
			}
		}
		psz = psz2;

		/*
			//	remove bracket
				と書くと、そこ以降、名前のあとの鍵括弧（「　」）が自動的に外れます。
				ディフォルトでは外れません。
			//	not remove bracket
				と書くと、そこ以降、名前のあとの鍵括弧（「　」）が自動的に外れません。
		*/
		if (CStringScanner::IsToken(psz,"remove") && 
			CStringScanner::IsToken(psz,"bracket")){
			bRemoveBracket = true;
			continue;
		}
		psz = psz2;
		if (CStringScanner::IsToken(psz,"not") &&
			CStringScanner::IsToken(psz,"remove") &&
			CStringScanner::IsToken(psz,"bracket")){
			bRemoveBracket = false;
			continue;
		}
		psz = psz2;

	}
	//	空行は無視
	//	ReadLineなので改行コードでなく'\0'を検出すればｏｋ
	if (*psz == '\0') continue;

	//	それ以外ならばそのまま書き込む

	//	１行書き込み
output:;
	if (bWrite) {
		if (bOutHR) {
		//	この行はシナリオなので、ブラケット外すのならば、ここで処理する
			if (bRemoveBracket){
				string::size_type it1 = line.find("「");
				string::size_type it2 = line.find("」");
				//	↑こんなんして、日本語に対してうまく処理できんのか．．
				if (it1!=string::npos && it2!=string::npos && it1<it2){
				//	鍵括弧外してまうでー！！
					line = line.substr(it1+2,it2-it1-2);
				}
			}
			line += "<HR>\n";

		} else {
			line += '\n';
		}
		dst_file.Write(line);
	}
	
/////////////////////////////////////////////////////////
//	解析部　～ここまで～
/////////////////////////////////////////////////////////
	}
	//	CFileは、デストラクタによって自動的に閉じられる

	return 0;
}

void	CScnConvert::Err(string ErrMes){
	CDbg().Out("Error>"+CStringScanner::NumToString(m_nLineNo) + "行目:" + ErrMes);
}
