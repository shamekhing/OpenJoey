#include "stdafx.h"
#include "../Auxiliary/yaneFile.h"
#include "yaneSoundStream.h"
#include "yaneACMStream.h"
#pragma comment(lib, "msacm32.lib")
#include "../Window/yaneDebugWindow.h"
// CACMStream
CACMStream::CACMStream() : m_hmmio(NULL), m_hAcm(NULL)
{
	m_vFile.Add(new CFile);
	// 閉じる
	Close();
}

CACMStream::~CACMStream()
{
	// 閉じる
	Close();
}

LRESULT	CACMStream::Open(const char* filename)
{
	// 閉じる
	Close();

	// 圧縮ファイルなんだから丸ごと読み込む
	if (GetFile()->Read(filename)) {
		Err.Out("CACMStream::Load " + string(filename) + "のファイル読み込み時のエラー");
		return 1; // ファイル読み込みエラー
	}

	// RIFF形式と仮定してオープン
	LRESULT ret = InitializeNormal();
	// あれー？MP3形式なんかなぁ
	if (ret==2){
		ret = InitializeMP3();
	}
	// あかんかった
	if (ret!=0){
		Close();
		return 2;
	}

	// ACMを初期化する
	m_vDestFormat.wFormatTag = WAVE_FORMAT_PCM;	//	PCMになって欲しいねん！
	if (acmFormatSuggest(NULL,m_pSrcFormat, m_pDestFormat, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG)!=0){
		Err.Out("CACMStream::Load acmFormatSuggest()に失敗");
		Close();
		return 3;	//	acm無いんとちゃう？
	}
	if (acmStreamOpen(&m_hAcm, NULL, m_pSrcFormat, m_pDestFormat, NULL, NULL, NULL, ACM_STREAMOPENF_NONREALTIME)!=0){
		Err.Out("CACMStream::Load acmStreamOpen()に失敗");
		Close();
		return 4;	//	acmおかしんとちゃう？
	}
	if (acmStreamSize(m_hAcm, m_dwSrcLength, &m_dwDestLength, ACM_STREAMSIZEF_SOURCE)!=0){
		Err.Out("CACMStream::Load acmStreamSize()に失敗");
		Close();
		return 5;	//	なんでやねんと言いたい（笑）
	}
	// 最小ブロック長を求める
	if (acmStreamSize(m_hAcm, m_dwSrcBlockLength, &m_dwDstBlockLength, ACM_STREAMSIZEF_SOURCE) !=0 ){
		Err.Out("CACMStream::Load acmStreamSize()に失敗");
		Close();
		return 6;	//	なんでやねんと言いたい（笑）
	}

	return 0;
}

LRESULT	CACMStream::Close()
{
	if (m_hmmio!=NULL) mmioClose(m_hmmio, 0);
	m_hmmio = NULL;
	GetFile()->Close();

	m_pSrcFormat = NULL;
	m_pDestFormat = &m_vDestFormat;
	ZERO(m_vMP3Format);
	ZERO(m_vDestFormat);

	m_dwSrcLength = 0;
	m_dwDataOffset = 0;
	m_dwReadPosition = 0;

	m_dwDestLength = 0;

	m_dwSrcBlockLength = 0;
	m_dwDstBlockLength = 0;

	if (m_hAcm!=NULL) acmStreamClose(m_hAcm, 0);
	m_hAcm = NULL;
	ZERO(m_vAcmHeader);

	return 0;
}

LONG CACMStream::GetLength() const
{
	return ByteToMsec(m_dwDestLength);
}

