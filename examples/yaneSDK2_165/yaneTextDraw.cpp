#include "stdafx.h"
#include  <mbctype.h> // _ismbblead
#include "yaneTextDraw.h"
#include "yaneTextPlane.h"
#include "yaneTextDIB32.h"
#include "yaneStringScanner.h"
#include "yaneSpriteChara.h"
#include "yaneDIB32Effect.h"

CTextDrawContext::CTextDrawContext(void){
	m_nFontNo	= 0;
	m_nBaseSize	= 28;
	m_nFontSize	= m_nBaseSize;
	m_bBold		= false;
	m_bItalic	= false;
	m_bUnderLine= false;
	m_bStrikeOut= false;
	m_rgbColor	= RGB(255,255,255);
	m_rgbColorBk= CLR_INVALID;	//	影はいらねーや！
	m_nWidth	= 640;
	m_nBlankHeight	= 2;							//	行間
	m_nBlankHeight2 = m_nBaseSize+m_nBlankHeight;	//	空行高さ
	m_nLineNo	= 1;
	m_nAlign	= 0;
	m_nAlign2	= 0;
	m_nHInterval= 4; // 文字と文字との間隔
	m_lpTextAdr = m_lpStr = NULL;
}

CTextDrawContext::~CTextDrawContext(){	//	merely place holder
}

void	CTextDrawContext::SetBaseFontSize(int nFontSize){
	m_nBaseSize	= nFontSize;
	m_nFontSize	= m_nBaseSize;
	m_nBlankHeight2 = m_nFontSize+m_nBlankHeight;	//	空行高さ
}

void	CTextDrawContext::SetTextPtr(LPCSTR lpszText){
	m_lpTextAdr = m_lpStr = lpszText;
}

int		CTextDrawContext::GetTextOffset(void){
	//	現在のテキスト解析位置を返す
	return m_lpStr - m_lpTextAdr;
}

void	CTextDrawContext::SetTextOffset(int nPos){
	//	現在のテキスト解析位置を設定する
	m_lpStr = m_lpTextAdr + nPos;
}

	//////////////////////////////////////////////////////////////

CTextDrawBase::CTextDrawBase(void){
	*m_szUserMes = '\0';
	m_bSkipUnknownTag = true;	//	未知のタグを飛ばす

	m_dwMojiBuffer = 0;			//	禁則処理で発生するヒステリシスを吸収する
	m_lrMojiBuffer = 0;

	m_bGrayColor= false;
	m_rgbGrayColor=RGB(128,128,128);
	m_rgbGrayColorBk=CLR_INVALID;
	m_lpRepString = NULL;
	ZERO(m_anIndent);

	m_bVertical = false;
}

CTextDrawBase::~CTextDrawBase(){
}

LRESULT	CTextDrawBase::UpdateTextFast(void){
	//	m_context_nowをベースに<HR>までのサイズを計算する
	m_context_next = m_context_now;
	CTextDrawContext& context = m_context_next;
	m_vRects.clear();	//	一応クリアしといたるわ！＾＾；
	m_vTagList.clear();

	DWORD dwMoji;
	LRESULT lr;

	while ((lr = GetNextMoji(dwMoji))){
		if (lr==1) continue; // 改行
		if (lr==2) break;	 // <HR>
		if (lr!=0) return lr; // error
	}
	return 0;
}

