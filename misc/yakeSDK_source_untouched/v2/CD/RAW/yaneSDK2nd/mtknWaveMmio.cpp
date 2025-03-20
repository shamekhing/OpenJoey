#include "stdafx.h"
#include "mtknWave.h"
#include "yaneFile.h"

namespace mtknLib {

class kmMmioFile : public virtual IkmWaveFileReader
{
	std::string m_filename;
	HMMIO hmmio;
	DWORD dwDataLen;
	WAVEFORMATEX  *m_pWaveFormat;
	LONG dwDataPos;
	CFile m_file;

public:
	kmMmioFile() 
		: m_pWaveFormat(NULL)
		,hmmio(NULL)
	{}

	~kmMmioFile(){close();}

	WAVEFORMATEX *getWaveFormat()
	{
		if(!hmmio)
			return NULL;
		return m_pWaveFormat;
	}

	bool close(){
		if(!hmmio)
			return false;
		mmioClose(hmmio,0);
		hmmio=NULL;
		if(m_pWaveFormat)
			free(m_pWaveFormat);
		return true;
	}
	bool open(const char *filename){
		m_filename=filename;
		// やっとオープンできる。（先は長いぞー）
//		char *tFile=strdup(m_filename.c_str());
//		hmmio = mmioOpen(tFile,NULL,MMIO_READ);
//		free(tFile);
//		if (hmmio==NULL) {
//			Err.Out("CSound::LoadWaveFile : mmioOpenの失敗");
//			return false;
//		}
		if (m_file.Read(m_filename)) {
			Err.Out("kmMmioFile::open " + m_filename + "のファイル読み込み時のエラー");
			return 1; // ファイル読み込みエラー
		}

		// WAVEFORMATEX構造体を得る
		// この形式でwaveデータを獲得しなくては！（大変かも）

		// データは、メモリ上、f.fileadrからf.filesize分だけある。
		MMIOINFO mmio; // メモリファイルをmmioOpenするのだ！！(C)yaneurao
		ZeroMemory(&mmio,sizeof(mmio));
		mmio.pchBuffer = (LPSTR)m_file.GetMemory();	// こいつにファイルバッファadr
		mmio.fccIOProc = FOURCC_MEM;				// メモリから！
		mmio.cchBuffer = m_file.GetSize();			// こいつにファイルサイズ

		// やっとオープンできる。（先は長いぞー）
		hmmio = mmioOpen(NULL,&mmio,MMIO_READ);
		if (hmmio==NULL) {
			Err.Out("kmMmioFile::open : mmioOpenの失敗");
			return false;
		}

		MMCKINFO ciPC,ciSC; // 親チャンクとサブチャンク
		ciPC.fccType = mmioFOURCC('W','A','V','E');
		if (mmioDescend(hmmio,(LPMMCKINFO)&ciPC,NULL,MMIO_FINDRIFF)){
			mmioClose(hmmio,0);
			//	ひょっとしてmp3か？
			return false;
		}
		
		ciSC.ckid = mmioFOURCC('f','m','t',' ');
		if (mmioDescend(hmmio,&ciSC,&ciPC,MMIO_FINDCHUNK)){
			Err.Out("CSound::LoadWaveFile : fmtチャンクが指定できない");
			mmioClose(hmmio,0);
			return false;
		}

		m_pWaveFormat = (WAVEFORMATEX*)calloc(1, max(sizeof(WAVEFORMATEX),(size_t)ciSC.cksize) );
		mmioRead(hmmio,(HPSTR)m_pWaveFormat,ciSC.cksize);
		if(m_pWaveFormat->cbSize ==0 )
			m_pWaveFormat->cbSize=sizeof(WAVEFORMATEX);


		mmioAscend(hmmio,&ciSC,0); // fmtサブチャンク外部へ移動
		ciSC.ckid = mmioFOURCC('d','a','t','a');
		if (mmioDescend(hmmio,&ciSC,&ciPC,MMIO_FINDCHUNK)){
			Err.Out("CSound::LoadWaveFile : データチャンクに行けない");
			mmioClose(hmmio,0);
			return false;
		}

		dwDataLen = ciSC.cksize; // データサイズ
		dwDataPos=mmioSeek(hmmio,0,SEEK_CUR);
		return true;
	}
	virtual DWORD		  getFileSize() const
	{	
		return dwDataLen;
	}
	
	LONG read(BYTE *buf,DWORD size)
	{
		if(!hmmio)
			return 0;
		return mmioRead(hmmio,(HPSTR)buf,size);
	}

	virtual int seekToTop()
	{
		if(!hmmio)
			return 0;
		return mmioSeek(hmmio,dwDataPos,SEEK_SET);

	}

	virtual int seek(LONG pos)
	{
		if(!hmmio)
			return 0;
		return mmioSeek(hmmio,pos,SEEK_CUR);
	}

	virtual LONG GetCurrentPos(void)
	{
		if ( !hmmio ) return -1;
		Err.Out("gcp dwDataPos %d",dwDataPos);
		return mmioSeek(hmmio,0,SEEK_CUR)-dwDataPos;
	}
	/*
	virtual LRESULT SetCurrentPos(LONG lPos)
	{
		if ( !hmmio ) return -1;
		seek(
		return mmioSeek(hmmio,lPos+dwDataPos,SEEK_SET);
	}
	*/

};

IkmWaveFileReader* IkmWaveFileReader::createMMIO()
{
	return new kmMmioFile;
}
};