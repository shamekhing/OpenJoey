#include "stdafx.h"

#include "mtknPcmreader.h"
#include "yanecombase.h"
//--- 追加 '01/11/19  by enra---
#include "enraPCMReaderFactory.h"
//------------------------------

#include <amstream.h>
#pragma comment(lib,"AMSTRMID.lib")

namespace mtknLib {
class kmDirectShowAudio :public virtual IkmPCMstream
{
protected:
	WAVEFORMATEX	m_destWFormat;	

	::IAMMultiMediaStream *pAMStream;
	::IMultiMediaStream	*pMMStream;
    ::IMediaStream		*pStream;
    ::IAudioStreamSample  *pSample;
    ::IAudioMediaStream   *pAudioStream;
    ::IAudioData		    *pAudioData;
	::IGraphBuilder		*pGraphBuilder;

	bool CreateMMStream();

	bool m_bLoop;
public:
	kmDirectShowAudio(::IMultiMediaStream *a_pMMStream)
	{
		pAMStream=NULL;
		pMMStream=NULL;
	    pStream=NULL;
	    pSample=NULL;	
	    pAudioStream=NULL;
	    pAudioData=NULL;
		pGraphBuilder=NULL;

		if( !a_pMMStream )
		{
			if(! CreateMMStream() )
				throw "InitDirectShow Failed";
		}else{
			a_pMMStream->QueryInterface(IID_IAMMultiMediaStream, (void**)&pAMStream);
			pMMStream = (a_pMMStream);
			pMMStream->AddRef();
			Open(NULL);
		}

		m_bLoop=true;
//		Err.Out("CStreamSound Create DirectShow");
	}

#define COM_FREE( x ) \
	do{\
	if(x){\
	   x->Release();\
	   x=NULL;\
	}\
	}while(0)

	~kmDirectShowAudio()
	{
		pMMStream->SetState(STREAMSTATE_STOP);

		COM_FREE(pAMStream);
		COM_FREE(pMMStream);
    	COM_FREE(pStream);
    	COM_FREE(pAudioStream);
    	COM_FREE(pAudioData);
    	COM_FREE(pSample);
		COM_FREE(pGraphBuilder);
	}
#undef COM_FREE

	bool SetLoop(bool f)
	{
		bool old=m_bLoop;
		m_bLoop=f;
		return old;
	}

	bool Open(const char *filename);
	
	DWORD 			Read(BYTE *pDestBuf,DWORD destSize)
	{
		pAudioData->SetBuffer(destSize, pDestBuf, 0);
		HRESULT hr=pSample->Update(0,NULL,NULL,0);

		if( hr == MS_S_ENDOFSTREAM)
		{
			if(m_bLoop){
				pMMStream->Seek(0);
				return Read(pDestBuf, destSize);
			}
//			else
//				pMMStream->SetState(STREAMSTATE_STOP);

			return 0;
		}
		if FAILED(hr) {
			pMMStream->SetState(STREAMSTATE_STOP);
//			_com_error e(hr);
//			Err.Out( "%s(%d) : %s(%x)",__FILE__, __LINE__, e.ErrorMessage(),hr );
			return 0;
		}
		DWORD writtenByte;
		pAudioData->GetInfo(NULL,NULL,&writtenByte);
		return writtenByte;
	}

	WAVEFORMATEX* GetFormat(void)
	{	return &m_destWFormat;	}

	DWORD GetBlockAlignedBufferSize(DWORD time)
	{
		DWORD destSize= m_destWFormat.nSamplesPerSec 
					  * m_destWFormat.nBlockAlign
					  * time ;
		return destSize;
	}

	virtual DWORD GetPCMLength(void)
	{
		STREAM_TIME Duration;
		if( !pMMStream)
			return 0;
		if FAILED( pMMStream->GetDuration(&Duration) )
			return 0;

			DWORD size=m_destWFormat.nSamplesPerSec
				   * m_destWFormat.nBlockAlign
				   * Duration
				   /1000/1000/10 ;
			if(m_destWFormat.nBlockAlign)
			{//どうせ、２の乗数やろ。(うわぁ～)
				size += m_destWFormat.nBlockAlign-1;
				size &= ~(m_destWFormat.nBlockAlign-1);
			}
			return size;
	}

	virtual LONG GetLength(void){
		STREAM_TIME Duration;
		if( !pMMStream)
			return 0;
		if FAILED( pMMStream->GetDuration(&Duration) )	return 0;
		return Duration/10000;
	}

