#include "stdafx.h"

//
//	ここに存在する関数はCDIB32のalpha付の処理関数であって、
//	class CDIB32Alphaのための処理関数ではない
//

#ifdef USE_DIB32

#include "yaneDIB32.h"

//	αチャンネル付きエフェクト系
LRESULT CDIB32P5::FadeAlpha(int nFade,LPRECT lpRect){
	//	プレーンのα値を落とす機能	added '00/11/07
	//	手抜きだけど、それなりに速いので、いいやー＾＾；

	WARNING(m_lpdwSrc == NULL,"CDIB32P5::FadeAlphaでm_lpdwSrc == NULL");

	if (nFade==256) return 0;	//	減衰なし

	RECT r = GetClipRect(lpRect);
	LONG lPitch	 = GetRect()->right;
	DWORD* pSurface = GetPtr();

	static DWORD dwTable[256];
	static int nSelectTable = -1;
	if (nSelectTable != nFade){
	//	テーブルの作りなおし
		nSelectTable = nFade;
		nFade <<= 16;	//　ゲタ履き
		DWORD dw = 0;
		for(int i=0;i<256;++i,dw+=nFade)
			dwTable[i] = dw & 0xff000000;
	}

	for(int y=r.top;y<r.bottom;y++){
		DWORD *p = pSurface + y*lPitch + r.left;
		for(int x=r.left;x<r.right;x++){
			DWORD dw = *p;
			//	αだけをテーブル参照で変換して、下位24ビットは無変更
			dw =  (dwTable[(dw & 0xff000000)>>24])
				+ (dw & 0xffffff);
			*(p++) = dw;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
LRESULT CDIB32P5::BltToAlpha(CDIB32* lpSrc,int nSrcMin,int nSrcMax,int nDstMin,int nDstMax,LPRECT lpRect){
	//	プレーンのα値を落とす機能	added '00/11/07
	//	手抜きだけど、それなりに速いので、いいやー＾＾；

	if (lpSrc->GetRect()->right!=GetRect()->right ||
		lpSrc->GetRect()->bottom!=GetRect()->bottom){
		//	転送元と転送先とのプレーンのサイズちゃうやん！
		return 1;
	}

	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BltToAlphaでm_lpdwSrc == NULL");
	{
		CDIB32Base* p = lpSrc->GetDIB32BasePtr();
		WARNING(p->GetPtr() == NULL,"CDIB32P5::BltToAlphaでp->GetPtr() == NULL");
	}

	RECT r = GetClipRect(lpRect);
	LONG lPitch	 = GetRect()->right;
	DWORD* pSurface = lpSrc->GetPtr();
	DWORD* pSurface2 = GetPtr();

	static DWORD dwTable[256];
	//	テーブルは毎回作りなおす
	{
		int i,j;
		DWORD dw;
		dw = nDstMin << 24;
		for(i=0;i<=nSrcMin && i<256;++i)
			dwTable[i] = dw;
		i = nSrcMin+1; if (i<0) i=0;
		for(j=1;i<nSrcMax && i<256;++i,++j) {
			dw = (DWORD)(nDstMin + j * (nDstMax-nDstMin) / (nSrcMax - nSrcMin));
			dwTable[i] = (dw<<24) & 0xff000000;
		}
		dw = nDstMax << 24;
		i=nSrcMax; if (i<0) i=0;
		for(;i<256;++i)
			dwTable[i] = dw;

	}

	for(int y=r.top;y<r.bottom;y++){
		DWORD *p = pSurface + y*lPitch + r.left;	// src
		DWORD *q = pSurface2+ y*lPitch + r.left;	// dst
		for(int x=r.left;x<r.right;x++){
			DWORD dw = *(p++);
			//	Bだけをテーブル参照で変換して、αに
			dw =  dwTable[dw & 0xff];
			//	dstの下位24ビットは無変更
			*q = (*q & 0xffffff) + dw;
			q++;
		}
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////
LRESULT CDIB32P5::FadeBltAlpha(CDIB32* lpSrc,int nFade,LPRECT lpRect){
	//	プレーンのα値を落としながら転送する機能	added '00/11/08
	//	手抜きだけど、それなりに速いので、いいやー＾＾；

	if (lpSrc->GetRect()->right!=GetRect()->right ||
		lpSrc->GetRect()->bottom!=GetRect()->bottom){
		//	転送元と転送先とのプレーンのサイズちゃうやん！
		return 1;
	}

	WARNING(m_lpdwSrc == NULL,"CDIB32P5::FadeBltAlphaでm_lpdwSrc == NULL");
	{
		CDIB32Base* p = lpSrc->GetDIB32BasePtr();
		WARNING(p->GetPtr() == NULL,"CDIB32P5::FadeBltAlphaでp->GetPtr() == NULL");
	}

	RECT r = GetClipRect(lpRect);
	LONG lPitch	 = GetRect()->right;
	DWORD* pSurface = lpSrc->GetPtr();
	DWORD* pSurface2 = GetPtr();

	static DWORD dwTable[256];
	static int nSelectTable = -1;
	if (nSelectTable != nFade){
	//	テーブルの作りなおし
		nSelectTable = nFade;
		nFade <<= 16;	//　ゲタ履き
		DWORD dw = 0;
		for(int i=0;i<256;++i,dw+=nFade)
			dwTable[i] = dw & 0xff000000;
	}

	for(int y=r.top;y<r.bottom;y++){
		DWORD *p = pSurface + y*lPitch + r.left;	// src
		DWORD *q = pSurface2+ y*lPitch + r.left;	// dst
		for(int x=r.left;x<r.right;x++){
			DWORD dw = *(p++);
			//	転送元αだけをテーブル参照で変換して、転送先αに
			*(q++) = (dw & 0xffffff) + dwTable[dw >> 24];
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//	αチャンネル付き転送系

//////////////////////////////////////
//	BltClearAlpha
//	抜き色有りの転送(α値無効化)
//////////////////////////////////////
LRESULT CDIB32P5::BltClearAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BltClearAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32P5::BltClearAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;
	DWORD	lPitchSrc = p->m_lPitch;
	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth =	  m_lPitch - (nWidth<<2);									// ASMで使用する 1ラインのバイト数の設定
	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;

// -----------	 Pentium   カラーキー:有   ミラー:無   拡縮:無	α値:無	 --------------
	if ( m_bActualSize == true )
	{
		DWORD		colKey = p->m_dwColorKey;


		_asm
		{
			MOV		ECX, nWidth
			MOV		EDX, nHeight

			MOV		EDI, lpDst
			MOV		ESI, lpSrc

			MOV		EBX, colKey				// UnPair

		LoopX_22:	// 6(5)クロック･サイクル
			MOV		EAX, [ESI]
			ADD		ESI, 4

			AND		EAX, nonAlphaMask		//	UnPair			//	α値をマスクする

			CMP		EAX, EBX
			JE		Skip_22

			MOV		[EDI], EAX				// UnPair

		Skip_22:
			ADD		EDI, 4										// EDIを先に進める
			DEC		ECX

			JNZ		LoopX_22				// UnPair

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nAddSrcWidth							// クリップした領域分を飛ばす
			DEC		EDX

			JNZ		LoopX_22				// UnPair
		}
	}// if
// -----------	 Pentium   カラーキー:有   ミラー:無   拡縮:有	α値:無	 --------------
	else
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		int		i, j;
		BYTE*	lpSrcBack;

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;
		LoopY_23:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX				// UnPair			// EY = InitializeX;

		LoopX_23: // 8(7)クロック･サイクル
			MOV		EAX, [ESI]									// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			AND		EAX, nonAlphaMask		//	UnPair			//	α値をマスクする

			CMP		EAX, colKey
			JE		SkipColKey_23

			MOV		[EDI], EAX				// UnPair

		SkipColKey_23:
			ADD		EDI, 4					// UnPair			// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_23									// if ( EX >= 0)

			ADD		ESI, 4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_23:
			DEC		i
			JNZ		LoopX_23

			MOV		ESI, lpSrcBack;								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_23				// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc
			SUB		EDX, EY2				// UnPair			// Yの補正
			
		SkipY_23:
			DEC		j
			JNZ		LoopY_23
		}
	} // if

	
	return 0;
} // BltClearAlpha


//////////////////////////////////////
//	BltFastClearAlpha
//	抜き色無しの転送 (α値は無視)
//////////////////////////////////////
LRESULT CDIB32P5::BltFastClearAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BltFastClearAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32P5::BltFastClearAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);				// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;


// -----------	 Pentium   カラーキー:無   ミラー:無   拡縮:無	α値:無	 --------------
	if ( m_bActualSize == true )
	{
		_asm
		{
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, nHeight
			MOV		ECX, nWidth

		LoopX_24:	// 4クロック･サイクル
			MOV		EAX, [ESI]
			ADD		EDI, 4										// EDIを先に進める

			AND		EAX, nonAlphaMask							//	α値をマスクする
			ADD		ESI, 4

			MOV		[EDI-4], EAX
			DEC		ECX

			JNZ		LoopX_24				// UnPair

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nAddSrcWidth							// クリップした領域分を飛ばす
			DEC		EDX

			JNZ		LoopX_24				// UnPair
		}
	} // if
// -----------	 Pentium   カラーキー:無   ミラー:無   拡縮:有	α値:無	 --------------
	else
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		int		i, j;
		BYTE*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;
		

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;

		LoopY_25:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX									// EY = InitializeX;

		LoopX_25: // 6クロック･サイクル
			MOV		EAX, [ESI]									// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			AND		EAX, nonAlphaMask		//	UnPair			//	α値をマスクする

			MOV		[EDI], EAX
			ADD		EDI, 4										// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_25									// if ( EX >= 0)

			ADD		ESI, 4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_25:
			DEC		i
			JNZ		LoopX_25

			MOV		ESI, lpSrcBack								// Srcをラインの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_25					// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc			// 1ライン分加算して、次の行へ
			SUB		EDX, EY2									// Yの補正

		SkipY_25:
			DEC		j
			JNZ		LoopY_25
		}
	} // if

	
	return 0;
} // BltFastClearAlpha


///////////////////////////////////////////////
//	BlendBltFastAlpha
//	抜き色関係なしのα値反映ブレンド転送
///////////////////////////////////////////////
LRESULT CDIB32P5::BlendBltFastAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BlendBltFastAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32P5::BlendBltFastAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);			// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	alphaMask = 0xff000000;

// -----------	 Pentium   カラーキー:有   ミラー:無   拡縮:無	 α値:反映	  --------------
	if ( m_bActualSize == true )
	{
		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
		int		i, j;
		DWORD	colKey = p->m_dwColorKey;


		_asm
		{
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX

		LoopX_10: // 35クロック･サイクル (MULを 9クロックと数えると59クロック)
			MOV		EAX, [ESI]
			ADD		ESI, 4

			MOV		EBX, EAX				// UnPair

			AND		EBX, alphaMask			// UnPair			// Srcのα値を得る

			MOV		ECX, EBX
			MOV		EDX, EBX

			SHR		EBX, 24					// UnPair

			SHR		ECX, 16					// UnPair

			SHR		EDX, 8
			ADD		EBX, ECX

			ADD		EBX, EDX				// UnPair

			MOV		EDX, EBX				// UnPair

			AND		EDX, 0x00ff00ff			// UnPair			// bgRateSrcは、0x00aa00aaにする
			// 補整
			CMP		EDX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiA10

			MOV		EDX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiA10:
			MOV		bgRateSrc, EDX								//	Srcのα値算出完了

			NOT		EBX						// UnPair

			AND		EBX, 0x00ff00ff			// bgRateDstは、0x00aa00aaにする
			AND		EDX, 0x000001ff
			// 補整
			CMP		EBX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiB10

			MOV		EBX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiB10:
			MOV		bgRateDst, EBX								//	Dstのα値算出完了
			AND		EBX, 0x01ff0000

			OR		EBX, EDX
			MOV		ECX, [EDI]				// 2

			MOV		rRate, EBX				// rRateは、0x00da00saを入れる
			MOV		EBX, EAX				// 1

			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする
			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0x00ff0000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			MOV		SrcBackup, EAX			// 1
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		ECX, EDX				// 3
			MOV		EAX, EBX				// 2

			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// EDIを先に進める
			DEC		i

			JNZ		LoopX_10				// UnPair

			MOV		EAX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nAddSrcWidth							// クリップした領域分を飛ばす
			MOV		i, EAX

			DEC		j
			JNZ		LoopX_10
		}
	}
// -----------	 Pentium   カラーキー:無   ミラー:無   拡縮:有	 α値:反映	  --------------
	else
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;

		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
		int		nEXCnt, nEYCnt;
		int		i, j;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;


		_asm
		{
			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX

			MOV		EDI, lpDst
			MOV		EAX, EIY

			MOV		ESI, lpSrc
			MOV		nEYCnt, EAX									// EY = InitializeY;

		LoopY_13:
			MOV		lpSrcBack, ESI
			MOV		EAX, EIX				// UnPair

			MOV		nEXCnt, EAX				// UnPair			// EX = InitializeX;

		LoopX_13: // 37クロック･サイクル (MULを 9クロックと数えると61クロック)
			MOV		EAX, [ESI]									// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			MOV		EBX, EAX				// UnPair

			AND		EBX, alphaMask			// UnPair			// Srcのα値を得る

			MOV		ECX, EBX
			MOV		EDX, EBX

			SHR		EBX, 24					// UnPair

			SHR		ECX, 16					// UnPair

			SHR		EDX, 8
			ADD		EBX, ECX

			ADD		EBX, EDX				// UnPair

			MOV		EDX, EBX				// UnPair

			AND		EDX, 0x00ff00ff			// UnPair			// bgRateSrcは、0xaa0000aaにする
			// 補整
			CMP		EDX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiA13

			MOV		EDX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiA13:
			MOV		bgRateSrc, EDX								//	Srcのα値算出完了

			NOT		EBX						// UnPair

			AND		EBX, 0x00ff00ff			// bgRateDstは、0xaa0000aaにする
			AND		EDX, 0x000001ff
			// 補整
			CMP		EBX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiB13

			MOV		EBX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiB13:
			MOV		bgRateDst, EBX								//	Dstのα値算出完了
			AND		EBX, 0x1ff0000

			OR		EBX, EDX
			MOV		ECX, [EDI]				// 2

			MOV		rRate, EBX				// rRateは、0xda0000saを入れる
			MOV		EBX, EAX				// 1

			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする
			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0x00ff0000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			MOV		SrcBackup, EAX			// 1
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		ECX, EDX				// 3
			MOV		EAX, EBX				// 2// UnPair

//			MUL		bgRateSrc				// UnPair
			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// Dstを次に進める
			MOV		EAX, EX

			ADD		nEXCnt, EAX									// Xの増分を加算
			JNB		SkipX_13										// if ( EX >= 0)

			ADD		ESI, 4										// Srcを次に進める
			MOV		EAX, EX2

			SUB		nEXCnt, EAX				// UnPair			// Xの補正

		SkipX_13:
			DEC		i
			JNZ		LoopX_13

			MOV		ESI, lpSrcBack								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			MOV		EAX, EY

			ADD		nEYCnt, EAX									// Yの増分を加算
			JNB		SkipY_13									// if ( EY >= 0 )

			ADD		ESI, lPitchSrc				// 1ライン分加算して、次の行へ
			MOV		EAX, EY2

			SUB		nEYCnt, EAX									// Yの補正

		SkipY_13:
			DEC		j
			JNZ		LoopY_13
		}
	} // if


	
	return 0;
} // BlendBltFastAlpha