LRESULT CACMStream::InitializeNormal()
{
	// データは、メモリ上、f.fileadrからf.filesize分だけある。
	MMIOINFO mmioinfo; // メモリファイルをmmioOpenするのだ！！(C)yaneurao
	ZeroMemory(&mmioinfo, sizeof(mmioinfo));
	mmioinfo.pchBuffer = (LPSTR)GetFile()->GetMemory();	// こいつにファイルバッファadr
	mmioinfo.fccIOProc = FOURCC_MEM;					// メモリから！
	mmioinfo.cchBuffer = GetFile()->GetSize();			// こいつにファイルサイズ

	// やっとオープンできる。（先は長いぞー）
	m_hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READ);
	if (m_hmmio==NULL) {
		Err.Out("CACMStream::Initialize mmioOpenに失敗");
		return 1;
	}

	MMCKINFO ciPC,ciSC; // 親チャンクとサブチャンク
	ciPC.fccType = mmioFOURCC('W','A','V','E');
	if (mmioDescend(m_hmmio,(LPMMCKINFO)&ciPC,NULL,MMIO_FINDRIFF)){
		Err.Out("CACMStream::Initialize Waveファイルでない");
		return 2;
	}
	ciSC.ckid = mmioFOURCC('f','m','t',' ');
	if (mmioDescend(m_hmmio,&ciSC,&ciPC,MMIO_FINDCHUNK)){
		Err.Out("CACMStream::Initialize fmtチャンクが指定できない");
		return 3;
	}

	// WAVEFORMATEXが可変サイズなので、ポインタを直接突っ込む
	m_pSrcFormat = (WAVEFORMATEX*)((BYTE*)GetFile()->GetMemory() + mmioSeek(m_hmmio,0,SEEK_CUR));
	if (m_pSrcFormat->cbSize==0) { m_pSrcFormat->cbSize = sizeof(WAVEFORMATEX); }
	if (m_pSrcFormat->wFormatTag == WAVE_FORMAT_PCM){
		Err.Out("CWaveStream::Initialize ACMを使うまでもない");
		return 4;
	}

	mmioAscend(m_hmmio,&ciSC,0); // fmtサブチャンク外部へ移動
	ciSC.ckid = mmioFOURCC('d','a','t','a');
	if (mmioDescend(m_hmmio,&ciSC,&ciPC,MMIO_FINDCHUNK)){
		Err.Out("CACMStream::Initialize データチャンクに行けない");
		return 4;
	}

	// さっきと同じ手法でデータへのポインタを得る
	m_dwSrcLength   = ciSC.cksize; // データサイズ
	m_dwDataOffset   = mmioSeek(m_hmmio,0,SEEK_CUR);
	m_dwSrcBlockLength = m_pSrcFormat->nBlockAlign;

	SetCurrentPos(0);

	return 0;
}