	// posBytesから再生する。
	// ただしこの値は、出力フォーマットによる。
	virtual bool SetPos( DWORD posByte )
	{
		if FAILED(pMMStream->Seek( 
			__int64( posByte) * 10000000 
			/ m_destWFormat.nAvgBytesPerSec)
			)
			return false;
		return true;
	}

	virtual LONG GetCurrentPos(void)
	{
		HRESULT hr;
		STREAM_TIME st,ed,cur;
		st=ed=cur=0;
		if ( pSample == NULL ) return -1;
		hr = pSample->GetSampleTimes(&st,&ed,&cur);
		if ( hr != 0 ){
			return -1;
		}
/*		Err.Out("st:%d",st/10000);
		Err.Out("ed:%d",ed/10000);
		Err.Out("cr:%d",cur/10000);
		*/
		return ed/10000;
	}

	virtual DWORD GetPosToByte( LONG lPos )
	{
		DWORD	dwBytes;
		dwBytes = lPos / 1000 * m_destWFormat.nAvgBytesPerSec;
		return dwBytes;
	}
	virtual LONG GetByteToPos(DWORD dwByte) 
	{
		LONG lPos;
		lPos = (LONG)( (DWORDLONG)dwByte * 1000 / m_destWFormat.nAvgBytesPerSec);
		return lPos;
	}

};
#define ERROR_COMFUNC(hr)\
	do{\
		Err.Out( "%s(%d) : %x",__FILE__, __LINE__, hr );\
		return  false;\
	}while(0)


bool kmDirectShowAudio::CreateMMStream()
{
	HRESULT hr;
	if FAILED( hr=CoCreateInstance(
		CLSID_AMMultiMediaStream, 
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IAMMultiMediaStream,
		(void**)&pAMStream
	) )
		ERROR_COMFUNC(hr);

	if FAILED( hr=pAMStream->Initialize(STREAMTYPE_READ, 0, NULL) )
		ERROR_COMFUNC(hr);

	if FAILED( hr=pAMStream->AddMediaStream(NULL, &MSPID_PrimaryAudio, 0, NULL) )
		ERROR_COMFUNC(hr);
	
	if FAILED( hr=pAMStream->QueryInterface(IID_IMultiMediaStream, (void**)&pMMStream) )
		ERROR_COMFUNC(hr);
	return true;
}

bool kmDirectShowAudio::Open(const char *filename)
{
	HRESULT hr;

	if(filename)
	{
		if(! CreateMMStream() )
			return false;
		WCHAR wszName[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0,filename,-1,
							wszName, MAX_PATH);
		if FAILED( hr=pAMStream->OpenFile(wszName, AMMSF_RUN|AMMSF_NOCLOCK ) ){
			Err.Out("%s:File OpenError.",filename);
			ERROR_COMFUNC(hr);
		}
	}

	if FAILED(hr=pMMStream->GetMediaStream(MSPID_PrimaryAudio, &pStream) )
		ERROR_COMFUNC(hr);
	
	if FAILED(hr=pStream->QueryInterface(IID_IAudioMediaStream, (void **)&pAudioStream) )
		ERROR_COMFUNC(hr);

	if FAILED(hr=pAudioStream->GetFormat(&m_destWFormat))
		ERROR_COMFUNC(hr);

	if FAILED(hr=CoCreateInstance(CLSID_AMAudioData, NULL, CLSCTX_INPROC_SERVER,
		                           IID_IAudioData, (void **)&pAudioData) )
		ERROR_COMFUNC(hr);

	if FAILED(hr=pAudioData->SetFormat(&m_destWFormat) )
		ERROR_COMFUNC(hr);

	if FAILED(hr=pAudioStream->CreateSample(pAudioData, 0, &pSample) )
		ERROR_COMFUNC(hr);

	if FAILED(hr=pAMStream->GetFilterGraph(&pGraphBuilder) )
		ERROR_COMFUNC(hr);

	return true;
}
};

//--- 修正 '01/11/19  by enra---
mtknLib::IkmPCMstream* CPCMReaderFactory::CreateDirectShow(::IMultiMediaStream *a_pMMStream)
{
	try{
		return new mtknLib::kmDirectShowAudio( a_pMMStream );
	}catch( char *)
	{
		return NULL;
	}
}