LRESULT	CTextDrawBase::UpdateText(bool bDraw /* =true */ ){
	//	m_context_nowをベースに<HR>までのサイズを計算する
	m_context_next = m_context_now;
	CTextDrawContext& context = m_context_next;

	RECT rc;
	LRESULT lr;
	CLineInfo lineinfo;
	int nAlign;

	//	これ入れておかないとサーフェースが作られないときにまずい
	m_vTagList.clear();

	m_nIndentForBracket = 0;

	//	描画矩形のクリア
	m_vRects.clear();
	m_vSelectTag.clear();
	//	外字であるのか
	vector<bool> abGaiji;
	//	縦方向のアライン情報
	vector<int>	 anAlign;

	int nStartRect=0;
	int nHeight=0;	//	生成するDIBの高さ
	int nWidthMax=0;

	int nIndent = 0;
	bool bProhibit = false; // その行の禁則処理は２度目か？
	while (true){
		nAlign = context.m_nAlign;
		lr = GetRect(rc);
		bool bProhi = IsProhibitMoji(m_dwMojiLast);

		//	行が折り返すべき位置まで来ているか？
		bool bOver = nIndent + lineinfo.cx + rc.right > context.m_nWidth;
		if (lr==0 && bOver && bProhi && !bProhibit){
			//	禁則文字列ならば１回だけならラインオーバーして入ってきても良い

			//	（次の文字が禁則文字で無いのならば禁則処理を行なう）
			DWORD dwNextMoji;
			LRESULT lr = GetNextMoji(dwNextMoji);
			{	//	ヒステリシス吸収バッファにrejectする
				if (lr!=0) dwNextMoji = -1;
				m_dwMojiBuffer = dwNextMoji;
				m_lrMojiBuffer = lr;
			}
			if ((lr!=0) || !IsProhibitMoji(dwNextMoji)){
				bOver = false;
				bProhibit = true;
			}
		}
		if (bOver
				 || lr==1 || lr==2) {	//	行あふれ
			bProhibit = false;

			switch (nAlign) {
				//	行からはみ出した文字の一文字前の位置における
				//	コンテクストのアライン情報に基づいてインデントを施す
			case 0 : lineinfo.sx =	nIndent + 0; break;
			case 1 : lineinfo.sx = (context.m_nWidth - lineinfo.cx)>>1; break;
			case 2 : lineinfo.sx =	context.m_nWidth - lineinfo.cx; break;
			}
			//	行の位置関係が定まったので、補整を行なう
			int nLeft = lineinfo.sx;
			for(int i=nStartRect;i<m_vRects.size();i++){
				//	マージンは、ラインで指定されたマージンに準拠
				int nW = m_vRects[i]->right;

				m_vRects[i]->left	+= nLeft;
				m_vRects[i]->right	+= nLeft;
				
				if (m_vRects[i]->right > nWidthMax) nWidthMax = m_vRects[i]->right;
				//	禁則で追い出された文字があるので、最大幅はここで計算せねばならない

				nLeft += nW +(context.m_nHInterval-4); // かぶりがあるので＾＾；

		/*
			nHeight は、その行の上部
			lineinfo.cyは、その行高さ
		*/
				if (!IsVertical()) {
					// 横書き
					switch (anAlign[i]) {
					case 0: {
						//	高さはベースラインに準拠
						m_vRects[i]->top	+= nHeight + lineinfo.cy-m_vRects[i]->bottom;
						m_vRects[i]->bottom	 = nHeight + lineinfo.cy;
							} break;
					case 1: {
						//	高さはセンタリング
						int nH = (lineinfo.cy -m_vRects[i]->bottom)/2;
						m_vRects[i]->top	+= nHeight + nH;
						m_vRects[i]->bottom	 = nHeight + lineinfo.cy -nH;
							} break;
					case 2: {
						//	高さは上寄せで
						m_vRects[i]->top	+= nHeight;
						m_vRects[i]->bottom	 = nHeight + m_vRects[i]->bottom;
							} break;
					}
				} else {
					// 縦書きの時は逆になる
					switch (anAlign[i]) {
					case 0: {
						//	高さは上寄せで
						m_vRects[i]->top	+= nHeight;
						m_vRects[i]->bottom	 = nHeight + m_vRects[i]->bottom;
							} break;
					case 1: {
						//	高さはセンタリング
						int nH = (lineinfo.cy -m_vRects[i]->bottom)/2;
						m_vRects[i]->top	+= nHeight + nH;
						m_vRects[i]->bottom	 = nHeight + lineinfo.cy -nH;
							} break;
					case 2: {
						//	高さはベースラインに準拠
						m_vRects[i]->top	+= nHeight + lineinfo.cy-m_vRects[i]->bottom;
						m_vRects[i]->bottom	 = nHeight + lineinfo.cy;
							} break;
					}
				}

				//	位置、補正しとこか＾＾；
				m_vRects[i]->right -= 4;

				//	外字に対する最終調整
				if (abGaiji[i]) m_vRects[i]->right+=2;
			
			}
			nStartRect = m_vRects.size();
			if (lineinfo.cy!=0) {
				nHeight += lineinfo.cy + context.m_nBlankHeight;
			} else {
				if (lr!=2) {
					//	なんでい！空行かよ！
					nHeight += context.m_nBlankHeight2;
				}
			}
			lineinfo.Reset();
			if (m_nIndentForBracket) {
				//	インデントのために次行以降の幅を縮めておく
				nIndent = (rc.right + context.m_nHInterval-4)* m_nIndentForBracket;
				m_nIndentForBracket = 0;
			}
		}
		if (lr==1) continue; // 改行
		if (lr==2) break;	 // 最終ライン終了 
		if (lr!=0) return lr; // error

		lineinfo.cx+=rc.right + (context.m_nHInterval-4);

		if (!m_bGaiji) {
			if (lineinfo.cy < rc.bottom) lineinfo.cy = rc.bottom;	// サイズ加算
		} else {
			//	外字用に行間の補正
			if (lineinfo.cy < rc.bottom+2) lineinfo.cy = rc.bottom+2;	// サイズ加算
		}

		//	もし、外字ばかりであるなどの理由により、通常テキストより小さい行幅であるならば補正する
		if (lineinfo.cy < context.m_nBlankHeight2) lineinfo.cy = context.m_nBlankHeight2;

		//	矩形情報を保存する
		auto_ptrEx<RECT> lpRect(new RECT);
		*lpRect = rc;
		m_vRects.insert(lpRect);
		abGaiji.push_back(m_bGaiji);
		//	アライン情報も保存
		anAlign.push_back(context.m_nAlign2);
	}

	if (!bDraw) return 0;	//	描画しないなら、ここで終了

	if (nHeight == 0 || nWidthMax == 0) {
		//	return -1; // サイズおかしいやん...
		m_vRects.clear();
		CreateSurface(1,1);	//	size 1×1のDIBをダミーで作成
		return 0;
	}


//----------------------------追加
	// 縦書きのための座標変換
	if (IsVertical()) {
		// 幅と高さを入れ替え
		swap(nWidthMax, nHeight);
	}
//----------------------------


	//	これで各列の情報が求まった。これに基づいてDIBを作成する
	CreateSurface(nWidthMax,nHeight);	//	サーフェース完成～

	//	コンテクストを引き戻して、再スキャンして文字配置
	m_context_next = m_context_now;
	m_vTagList.clear();
//	m_vSelectTag.clear();

	auto_vector_ptr<RECT>::iterator it = m_vRects.begin();
	while (true) {
		//	↑終了チェックは、ここで行なわず、lr==2で行なう。
		DWORD dwMoji;
		lr = GetNextMoji(dwMoji);
		if (lr==2) break;
		if (lr==1) continue;
		if (lr!=0) return lr; // ありえないが...

//----------------------------追加
		//	縦書きのための座標変換
		if (IsVertical()) {
			const int left	= (*it)->left;	// バックアップ^^
			(*it)->left		= nWidthMax-(*it)->bottom;	// 縦書きの行は右→左
			(*it)->bottom	= (*it)->right;
			(*it)->right	= nWidthMax-(*it)->top;		// 縦書きの行は右→左
			(*it)->top		= left;
		}
//----------------------------

		//	---　外字の処理
		int nGaiji = -((int)dwMoji) - 1;
		if (nGaiji >= 0) {
			if (m_vGaiji->GetSpriteMax() <= nGaiji) {
				return 1; // 範囲外の外字
			}
			//	スプライトは、アニメーションがあろうと、先頭ので良い
			SSprite* lpSp = m_vGaiji->GetSprite()[nGaiji].GetSSprite(0);
			// そのスプライトを表示
			OnDrawSprite(lpSp,(*it)->left,(*it)->top);
			//	このスプライト、Contextに応じて拡大すべきのような気もするが？
			//	また、CDIB32A系であれば、非ヌキ部分に対してα値をffで埋める必要がある..

		} else {
		//	通常の描画
			//OnDrawText(m_context_next,(*it)->left,(*it)->top, dwMoji);
			OnDrawText(m_context_next, dwMoji, *it);	//	OnDrawTextでRECTの最終調整が行われる
		}
		it++;
	}
	return 0;
}