LRESULT CACMStream::InitializeMP3()
{
	DWORD dwFileSize = GetFile()->GetSize();
	DWORD availSize = dwFileSize;
	DWORD dwSkipLen = 0;
	if (dwFileSize <= 128) {
		return 1; // そんな小さいわけあらへんがな:p
	}

	//	フレームヘッダからWAVEFORMATEXのデータを用意する
	
	//  ID3v2タグがついているならば、読み飛ばす
	BYTE* src = (BYTE*)GetFile()->GetMemory();
	if ((src[0] == 'I') && (src[1] == 'D') && (src[2] == '3')) {
		DWORD dwID3Size = src[9] + (src[8]<<7) + (src[7]<<14) + (src[6]<<21);
		// バージョンチェック
		if(src[3]>=0x04)
		{
			// ID3v2.4.0以降
			if(src[5]&0x10){	// フッタの有無
				dwID3Size+=20; // フッタあり
			}else{
				dwID3Size+=10; // フッタなし
			}
		}else{
			// ID3v2.3.0以前
			dwID3Size+=10; // フッタなし
		}
		
		if (availSize <= dwID3Size + 128) return 1;
		
		src += dwID3Size;
		availSize -= dwID3Size;
		dwSkipLen += dwID3Size;
	}

	//	MP3チェック
	if (src[0] !=0xff)		return 1;
	if (src[1]&0xf8 !=0xf8) return 1;

	int	anBitrate[2][3][16] = {
		{
			// MPEG-1
			{ 0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0 },	//	32000Hz(layer1)
			{ 0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,0 },	//	44100Hz(layer2)
			{ 0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,0 },	//	48000Hz(layer3)
		},
		{
				// MPEG-2
			{ 0,32,48,56, 64, 80, 96,112,128,144,160,176,192,224,256,0 },	//	32000Hz(layer1)
			{ 0, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0 },	//	44100Hz(layer2)
			{ 0, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,0 },	//	48000Hz(layer3)
			},
	};
	int anFreq[2][4] = {
		{ 44100,48000,32000,0 },
		{ 22050,24000,16000,0 },
	};

	int nLayer = 4-((src[1] >> 1) & 3);
	if (nLayer == 4) {
		Err.Out("CACMStream::InitializeMP3 MP3が無効なレイヤ");
		return 1;
	}

	int nMpeg		= ((src[1] & 8) == 0) ? 1 : 0;
	int nBitrate	= anBitrate[nMpeg][nLayer-1][ src[2]>>4 ];
	int nFreq		= anFreq[nMpeg][(src[2] >> 2) & 3];
	int nChannel	= ((src[3] >> 6) == 3) ? 1 : 2;
	int nFrameSize	= 144000 * nBitrate / nFreq;

	//	無理矢理MP3のタグを用意する
	ZERO(m_vMP3Format);
	m_vMP3Format.wfx.cbSize				= MPEGLAYER3_WFX_EXTRA_BYTES;
	m_vMP3Format.wfx.wFormatTag			= WAVE_FORMAT_MPEGLAYER3;
	m_vMP3Format.wfx.nChannels			= nChannel;
	m_vMP3Format.wfx.nSamplesPerSec		= nFreq;
	m_vMP3Format.wfx.nAvgBytesPerSec	= nBitrate * 1000 / 8;
	m_vMP3Format.wfx.nBlockAlign		= 1;
	m_vMP3Format.wfx.wBitsPerSample		= 0;
	m_vMP3Format.wID					= MPEGLAYER3_ID_MPEG;
	m_vMP3Format.fdwFlags				= MPEGLAYER3_FLAG_PADDING_OFF;
	m_vMP3Format.nBlockSize				= nFrameSize;
	m_vMP3Format.nFramesPerBlock		= 1;
	m_vMP3Format.nCodecDelay			= 0x0571;
	//	ID3タグがついているならば、その分を除外する
	if ((src[dwFileSize-128] == 'T') && (src[dwFileSize-127] == 'A') && (src[dwFileSize-126] == 'G')) {
		availSize -= 128;
	}
	availSize -= 4;
	dwSkipLen += 4;

	m_dwSrcLength = availSize;
	m_dwDataOffset = dwSkipLen;

	// このWAVEFORMATEXを使ってちょ
	m_pSrcFormat = (WAVEFORMATEX*)&m_vMP3Format;
	m_dwSrcBlockLength = m_vMP3Format.nBlockSize;

	SetCurrentPos(0);

	return 0;
}

LRESULT	CACMStream::SetCurrentPos(LONG lPosition)
{
	DWORD dwBytePosition = MsecToByte(lPosition);
	if (dwBytePosition==0xffffffff) return 1;	// 読み込んでないんちゃう？

	if (lPosition >= GetLength()) return 2;		// 飛ばしすぎなんじゃないの？

	// ACMを再初期化する
	if (m_hAcm!=NULL) {
		acmStreamClose(m_hAcm, 0);
		m_hAcm = NULL;
		ZERO(m_vAcmHeader);
		if (acmStreamOpen(&m_hAcm, NULL, m_pSrcFormat, m_pDestFormat, NULL, NULL, NULL, ACM_STREAMOPENF_NONREALTIME)!=0){
			Err.Out("CACMStream::SetCurrentPos acmStreamOpen()に失敗");
			return 3;	//	acmおかしんとちゃう？
		}
	}	
	DWORD dwSrcPos = 0;
	acmStreamSize(m_hAcm, dwBytePosition, &dwSrcPos, ACM_STREAMSIZEF_DESTINATION);
	m_dwReadPosition = dwSrcPos;
	return 0;
}