// kaine 2001/2/28
///////////////////////////////////////////////
//	BlendBltFastAlpha
//	抜き色関係なしのα値反映ブレンド比率指定転送
///////////////////////////////////////////////
LRESULT CDIB32P5::BlendBltFastAlpha(CDIB32* lpDIB32,int x,int y,DWORD dwDstRGBRate,DWORD dwSrcRGBRate
							   ,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32Base::BlendBltFastでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p == NULL || p->GetPtr() == NULL,"CDIB32Base::BlendBltFastでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);			// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	alphaMask = 0xff000000;

	// (E-dwDstRGBRate)[alpha] = (E-dwDstRGBRate) * alpha
	DWORD	dwDstR = 255 - (   dwDstRGBRate >> 16) ;
	DWORD	dwDstG = 255 - ( ( dwDstRGBRate & 0x0000ff00) >> 8 ) ;
	DWORD	dwDstB = 255 - (   dwDstRGBRate & 0x000000ff) ;
	DWORD	dwSrcR = dwSrcRGBRate >> 16 ;
	DWORD	dwSrcG = ( dwSrcRGBRate & 0x0000ff00) >> 8	;
	DWORD	dwSrcB = dwSrcRGBRate & 0x000000ff ;
	static DWORD	dwDstGBRate[256];
	static DWORD	dwSrcGBRate[256];
	static DWORD	dwRRate[256];
	DWORD	dR,dG,dB,sR,sG,sB;
	dR = dG = dB = sR =sG=sB=0;
	for ( int k = 0	 ; k < 256 ; k++, 
					dR+=dwDstR,dG+=dwDstG,dB+=dwDstB,
					sR+=dwSrcR,sG+=dwSrcG,sB+=dwSrcB
		){
		if ( k == 255 ) {
			dR+=dwDstR;dG+=dwDstG;dB+=dwDstB;
			sR+=dwSrcR;sG+=dwSrcG;sB+=dwSrcB;}
		dwDstGBRate[k]	= ( (~ ( dB >> 8 ) << 24 & 0xff000000) ) | ( ~( dG >> 8 ) & 0x000000ff) ;
		dwSrcGBRate[k]	= ( ( sB >> 8 ) << 24 ) | ( sG >> 8 ) ;
		dwRRate[k]		= ( ~( dR >> 8 ) << 24 ) | ( sR >> 8 ) ;
	}
	
// -----------	 Pentium   カラーキー:無   ミラー:無   拡縮:無	 RGBブレンド:有	  --------------
	if ( m_bActualSize == true )
	{
		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
//		DWORD	DstBackup;
		int		i, j;
		DWORD	*lpDstGBRate = &dwDstGBRate[0];
		DWORD	*lpSrcGBRate = &dwSrcGBRate[0];
		DWORD	*lpRRate = &dwRRate[0];
	
		_asm
		{
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX

		LoopX_12: // ??クロック･サイクル (MULを 9クロックと数えると クロック)
			mov		eax, [ESI]
			mov		ebx, lpSrcGBRate

			and		eax, 0xff000000
			mov		ecx, lpDstGBRate

			shr		eax,22
			ADD		EDI,4

			mov		edx, lpRRate // rate0
			ADD		ESI,8

			mov		ebx,[ebx+eax] // Src2
			mov		ecx,[ecx+eax] // Dst2
			
			mov		bgRateSrc,ebx // Src3
			ADD		EDI,-4

			mov		edx,[edx+eax] // rate2
			mov		bgRateDst,ecx // Dst3

			MOV		EAX, [ESI-8]
			mov		rRate,edx //rate3

			ADD		ESI,-4
//------------------------------
			MOV		ECX, [EDI]				// 2
			MOV		EBX, EAX				// 1

			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る
			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0xff000000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			SHR		EDX, 8					// 3
			MOV		SrcBackup, EAX			// 1

			MOV		ECX, EDX				// 3
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		EAX, EBX				// 2// UnPair

			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// EDIを先に進める
			DEC		i

			JNZ		LoopX_12				// UnPair

			MOV		EAX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nAddSrcWidth							// クリップした領域分を飛ばす
			MOV		i, EAX

			DEC		j
			JNZ		LoopX_12
		}
	}// if
// -----------	 Pentium   カラーキー:無   ミラー:無   拡縮:有	 RGBブレンド:有	  --------------
	else
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;

		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
		int		nEXCnt, nEYCnt;
		int		i, j;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;
		DWORD	*lpDstGBRate = &dwDstGBRate[0];
		DWORD	*lpSrcGBRate = &dwSrcGBRate[0];
		DWORD	*lpRRate = &dwRRate[0];


		_asm
		{
			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX

			MOV		EDI, lpDst

			MOV		ESI, lpSrc
			MOV		EAX, EIY

			MOV		nEYCnt, EAX				// UnPair			// EY = InitializeY;

		LoopY_13:
			MOV		lpSrcBack, ESI
			MOV		EAX, EIX				// UnPair

			MOV		nEXCnt, EAX				// UnPair			// EX = InitializeX;

		LoopX_13: // 37クロック･サイクル (MULを 9クロックと数えると61クロック)
			MOV		EAX, [ESI]									// ピクセルのコピー
			mov		ebx, lpSrcGBRate

			and		eax, 0xff000000
			mov		ecx, lpDstGBRate

			shr		eax,22
			ADD		EDI,2

			mov		edx, lpRRate // rate0
			ADD		EDI,2

			mov		ebx,[ebx+eax] // Src2
			mov		ecx,[ecx+eax] // Dst2
			
			mov		bgRateSrc,ebx // Src3
			ADD		EDI,-4

			mov		edx,[edx+eax] // rate2
			mov		bgRateDst,ecx // Dst3

			mov		rRate,edx //rate3
//---------
			MOV		EAX, [ESI]
			ADD		ESI, AddSrcPixel							// 整数部の加算
			
			MOV		ECX, [EDI]				// 2
			MOV		EBX, EAX				// 1

			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする
			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0xff000000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			SHR		EDX, 8					// 3
			MOV		SrcBackup, EAX			// 1

			MOV		ECX, EDX				// 3
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		EAX, EBX				// 2// UnPair

			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// Dstを次に進める
			MOV		EAX, EX

			ADD		nEXCnt, EAX									// Xの増分を加算
			JNB		SkipX_13										// if ( EX >= 0)

			ADD		ESI, 4										// Srcを次に進める
			MOV		EAX, EX2

			SUB		nEXCnt, EAX				// UnPair			// Xの補正

		SkipX_13:
			DEC		i
			JNZ		LoopX_13

			MOV		ESI, lpSrcBack								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			MOV		EAX, EY

			ADD		nEYCnt, EAX									// Yの増分を加算
			JNB		SkipY_13									// if ( EY >= 0 )

			ADD		ESI, lPitchSrc				// 1ライン分加算して、次の行へ
			MOV		EAX, EY2

			SUB		nEYCnt, EAX									// Yの補正

		SkipY_13:
			DEC		j
			JNZ		LoopY_13
		}
	} // if

	
	return 0;
} // BlendBltFastAlpha

//////////////////////////////////////
//	BltClearAlphaM
//	抜き色有りの転送 (α値無効化)
//////////////////////////////////////
LRESULT CDIB32P5::BltClearAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BltClearAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32P5::BltClearAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;

// -----------	 Pentium   カラーキー:有   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
		DWORD	colKey = p->m_dwColorKey;

				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV		ECX, nWidth
			MOV		EDX, nHeight

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EBX, colKey				// UnPair

		LoopY_1:

		LoopX_1:	// 5(4)クロック･サイクル
			MOV		EAX, [ESI-4]
			ADD		ESI, -4

			AND		EAX, nonAlphaMask		//	UnPair			//	α値をマスクする

			CMP		EAX, EBX
			JE		Skip_1

			MOV		[EDI], EAX				// UnPair

		Skip_1:
			ADD		EDI, 4										// EDIを先に進める
			DEC		ECX

			JNZ		LoopX_1					// UnPair

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nSrcWidth								// 次ラインへの移動
			DEC		EDX

			JNZ		LoopY_1					// UnPair
		}
	}// if
// -----------	 Pentium   カラーキー:有   ミラー:有   拡縮:有	 --------------
	else
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		int		i, j;
		BYTE*	lpSrcBack;
		DWORD	lPitchSrc =	 p->m_lPitch;

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;

		LoopY_2:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX				// UnPair			// EY = InitializeX;

		LoopX_2: // 7(6)クロック･サイクル
			MOV		EAX, [ESI-4]								// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			AND		EAX, nonAlphaMask		//	UnPair			//	α値をマスクする

			CMP		EAX, colKey
			JE		SkipColKey_2

			MOV		[EDI], EAX				// UnPair

		SkipColKey_2:
			ADD		EDI, 4					// UnPair			// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_2										// if ( EX >= 0)

			ADD		ESI, -4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_2:
			DEC		i
			JNZ		LoopX_2

			MOV		ESI, lpSrcBack;								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_2					// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc
			SUB		EDX, EY2									// Yの補正

		SkipY_2:
			DEC		j
			JNZ		LoopY_2
		}
	} // if


	return 0;
} // BltClearAlphaM


//////////////////////////////////////
//	BltFastClearAlphaM
//	抜き色無しの転送 (α値無視)
//////////////////////////////////////
LRESULT CDIB32P5::BltFastClearAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BltFastClearAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32P5::BltFastClearAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;

// -----------	 Pentium   カラーキー:無   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, nHeight
			MOV		ECX, nWidth

		LoopX_3: // 3クロック･サイクル
			MOV		EAX, [ESI-4]
			ADD		EDI, 4

			AND		EAX, nonAlphaMask							//	α値をマスクする
			ADD		ESI, -4

			MOV		[EDI-4], EAX 
			DEC		ECX

			JNZ		LoopX_3					// UnPair

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nSrcWidth								// 次ラインへの移動
			DEC		EDX

			JNZ		LoopX_3					// UnPair
		}
	} // if
// -----------	 Pentium   カラーキー:無   ミラー:有   拡縮:有	 --------------
	else
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		int		i, j;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;

		LoopY_4:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX				// UnPair			// EY = InitializeX;

		LoopX_4: // 5クロック･サイクル
			MOV		EAX, [ESI-4]								// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			AND		EAX, nonAlphaMask		//	UnPair			//	α値をマスクする

			MOV		[EDI], EAX
			ADD		EDI, 4										// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_4										// if ( EX >= 0)

			ADD		ESI, -4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_4:
			DEC		i
			JNZ		LoopX_4

			MOV		ESI, lpSrcBack								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_4					// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc
			SUB		EDX, EY2									// Yの補正

		SkipY_4:
			DEC		j
			JNZ		LoopY_4
		}
	} // if

	
	return 0;
} // BltFastClearAlphaM


//////////////////////////////////////
//	BlendBltFastAlphaM
//	抜き色関係なしのα値反映ブレンド転送
//////////////////////////////////////
LRESULT CDIB32P5::BlendBltFastAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BlendBltFastAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32P5::BlendBltFastAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	alphaMask = 0xff000000;

// -----------	 Pentium   カラーキー:有   ミラー:有   拡縮:無	 RGBブレンド:有	  --------------
	if ( m_bActualSize == true )
	{
		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
		int		i, j;
		DWORD	colKey = p->m_dwColorKey;

				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX


		LoopX_9: // 35クロック･サイクル (MULを 9クロックと数えると59クロック)
			MOV		EAX, [ESI-4]
			ADD		ESI, -4

			MOV		EBX, EAX				// UnPair

			AND		EBX, alphaMask			// UnPair			// Srcのα値を得る

			MOV		ECX, EBX
			MOV		EDX, EBX

			SHR		EBX, 24					// UnPair

			SHR		ECX, 16					// UnPair
			
			SHR		EDX, 8
			ADD		EBX, ECX

			ADD		EBX, EDX				// UnPair

			MOV		EDX, EBX				// UnPair

			AND		EDX, 0x00ff00ff			// UnPair			// bgRateSrcは、0x00aa00aaにする
			// 補整
			CMP		EDX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiA9

			MOV		EDX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiA9:
			MOV		bgRateSrc, EDX								//	Srcのα値算出完了

			NOT		EBX						// UnPair

			AND		EBX, 0x00ff00ff			// bgRateDstは、0x00aa00aaにする
			AND		EDX, 0x000001ff
			// 補整
			CMP		EBX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiB9

			MOV		EBX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiB9:
			MOV		bgRateDst, EBX								//	Dstのα値算出完了
			AND		EBX, 0x01ff0000

			OR		EBX, EDX
			MOV		ECX, [EDI]				// 2

			MOV		rRate, EBX				// rRateは、0x00da00saを入れる
			MOV		EBX, EAX				// 1

			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする
			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0x00ff0000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			MOV		SrcBackup, EAX			// 1
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		ECX, EDX				// 3
			MOV		EAX, EBX				// 2// UnPair

//			MUL		bgRateSrc				// UnPair
			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// EDIを先に進める
			DEC		i

			JNZ		LoopX_9				// UnPair

			MOV		EAX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nSrcWidth								// 次ラインへの移動
			MOV		i, EAX

			DEC		j
			JNZ		LoopX_9
		}
	}// if