bool	CTextDrawBase::IsProhibitMoji(DWORD dwMoji){
	//	禁則文字のひとつか？
	if (m_szProhibition.empty()) return false;	//	禁則処理を行なわない？

	LPCSTR lp = m_szProhibition.c_str();
	while (*lp!='\0'){
		DWORD dw = (DWORD)*((BYTE*)lp++);
//		if ((dw >=0x80 && dw<=0xa0) || (dw>=0xe0 && dw<=0xff)) {
		if (_ismbblead(dw)){
			//	漢字の１バイト目らしい
			dw |= (DWORD)*(lp++) << 8;
		}
		if (dw == dwMoji) return true;	//	禁則文字に一致した
	}
	return false;
}

LRESULT	CTextDrawBase::GetRect(RECT&rc){
	LRESULT lr;
	m_bGaiji = false;
	lr = GetNextMoji(m_dwMojiLast);
	//	もし必要ならばこのm_dwMojiLastを介してアクセスすること
	if (lr!=0) return lr;
	//	m_context_nextに基づき、フォントを作成。
	//	その後、m_dwMojiLastの文字のRECTを求めれば良い

	//	外字なの？
	int nGaiji = -((int)m_dwMojiLast) - 1;
	if (nGaiji >= 0) {
		if (m_vGaiji->GetSpriteMax() <= nGaiji) {
			return -1; // 範囲外の外字
		}
		//	スプライトは、アニメーションがあろうと、先頭ので良い
		SSprite* lpSp = m_vGaiji->GetSprite()[nGaiji].GetSSprite(0);
		// そのスプライトのスプライト矩形サイズをコピー
		::SetRect(&rc,0,0,lpSp->rcRect.right - lpSp->rcRect.left+2,lpSp->rcRect.bottom - lpSp->rcRect.top);
		//	文字になぞらえて横サイズ+2しとこか＾＾；
		m_bGaiji = true;
		return 0;
	}

	CTextDrawContext& context = m_context_next;
	CFont f;
	SetContext(f,context,(LPCSTR)&m_dwMojiLast);

	const string font_name(f.GetFont());
	if (!IsVertical()) {	//	横書きの場合
		//	横書きの時は追加処理なし
	} else {				//	縦書きの場合
		if (!font_name.empty() && font_name[0]=='@') {
			//	元から縦書きフォントだから何もしない
		} else {
			//	一時的に縦書きフォントに変更する
			f.SetFont('@'+font_name);
		}
		//	↑このようにフォントを縦書き用に変更しておかないと、
		//	　ただx,yを入れ替えるだけとかでは半角文字(縦にならない文字)が正しく表示されない
	}
	int x,y;
	if (f.GetSize(x,y)!=0) return -1;
	::SetRect(&rc,0,0,x,y);

	//	フォントを元に戻す
	f.SetFont(font_name);

	return 0;
}

LRESULT	CTextDrawBase::SetTextOffset(int nPos){
	m_context_next = m_context_now;
	m_context_next.SetTextOffset(0);	//	reset
	DWORD dwMoji;
	while (m_context_next.GetTextOffset()<nPos)
		if (GetNextMoji(dwMoji)==3) return 1;
	m_context_now = m_context_next;
	return 0;
}

