//	yaneAcm.cpp
#include "stdafx.h"
#include <memory>

#include "mtknPCMReader.h"
#include "mtknWave.h"
//--- 追加 '01/11/19  by enra---
#include "enraPCMReaderFactory.h"
//------------------------------

namespace mtknLib {
	
	//実際の所、単なるラッパ。
	class CkmRawWave :public virtual IkmPCMstream{
	protected:
		WAVEFORMATEX	*m_pWFormat;		
		IkmWaveFileReader *f;
		bool m_bLoop;
	public:
		CkmRawWave(void) : f(NULL), m_bLoop(true){}
		
		bool SetLoop(bool f)
		{
			bool old=m_bLoop;
			m_bLoop=f;
			return old;
		}
		
		bool Open(const char *filename)
		{
			std::auto_ptr<IkmWaveFileReader> fr( IkmWaveFileReader::create(filename) );
			if( !fr.get() ){
				return false;
			}
			
			if( !( m_pWFormat = fr->getWaveFormat() ) )
			{
				Err.Out("CkmRawWave:getWaveFotmat失敗");
				return false;
			}
			
			if(m_pWFormat->wFormatTag != WAVE_FORMAT_PCM){
				return false;
			}
			f=fr.release();
			return true;
		}
		
		~CkmRawWave()
		{
			delete f;	f=NULL;
		}
		
		WAVEFORMATEX	*GetFormat(void)
		{	
			return m_pWFormat;
		}
		
		virtual DWORD	GetPCMLength(void)
		{
			if(f) return f->getFileSize();
			return 0;
		}
		
		virtual DWORD	GetBlockAlignedBufferSize(DWORD time)
		{
			return	m_pWFormat->nSamplesPerSec 
				* m_pWFormat->nBlockAlign
				* time ;
		}

		virtual DWORD	GetPCMLength(void) const
		{	
			return f->getFileSize();
		}

		virtual LONG	GetLength(void)
		{
			DWORD size = GetPCMLength();
			return (LONG)( (DWORDLONG)size * 1000  / m_pWFormat->nAvgBytesPerSec);
		}
		
		DWORD Read(BYTE *lpDestBuf,DWORD destSize)
		{
			if(destSize == 0 || !lpDestBuf)
				return 0;
			LONG read=f->read(lpDestBuf,destSize);
			if( read != destSize && m_bLoop){
				f->seekToTop();
				read += f->read((BYTE*)lpDestBuf+read, 
					destSize - read);
			}
			return read;
		}
		bool SetPos( DWORD posByte )
		{
			f->seekToTop();
			f->seek(posByte);
			return true;
		}
		virtual LONG	GetCurrentPos(void)	// [ms]
		{
			DWORD dwBytes = f->GetCurrentPos();
//			Err.Out("RAW GetCurPos %d",dwBytes);
			return (LONG)( (DWORDLONG)dwBytes * 1000  / m_pWFormat->nAvgBytesPerSec);
		}

		virtual DWORD GetPosToByte(LONG lPos)	// [ms]
		{
			DWORD	dwBytes;
			dwBytes = lPos / 1000 * m_pWFormat->nAvgBytesPerSec;
			return dwBytes;
		}
		virtual LONG GetByteToPos(DWORD dwByte) 
		{
			LONG lPos;
			lPos = (LONG)( (DWORDLONG)dwByte * 1000 / m_pWFormat->nAvgBytesPerSec);
			return lPos;
		}

	};
};

//--- 修正 '01/11/19  by enra---
mtknLib::IkmPCMstream* CPCMReaderFactory::CreateRAW()
{
	return new mtknLib::CkmRawWave;
}
//------------------------------