// -----------	 Pentium   カラーキー:有   ミラー:有   拡縮:有	 RGBブレンド:有	  --------------
	else
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;

		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
		int		nEXCnt, nEYCnt;
		int		i, j;
		DWORD	colKey = p->m_dwColorKey;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX

			MOV		EDI, lpDst
			MOV		EAX, EIY

			MOV		ESI, lpSrc
			MOV		nEYCnt, EAX									// EY = InitializeY;

		LoopY_10:
			MOV		lpSrcBack, ESI
			MOV		EAX, EIX				// UnPair			// ペアにならないけど・・・まぁ、yループ毎だからいっか:p

			MOV		nEXCnt, EAX				// UnPair			// EX = InitializeX;

		LoopX_10: // 34クロック･サイクル (MULを 9クロックと数えると58クロック)
			MOV		EAX, [ESI-4]								// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			MOV		EBX, EAX				// UnPair

			AND		EBX, alphaMask			// UnPair			// Srcのα値を得る

			MOV		ECX, EBX
			MOV		EDX, EBX

			SHR		EBX, 24					// UnPair

			SHR		ECX, 16					// UnPair
			
			SHR		EDX, 8
			ADD		EBX, ECX

			ADD		EBX, EDX				// UnPair

			MOV		EDX, EBX				// UnPair

			AND		EDX, 0x00ff00ff			// UnPair			// bgRateSrcは、0x00aa00aaにする
			// 補整
			CMP		EDX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiA10

			MOV		EDX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiA10:
			MOV		bgRateSrc, EDX								//	Srcのα値算出完了

			NOT		EBX						// UnPair

			AND		EBX, 0x00ff00ff			// bgRateDstは、0x00aa00aaにする
			AND		EDX, 0x000001ff
			// 補整
			CMP		EBX, 0x00ff00ff								//	255か否かの判定
			JNE		Skip_ManiB10

			MOV		EBX, 0x01000100			// UnPair			//	255を256に補整する

		Skip_ManiB10:
			MOV		bgRateDst, EBX								//	Dstのα値算出完了
			AND		EBX, 0x01ff0000

			OR		EBX, EDX
			MOV		ECX, [EDI]				// 2

			MOV		rRate, EBX				// rRateは、0xda0000saを入れる
			MOV		EBX, EAX				// 1

			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする
			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0x00ff0000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			MOV		SrcBackup, EAX			// 1
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		ECX, EDX				// 3
			MOV		EAX, EBX				// 2// UnPair

//			MUL		bgRateSrc				// UnPair
			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 16
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// Dstを次に進める
			MOV		EAX, EX

			ADD		nEXCnt, EAX									// Xの増分を加算
			JNB		SkipX_10									// if ( EX >= 0)

			ADD		ESI, -4										// Srcを次に進める
			MOV		EAX, EX2

			SUB		nEXCnt, EAX				// UnPair			// Xの補正

		SkipX_10:
			DEC		i
			JNZ		LoopX_10

			MOV		ESI, lpSrcBack								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			MOV		EAX, EY										// Yの増分を加算

			ADD		nEYCnt, EAX									// Yの増分を加算
			JNB		SkipY_10									// if ( EY >= 0 )

			MOV		EAX, EY2									// Yの補正
			ADD		ESI, lPitchSrc								// クリップした領域分を飛ばす

			SUB		nEYCnt, EAX				// UnPair			// Yの補正

		SkipY_10:
			DEC		j
			JNZ		LoopY_10
		}
	} // if

	
	return 0;
} // BlendBltFastAlphaM

// kaine 2001/2/28
///////////////////////////////////////////////
//	BlendBltFastAlphaM
//	抜き色関係なしのα値反映ブレンド比率指定転送
///////////////////////////////////////////////
LRESULT CDIB32P5::BlendBltFastAlphaM(CDIB32* lpDIB32,int x,int y,DWORD dwDstRGBRate,DWORD dwSrcRGBRate
							   ,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32P5::BlendBltFastAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32P5::BlendBltFastAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	alphaMask = 0xff000000;

	// (E-dwDstRGBRate)[alpha] = (E-dwDstRGBRate) * alpha
	DWORD	dwDstR = 255 - (   dwDstRGBRate >> 16) ;
	DWORD	dwDstG = 255 - ( ( dwDstRGBRate & 0x0000ff00) >> 8 ) ;
	DWORD	dwDstB = 255 - (   dwDstRGBRate & 0x000000ff) ;
	DWORD	dwSrcR = dwSrcRGBRate >> 16 ;
	DWORD	dwSrcG = ( dwSrcRGBRate & 0x0000ff00) >> 8	;
	DWORD	dwSrcB = dwSrcRGBRate & 0x000000ff ;
	static DWORD	dwDstGBRate[256];
	static DWORD	dwSrcGBRate[256];
	static DWORD	dwRRate[256];
	DWORD	dR,dG,dB,sR,sG,sB;
	dR = dG = dB = sR =sG=sB=0;
	for ( int k = 0	 ; k < 256 ; k++, 
					dR+=dwDstR,dG+=dwDstG,dB+=dwDstB,
					sR+=dwSrcR,sG+=dwSrcG,sB+=dwSrcB
		){
		if ( k == 255 ) {
			dR+=dwDstR;dG+=dwDstG;dB+=dwDstB;
			sR+=dwSrcR;sG+=dwSrcG;sB+=dwSrcB;}
		dwDstGBRate[k]	= ( (~ ( dB >> 8 ) << 24 & 0xff000000) ) | ( ~( dG >> 8 ) & 0x000000ff) ;
		dwSrcGBRate[k]	= ( ( sB >> 8 ) << 24 ) | ( sG >> 8 ) ;
		dwRRate[k]		= ( ~( dR >> 8 ) << 24 ) | ( sR >> 8 ) ;
	}
	
// -----------	 Pentium   カラーキー:無   ミラー:有   拡縮:無	 RGBブレンド:有	  --------------
	if ( m_bActualSize == true )
	{
		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
//		DWORD	DstBackup;
		int		i, j;
		DWORD	*lpDstGBRate = &dwDstGBRate[0];
		DWORD	*lpSrcGBRate = &dwSrcGBRate[0];
		DWORD	*lpRRate = &dwRRate[0];
		nSrcWidth = nSrcWidth + p->m_lPitch;

		_asm
		{
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX

		LoopX_9: // ??クロック･サイクル (MULを 9クロックと数えると クロック)
			mov		eax, [ESI-4]
			mov		ebx, lpSrcGBRate

			and		eax, 0xff000000
			mov		ecx, lpDstGBRate

			shr		eax,22
			ADD		EDI,2

			add		ebx,eax // Src1
			add		ecx,eax // Dst1

			mov		edx, lpRRate // rate0
			ADD		ESI,-2

			mov		ebx,[ebx] // Src2
			mov		ecx,[ecx] // Dst2
			
			mov		bgRateSrc,ebx // Src3
			add		edx,eax	 // rate1

			MOV		EAX, [ESI-4+2]
			ADD		EDI,-2

			mov		edx,[edx] // rate2
			mov		bgRateDst,ecx // Dst3
		
			mov		rRate,edx //rate3
			ADD		ESI,-2
//------------------------------
			MOV		ECX, [EDI]				// 2
			MOV		EBX, EAX				// 1

			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする
			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0xff000000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			SHR		EDX, 8					// 3
			MOV		SrcBackup, EAX			// 1

			MOV		ECX, EDX				// 3
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		EAX, EBX				// 2// UnPair

			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// EDIを先に進める
			DEC		i

			JNZ		LoopX_9				// UnPair

			MOV		EAX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nSrcWidth								// 次ラインへの移動
			MOV		i, EAX

			DEC		j
			JNZ		LoopX_9
		}
	}// if
// -----------	 Pentium   カラーキー:無   ミラー:有  拡縮:有  RGBブレンド:有	  --------------
	else
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;

		DWORD	feMask = 0x00fefefe;
		DWORD	ovMask = 0x01010100;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD	bgRateSrc;
		DWORD	bgRateDst;
		DWORD	rRate;
		DWORD	SrcBackup;
		int		nEXCnt, nEYCnt;
		int		i, j;
		DWORD	colKey = p->m_dwColorKey;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		DWORD	*lpDstGBRate = &dwDstGBRate[0];
		DWORD	*lpSrcGBRate = &dwSrcGBRate[0];
		DWORD	*lpRRate = &dwRRate[0];


		_asm
		{
			MOV		EAX, nHeight
			MOV		EBX, nWidth

			MOV		j, EAX
			MOV		i, EBX

			MOV		EDI, lpDst

			MOV		ESI, lpSrc
			MOV		EAX, EIY

			MOV		nEYCnt, EAX				// UnPair			// EY = InitializeY;

		LoopY_10:
			MOV		lpSrcBack, ESI
			MOV		EAX, EIX				// UnPair

			MOV		nEXCnt, EAX				// UnPair			// EX = InitializeX;

		LoopX_10: // 37クロック･サイクル (MULを 9クロックと数えると61クロック)
			MOV		EAX, [ESI-4]									// ピクセルのコピー
			mov		ebx, lpSrcGBRate

			and		eax, 0xff000000
			mov		ecx, lpDstGBRate

			shr		eax,22
			ADD		EDI,2

			add		ebx,eax // Src1
			add		ecx,eax // Dst1

			mov		edx, lpRRate // rate0
			ADD		EDI,2

			mov		ebx,[ebx] // Src2
			mov		ecx,[ecx] // Dst2
			
			mov		bgRateSrc,ebx // Src3
			add		edx,eax	 // rate1

			MOV		EAX, [ESI-4]
			SUB		EDI,4

			mov		edx,[edx] // rate2
			mov		bgRateDst,ecx // Dst3
		
			mov		rRate,edx //rate3
			ADD		ESI, AddSrcPixel							// 整数部の加算
			
			MOV		ECX, [EDI]				// 2
			MOV		EBX, EAX				// 1

			ROR		EAX, 8					// 1				// αＲＧＢをＢαＲＧにする
			AND		ECX, 0x00ff0000			// 2				// ０Ｒ００にするためにマスクを取る

			SHL		ECX, 8					// 2
			AND		EBX, 0x00ff0000			// 1				// ０Ｒ００にするためにマスクを取る

			SHR		EBX, 16					// 1
			AND		EAX, 0xff0000ff			// 1				// Ｂ００Ｇにするためにマスクを取る

			MUL		bgRateSrc				// UnPair

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			OR		EBX, ECX									// dR００sRができた

			MOV		ECX, EAX									// Srcの0x0000GGBBができた
			MOV		EAX, EBX

			MUL		rRate					// UnPair			// EAX:SrcのＲ、EDX:DstのＲ

			AND		EAX, 0x0000ff00			// 1
			MOV		EBX, [EDI]				// 2

			SHL		EAX, 8					// 1
			AND		EDX, 0xff000000			// 3

			ROR		EBX, 8					// 2				// αＲＧＢをＢαＲＧにする
			OR		EAX, ECX				// 1				// Srcの0x00RRGGBBの完成

			SHR		EDX, 8					// 3
			MOV		SrcBackup, EAX			// 1

			MOV		ECX, EDX				// 3
			AND		EBX, 0xff0000ff			// 2				// Ｂ００Ｇにするためにマスクを取る

			MOV		EAX, EBX				// 2// UnPair

			MUL		bgRateDst				// UnPair			//	間違ってた by Tia

			SHR		EDX, 24
			AND		EAX, 0x0000ff00

			OR		EAX, EDX
			MOV		EBX, SrcBackup

			OR		EAX, ECX				// UnPair			// Dstの0x00RRGGBBの完成

			// ---------- サチュレーションADD ------------
			MOV		EDX, EAX									// DSTの保存
			MOV		ECX, EAX

			AND		EAX, EBX				// UnPair			// (src&dst)

			SHL		EAX, 1										// <<1
			XOR		ECX, EBX									// (src^dst)

			AND		ECX, feMask				// UnPair			// & feMask

			ADD		EAX, ECX				// UnPair			// ()+()

			AND		EAX, ovMask				// UnPair			// & ovMask
			
			SHR		EAX, 8										//	(c >> 8)
			MOV		EBX, SrcBackup

			ADD		EAX, Mask7f									// + 0x7f7f7f
			MOV		ECX, EDX

			XOR		EAX, Mask7f									//	^ 0x7f7f7f
			ADD		EBX, ECX									// src + dst

			SUB		EBX, EAX				// UnPair			// ( - c)

			OR		EBX, EAX				// UnPair			// () | mask

			MOV		[EDI], EBX				// UnPair

			ADD		EDI, 4										// Dstを次に進める
			MOV		EAX, EX

			ADD		nEXCnt, EAX									// Xの増分を加算
			JNB		SkipX_10									// if ( EX >= 0)

			ADD		ESI, -4										// Srcを次に進める
			MOV		EAX, EX2

			SUB		nEXCnt, EAX				// UnPair			// Xの補正

		SkipX_10:
			DEC		i
			JNZ		LoopX_10

			MOV		ESI, lpSrcBack								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			MOV		EAX, EY										// Yの増分を加算

			ADD		nEYCnt, EAX									// Yの増分を加算
			JNB		SkipY_10									// if ( EY >= 0 )

			MOV		EAX, EY2									// Yの補正
			ADD		ESI, lPitchSrc								// クリップした領域分を飛ばす

			SUB		nEYCnt, EAX				// UnPair			// Yの補正

		SkipY_10:
			DEC		j
			JNZ		LoopY_10
		}
	} // if


	
	return 0;
} // BlendBltFastAlphaM blend


//////////////////////////////////////
//	BltClearAlpha
//	抜き色有りの転送
//////////////////////////////////////
LRESULT CDIB32PMMX::BltClearAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltClearAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltClearAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;
	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);			// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;