LONG CACMStream::GetCurrentPos() const
{
	DWORD dwDestPos = 0;
	acmStreamSize(m_hAcm, m_dwReadPosition, &dwDestPos, ACM_STREAMSIZEF_SOURCE);
	LONG lMsecPosition = ByteToMsec(dwDestPos);

	return lMsecPosition;
}

DWORD CACMStream::Read(BYTE* lpDest, DWORD dwSize)
{
	// 読んでないやん
	if (GetFile()->GetMemory()==NULL || m_hAcm==NULL) return -1;

	// 終端やん
	if (m_dwReadPosition==m_dwSrcLength) return 0;

	DWORD dwReaded = 0;
	DWORD ret;
	InnerRead(lpDest+dwReaded, dwSize-dwReaded, ret);
	dwReaded += ret;
	return dwReaded;
}

LRESULT CACMStream::InnerRead(BYTE* lpDest, DWORD dwSize, DWORD& dwReaded)
{
	// ACMSTREAMHEADERを初期化する
	ZERO(m_vAcmHeader);

	m_vAcmHeader.cbStruct		 = sizeof(ACMSTREAMHEADER);
	m_vAcmHeader.pbSrc			 = (BYTE*)GetFile()->GetMemory() + m_dwDataOffset+m_dwReadPosition;
	m_vAcmHeader.cbSrcLength	 = m_dwSrcLength - m_dwReadPosition;
//	acmStreamSize(m_hAcm, dwSize, &m_vAcmHeader.cbSrcLength, ACM_STREAMSIZEF_DESTINATION);
	m_vAcmHeader.cbSrcLengthUsed = 0;
	m_vAcmHeader.pbDst			 = lpDest;
	m_vAcmHeader.cbDstLength	 = dwSize;
//	acmStreamSize(m_hAcm, m_vAcmHeader.cbSrcLength, &m_vAcmHeader.cbDstLength, ACM_STREAMSIZEF_SOURCE);
	m_vAcmHeader.cbDstLengthUsed = 0;

	// 設定する
	if(acmStreamPrepareHeader(m_hAcm, &m_vAcmHeader, NULL)!=0){
		Err.Out("CACMStream::Read acmStreamPrepareHeader()に失敗");
		return 3;
	}
	// 変換する
	if(acmStreamConvert(m_hAcm, &m_vAcmHeader, 0)!=0){
//	if(acmStreamConvert(m_hAcm, &m_vAcmHeader, ACM_STREAMCONVERTF_BLOCKALIGN)!=0){
		Err.Out("CACMStream::Read acmStreamConvert()に失敗");
		return 4;
	}
	// 設定を解除する
	if(acmStreamUnprepareHeader(m_hAcm, &m_vAcmHeader, NULL)!=0){
		Err.Out("CACMStream::Read acmStreamPrepareHeader()に失敗");
		return 5;
	}
	// 変換出来たサイズを調べる
	dwReaded = m_vAcmHeader.cbDstLengthUsed;
	if (dwReaded==0) {
		// この嘘つき！変換してないのにSrc使ったって言いやがるヽ(`Д´)ノ
	} else {
		m_dwReadPosition += m_vAcmHeader.cbSrcLengthUsed;
	}

	if (m_dwReadPosition > m_dwSrcLength) {
		m_dwReadPosition = m_dwSrcLength;
	}

	return 0;
}

LONG CACMStream::ByteToMsec(DWORD dwByte) const
{
	if (GetFormat() && GetFormat()->nAvgBytesPerSec) {
		return (LONG)( (DWORDLONG)dwByte * 1000  / GetFormat()->nAvgBytesPerSec);
	}

	return -1;
}

DWORD CACMStream::MsecToByte(LONG lMsec) const
{
	if (GetFormat() && GetFormat()->nAvgBytesPerSec) {
		return (DWORD)( (DWORDLONG)lMsec * GetFormat()->nAvgBytesPerSec / 1000);
	}

	return -1;
}