//	独自に作ったHTML風言語＾＾；の解析
LRESULT CTextDrawBase::GetNextMoji(DWORD&dwMoji){
	//	返し値は、改行ならば 1,水平線ならば2。ファイル終端ならば3。
	//	正常終了ならば0。

	//	dwMojiは、外字ならばマイナスの値。

	//	禁則文字列の差し戻しバッファ
	if (m_dwMojiBuffer != 0) {
		//	ヒステリシス処理
		dwMoji = m_dwMojiBuffer;
		m_dwMojiBuffer = 0;
		return m_lrMojiBuffer;
	}

RepString:;
	/*
		もし置換文字列ならば、そいつを与える。
	*/
	if (m_lpRepString != NULL) {
		dwMoji = (BYTE)*(m_lpRepString++);	//	LPSTRは何とsignedなのだ＾＾；
		if (_ismbblead(dwMoji)){
			//	漢字の１バイト目らしい
			dwMoji |= (DWORD)((BYTE)*(m_lpRepString++)) << 8;
		}
		//	文字列終端に達したならば、そこで終わり！
		if (*m_lpRepString == '\0') m_lpRepString = NULL;
		return 0;
	}


	CTextDrawContext& context = m_context_next;
	LPCSTR& lpSrc = context.m_lpStr;

	//	最上位バイトが0であることを利用する
	while (true) {
		if (*lpSrc=='\0') return 3;
		if (IsToken(lpSrc,"<BR>"))	{	// break rule : 改行
			return 1;
		}
		if (IsToken(lpSrc,"</P>")) {	//	閉じタグは改行扱い
			context.m_nAlign = 0;
			return 1;
		}
		if (IsToken(lpSrc,"<HR>")) {	// horizontal rule : 水平線＝次の段落へ
			return 2;	//	end of paragraph
		}
		if (IsToken(lpSrc,"&lt;")) {	// これは <
			dwMoji = '<'; break;
		}
		if (IsToken(lpSrc,"&gt;")) { // これは >
			dwMoji = '>'; break;
		}
		if (IsToken(lpSrc,"&amp;")){ // これば &
			dwMoji = '&'; break;
		}
		if (IsToken(lpSrc,"<!--")) { //	コメントタグ
			if (SkipTo2(lpSrc,"-->",m_szUserMes)!=0) return 3;			//	閉じタグを探す
			//	↑コメントタグは、文字バッファへコピーしておく
			continue;
		}
		if (IsToken(lpSrc,"<head>")) { //	ヘッダタグ
			if (SkipTo(lpSrc,"</head>")!=0) return 3;	//	閉じタグを探す
			continue;
		}
		if (IsToken(lpSrc,"<html>")||IsToken(lpSrc,"</html>")||
			IsToken(lpSrc,"<body>")||IsToken(lpSrc,"</body>")) {
			continue;	//	こいつらは無視！
		}
		if (IsToken(lpSrc,"<B>")) {	// bold
			context.m_bBold = true;
			continue;
		}
		if (IsToken(lpSrc,"</B>")) {// unbold
			context.m_bBold = false;
			continue;
		}
		if (IsToken(lpSrc,"<FONT")){	//	font
			//	現在のフォント属性を保存する
			context.m_nFontNoOld.push_back(context.m_nFontNo);
			context.m_nFontSizeOld.push_back(context.m_nFontSize);
			context.m_rgbColorOld.push_back(context.m_rgbColor);

			while (true){
				if (SkipSpace(lpSrc)!=0) return 3;
				if (IsToken(lpSrc,"size=")){
				//	サイズ指定
					int nRate;
					if (GetStrNum(lpSrc,nRate)!=0) return 3;	//	"+1" => 1が返る
					switch (nRate){
					case -4: context.m_nFontSize = context.m_nBaseSize/4; break;
					case -3: context.m_nFontSize = context.m_nBaseSize/3; break;
					case -2: context.m_nFontSize = context.m_nBaseSize/2; break;
					case -1: context.m_nFontSize = context.m_nBaseSize*2/3; break;
					case  0: context.m_nFontSize = context.m_nBaseSize; break;
					case  1: context.m_nFontSize = context.m_nBaseSize*3/2; break;
					case  2: context.m_nFontSize = context.m_nBaseSize*2; break;
					case  3: context.m_nFontSize = context.m_nBaseSize*3; break;
					case  4: context.m_nFontSize = context.m_nBaseSize*4; break;
					default: context.m_nFontSize = context.m_nBaseSize*nRate;
					}
					continue;
				}
				if (IsToken(lpSrc,"color=")){
				//	カラー指定
					if (GetStrColor(lpSrc,context.m_rgbColor)!=0) return 3; // "#ff0000" => RGB(255,0,0)が返る
					continue;
				}
				if (IsToken(lpSrc,"face=\"")){
				//	フォント名指定
					if (IsToken(lpSrc,"ＭＳ ゴシック"))	 { context.m_nFontNo = 0; }
					ef (IsToken(lpSrc,"ＭＳ Ｐゴシック")){ context.m_nFontNo = 1; }
					ef (IsToken(lpSrc,"ＭＳ 明朝"))		 { context.m_nFontNo = 2; }
					ef (IsToken(lpSrc,"ＭＳ Ｐ明朝"))	 { context.m_nFontNo = 3; }
					
					if (!IsToken(lpSrc,"\"")) return 3; // コーテーション閉じてないやん
					continue;
				}
				if (IsToken(lpSrc,">")) break; // fontの終了記号
			}
			continue;
		}
		if (IsToken(lpSrc,"</FONT>")) { // unfont
			if (context.m_nFontNoOld.empty()) {
				Err.Out("CTextDrawで<Font>の存在していない</Font>タグ");
				continue;
			}
			context.m_nFontNo = *context.m_nFontNoOld.rbegin();
			context.m_nFontNoOld.pop_back();
			context.m_nFontSize = *context.m_nFontSizeOld.rbegin();
			context.m_nFontSizeOld.pop_back();
			context.m_rgbColor = *context.m_rgbColorOld.rbegin();
			context.m_rgbColorOld.pop_back();
			continue;
		}
		if (IsToken(lpSrc,"<U>")) { // underline
			context.m_bUnderLine = true;
			continue;
		}
		if (IsToken(lpSrc,"</U>")) { // underline off
			context.m_bUnderLine = false;
			continue;
		}
		if (IsToken(lpSrc,"<S>")) { // strike out
			context.m_bStrikeOut = true;
			continue;
		}
		if (IsToken(lpSrc,"</S>")) { // strike out off
			context.m_bStrikeOut = false;
			continue;
		}
		if (IsToken(lpSrc,"<I>")) { // italic out
			context.m_bItalic = true;
			continue;
		}
		if (IsToken(lpSrc,"</I>")) { // italic off
			context.m_bItalic = false;
			continue;
		}
		if (IsToken(lpSrc,"<P>")) {	//	無視しとけー＾＾；
			continue;
		}
		if (IsToken(lpSrc,"<P ")) { // paragraph
			while (true){
				if (SkipSpace(lpSrc)!=0) return 3;
				if (IsToken(lpSrc,"align=\"")){
					if (IsToken(lpSrc,"left")) {
						context.m_nAlign = 0;
					} ef (IsToken(lpSrc,"center")){
						context.m_nAlign = 1;
					} ef (IsToken(lpSrc,"right")){
						context.m_nAlign = 2;
					}
					if (SkipSpace(lpSrc)!=0) return 3;
					if (*lpSrc!='"') return 3; // ちゃんと閉じてないやん
					lpSrc++;
					continue;
				}
				if (IsToken(lpSrc,">")) break;
				return 3; // ありゃー？なんなんやろ？未知の指定だわ＾＾；
			}
			continue;
		}
		if (IsToken(lpSrc,"<VDATA=\"")) { // secrect data
			int nChunk;
			if (GetNum(lpSrc,nChunk)!=0) return 3; // 数字ないやん..
			m_vData[nChunk].clear();
			if (SkipSpace(lpSrc)!=0) return 3;
			if (!IsToken(lpSrc,",")) return 3;	//	数字ひとつで終わってるってどうゆうことー？
			while (true){
				if (SkipSpace(lpSrc)!=0) return 3;
				int nData;
				if (GetNum(lpSrc,nData)!=0) return 3; // 数字ちゃうやん..
				m_vData[nChunk].push_back(nData);
				if (SkipSpace(lpSrc)!=0) return 3;
				if (IsToken(lpSrc,"\"")) break;
				if (IsToken(lpSrc,",")) continue;
				return 3; // ありゃー？なんなんやろ？未知の指定だわ＾＾；
			}
			if (SkipSpace(lpSrc)!=0) return 3;
			if (!IsToken(lpSrc,">")) return 3;	//	タグ閉じてへんやん..
			continue;
		}
		if (IsToken(lpSrc,"<IndentForBracket")) { // ２行目以降をインデントするのか？
			if (CStringScanner::GetNumFromCsv(lpSrc,m_nIndentForBracket)!=0) {
				m_nIndentForBracket = 1;
			}
			if (m_nIndentForBracket<0) {
			//	マイナスのときは、これでオッケー。
				m_nIndentForBracket = m_anIndent[-m_nIndentForBracket-1];
			}
			if (!IsToken(lpSrc,">")) return 3;	//	タグ閉じてへんやん..
			continue;
		}

		//	外字タグか？
		if (IsToken(lpSrc,"<GAIJI ")){	//	gaiji
			int n;
			if (CStringScanner::GetNumFromCsv(lpSrc,n)!=0) return 3;
			if (n<0) return 3; // なぜじゃー
			dwMoji = (DWORD)(-n-1);
			if (!IsToken(lpSrc,">")) return 3;	//	タグ閉じてへんやん..
			break ; // このままリターンする
		}

		//	置換文字列の指定か？
		if (IsToken(lpSrc,"<ReplaceString ")){	//	gaiji
			int n;
			if (CStringScanner::GetNumFromCsv(lpSrc,n)!=0) return 3;
			if (m_vRepString->size() < n) return 3; // 範囲外の置換文字列
			if (!IsToken(lpSrc,">")) return 3;	//	タグ閉じてへんやん..
			//	えぐいな、この処理＾＾；
			m_lpRepString = (*m_vRepString)[n].c_str();
			goto RepString;
		}

		//	--- 縦方向マージン指定
		if (IsToken(lpSrc,"<Bottom>")) {// bottom (default)
			context.m_nAlign2 = 0;
			continue;
		}
		if (IsToken(lpSrc,"<Middle>")) {// middle
			context.m_nAlign2 = 1;
			continue;
		}
		if (IsToken(lpSrc,"<Top>")) {// top
			context.m_nAlign2 = 2;
			continue;
		}


		//	セレクトタグか？
		if (IsToken(lpSrc,"<Select ")){	//	select
			int n;
			if (CStringScanner::GetNumFromCsv(lpSrc,n)!=0) return 3;
			if (n<0) return 3; // なぜじゃー
			n <<= 1; // ２倍して、、
			if (m_vSelectTag.size()<n+1) {
			//	配列サイズが小さければ、サイズ拡張を行なう
				m_vSelectTag.resize(n+1,-1);
			}
			//	２度目ではvRectの数が確定しているので、
			//	ここが何文字目かを取得することは不可能。よって、
			//	一度目に、vRect.size()から、位置を確定させてしまう
			if (m_vSelectTag[n]==-1){
				m_vSelectTag[n] = m_vRects.size(); // ここの文字数を入れる
			}
			if (!IsToken(lpSrc,">")) return 3;	//	タグ閉じてへんやん..
			continue ;
		}

		//	閉じセレクトタグか？
		if (IsToken(lpSrc,"</Select ")){	//	select
			int n;
			if (CStringScanner::GetNumFromCsv(lpSrc,n)!=0) return 3;
			if (n<0) return 3; // なぜじゃー
			n = n*2 + 1; // ２倍して１加算、、
			//	↑この部分以外は、上のSelectTagと同じ処理！
			if (m_vSelectTag.size()<n+1) {
			//	配列サイズが小さければ、サイズ拡張を行なう
				m_vSelectTag.resize(n+1,-1);
			}
			//	２度目ではvRectの数が確定しているので、
			//	ここが何文字目かを取得することは不可能。よって、
			//	一度目に、vRect.size()から、位置を確定させてしまう
			if (m_vSelectTag[n]==-1){
				m_vSelectTag[n] = m_vRects.size(); // ここの文字数を入れる
			}
			if (!IsToken(lpSrc,">")) return 3;	//	タグ閉じてへんやん..
			continue ;
		}

		//	----	ネストされたif～else～endifを簡易的に判定する(C)yaneurao

		//	ifタグか？
		if (IsToken(lpSrc,"<if ")){	//	if
			int n;
			if (CStringScanner::GetNumFromCsv(lpSrc,n)!=0) return 3;
			if (n<0) return 3; // なぜじゃー
			if (!IsToken(lpSrc,">")) return 3;	//	タグ閉じてへんやん..

			if (GetScenarioIfListener()==NULL) continue;
			//	functorが設定されていなければあとは無視するなり

			bool bCond = GetScenarioIfListener()->Abstract_If(n);
			//	条件が成立してるならば、無視してｏｋ
			if (bCond) continue ;

			//	条件が成立していないならば、ネストされたif～endifを無視しつつ
			//	このifに対応するendifを探す。
			int nNest = 0;
			while (true) {
				if (IsToken(lpSrc,"<if ")) {
					nNest++;
				}
				if (IsToken(lpSrc,"<else>")) {
					if (nNest==0) {	//	同次レベルのelseを発見
						break;
					}
				}
				if (IsToken(lpSrc,"<endif>")){
					nNest--;
					if (nNest<0) break;
				}
				lpSrc++;
				//	終端に達するまで
				if (*lpSrc=='\0') return 4; // endifが存在しない..
			}

			continue;
		}
		//	elseタグか？
		if (IsToken(lpSrc,"<else>")){	//	else
			//	ネストされたif～endifを無視しつつ
			//	このelseに対応するendifを探す。

			int nNest = 0;
			while (true) {
				if (IsToken(lpSrc,"<if ")) {
					nNest++;
				}
				if (IsToken(lpSrc,"<endif>")){
					nNest--;
					if (nNest<0) break;
				}
				lpSrc++;
				//	終端に達するまで
				if (*lpSrc=='\0') return 4; // endifが存在しない..
			}
			continue;
		}
		//	endifタグか？
		if (IsToken(lpSrc,"<endif>")){	//	endif
			//	実は、このタグは無視すれば良い。
			continue ;
		}

		// ------------------------

		//	それら以外のタグなんか？
		if (m_bSkipUnknownTag && IsToken(lpSrc,"<")) {
			m_vTagList.push_back(lpSrc);	//	ここに保存するちん＾＾；
			//	未知のタグはとばそう
			if (SkipTo(lpSrc,">")!=0) return 3;	//	閉じタグを探す
			continue;		
		}

		{	//	それら以外ということは、おそらくは文字なのだ
			dwMoji = (BYTE)*(lpSrc++);	//	LPSTRは何とsignedなのだ＾＾；
			if (dwMoji == 0x0a) { continue; }	//	LF
			if (dwMoji == 0x0d) { context.m_nLineNo++; continue; } // CR
//			if ((dwMoji >=0x80 && dwMoji<=0xa0) || (dwMoji>=0xe0 && dwMoji<=0xff)) {
			if (_ismbblead(dwMoji)){
				//	漢字の１バイト目らしい
				dwMoji |= (DWORD)((BYTE)*(lpSrc++)) << 8;
			}
			break;
		}
	}
	//	文字コードはdwMojiに入っている
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//	ヘルパ関数 (CStringScannerに委譲)
	//	CStringScannerに委譲＾＾；
bool	CTextDrawBase::IsToken(LPCSTR &lp,LPCSTR lp2){
	return CStringScanner::IsToken(lp,lp2);
}
LRESULT	CTextDrawBase::SkipTo(LPCSTR &lp,LPCSTR lp2){
	return CStringScanner::SkipTo(lp,lp2);
}
LRESULT	CTextDrawBase::SkipTo2(LPCSTR &lp,LPCSTR lp2,LPSTR lp3){
	return CStringScanner::SkipTo2(lp,lp2,lp3);
}
LRESULT CTextDrawBase::SkipSpace(LPCSTR &lp){
	return CStringScanner::SkipSpace(lp);
}
LRESULT	CTextDrawBase::GetStrNum(LPCSTR &lp,int& nRate){
	return CStringScanner::GetStrNum(lp,nRate);
}
LRESULT	CTextDrawBase::GetNum(LPCSTR &lp,int& nVal){
	return CStringScanner::GetNum(lp,nVal);
}
LRESULT CTextDrawBase::GetStrColor(LPCSTR &lp,COLORREF& nFontColor){
	return CStringScanner::GetStrColor(lp,nFontColor);
}

////////////////////////////////////////////////////////////////////////

void	CTextDrawBase::SetContext(CFont&font,const CTextDrawContext& context,LPCSTR lpStr){
	font.SetText(lpStr);
	if (!context.m_bBold){
		font.SetWeight(FW_NORMAL);
//		font.SetWeight(FW_THIN);
	} else {
		font.SetWeight(FW_BOLD);
	}
	font.SetSize(context.m_nFontSize);
	font.SetFont(context.m_nFontNo);

	//	グレーカラーに対応
	if (!m_bGrayColor) {
		font.SetColor(context.m_rgbColor);
		font.SetBackColor(context.m_rgbColorBk);
	} else {
		font.SetColor(m_rgbGrayColor);
		font.SetBackColor(m_rgbGrayColorBk);
	}

	font.SetItalic(context.m_bItalic);
	font.SetUnderLine(context.m_bUnderLine);
	font.SetStrikeOut(context.m_bStrikeOut);
	font.SetQuality(4); // NONANTIALIASED_QUALITY
}

////////////////////////////////////////////////////////////////////////
#ifdef USE_DIB32
void	CTextDrawDIB32::OnDrawText(const CTextDrawContext& context,DWORD dwMoji,LPRECT lpDrawPos)
{
	CTextDIB32 t;
	SetContext(*t.GetFont(),context,(LPSTR)&dwMoji);
	t.SetVertical(IsVertical());	//　縦書きかどうかを設定
	t.UpdateText();
	m_dib.BltFast(&t,lpDrawPos->left,lpDrawPos->top);

	//	RECTの最終調整
	int sx, sy;
	t.GetSize(sx,sy);
	lpDrawPos->right = lpDrawPos->left + sx;
	lpDrawPos->bottom = lpDrawPos->top + sy;
}
void	CTextDrawDIB32A::OnDrawText(const CTextDrawContext& context,DWORD dwMoji,LPRECT lpDrawPos)
{
	CTextDIB32 t;
	SetContext(*t.GetFont(),context,(LPSTR)&dwMoji);
	t.SetVertical(IsVertical());	//　縦書きかどうかを設定
	t.UpdateTextA();
	m_dib.BltFast(&t,lpDrawPos->left,lpDrawPos->top);

	//	RECTの最終調整
	int sx, sy;
	t.GetSize(sx,sy);
	lpDrawPos->right = lpDrawPos->left + sx;
	lpDrawPos->bottom = lpDrawPos->top + sy;
}
void	CTextDrawDIB32AA::OnDrawText(const CTextDrawContext& context,DWORD dwMoji,LPRECT lpDrawPos)
{
	CTextDIB32 t;
	SetContext(*t.GetFont(),context,(LPSTR)&dwMoji);
	t.SetVertical(IsVertical());	//　縦書きかどうかを設定
	t.UpdateTextAA();
	m_dib.BltFast(&t,lpDrawPos->left,lpDrawPos->top);

	//	RECTの最終調整
	int sx, sy;
	t.GetSize(sx,sy);
	lpDrawPos->right = lpDrawPos->left + sx;
	lpDrawPos->bottom = lpDrawPos->top + sy;
}

void	CTextDrawDIB32::OnDrawSprite(SSprite*lpSp,int x,int y){
	if (lpSp->bFast){
		m_dib.BltFast((CDIB32*)lpSp->lpPlane,x + lpSp->nOx,y + lpSp->nOy,&lpSp->rcRect);
	} else {
		m_dib.Blt((CDIB32*)lpSp->lpPlane,x + lpSp->nOx,y + lpSp->nOy,&lpSp->rcRect);
	}
}

void	CTextDrawDIB32A::OnDrawSprite(SSprite*lpSp,int x,int y){
	if (lpSp->bFast){
		int x2 = x + lpSp->nOx;
		int y2 = y + lpSp->nOy;
		m_dib.BltFast((CDIB32*)lpSp->lpPlane,x2,y2,&lpSp->rcRect);
		RECT rc;
		::SetRect(&rc,x2,y2,
			x2 + lpSp->rcRect.right - lpSp->rcRect.left,
			y2 + lpSp->rcRect.bottom - lpSp->rcRect.top
		);
		m_dib.AddColorFast(0xff000000,&rc); // 最上位だけ潰す＾＾
		//	こういう転送作れって話もある...（いずれ作る＾＾；）
	} else {
		//	こういう転送作れって話もある...（いずれ作る＾＾；）
		int x2 = x + lpSp->nOx;
		int y2 = y + lpSp->nOy;
		//	転送元スプライト、潰してしもてええかな？＾＾；
		CDIB32Effect::Effect(
			((CDIB32*)lpSp->lpPlane),
			CDIB32MaskMSB(((CDIB32*)lpSp->lpPlane)->GetColorKey()),
			&lpSp->rcRect
		); // カラーキーに応じて最上位だけ潰す＾＾
		m_dib.Blt((CDIB32*)lpSp->lpPlane,x2,y2,&lpSp->rcRect);
	}
}

#endif

#ifdef USE_DirectDraw
void	CTextDrawPlane::OnDrawText(const CTextDrawContext& context,DWORD dwMoji,LPRECT lpDrawPos)
{
	CTextPlane t;
	SetContext(*t.GetFont(),context,(LPSTR)&dwMoji);
//	t.SetVertical(IsVertical());	//　縦書きには対応してません^^;
	t.UpdateText();
	m_plane.BltFast(&t,lpDrawPos->left,lpDrawPos->top);

	//	RECTの最終調整
	int sx, sy;
	t.GetSize(sx,sy);
	lpDrawPos->right = lpDrawPos->left + sx;
	lpDrawPos->bottom = lpDrawPos->top + sy;
}

void	CTextDrawPlane::OnDrawSprite(SSprite*lpSp,int x,int y){
	if (lpSp->bFast){
		m_plane.BltFast((CPlane*)lpSp->lpPlane,x + lpSp->nOx,y + lpSp->nOy,&lpSp->rcRect);
	} else {
		m_plane.Blt((CPlane*)lpSp->lpPlane,x + lpSp->nOx,y + lpSp->nOy,&lpSp->rcRect);
	}
}
#endif

#ifdef USE_FastDraw
void	CTextDrawFastPlane::OnDrawText(const CTextDrawContext& context,DWORD dwMoji,LPRECT lpDrawPos)
{
	CTextDIB32 t;
	SetContext(*t.GetFont(),context,(LPSTR)&dwMoji);
	t.SetVertical(IsVertical());	//　縦書きかどうかを設定
	t.UpdateText();
	m_plane.Blt((CDIB32*)&t,lpDrawPos->left,lpDrawPos->top);
		//	CDIB32 ⇔ CFastPlaneはサポートされている！

	//	RECTの最終調整
	int sx, sy;
	t.GetSize(sx,sy);
	lpDrawPos->right = lpDrawPos->left + sx;
	lpDrawPos->bottom = lpDrawPos->top + sy;
}
void	CTextDrawFastPlaneA::OnDrawText(const CTextDrawContext& context,DWORD dwMoji,LPRECT lpDrawPos)
{
	CTextDIB32 t;
	SetContext(*t.GetFont(),context,(LPSTR)&dwMoji);
	t.SetVertical(IsVertical());	//　縦書きかどうかを設定
	t.UpdateTextA();
	m_plane.Blt((CDIB32*)&t,lpDrawPos->left,lpDrawPos->top);
		//	CDIB32 ⇔ CFastPlaneはサポートされている！

	//	RECTの最終調整
	int sx, sy;
	t.GetSize(sx,sy);
	lpDrawPos->right = lpDrawPos->left + sx;
	lpDrawPos->bottom = lpDrawPos->top + sy;
}
void	CTextDrawFastPlaneAA::OnDrawText(const CTextDrawContext& context,DWORD dwMoji,LPRECT lpDrawPos)
{
	CTextDIB32 t;
	SetContext(*t.GetFont(),context,(LPSTR)&dwMoji);
	t.SetVertical(IsVertical());	//　縦書きかどうかを設定
	t.UpdateTextAA();
	m_plane.Blt((CDIB32*)&t,lpDrawPos->left,lpDrawPos->top);
		//	CDIB32 ⇔ CFastPlaneはサポートされている！

	//	RECTの最終調整
	int sx, sy;
	t.GetSize(sx,sy);
	lpDrawPos->right = lpDrawPos->left + sx;
	lpDrawPos->bottom = lpDrawPos->top + sy;
}

void	CTextDrawFastPlane::OnDrawSprite(SSprite*lpSp,int x,int y){
	if (lpSp->bFast){
		m_plane.BltFast((CFastPlane*)lpSp->lpPlane,x + lpSp->nOx,y + lpSp->nOy,&lpSp->rcRect);
	} else {
		m_plane.Blt((CFastPlane*)lpSp->lpPlane,x + lpSp->nOx,y + lpSp->nOy,&lpSp->rcRect);
	}
}

void	CTextDrawFastPlaneA::OnDrawSprite(SSprite*lpSp,int x,int y){

	if (lpSp->bFast){
		int x2 = x + lpSp->nOx;
		int y2 = y + lpSp->nOy;
		m_plane.BltFast((CFastPlane*)lpSp->lpPlane,x2,y2,&lpSp->rcRect);
		//	BltFastは、非Alphaサーフェース to Alphaサーフェースをサポートしている！
	/*
		RECT rc;
		::SetRect(&rc,x2,y2,
			x2 + lpSp->rcRect.right - lpSp->rcRect.left,
			y2 + lpSp->rcRect.bottom - lpSp->rcRect.top
		);
		m_plane.AddColorFast(0xff000000,&rc); // 最上位だけ潰す＾＾
		//	こういう転送作れって話もある...（いずれ作る＾＾；）
	*/
	} else {
		//	こういう転送作れって話もある...（いずれ作る＾＾；）
		int x2 = x + lpSp->nOx;
		int y2 = y + lpSp->nOy;
		/*
		//	転送元スプライト、潰してしもてええかな？＾＾；
		CDIB32Effect::Effect(
			((CFastPlane*)lpSp->lpPlane),
			CDIB32MaskMSB(((CDIB32*)lpSp->lpPlane)->GetColorKey()),
			&lpSp->rcRect
		); // カラーキーに応じて最上位だけ潰す＾＾
		*/
		m_plane.Blt((CFastPlane*)lpSp->lpPlane,x2,y2,&lpSp->rcRect);
		//	Bltは、非Alphaサーフェース to Alphaサーフェースをサポートしている！
	}
}

#endif

/////////////////////////////////////////////////////////////////////////////