// -----------	 MMX   カラーキー:有   ミラー:無   拡縮:無	  α値:無視	 --------------
	if ( m_bActualSize == true )
	{
		DWORD	colKey = p->m_dwColorKey;


		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOVD		MM7, colKey								//	カラーキーを設定する
			MOVD		MM6, nonAlphaMask						//	α値取得用マスク

			MOVQ		MM1, MM7
			MOVQ		MM2, MM6

			PUNPCKLDQ	MM7, MM1								//	ColKey
			MOV			EDI, lpDst

			PUNPCKLDQ	MM6, MM2								//	AlphaMask
			MOV			ESI, lpSrc

		LoopY_1:
			SHR			ECX, 1
			JNB			Skip_1									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI]			// UnPair

			MOVD		MM2, [EDI]
			PAND		MM0, MM6								//	α値をマスクしておく

			MOVQ		MM1, MM0			// UnPair

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1								// & mask
			ADD			EDI, 4

			PXOR		MM0, MM2			// UnPair			// src ^ ()
			NOP

			ADD			ESI, 4				// AGI回避

			MOVD		[EDI-4], MM0		// UnPair

			OR			ECX, ECX
			JZ			EndLoop_1

		Skip_1:
			SHR			ECX, 1
			JNB			LoopX_1									//	4の倍数か否かの判定
			// Step2
			MOVQ		MM0, [ESI]			// UnPair

			MOVQ		MM2, [EDI]
			PAND		MM0, MM6								//	α値をマスクしておく

			MOVQ		MM1, MM0			// UnPair

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1			// UnPair			// & mask
			ADD			EDI, 8

			PXOR		MM0, MM2			// UnPair			// src ^ ()
			NOP

			ADD			ESI, 8				// AGI回避

			MOVQ		[EDI-8], MM0		// UnPair

			OR			ECX, ECX
			JZ			EndLoop_1

		LoopX_1: // 12クロック･サイクル
			// Step4
			MOVQ		MM0, [ESI]			// 1// UnPair

			MOVQ		MM3, [ESI+8]		// 2
			PAND		MM0, MM6			// 1				//	α値をマスクしておく

			MOVQ		MM1, MM0			// 1
			PAND		MM3, MM6			// 2				//	α値をマスクしておく

			MOVQ		MM4, MM3			// 2
			PCMPEQD		MM1, MM7			// 1				// ColKey:0xffffffff NonColKey:0x00000000

			MOVQ		MM2, [EDI]			// 1
			PCMPEQD		MM4, MM7			// 2				// ColKey:0xffffffff NonColKey:0x00000000

			MOVQ		MM5, [EDI+8]		// 2
			PXOR		MM2, MM0			// 1				// (Src ^ Dst)

			PXOR		MM5, MM3			// 2				// (Src ^ Dst)
			PAND		MM2, MM1			// 1				// & mask

			PAND		MM5, MM4			// 2				// &mask
			PXOR		MM0, MM2			// 1				// Src ^ ()

			PXOR		MM3, MM5			// 2				// Src ^ ()
			ADD			ESI, 16

			MOVQ		[EDI], MM0			// UnPair

			MOVQ		[EDI+8], MM3		// UnPair

			ADD			EDI, 16
			DEC			ECX

			JNZ			LoopX_1				// UnPair

		EndLoop_1:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nAddSrcWidth						// クリップした領域分を飛ばす

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_1				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:有   ミラー:無   拡縮:有	  α値:無視	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		DWORD	lPitchSrc = p->m_lPitch;
		DWORD*	lpSrcBack;
	
		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			MOVD		MM7, colKey								//	カラーキーを設定する
			MOVD		MM6, nonAlphaMask						//	α値取得用マスク

			MOVQ		MM1, MM7
			MOVQ		MM2, MM6

			PUNPCKLDQ	MM7, MM1								// ColKey
			MOV			ECX, nWidth

			PUNPCKLDQ	MM6, MM2								// AlphaMask
			MOV			EDX, nHeight

			MOV			EBX, EIY			// UnPair			// nEyCnt = EIY;

		LoopY_2:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX								// nExCnt = EIX;

		LoopX_2: // 10クロック･サイクル
			MOVD		MM2, [ESI]			// UnPair			// *lpDst = *lpSrc;

			MOVD		MM4, [EDI]
			PAND		MM2, MM6								//	α値をマスクしておく

			MOVQ		MM3, MM2			// UnPair

			PCMPEQD		MM3, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM4, MM2								// (Src ^ Dst)

			PAND		MM4, MM3								// & mask
			ADD			ESI, AddSrcPixel						// 整数部の加算

			PXOR		MM2, MM4								// Src ^ ()
			ADD			EDI, 4									// lpDst++;
			
			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_2									// if ( EX >= 0)

			ADD			ESI, 4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_2:
			MOVD		[EDI-4], MM2		// UnPair

			DEC			ECX
			JNZ			LoopX_2

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY									// Yの増分
			JNB			SkipY_2									// if ( EY >= 0 )

			ADD			ESI, lPitchSrc						// 1ライン分加算して、次の行へ
			SUB			EBX, EY2								// Yの補正値

		SkipY_2:
			DEC			EDX
			JNZ			LoopY_2

			EMMS
		}
	} // if


	return 0;
} // BltClearAlpha


//////////////////////////////////////
//	BltFastClearAlpha
//	抜き色無しの転送 (α値無視)
//////////////////////////////////////
LRESULT CDIB32PMMX::BltFastClearAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltFastClearAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltFastClearAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);										// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;

// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:無	  α値:無視	 --------------
	if ( m_bActualSize == true )
	{
		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOVD		MM6, nonAlphaMask						//	α値取得用マスク
			MOV			EDI, lpDst

			MOVQ		MM0, MM6
			MOV			ESI, lpSrc

			PUNPCKLDQ	MM6, MM0								//	alphaMask


		LoopY_3:
			SHR			ECX, 1
			JNB			Skip_3									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI]			// UnPair

			PAND		MM0, MM6								//	α値をマスクしておく
			ADD			EDI, 4									// lpDst++;

			ADD			ESI, 4				// AGI回避

			MOVD		[EDI-4], MM0		// UnPair
			
			OR			ECX, ECX
			JZ			EndLoop_3

		Skip_3:
			SHR			ECX, 1
			JNB			LoopX_3									//	4の倍数か否かの判定
			// Step2
			MOVQ		MM0, [ESI]			// UnPair

			PAND		MM0, MM6								//	α値をマスクしておく
			ADD			EDI, 8

			ADD			ESI, 8				// AGI回避

			MOVQ		[EDI-8], MM0		// UnPair

			OR			ECX, ECX
			JZ			EndLoop_3

		LoopX_3: // 6クロック･サイクル
			// Step4
			MOVQ		MM0, [ESI]			// UnPair

			MOVQ		MM3, [ESI+8]		// UnPair
			PAND		MM0, MM6								//	α値をマスクしておく

			PAND		MM3, MM6								//	α値をマスクしておく
			ADD			EDI, 16

			ADD			ESI, 16				// UnPair

			MOVQ		[EDI-16], MM0		// UnPair

			MOVQ		[EDI-8], MM3		// UnPair

			DEC			ECX
			JNZ			LoopX_3

		EndLoop_3:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nAddSrcWidth						// クリップした領域分を飛ばす

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_3				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:有	  α値:無視	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			PXOR		MM1, MM1
			PXOR		MM0, MM0

			MOVD		MM6, nonAlphaMask						//	α値取得用マスク
			MOV			ECX, nWidth

			MOVQ		MM2, MM6
			MOV			EDX, nHeight

			PUNPCKLDQ	MM6, MM2								//	alphaMask
			MOV			EBX, EIY								// nEyCnt = EIY;

		LoopY_4:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_4: // 7クロック･サイクル
			MOVD		MM0, [ESI]			// UnPair			// *lpDst = *lpSrc;

			ADD			EDI, 4									// lpDst++;
			ADD			ESI, AddSrcPixel						// 整数部の加算

			PAND		MM0, MM6								//	α値をマスクしておく
			ADD			EAX, EX									// EX += 2*DX;

			JNB			SkipX_4				// Unpair			// if ( EX >= 0)

			ADD			ESI, 4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_4:
			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_4

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY									// Yの増分
			JNB			SkipY_4									// if ( EY >= 0 )

			ADD			ESI, lPitchSrc						// 1ライン分加算して、次の行へ
			SUB			EBX, EY2								// Yの補正値
		SkipY_4:
			DEC			EDX
			JNZ			LoopY_4

			EMMS
		}
	} // if

	
	return 0;
} // BltFastClearAlpha


////////////////////////////////////////////
//	BlendBltFastAlpha
//	抜き色関係なしのα値反映ブレンド転送
////////////////////////////////////////////
LRESULT CDIB32PMMX::BlendBltFastAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BlendBltFastAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BlendBltFastAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);										// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	alphaMask	= 0xff000000;
	DWORD	manipuMask	= 0x00010101;


// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:無	 α値:反映	  --------------
	if ( m_bActualSize == true )
	{
		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			PXOR		MM7, MM7
			PXOR		MM4, MM4

			MOVD		MM7, alphaMask							//	0x00000000ff000000を作成
			MOV			EDI, lpDst

			MOV			ESI, lpSrc			// UnPair

		LoopY_11:
		LoopX_11: // 26クロック･サイクル(乗算が 3クロックで 30クロック)
			MOVD		MM0, [ESI]

			MOVQ		MM2, MM0
			PUNPCKLBW	MM0, MM4								// WORD単位で乗算するので

			PAND		MM2, MM7			// UnPair			//	0xff000000ff000000でα値を得る

			MOVQ		MM1, MM2
			MOVQ		MM3, MM2

			PSRLD		MM2, 24
			PSRLD		MM1, 16

			PSRLD		MM3, 8
			PADDD		MM2, MM1

			PADDD		MM2, MM3			// UnPair			//	0x0000000000aaaaaaでソースのα値は得られた

			PUNPCKLBW	MM2, MM4								//	WORD単位で乗算するので
			// 補整
			PCMPEQD		MM1, MM1								//	故意に0xffffffffffffffffを作り出す

			MOVQ		MM3, MM7								//	0xff000000
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM6, MM4								//	0x0000000100010001を作成
			PANDN		MM3, MM1								//	0x0000000000ffffffを作成

			PUNPCKLBW	MM3, MM4			// UnPair			//	0x000000ff00ff00ffになる

			PCMPEQD		MM3, MM2			// UnPair			//	MM2=0x000000ff00ff00ffだったらMM3=0xffffffffffffffffになる

			PAND		MM3, MM6			// UnPair			//	MM3=0xffffffffffffffffならMM3=0x0000000100010001になる

			PADDD		MM2, MM3			// UnPair			//	MM3=0x0000000000000000かMM3=0x0000000100010001を足す

			// Dst
			MOVQ		MM5, MM2
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM4, MM6			// MM4借りまする	//	256 = 0x00 00 01 00 | 01 00 01 00

			PSUBW		MM4, MM5			// UnPair			//	256 - alpha
			
			MOVQ		MM5, MM4
			PXOR		MM4, MM4
			// 補整終了
			MOVD		MM1, [EDI]			// UnPair

			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM5
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			EDI, 4

			ADD			ESI, 4				// AGI回避

			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_11

			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nAddSrcWidth						// クリップした領域分を飛ばす

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_11			// UnPair

			EMMS
		}
	}
// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:有	 α値:反映	  --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV			ECX, nWidth
			MOV			EDX, nHeight

			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			PXOR		MM7, MM7
			PXOR		MM4, MM4

			MOVD		MM7, alphaMask							//	0x00000000ff000000を作成
			MOV			EBX, EIY								// nEyCnt = EIY;

		LoopY_12:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_12: // 14クロック･サイクル(乗算が 3クロックで 18クロック)
			MOVD		MM0, [ESI]

			MOVQ		MM2, MM0
			PUNPCKLBW	MM0, MM4								// WORD単位で乗算するので

			PAND		MM2, MM7			// UnPair			//	0xff000000ff000000でα値を得る

			MOVQ		MM1, MM2
			MOVQ		MM3, MM2

			PSRLD		MM2, 24
			PSRLD		MM1, 16

			PSRLD		MM3, 8
			PADDD		MM2, MM1

			PADDD		MM2, MM3			// UnPair			//	0x0000000000aaaaaaでソースのα値は得られた

			PUNPCKLBW	MM2, MM4								//	WORD単位で乗算するので
			// 補整
			PCMPEQD		MM1, MM1								//	故意に0xffffffffffffffffを作り出す

			MOVQ		MM3, MM7								//	0xff000000
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM6, MM4								//	0x0000000100010001を作成
			PANDN		MM3, MM1								//	0x0000000000ffffffを作成

			PUNPCKLBW	MM3, MM4			// UnPair			//	0x000000ff00ff00ffになる

			PCMPEQD		MM3, MM2			// UnPair			//	MM2=0x000000ff00ff00ffだったらMM3=0xffffffffffffffffになる

			PAND		MM3, MM6			// UnPair			//	MM3=0xffffffffffffffffならMM3=0x0000000100010001になる

			PADDD		MM2, MM3			// UnPair			//	MM3=0x0000000000000000かMM3=0x0000000100010001を足す

			// Dst
			MOVQ		MM5, MM2
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM4, MM6			// MM4借りまする	//	256 = 0x00 00 01 00 | 01 00 01 00

			PSUBW		MM4, MM5			// UnPair			//	256 - alpha
			
			MOVQ		MM5, MM4
			PXOR		MM4, MM4
			// 補整終了
			MOVD		MM1, [EDI]			// UnPair

			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM5
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			ESI, AddSrcPixel						// 整数部の加算

			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_12								// if ( EX >= 0)

			ADD			ESI, 4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_12:
			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_12

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY									// Yの増分
			JNB			SkipY_12								// if ( EY >= 0 )

			ADD			ESI, lPitchSrc							// 1ライン分加算して、次の行へ
			SUB			EBX, EY2								// Yの補正値

		SkipY_12:
			DEC			EDX
			JNZ			LoopY_12

			EMMS
		}
	} // if


	return 0;
} // BlendBltFastAlpha

// kaine 2001/2/28
////////////////////////////////////////////
//	BlendBltFastAlpha
//	抜き色関係なしのα値反映ブレンド比率指定転送
////////////////////////////////////////////
LRESULT CDIB32PMMX::BlendBltFastAlpha(CDIB32* lpDIB32,int x,int y,DWORD dwDstRGBRate,DWORD dwSrcRGBRate
							   ,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BlendBltFastAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BlendBltFastAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);										// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整

	DWORD	alphaMask	= 0xff000000;
	DWORD	manipuMask	= 0x00010101;

	// (E-dwDstRGBRate)[alpha] = (E-dwDstRGBRate) * alpha
	DWORD	dwDstR = 255 - (   dwDstRGBRate >> 16) ;
	DWORD	dwDstG = 255 - ( ( dwDstRGBRate & 0x0000ff00) >> 8 ) ;
	DWORD	dwDstB = 255 - (   dwDstRGBRate & 0x000000ff) ;
	DWORD	dwSrcR = dwSrcRGBRate >> 16 ;
	DWORD	dwSrcG = ( dwSrcRGBRate & 0x0000ff00) >> 8	;
	DWORD	dwSrcB = dwSrcRGBRate & 0x000000ff ;
	static DWORD	dwDstRGBRateT[256];
	static DWORD	dwSrcRGBRateT[256];
	DWORD	dR,dG,dB,sR,sG,sB;
	dR = dG = dB = sR =sG=sB=0;
	for ( int k = 0	 ; k < 256 ; k++, 
					dR+=dwDstR,dG+=dwDstG,dB+=dwDstB,
					sR+=dwSrcR,sG+=dwSrcG,sB+=dwSrcB
		){
		if ( k == 255 ) {
			dR+=dwDstR;dG+=dwDstG;dB+=dwDstB;
			sR+=dwSrcR;sG+=dwSrcG;sB+=dwSrcB;}
		dwDstRGBRateT[k] =	( ~( dR >> 8 ) << 16 & 0x00ff0000) |	
							( ~( dG >> 8 ) <<  8 & 0x0000ff00) |
							( ~( dB >> 8 )		 & 0x000000ff);
		dwSrcRGBRateT[k] =	(  ( sR >> 8 ) << 16 & 0x00ff0000) |	
							(  ( sG >> 8 ) <<  8 & 0x0000ff00) |
							(  ( sB >> 8 )		 & 0x000000ff);
	}
	

// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:無	 α値:反映	RGBブレンド:有	 --------------
	if ( m_bActualSize == true )
	{
		
		DWORD	*lpDstRGBRate = &dwDstRGBRateT[0];
		DWORD	*lpSrcRGBRate = &dwSrcRGBRateT[0];

		_asm
		{
			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			MOV			EDX, nHeight
			MOV			ECX, nWidth

		LoopY_11:
		LoopX_11: // 26クロック･サイクル(乗算が 3クロックで 30クロック)

			PXOR		MM0, MM0
			mov			eax,[ESI]
			
			and			eax,0xff000000
			mov			ebx,lpSrcRGBRate

			shr			eax,22
			PXOR		MM1, MM1

			movd		mm0,[ESI]
			PXOR		MM2, MM2

			movd		mm2,[ebx+eax]
			PXOR		MM4, MM4

			mov			ebx,lpDstRGBRate
			PUNPCKLBW	MM2, MM4				// WORD単位で乗算するので

			MOVD		MM1, [EDI]			
			PUNPCKLBW	MM0, MM4					

			MOVD		MM3, [ebx+eax]		// UnPair

			PUNPCKLBW	MM3, MM4			
			NOP
// ------------			

			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM3
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			EDI, 4

			ADD			ESI, 4				// AGI回避

			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_11

			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nAddSrcWidth						// クリップした領域分を飛ばす

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_11			// UnPair

			EMMS
		}
	}
// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:有	 α値:反映	  --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;
		DWORD	*lpDstRGBRate = &dwDstRGBRateT[0];
		DWORD	*lpSrcRGBRate = &dwSrcRGBRateT[0];

		_asm
		{
			MOV			ECX, nWidth
			MOV			EDX, nHeight

			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			PXOR		MM7, MM7
			PXOR		MM4, MM4

			MOV			EBX, EIY								// nEyCnt = EIY;

		LoopY_12:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_12: // 14クロック･サイクル(乗算が 3クロックで 18クロック)
			push		ebx
			push		eax

			pxor		mm0,mm0
			mov			eax,[ESI]
			
			and			eax,0xff000000
			mov			ebx,lpSrcRGBRate

			shr			eax,22
			PXOR		MM1, MM1

			movd		mm0,[ESI]
			pxor		mm2,mm2

			movd		mm2,[ebx+eax]
			PXOR		MM4, MM4

			mov			ebx,lpDstRGBRate
			PUNPCKLBW	MM2, MM4				// WORD単位で乗算するので

			MOVD		MM1, [EDI]			
			PUNPCKLBW	MM0, MM4					

			MOVD		MM3, [ebx+eax]		// UnPair

			PUNPCKLBW	MM3, MM4			
			nop

			pop			eax
			pop			ebx
//----
			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM3
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			ESI, AddSrcPixel						// 整数部の加算

			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_12								// if ( EX >= 0)

			ADD			ESI, 4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_12:
			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_12

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY									// Yの増分
			JNB			SkipY_12								// if ( EY >= 0 )

			ADD			ESI, lPitchSrc							// 1ライン分加算して、次の行へ
			SUB			EBX, EY2								// Yの補正値

		SkipY_12:
			DEC			EDX
			JNZ			LoopY_12

			EMMS
		}
	} // if


	return 0;
} // BlendBltFastAlpha blend



//////////////////////////////////////
//	BltClearAlphaM
//	抜き色有りの転送 (α値無効化)
//////////////////////////////////////
LRESULT CDIB32PMMX::BltClearAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltClearAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltClearAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;


// -----------	 MMX   カラーキー:有   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
		DWORD	colKey = p->m_dwColorKey;

				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOVD		MM7, colKey								//	カラーキーを設定する
			MOVD		MM6, nonAlphaMask						//	α値取得用マスク

			MOVQ		MM0, MM7
			MOVQ		MM1, MM6

			PUNPCKLDQ	MM7, MM0								// ColKey
			MOV			EDI, lpDst

			PUNPCKLDQ	MM6, MM1								//	AlphaMask
			MOV			ESI, lpSrc

		LoopY_1:
			SHR			ECX, 1
			JNB			Skip_1									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI-4]		// UnPair

			MOVD		MM2, [EDI]
			PAND		MM0, MM6								//	α値をマスクしておく

			MOVQ		MM1, MM0			// UnPair

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1			// UnPair			// & mask

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 4

			ADD			ESI, -4				// AGI回避

			MOVD		[EDI-4], MM0		// UnPair

			OR			ECX, ECX
			JZ			EndLoop_1

		Skip_1:
			SHR			ECX, 1
			JNB			LoopX_1									//	4の倍数か否かの判定
			// Step2
			MOVD		MM0, [ESI-4]		// UnPair

			MOVD		MM1, [ESI-8]		// UnPair

			MOVQ		MM2, [EDI]
			PUNPCKLDQ	MM0, MM1								// ソースの 2ピクセルをミラーする

			PAND		MM0, MM6			// UnPair			//	α値をマスクしておく

			MOVQ		MM1, MM0			// UnPair

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1			// UnPair			// & mask

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 8

			ADD			ESI, -8				// AGI回避

			MOVQ		[EDI-8], MM0		// UnPair

			OR			ECX, ECX
			JZ			EndLoop_1

		LoopX_1: // 15クロック･サイクル
			// Step4
			MOVD		MM0, [ESI-4]		// 1// UnPair

			MOVD		MM1, [ESI-8]		// 1// UnPair

			MOVD		MM3, [ESI-12]		// 2
			PUNPCKLDQ	MM0, MM1			// 1				// ソースの 2ピクセルをミラーする

			MOVD		MM4, [ESI-16]		// 2
			PAND		MM0, MM6			// 1				//	α値をマスクしておく

			MOVQ		MM2, [EDI]			// 1
			PUNPCKLDQ	MM3, MM4			// 2				// ソースの 2ピクセルをミラーする

			MOVQ		MM5, [EDI+8]		// 2
			PAND		MM3, MM6			// 2				//	α値をマスクしておく

			MOVQ		MM1, MM0			// 1
			MOVQ		MM4, MM3			// 2

			PCMPEQD		MM1, MM7			// 1				// ColKey:0xffffffff NonColKey:0x00000000
			PCMPEQD		MM4, MM7			// 2				// ColKey:0xffffffff NonColKey:0x00000000

			PXOR		MM2, MM0			// 1				// (src ^ dst)
			PXOR		MM5, MM3			// 2				// (src ^ dst)

			PAND		MM2, MM1			// 1				// & mask
			PAND		MM5, MM4			// 2				// & mask

			PXOR		MM0, MM2			// 1				// src ^ ()
			PXOR		MM3, MM5			// 2				// src ^ ()

			ADD			ESI, -16			// AGI回避

			MOVQ		[EDI], MM0			// 1// UnPair

			MOVQ		[EDI+8], MM3		// 2// UnPair

			ADD			EDI, 16
			DEC			ECX

			JNZ			LoopX_1				// UnPair

		EndLoop_1:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nSrcWidth							// 次ラインへの移動

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_1				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:有   ミラー:有   拡縮:有	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			MOVD		MM7, colKey								//	カラーキーを設定する
			MOVD		MM6, nonAlphaMask						//	α値取得用マスク

			MOVQ		MM0, MM7
			MOVQ		MM1, MM6

			PUNPCKLDQ	MM7, MM0								// ColKey
			MOV			ECX, nWidth

			PUNPCKLDQ	MM6, MM1								//	AlphaMask
			MOV			EDX, nHeight

			MOV			EBX, EIY			// UnPair			// nEyCnt = EIY;

		LoopY_2:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_2: // 10クロック･サイクル
			MOVD		MM0, [ESI-4]		// UnPair

			MOVD		MM2, [EDI]
			PAND		MM0, MM6								//	α値をマスクしておく

			MOVQ		MM1, MM0			// UnPair

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1								// & mask
			ADD			ESI, AddSrcPixel						// 整数部の加算

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_2									// if ( EX >= 0)

			ADD			ESI, -4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_2:
			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_2

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY										// Yの増分を加算
			JNB			SkipY_2										// if ( EY >= 0 )

			ADD			ESI, lPitchSrc
			SUB			EBX, EY2									// Yの補正

		SkipY_2:
			DEC			EDX
			JNZ			LoopY_2

			EMMS
		}
	} // if


	return 0;
} // BltClearAlphaM


//////////////////////////////////////
//	BltFastClearAlphaM
//	抜き色無しの転送 (α値無視)
//////////////////////////////////////
LRESULT CDIB32PMMX::BltFastClearAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltFastClearAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltFastClearAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	nonAlphaMask = 0x00ffffff;

// -----------	 MMX   カラーキー:無   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOVD		MM6, nonAlphaMask						//	α値取得用マスク
			MOV			EDI, lpDst

			MOVQ		MM1, MM6
			MOV			ESI, lpSrc

			PUNPCKLDQ	MM6, MM1								//	AlphaMask

		LoopY_3:
			SHR			ECX, 1
			JNB			Skip_3									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI-4]		// UnPair
			ADD			EDI, 4

			PAND		MM0, MM6								//	α値をマスクしておく
			ADD			ESI, -4				// AGI回避

			MOVD		[EDI-4], MM0		// UnPair

			OR			ECX, ECX
			JZ			EndLoop_3

		Skip_3:
			SHR			ECX, 1
			JNB			LoopX_3									//	4の倍数か否かの判定
			// Step2
			MOVD		MM0, [ESI-4]		// UnPair

			MOVD		MM1, [ESI-8]		// UnPair

			PUNPCKLDQ	MM0, MM1			// UnPair			// ソースの 2ピクセルをミラーする

			PAND		MM0, MM6								//	α値をマスクしておく
			ADD			EDI, 8

			ADD			ESI, -8				// AGI回避

			MOVQ		[EDI], MM0			// UnPair

			OR			ECX, ECX
			JZ			EndLoop_3

		LoopX_3: // 9クロック･サイクル							// もうちょっとアンロールしたいけど・・・ま、いっか:p
			// Step4
			MOVD		MM0, [ESI-4]		// 1// UnPair

			MOVD		MM1, [ESI-8]		// 1// UnPair

			MOVD		MM3, [ESI-12]		// 2
			PUNPCKLDQ	MM0, MM1			// 1				// ソースの 2ピクセルをミラーする

			MOVD		MM4, [ESI-16]		// 2// UnPair
			PAND		MM0, MM6			// 1				//	α値をマスクしておく

			PUNPCKLDQ	MM3, MM4			// 2				// ソースの 2ピクセルをミラーする
			ADD			EDI, 16

			PAND		MM3, MM6			// 2				//	α値をマスクしておく
			ADD			ESI, -16

			MOVQ		[EDI-16], MM0		// 1// UnPair

			MOVQ		[EDI-8], MM3		// 2// UnPair

			DEC			ECX
			JNZ			LoopX_3

		EndLoop_3:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nSrcWidth							// 次ラインへの移動

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_3				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:無   ミラー:有   拡縮:有	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			MOV			ECX, nWidth
			MOV			EDX, nHeight

			MOV			EBX, EIY			// UnPair			// nEyCnt = EIY;

		LoopY_4:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_4: // 7クロック･サイクル
			MOVD		MM0, [ESI-4]		// UnPair

			ADD			ESI, AddSrcPixel						// 整数部の加算
			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_4									// if ( EX >= 0)

			ADD			ESI, -4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_4:
			MOVD		[EDI-4], MM0			// UnPair

			DEC			ECX
			JNZ			LoopX_4

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY										// Yの増分を加算
			JNB			SkipY_4										// if ( EY >= 0 )

			SUB			EBX, EY2									// Yの補正
			ADD			ESI, lPitchSrc								// クリップした領域分を飛ばす

		SkipY_4:
			DEC			EDX
			JNZ			LoopY_4

			EMMS
		}
	} // if


	return 0;
} // BltFastClearAlphaM


///////////////////////////////////////////
//	BlendBltFastAlphaM
//	抜き色関係なしのα値反映ブレンド転送
///////////////////////////////////////////
LRESULT CDIB32PMMX::BlendBltFastAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BlendBltFastAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BlendBltFastAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	alphaMask	= 0xff000000;
	DWORD	manipuMask	= 0x00010101;

// -----------	 MMX   カラーキー:有   ミラー:有   拡縮:無	 α値:反映	  --------------
	if ( m_bActualSize == true )
	{
		DWORD	colKey = p->m_dwColorKey;

				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			PXOR		MM7, MM7
			PXOR		MM4, MM4

			MOVD		MM7, alphaMask							//	0x00000000ff000000を作成
			MOV			EDI, lpDst

			MOV			ESI, lpSrc			// UnPair

		LoopY_9:
		LoopX_9: // 26クロック･サイクル(乗算が 3クロックで 30クロック)
			MOVD		MM0, [ESI-4]

			MOVQ		MM2, MM0
			PUNPCKLBW	MM0, MM4								// WORD単位で乗算するので

			PAND		MM2, MM7			// UnPair			//	0xff000000ff000000でα値を得る

			MOVQ		MM1, MM2
			MOVQ		MM3, MM2

			PSRLD		MM2, 24
			PSRLD		MM1, 16

			PSRLD		MM3, 8
			PADDD		MM2, MM1

			PADDD		MM2, MM3			// UnPair			//	0x0000000000aaaaaaでソースのα値は得られた

			PUNPCKLBW	MM2, MM4								//	WORD単位で乗算するので
			// 補整
			PCMPEQD		MM1, MM1								//	故意に0xffffffffffffffffを作り出す

			MOVQ		MM3, MM7								//	0xff000000
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM6, MM4								//	0x0000000100010001を作成
			PANDN		MM3, MM1								//	0x0000000000ffffffを作成

			PUNPCKLBW	MM3, MM4			// UnPair			//	0x000000ff00ff00ffになる

			PCMPEQD		MM3, MM2			// UnPair			//	MM2=0x000000ff00ff00ffだったらMM3=0xffffffffffffffffになる

			PAND		MM3, MM6			// UnPair			//	MM3=0xffffffffffffffffならMM3=0x0000000100010001になる

			PADDD		MM2, MM3			// UnPair			//	MM3=0x0000000000000000かMM3=0x0000000100010001を足す

			// Dst
			MOVQ		MM5, MM2
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM4, MM6			// MM4借りまする	//	256 = 0x00 00 01 00 | 01 00 01 00

			PSUBW		MM4, MM5			// UnPair			//	256 - alpha
			
			MOVQ		MM5, MM4
			PXOR		MM4, MM4
			// 補整終了
			MOVD		MM1, [EDI]			// UnPair

			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM5
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			EDI, 4

			ADD			ESI, -4

			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_9

			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nSrcWidth							// 次ラインへの移動

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_9				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:有   ミラー:有   拡縮:有	 α値:反映	  --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			PXOR		MM7, MM7
			PXOR		MM4, MM4

			MOVD		MM6, manipuMask							//	0x0000000100010001を作成
			MOVD		MM7, alphaMask							//	0x00000000ff000000を作成

			PUNPCKLBW	MM6, MM4								//	0x0000000100010001
			MOV			EBX, EIY								// nEyCnt = EIY;

		LoopY_10:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_10: // 18クロック･サイクル (PMULが 3クロックだと 22クロック)
			MOVD		MM0, [ESI-4]

			MOVQ		MM2, MM0
			PUNPCKLBW	MM0, MM4								// WORD単位で乗算するので

			PAND		MM2, MM7			// UnPair			//	0xff000000ff000000でα値を得る

			MOVQ		MM1, MM2
			MOVQ		MM3, MM2

			PSRLD		MM2, 24
			PSRLD		MM1, 16

			PSRLD		MM3, 8
			PADDD		MM2, MM1

			PADDD		MM2, MM3			// UnPair			//	0x0000000000aaaaaaでソースのα値は得られた

			PUNPCKLBW	MM2, MM4								//	WORD単位で乗算するので
			// 補整
			PCMPEQD		MM1, MM1								//	故意に0xffffffffffffffffを作り出す

			MOVQ		MM3, MM7								//	0xff000000
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM6, MM4								//	0x0000000100010001を作成
			PANDN		MM3, MM1								//	0x0000000000ffffffを作成

			PUNPCKLBW	MM3, MM4			// UnPair			//	0x000000ff00ff00ffになる

			PCMPEQD		MM3, MM2			// UnPair			//	MM2=0x000000ff00ff00ffだったらMM3=0xffffffffffffffffになる

			PAND		MM3, MM6			// UnPair			//	MM3=0xffffffffffffffffならMM3=0x0000000100010001になる

			PADDD		MM2, MM3			// UnPair			//	MM3=0x0000000000000000かMM3=0x0000000100010001を足す

			// Dst
			MOVQ		MM5, MM2
			MOVD		MM6, manipuMask							//	0x00010101

			PUNPCKLBW	MM4, MM6			// MM4借りまする	//	256 = 0x00 00 01 00 | 01 00 01 00

			PSUBW		MM4, MM5			// UnPair			//	256 - alpha
			
			MOVQ		MM5, MM4
			PXOR		MM4, MM4
			// 補整終了
			MOVD		MM1, [EDI]			// UnPair

			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM5
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			ESI, AddSrcPixel						// 整数部の加算

			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_10								// if ( EX >= 0)

			ADD			ESI, -4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_10:
			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_10

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY										// Yの増分を加算
			JNB			SkipY_10									// if ( EY >= 0 )

			SUB			EBX, EY2									// Yの補正
			ADD			ESI, lPitchSrc								// クリップした領域分を飛ばす

		SkipY_10:
			DEC			EDX
			JNZ			LoopY_10

			EMMS
		}
	} // if

	
	return 0;
} // BlendBltFastAlphaM


// kaine 2001/2/28
////////////////////////////////////////////
//	BlendBltFastAlphaM blend
//	抜き色関係なしのα値反映ブレンド比率指定転送
////////////////////////////////////////////
LRESULT CDIB32PMMX::BlendBltFastAlphaM(CDIB32* lpDIB32,int x,int y,DWORD dwDstRGBRate,DWORD dwSrcRGBRate
							   ,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BlendBltFastAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BlendBltFastAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

	DWORD	alphaMask	= 0xff000000;
	DWORD	manipuMask	= 0x00010101;

	// (E-dwDstRGBRate)[alpha] = (E-dwDstRGBRate) * alpha
	DWORD	dwDstR = 255 - (   dwDstRGBRate >> 16) ;
	DWORD	dwDstG = 255 - ( ( dwDstRGBRate & 0x0000ff00) >> 8 ) ;
	DWORD	dwDstB = 255 - (   dwDstRGBRate & 0x000000ff) ;
	DWORD	dwSrcR = dwSrcRGBRate >> 16 ;
	DWORD	dwSrcG = ( dwSrcRGBRate & 0x0000ff00) >> 8	;
	DWORD	dwSrcB = dwSrcRGBRate & 0x000000ff ;
	static DWORD	dwDstRGBRateT[256];
	static DWORD	dwSrcRGBRateT[256];
	DWORD	dR,dG,dB,sR,sG,sB;
	dR = dG = dB = sR =sG=sB=0;
	for ( int k = 0	 ; k < 256 ; k++, 
					dR+=dwDstR,dG+=dwDstG,dB+=dwDstB,
					sR+=dwSrcR,sG+=dwSrcG,sB+=dwSrcB)
	{
		dwDstRGBRateT[k] =	( ~( dR >> 8 ) << 16 & 0x00ff0000) |	
							( ~( dG >> 8 ) <<  8 & 0x0000ff00) |
							( ~( dB >> 8 )		 & 0x000000ff);
		dwSrcRGBRateT[k] =	(  ( sR >> 8 ) << 16 & 0x00ff0000) |	
							(  ( sG >> 8 ) <<  8 & 0x0000ff00) |
							(  ( sB >> 8 )		 & 0x000000ff);
	}

	dwDstRGBRateT[255] =	( ~( dR >> 8 ) << 16 & 0x00ff0000) |	
							( ~( dG >> 8 ) <<  8 & 0x0000ff00) |
							( ~( dB >> 8 )		 & 0x000000ff);
	dwSrcRGBRateT[255] =	(  ( sR >> 8 ) << 16 & 0x00ff0000) |	
							(  ( sG >> 8 ) <<  8 & 0x0000ff00) |
							(  ( sB >> 8 )		 & 0x000000ff);


// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:無	 α値:反映	RGBブレンド:有	 --------------
	if ( m_bActualSize == true )
	{
		nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅

		DWORD	*lpDstRGBRate = &dwDstRGBRateT[0];
		DWORD	*lpSrcRGBRate = &dwSrcRGBRateT[0];

		_asm
		{
			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			MOV			EDX, nHeight
			MOV			ECX, nWidth

		LoopY_9:
		LoopX_9: // 26クロック･サイクル(乗算が 3クロックで 30クロック)

			PXOR		MM0, MM0
			mov			eax,[ESI-4]
			
			and			eax,0xff000000
			mov			ebx,lpSrcRGBRate

			shr			eax,22
			PXOR		MM1, MM1

			movd		mm0,[ESI-4]
			PXOR		MM2, MM2

			movd		mm2,[ebx+eax]
			PXOR		MM4, MM4

			mov			ebx,lpDstRGBRate
			PUNPCKLBW	MM2, MM4				// WORD単位で乗算するので

			MOVD		MM1, [EDI]			
			PUNPCKLBW	MM0, MM4					

			MOVD		MM3, [ebx+eax]		// UnPair

			PUNPCKLBW	MM3, MM4			
			NOP
// ------------			

			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM3
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			EDI, 4

			ADD			ESI, -4

			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_9

			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nSrcWidth							// 次ラインへの移動

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_9				// UnPair

			EMMS
		}
	}
// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:有	 α値:反映	  --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	Mask7f = 0x007f7f7f;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;
		DWORD	*lpDstRGBRate = &dwDstRGBRateT[0];
		DWORD	*lpSrcRGBRate = &dwSrcRGBRateT[0];

		_asm
		{
			MOV			ECX, nWidth
			MOV			EDX, nHeight

			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			PXOR		MM7, MM7
			PXOR		MM4, MM4

			MOV			EBX, EIY								// nEyCnt = EIY;

		LoopY_10:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_10: // 14クロック･サイクル(乗算が 3クロックで 18クロック)
			push		ebx
			push		eax

			pxor		mm0,mm0
			mov			eax,[ESI-4]
			
			and			eax,0xff000000
			mov			ebx,lpSrcRGBRate

			shr			eax,22
			PXOR		MM1, MM1

			movd		mm0,[ESI-4]
			pxor		mm2,mm2

			movd		mm2,[ebx+eax]
			PXOR		MM4, MM4

			mov			ebx,lpDstRGBRate
			PUNPCKLBW	MM2, MM4				// WORD単位で乗算するので

			MOVD		MM1, [EDI]			
			PUNPCKLBW	MM0, MM4					

			MOVD		MM3, [ebx+eax]		// UnPair

			PUNPCKLBW	MM3, MM4			
			nop

			pop			eax
			pop			ebx
//----
			PMULLW		MM0, MM2
			PUNPCKLBW	MM1, MM4

			PMULLW		MM1, MM3
			NOP

			NOP			// PMULストール回避 (2命令追加)
			NOP

			NOP
			NOP

			PADDUSB		MM0, MM1			// UnPair

			PSRLW		MM0, 8				// UnPair

			PACKUSWB	MM0, MM4
			ADD			ESI, AddSrcPixel						// 整数部の加算

			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_10								// if ( EX >= 0)

			ADD			ESI, -4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_10:
			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_10

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY										// Yの増分を加算
			JNB			SkipY_10									// if ( EY >= 0 )

			SUB			EBX, EY2									// Yの補正
			ADD			ESI, lPitchSrc								// クリップした領域分を飛ばす

		SkipY_10:
			DEC			EDX
			JNZ			LoopY_10

			EMMS
		}
	} // if


	return 0;
} // BlendBltFastAlphaM blend


/*----------------------------------------------------------------------------
//						following lines are added by yaneurao				//
----------------------------------------------------------------------------*/


//////////////////////////////////////
//	BltWithoutAlpha(for stencil alpha effect)
//	抜き色有りの転送
//////////////////////////////////////

LRESULT CDIB32P5::BltWithoutAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32Base::BltWithoutAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32Base::BltWithoutAlphaでp->GetPtr() == NULL");

// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;
	DWORD	lPitchSrc = p->m_lPitch;
	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth =	  m_lPitch - (nWidth<<2);									// ASMで使用する 1ラインのバイト数の設定
	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整


// -----------	 Pentium   カラーキー:有   ミラー:無   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
		DWORD		colKey = p->m_dwColorKey;


		_asm
		{
			MOV		ECX, nWidth
			MOV		EDX, nHeight

			MOV		EDI, lpDst
			MOV		ESI, lpSrc

			MOV		EBX, colKey				// UnPair

		LoopX_1:	// 5(4)クロック･サイクル
			MOV		EAX, [ESI]
			ADD		ESI, 4

			CMP		EAX, EBX
			JE		Skip_1

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOV		[EDI], EAX				// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			mov		ebx,[edi]
			and		eax,0x00ffffff	//	最上位バイトのマスク(一応ね)
			and		ebx,0xff000000	//	α値だけ残す
			or		eax,ebx
			mov		[edi],eax

		Skip_1:
			ADD		EDI, 4										// EDIを先に進める
			DEC		ECX

			JNZ		LoopX_1					// UnPair

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nAddSrcWidth							// クリップした領域分を飛ばす
			DEC		EDX

			JNZ		LoopX_1					// UnPair
		}
	}// if
// -----------	 Pentium   カラーキー:有   ミラー:無   拡縮:有	 --------------
	else
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		int		i, j;
		BYTE*	lpSrcBack;

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;
		LoopY_2:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX				// UnPair			// EY = InitializeX;

		LoopX_2: // 7(6)クロック･サイクル
			MOV		EAX, [ESI]									// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			CMP		EAX, colKey
			JE		SkipColKey_2

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOV		[EDI], EAX				// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			mov		ebx,[edi]
			and		eax,0x00ffffff	//	最上位バイトのマスク(一応ね)
			and		ebx,0xff000000	//	α値だけ残す
			or		eax,ebx
			mov		[edi],eax

		SkipColKey_2:
			ADD		EDI, 4					// UnPair			// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_2										// if ( EX >= 0)

			ADD		ESI, 4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_2:
			DEC		i
			JNZ		LoopX_2

			MOV		ESI, lpSrcBack;								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_2					// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc
			SUB		EDX, EY2				// UnPair			// Yの補正
			
		SkipY_2:
			DEC		j
			JNZ		LoopY_2
		}
	} // if

	
	return 0;
} // BltWithoutAlpha


//////////////////////////////////////
//	BltFastWithoutAlpha
//	抜き色無しの転送
//////////////////////////////////////
LRESULT CDIB32P5::BltFastWithoutAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32Base::BltFastWithoutAlphaでm_lpdwSrc == NULL");
	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32Base::BltFastWithoutAlphaでp->GetPtr() == NULL");

// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);				// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整


// -----------	 Pentium   カラーキー:無   ミラー:無   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
		_asm {
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, nHeight
			MOV		ECX, nWidth

		LoopX_3: // ???クロック・サイクル
			mov		eax,[esi]
			add		esi,4
		
			mov		ebx,[edi]
			and		eax,0x00ffffff
			and		ebx,0xff000000
			or		eax,ebx
			mov		[edi],eax

			add		edi,4

			dec		ecx
			jnz		LoopX_3

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nAddSrcWidth							// クリップした領域分を飛ばす
			DEC		EDX

			JNZ		LoopX_3					// UnPair

		}
	} // if
// -----------	 Pentium   カラーキー:無   ミラー:無   拡縮:有	 --------------
	else
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		int		i, j;
		BYTE*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;
		

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;

		LoopY_5:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX									// EY = InitializeX;

		LoopX_5: // 5クロック･サイクル
			MOV		EAX, [ESI]									// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOV		[EDI], EAX				// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			mov		ebx,[edi]
			and		eax,0x00ffffff	//	最上位バイトのマスク(一応ね)
			and		ebx,0xff000000	//	α値だけ残す
			or		eax,ebx
			mov		[edi],eax

			ADD		EDI, 4										// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_5										// if ( EX >= 0)

			ADD		ESI, 4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_5:
			DEC		i
			JNZ		LoopX_5

			MOV		ESI, lpSrcBack								// Srcをラインの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_5					// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc			// 1ライン分加算して、次の行へ
			SUB		EDX, EY2									// Yの補正

		SkipY_5:
			DEC		j
			JNZ		LoopY_5
		}
	} // if

	
	return 0;
} // BltFastWithoutAlpha

////////////////////////////////////////////////////////////////////
//	ミラー有り矩形転送
////////////////////////////////////////////////////////////////////
//////////////////////////////////////
//	BltWithoutAlphaM
//	抜き色有りの転送
//////////////////////////////////////
LRESULT CDIB32P5::BltWithoutAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32Base::BltWithoutAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32Base::BltWithoutAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整


// -----------	 Pentium   カラーキー:有   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
		DWORD	colKey = p->m_dwColorKey;

				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV		ECX, nWidth
			MOV		EDX, nHeight

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EBX, colKey				// UnPair

		LoopY_1:

		LoopX_1:	// 5(4)クロック･サイクル
			MOV		EAX, [ESI-4]
			ADD		ESI, -4

			CMP		EAX, EBX
			JE		Skip_1

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOV		[EDI], EAX				// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			mov		ebx,[edi]
			and		eax,0x00ffffff	//	最上位バイトのマスク(一応ね)
			and		ebx,0xff000000	//	α値だけ残す
			or		eax,ebx
			mov		[edi],eax

		Skip_1:
			ADD		EDI, 4										// EDIを先に進める
			DEC		ECX

			JNZ		LoopX_1					// UnPair

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nSrcWidth								// 次ラインへの移動
			DEC		EDX

			JNZ		LoopY_1					// UnPair
		}
	}// if
// -----------	 Pentium   カラーキー:有   ミラー:有   拡縮:有	 --------------
	else
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		int		i, j;
		BYTE*	lpSrcBack;
		DWORD	lPitchSrc =	 p->m_lPitch;

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;

		LoopY_2:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX				// UnPair			// EY = InitializeX;

		LoopX_2: // 7(6)クロック･サイクル
			MOV		EAX, [ESI-4]								// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			CMP		EAX, colKey
			JE		SkipColKey_2

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOV		[EDI], EAX				// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			mov		ebx,[edi]
			and		eax,0x00ffffff	//	最上位バイトのマスク(一応ね)
			and		ebx,0xff000000	//	α値だけ残す
			or		eax,ebx
			mov		[edi],eax

		SkipColKey_2:
			ADD		EDI, 4					// UnPair			// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_2										// if ( EX >= 0)

			ADD		ESI, -4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_2:
			DEC		i
			JNZ		LoopX_2

			MOV		ESI, lpSrcBack;								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_2					// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc
			SUB		EDX, EY2									// Yの補正

		SkipY_2:
			DEC		j
			JNZ		LoopY_2
		}
	} // if


	return 0;
} // BltWithoutAlphaM


//////////////////////////////////////
//	BltFastWithoutAlphaM
//	抜き色無しの転送
//////////////////////////////////////
LRESULT CDIB32P5::BltFastWithoutAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32Base::BltFastWithoutAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32Base::BltFastWithoutAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;


	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整


// -----------	 Pentium   カラーキー:無   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅


		_asm
		{
			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, nHeight
			MOV		ECX, nWidth

		LoopX_3: // 3クロック･サイクル
			MOV		EAX, [ESI-4]
			ADD		ESI, -4

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOV		[EDI], EAX				// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			mov		ebx,[edi]
			and		eax,0x00ffffff	//	最上位バイトのマスク(一応ね)
			and		ebx,0xff000000	//	α値だけ残す
			or		eax,ebx
			mov		[edi],eax

			ADD		EDI, 4

			DEC		ECX
			JNZ		LoopX_3

			MOV		ECX, nWidth
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			ADD		ESI, nSrcWidth								// 次ラインへの移動
			DEC		EDX

			JNZ		LoopX_3					// UnPair
		}
	} // if
// -----------	 Pentium   カラーキー:無   ミラー:有   拡縮:有	 --------------
	else
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		int		i, j;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV		EAX, nWidth
			MOV		EBX, nHeight

			MOV		i, EAX
			MOV		j, EBX

			MOV		ESI, lpSrc
			MOV		EDI, lpDst

			MOV		EDX, EIY				// UnPair			// EX = InitializeY;

		LoopY_4:
			MOV		lpSrcBack, ESI
			MOV		ECX, EIX				// UnPair			// EY = InitializeX;

		LoopX_4: // 5クロック･サイクル
			MOV		EAX, [ESI-4]								// ピクセルのコピー
			ADD		ESI, AddSrcPixel							// 整数部の加算

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOV		[EDI], EAX				// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			mov		ebx,[edi]
			and		eax,0x00ffffff	//	最上位バイトのマスク(一応ね)
			and		ebx,0xff000000	//	α値だけ残す
			or		eax,ebx
			mov		[edi],eax

			ADD		EDI, 4										// Dstを次に進める

			ADD		ECX, EX										// Xの増分を加算
			JNB		SkipX_4										// if ( EX >= 0)

			ADD		ESI, -4										// Srcを次に進める
			SUB		ECX, EX2									// Xの補正

		SkipX_4:
			DEC		i
			JNZ		LoopX_4

			MOV		ESI, lpSrcBack								// Srcの先頭に戻す
			ADD		EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV		EAX, nWidth
			ADD		ESI, AddWidthSrc							// 整数部の加算 '00.09.12.byTia

			MOV		i, EAX
			ADD		EDX, EY										// Yの増分を加算

			JNB		SkipY_4					// UnPair			// if ( EY >= 0 )

			ADD		ESI, lPitchSrc
			SUB		EDX, EY2									// Yの補正

		SkipY_4:
			DEC		j
			JNZ		LoopY_4
		}
	} // if

	return 0;
} // BltFastWithoutAlphaM

////////////////////////////////////////////////////////////////////////////
//	PentiumMMX用のルーチン
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//	ミラー無し矩形転送
////////////////////////////////////////////////////////////////////
//////////////////////////////////////
//	BltWithoutAlpha
//	抜き色有りの転送
//////////////////////////////////////
LRESULT CDIB32PMMX::BltWithoutAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltWithoutAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltWithoutAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;
	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);			// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整


// -----------	 MMX   カラーキー:有   ミラー:無   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
		DWORD	colKey = p->m_dwColorKey;

			//	mask data setting by yane
			static DWORD mask1[2] = {0x00ffffff, 0x00ffffff};
			static DWORD mask2[2] = {0xff000000, 0xff000000};

		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOVD		MM7, colKey								//	カラーキーを設定する
			MOV			EDI, lpDst

			MOVQ		MM1, MM7
			MOV			ESI, lpSrc

			PUNPCKLDQ	MM7, MM1			// UnPair			// ColKey

		LoopY_1:
			SHR			ECX, 1
			JNB			Skip_1									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI]			// UnPair

			MOVD		MM2, [EDI]
			MOVQ		MM1, MM0

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1			// UnPair			// & mask

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 4

			ADD			ESI, 4				// AGI回避

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVD		[EDI-4], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movd	mm1,[edi-4]
			pand	mm0,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movd	[edi-4],mm0

			OR			ECX, ECX
			JZ			EndLoop_1

		Skip_1:
			SHR			ECX, 1
			JNB			LoopX_1									//	4の倍数か否かの判定
			// Step2
			MOVQ		MM0, [ESI]			// UnPair

			MOVQ		MM2, [EDI]
			MOVQ		MM1, MM0

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1			// UnPair			// & mask

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 8

			ADD			ESI, 8				// UnPair

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVQ		[EDI-8], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi-8]
			pand	mm0,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi-8],mm0

			OR			ECX, ECX
			JZ			EndLoop_1

		LoopX_1: // 12クロック･サイクル
			// Step4
			MOVQ		MM0, [ESI]			// 1// UnPair

			MOVQ		MM3, [ESI+8]		// 2
			MOVQ		MM1, MM0			// 1

			MOVQ		MM4, MM3			// 2
			PCMPEQD		MM1, MM7			// 1				// ColKey:0xffffffff NonColKey:0x00000000

			MOVQ		MM2, [EDI]			// 1
			PCMPEQD		MM4, MM7			// 2				// ColKey:0xffffffff NonColKey:0x00000000

			MOVQ		MM5, [EDI+8]		// 2
			PXOR		MM2, MM0			// 1				// (Src ^ Dst)

			PXOR		MM5, MM3			// 2				// (Src ^ Dst)
			PAND		MM2, MM1			// 1				// & mask

			PAND		MM5, MM4			// 2				// &mask
			PXOR		MM0, MM2			// 1				// Src ^ ()

			PXOR		MM3, MM5			// 2				// Src ^ ()
			ADD			ESI, 16

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//	MOVQ		[EDI], MM0			// UnPair
			//	MOVQ		[EDI+8], MM3		// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi]
			pand	mm0,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi],mm0
			movq	mm1,[edi+8]
			pand	mm3,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm3,mm1
			movq	[edi+8],mm3

			ADD			EDI, 16
			DEC			ECX

			JNZ			LoopX_1				// UnPair

		EndLoop_1:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nAddSrcWidth						// クリップした領域分を飛ばす

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_1				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:有   ミラー:無   拡縮:有	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		DWORD	lPitchSrc = p->m_lPitch;
		DWORD*	lpSrcBack;
	
			//	mask data setting by yane
			static DWORD mask1[2] = {0x00ffffff, 0x00ffffff};
			static DWORD mask2[2] = {0xff000000, 0xff000000};

		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			MOVD		MM7, colKey								//	カラーキーを設定する
			PXOR		MM1, MM1

			MOVQ		MM2, MM7
			PXOR		MM0, MM0

			PUNPCKLDQ	MM7, MM2								// ColKey
			MOV			ECX, nWidth

			MOV			EDX, nHeight
			MOV			EBX, EIY								// nEyCnt = EIY;

		LoopY_2:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX								// nExCnt = EIX;

		LoopX_2: // 10クロック･サイクル
			MOVD		MM2, [ESI]			// UnPair			// *lpDst = *lpSrc;

			MOVD		MM4, [EDI]
			MOVQ		MM3, MM2

			PCMPEQD		MM3, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM4, MM2								// (Src ^ Dst)

			PAND		MM4, MM3								// & mask
			ADD			ESI, AddSrcPixel						// 整数部の加算

			PXOR		MM2, MM4								// Src ^ ()
			ADD			EDI, 4									// lpDst++;

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_2									// if ( EX >= 0)

			ADD			ESI, 4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_2:
			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVD		[EDI-4], MM2		// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movd	mm1,[edi-4]
			pand	mm0,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movd	[edi-4],mm0

			DEC			ECX
			JNZ			LoopX_2

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY									// Yの増分
			JNB			SkipY_2									// if ( EY >= 0 )

			ADD			ESI, lPitchSrc						// 1ライン分加算して、次の行へ
			SUB			EBX, EY2								// Yの補正値

		SkipY_2:
			DEC			EDX
			JNZ			LoopY_2

			EMMS
		}
	} // if


	return 0;
} // BltWithoutAlpha


//////////////////////////////////////
//	BltFastWithoutAlpha
//	抜き色無しの転送
//////////////////////////////////////
LRESULT CDIB32PMMX::BltFastWithoutAlpha(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltFastWithoutAlphaでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltFastWithoutAlphaでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( Clipping( lpDIB32, x, y, lpSrcRect, lpDstSize,	 lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nAddSrcWidth = p->m_lPitch - ((m_rcSrcRect.right - m_rcSrcRect.left)<<2);	// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);										// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc +(m_rcSrcRect.left<<2)+m_rcSrcRect.top*p->m_lPitch );		// クリッピング部分のカット
	DWORD*	lpDst = (DWORD*)((BYTE*)   m_lpdwSrc +(m_rcDstRect.left<<2)+m_rcDstRect.top*   m_lPitch );		// 指定されたx,yの位置調整


// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
			//	mask data setting by yane
			static DWORD mask1[2] = {0x00ffffff, 0x00ffffff};
			static DWORD mask2[2] = {0xff000000, 0xff000000};

		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			//	mask data setting by yane
			movq		mm3,mask1
			movq		mm4,mask2

		LoopY_3:
			SHR			ECX, 1
			JNB			Skip_3									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI]			// UnPair

			ADD			ESI, 4				// AGI回避


			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVD		[EDI], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movd	mm1,[edi]
			pand	mm0,mm3	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm4	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movd	[edi],mm0

			
			ADD			EDI, 4
			OR			ECX, ECX

			JZ			EndLoop_3			// UnPair

		Skip_3:
			SHR			ECX, 1
			JNB			LoopX_3									//	4の倍数か否かの判定
			// Step2
			MOVQ		MM0, [ESI]			// UnPair

			ADD			ESI, 8				// AGI回避

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVQ		[EDI], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi]
			pand	mm0,mm3	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm4	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi],mm0

			ADD			EDI, 8
			OR			ECX, ECX

			JZ			EndLoop_3			// UnPair

		LoopX_3: // 6クロック･サイクル
			// Step4
			MOVQ		MM0, [ESI]			// UnPair

			MOVQ		MM2, [ESI+8]		// UnPair

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//	MOVQ		[EDI], MM0			// UnPair
			//	MOVQ		[EDI+8], MM2		// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi]
			pand	mm0,mm3	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm4	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi],mm0
			movq	mm1,[edi+8]
			pand	mm2,mm3	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm4	//	0xff000000ff000000	//	α値だけ残す
			por		mm2,mm1
			movq	[edi+8],mm2

			ADD			ESI, 16
			ADD			EDI, 16

			DEC			ECX
			JNZ			LoopX_3

		EndLoop_3:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nAddSrcWidth						// クリップした領域分を飛ばす

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_3				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:無   ミラー:無   拡縮:有	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = 4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			PXOR		MM1, MM1
			PXOR		MM0, MM0

			MOV			ECX, nWidth
			MOV			EDX, nHeight

			MOV			EBX, EIY			// UnPair			// nEyCnt = EIY;

		LoopY_4:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_4: // 7クロック･サイクル
			MOVD		MM0, [ESI]			// UnPair			// *lpDst = *lpSrc;

			ADD			EDI, 4									// lpDst++;
			ADD			ESI, AddSrcPixel							// 整数部の加算

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_4									// if ( EX >= 0)

			ADD			ESI, 4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_4:
			MOVD		[EDI-4], MM0		// UnPair

			DEC			ECX
			JNZ			LoopX_4

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY									// Yの増分
			JNB			SkipY_4									// if ( EY >= 0 )

			ADD			ESI, lPitchSrc						// 1ライン分加算して、次の行へ
			SUB			EBX, EY2								// Yの補正値
		SkipY_4:
			DEC			EDX
			JNZ			LoopY_4

			EMMS
		}
	} // if

	
	return 0;
} // BltFastWithoutAlpha

////////////////////////////////////////////////////////////////////
//	ミラー有り矩形転送
////////////////////////////////////////////////////////////////////
//////////////////////////////////////
//	BltWithoutAlphaM
//	抜き色有りの転送
//////////////////////////////////////
LRESULT CDIB32PMMX::BltWithoutAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltWithoutAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltWithoutAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整


// -----------	 MMX   カラーキー:有   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
		DWORD	colKey = p->m_dwColorKey;

				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅

			//	mask data setting by yane
			static DWORD mask1[2] = {0x00ffffff, 0x00ffffff};
			static DWORD mask2[2] = {0xff000000, 0xff000000};

		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOVD		MM7, colKey								//	カラーキーを設定する
			MOV			EDI, lpDst

			MOVQ		MM0, MM7
			MOV			ESI, lpSrc

			PUNPCKLDQ	MM7, MM0			// UnPair			// ColKey

		LoopY_1:
			SHR			ECX, 1
			JNB			Skip_1									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI-4]		// UnPair

			MOVD		MM2, [EDI]
			MOVQ		MM1, MM0

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1			// UnPair			// & mask

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 4

			ADD			ESI, -4				// AGI回避

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVD		[EDI-4], MM0		// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movd	mm1,[edi-4]
			pand	mm0,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movd	[edi-4],mm0

			OR			ECX, ECX
			JZ			EndLoop_1

		Skip_1:
			SHR			ECX, 1
			JNB			LoopX_1									//	4の倍数か否かの判定
			// Step2
			MOVD		MM0, [ESI-4]		// UnPair

			MOVD		MM1, [ESI-8]		// UnPair

			MOVQ		MM2, [EDI]
			PUNPCKLDQ	MM0, MM1								// ソースの 2ピクセルをミラーする

			MOVQ		MM1, MM0			// UnPair

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1			// UnPair			// & mask

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 8

			ADD			ESI, -8				// AGI回避

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVQ		[EDI-8], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi-8]
			pand	mm0,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi-8],mm0

			OR			ECX, ECX
			JZ			EndLoop_1

		LoopX_1: // 15クロック･サイクル
			// Step4
			MOVD		MM0, [ESI-4]		// 1// UnPair

			MOVD		MM1, [ESI-8]		// 1// UnPair

			MOVD		MM3, [ESI-12]		// 2
			PUNPCKLDQ	MM0, MM1			// 1				// ソースの 2ピクセルをミラーする

			MOVD		MM4, [ESI-16]		// 2
			MOVQ		MM1, MM0			// 1

			MOVQ		MM2, [EDI]			// 1
			PUNPCKLDQ	MM3, MM4			// 2				// ソースの 2ピクセルをミラーする

			MOVQ		MM5, [EDI+8]		// 2
			MOVQ		MM4, MM3			// 2

			PCMPEQD		MM1, MM7			// 1				// ColKey:0xffffffff NonColKey:0x00000000
			PCMPEQD		MM4, MM7			// 2				// ColKey:0xffffffff NonColKey:0x00000000

			PXOR		MM2, MM0			// 1				// (src ^ dst)
			PXOR		MM5, MM3			// 2				// (src ^ dst)

			PAND		MM2, MM1			// 1				// & mask
			PAND		MM5, MM4			// 2				// & mask

			PXOR		MM0, MM2			// 1				// src ^ ()
			PXOR		MM3, MM5			// 2				// src ^ ()

			ADD			ESI, -16			// AGI回避

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//	MOVQ		[EDI], MM0			// UnPair
			//	MOVQ		[EDI+8], MM3		// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi]
			pand	mm0,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi],mm0
			movq	mm1,[edi+8]
			pand	mm3,mask1	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mask2	//	0xff000000ff000000	//	α値だけ残す
			por		mm3,mm1
			movq	[edi+8],mm3

			ADD			EDI, 16
			DEC			ECX

			JNZ			LoopX_1				// UnPair

		EndLoop_1:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nSrcWidth							// 次ラインへの移動

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_1				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:有   ミラー:有   拡縮:有	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD	colKey = p->m_dwColorKey;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

			//	mask data setting by yane
			static DWORD mask1[2] = {0x00ffffff, 0x00ffffff};
			static DWORD mask2[2] = {0xff000000, 0xff000000};

		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			MOVD		MM7, colKey								//	カラーキーを設定する
			MOV			ECX, nWidth

			MOVQ		MM0, MM7
			MOV			EDX, nHeight

			PUNPCKLDQ	MM7, MM0								// ColKey
			MOV			EBX, EIY								// nEyCnt = EIY;
			//	mask data setting by yane
			movq		mm3,mask1
			movq		mm4,mask2

		LoopY_2:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_2: // 10クロック･サイクル
			MOVD		MM0, [ESI-4]		// UnPair

			MOVD		MM2, [EDI]
			MOVQ		MM1, MM0

			PCMPEQD		MM1, MM7								// ColKey:0xffffffff NonColKey:0x00000000
			PXOR		MM2, MM0								// (src ^ dst)

			PAND		MM2, MM1								// & mask
			ADD			ESI, AddSrcPixel						// 整数部の加算

			PXOR		MM0, MM2								// src ^ ()
			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_2									// if ( EX >= 0)

			ADD			ESI, -4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_2:
			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVD		[EDI-4], MM0		// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movd	mm1,[edi-4]
			pand	mm0,mm3		//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm4		//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movd	[edi-4],mm0

			DEC			ECX
			JNZ			LoopX_2

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY										// Yの増分を加算
			JNB			SkipY_2										// if ( EY >= 0 )

			ADD			ESI, lPitchSrc
			SUB			EBX, EY2									// Yの補正

		SkipY_2:
			DEC			EDX
			JNZ			LoopY_2

			EMMS
		}
	} // if


	return 0;
} // BltWithoutAlphaM


//////////////////////////////////////
//	BltFastWithoutAlphaM
//	抜き色無しの転送
//////////////////////////////////////
LRESULT CDIB32PMMX::BltFastWithoutAlphaM(CDIB32* lpDIB32,int x,int y,LPRECT lpSrcRect,LPSIZE lpDstSize,LPRECT lpClipRect)
{
	WARNING(m_lpdwSrc == NULL,"CDIB32PMMX::BltFastWithoutAlphaMでm_lpdwSrc == NULL");

	CDIB32Base* p = lpDIB32->GetDIB32BasePtr();
	WARNING(p->GetPtr() == NULL,"CDIB32PMMX::BltFastWithoutAlphaMでp->GetPtr() == NULL");


// クリッピング等の処理
	// 転送範囲なしの時は、このまま帰る
	if ( ClippingM( lpDIB32, x, y, lpSrcRect, lpDstSize,  lpClipRect ) != 0 )
		return 0;

	// 転送先の横幅と縦幅の設定
	int		nWidth = m_rcDstRect.right - m_rcDstRect.left;
	int		nHeight = m_rcDstRect.bottom - m_rcDstRect.top;

	DWORD	nSrcWidth = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;	// 転送する横幅の計算
	DWORD	nAddSrcWidth = p->m_lPitch - nSrcWidth;					// クリッピングして飛ばす分の算出
	DWORD	nAddDstWidth = m_lPitch - (nWidth<<2);					// ASMで使用する 1ラインのバイト数の設定

	DWORD*	lpSrc = (DWORD*)((BYTE*)p->m_lpdwSrc + (m_rcSrcRect.right<<2) + m_rcSrcRect.top*p->m_lPitch);
	DWORD*	lpDst = (DWORD*)((BYTE*)m_lpdwSrc + (m_rcDstRect.left<<2) + m_rcDstRect.top*m_lPitch );	// 指定されたx,yの位置調整

// -----------	 MMX   カラーキー:無   ミラー:有   拡縮:無	 --------------
	if ( m_bActualSize == true )
	{
				nSrcWidth = nSrcWidth + p->m_lPitch;				// 次ラインへの移動距離 = 転送する横幅 + Srcの全横幅

			//	mask data setting by yane
			static DWORD mask1[2] = {0x00ffffff, 0x00ffffff};
			static DWORD mask2[2] = {0xff000000, 0xff000000};

		_asm
		{
			MOV			EDX, nHeight
			MOV			ECX, nWidth

			MOV			EDI, lpDst
			MOV			ESI, lpSrc

			//	mask data setting by yane
			movq		mm5,mask1
			movq		mm6,mask2

		LoopY_3:
			SHR			ECX, 1
			JNB			Skip_3									//	偶数か否かの判定
			// Step	1
			MOVD		MM0, [ESI-4]		// UnPair

			ADD			ESI, -4				// AGI回避

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVD		[EDI], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movd	mm1,[edi]
			pand	mm0,mm5	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm6	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movd	[edi],mm0

			ADD			EDI, 4
			OR			ECX, ECX

			JZ			EndLoop_3			// UnPair

		Skip_3:
			SHR			ECX, 1
			JNB			LoopX_3									//	4の倍数か否かの判定
			// Step2
			MOVD		MM0, [ESI-4]		// UnPair
			MOVD		MM1, [ESI-8]		// UnPair
			PUNPCKLDQ	MM0, MM1								// ソースの 2ピクセルをミラーする
			ADD			EDI, 8

			ADD			ESI, -8				// AGI回避

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVQ		[EDI], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi]
			pand	mm0,mm5	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm6	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi],mm0

			OR			ECX, ECX
			JZ			EndLoop_3

		LoopX_3: // 9クロック･サイクル							// もうちょっとアンロールしたいけど・・・ま、いっか:p
			// Step4
			MOVD		MM0, [ESI-4]		// 1// UnPair

			MOVD		MM1, [ESI-8]		// 1// UnPair

			MOVD		MM3, [ESI-12]		// 2
			PUNPCKLDQ	MM0, MM1			// 1				// ソースの 2ピクセルをミラーする

			MOVD		MM4, [ESI-16]		// 2// UnPair

			PUNPCKLDQ	MM3, MM4			// 2				// ソースの 2ピクセルをミラーする
			ADD			ESI, -16

			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVQ		[EDI], MM0			// 1// UnPair
			//		MOVQ		[EDI+8], MM3		// 2// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movq	mm1,[edi]
			pand	mm0,mm5	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm6	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movq	[edi],mm0
			movq	mm1,[edi+8]
			pand	mm3,mm5	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm6	//	0xff000000ff000000	//	α値だけ残す
			por		mm3,mm1
			movq	[edi+8],mm3

			ADD			EDI, 16
			DEC			ECX

			JNZ			LoopX_3				// UnPair

		EndLoop_3:
			ADD			EDI, nAddDstWidth						// Dstの先頭を次のラインに移動させる
			ADD			ESI, nSrcWidth							// 次ラインへの移動

			MOV			ECX, nWidth								//	ECXの更新
			DEC			EDX

			JNZ			LoopY_3				// UnPair

			EMMS
		}
	} // if
// -----------	 MMX   カラーキー:無   ミラー:有   拡縮:有	 --------------
	else if ( 1 )
	{
		DWORD	AddSrcPixel = -4 * m_nStepsX;
//		DWORD	AddWidthSrc = ((p->m_rcRect.right - p->m_rcRect.left)<<2) * m_nStepsY;
		DWORD	AddWidthSrc = p->m_lPitch * m_nStepsY;
//		DWORD	AddWidthSrc2 = (m_rcSrcRect.right - m_rcSrcRect.left)<<2;
		int		EIX= m_nInitialX;
		int		EIY= m_nInitialY;
		int		EX = m_nStepX;
		int		EY = m_nStepY;
		int		EX2= m_nCmpX;
		int		EY2= m_nCmpY;
		DWORD*	lpSrcBack;
		DWORD	lPitchSrc = p->m_lPitch;

			//	mask data setting by yane
			static DWORD mask1[2] = {0x00ffffff, 0x00ffffff};
			static DWORD mask2[2] = {0xff000000, 0xff000000};

		_asm
		{
			MOV			ESI, lpSrc
			MOV			EDI, lpDst

			MOV			ECX, nWidth
			MOV			EDX, nHeight

			MOV			EBX, EIY			// UnPair			// nEyCnt = EIY;
			//	mask data setting by yane
			movq		mm5,mask1
			movq		mm6,mask2

		LoopY_4:
			MOV			lpSrcBack, ESI
			MOV			EAX, EIX			// UnPair			// nExCnt = EIX;

		LoopX_4: // 7クロック･サイクル
			MOVD		MM0, [ESI-4]		// UnPair

			ADD			ESI, AddSrcPixel						// 整数部の加算
			ADD			EDI, 4

			ADD			EAX, EX									// EX += 2*DX;
			JNB			SkipX_4									// if ( EX >= 0)

			ADD			ESI, -4									// lpSrc++;
			SUB			EAX, EX2								// Xの補正値

		SkipX_4:
			//	これが書き込みしている場所なので変更する(by yane)
			//	Bltのコードはこれ↓
			//		MOVD		[EDI-4], MM0			// UnPair
			//	これをdst = (dst&0xff000000) | (src&0xffffff);にする
			movd	mm1,[edi-4]
			pand	mm0,mm5	//	0x00ffffff00ffffff	//	最上位バイトのマスク(一応ね)
			pand	mm1,mm6	//	0xff000000ff000000	//	α値だけ残す
			por		mm0,mm1
			movd	[edi-4],mm0

			DEC			ECX
			JNZ			LoopX_4

			MOV			ESI, lpSrcBack							// Srcの先頭に戻す
			ADD			EDI, nAddDstWidth							// Dstの先頭を次のラインに移動させる

			MOV			ECX, nWidth
			ADD			ESI, AddWidthSrc						// 整数部の加算 '00.09.12.byTia

			ADD			EBX, EY										// Yの増分を加算
			JNB			SkipY_4										// if ( EY >= 0 )

			SUB			EBX, EY2									// Yの補正
			ADD			ESI, lPitchSrc								// クリップした領域分を飛ばす

		SkipY_4:
			DEC			EDX
			JNZ			LoopY_4

			EMMS
		}
	} // if


	return 0;
} // BltFastWithoutAlphaM

///////////////////////////////////////////////////////////////////////////////

//	αチャンネル付きエフェクト系
LRESULT CDIB32P5::FlushAlpha(LPRECT lpRect){

	WARNING(m_lpdwSrc == NULL,"CDIB32P5::FlushAlphaでm_lpdwSrc == NULL");

	RECT r = GetClipRect(lpRect);
	LONG lPitch	 = GetRect()->right;
	DWORD* pSurface = GetPtr();

	for(int y=r.top;y<r.bottom;y++){
		DWORD *p = pSurface + y*lPitch + r.left;
		for(int x=r.left;x<r.right;x++){
			//	αだけを反転、下位24ビットは無変更
			*(p++) = *p ^ 0xff000000;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

#endif
