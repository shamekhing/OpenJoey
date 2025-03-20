#include "../../yaneSDK3rdTest/yaneSDK/AppFrame/stdafx.h"

#include "../../yaneSDK3rdTest/yaneSDK/Multimedia/yaneSoundStream.h"
#include "../../yaneSDK3rdTest/yaneSDK/Auxiliary/yaneFile.h"
#include "yaneObjectCreater.h"

#include <math.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#pragma comment(lib, "./libvorbis/lib/ogg_static.lib")
#pragma comment(lib, "./libvorbis/lib/vorbis_static.lib")
#pragma comment(lib, "./libvorbis/lib/vorbisfile_static.lib")
#include "enraVorbisStream.h"

CVorbisStream::CVorbisStream()
{
	memset(&m_vVorbisFile, 0, sizeof(m_vVorbisFile));
	Close();
}

CVorbisStream::~CVorbisStream()
{
	Close();
}

// C-Runtimeのfreadのエミュレーション
size_t CVorbisStream::fread_emulation(void* buffer, size_t size, size_t count, void *p)
{
	CVorbisStream* pThis = (CVorbisStream*)p;
	if(p==NULL)return 0;

	// バッファを超えないか調べる
	int size_temp = size*count;
	if((pThis->m_nPos+size_temp) > pThis->m_vFile.GetSize())
	{
		size_temp = pThis->m_vFile.GetSize() - pThis->m_nPos;
	}
	memcpy((BYTE*)buffer, (BYTE*)pThis->m_vFile.GetMemory()+pThis->m_nPos, size_temp);
	fseek_emulation(p, size_temp, SEEK_CUR);
	return (size_temp/size);
}
// C-Runtimeのfseekのエミュレーション
int CVorbisStream::fseek_emulation(void* p, ogg_int64_t offset, int origin)
{
	if(p==NULL)return -1;

	CVorbisStream* pThis = (CVorbisStream*)p;
	switch(origin){
	case SEEK_CUR:
		pThis->m_nPos += (int)offset;
		break;
	case SEEK_END:
		pThis->m_nPos = pThis->m_vFile.GetSize() + (int)offset;
		break;
	case SEEK_SET:
		pThis->m_nPos = (int)offset;
		break;
	default:
		return -1;
	}
	return 0;
}
// C-Runtimeのfcloseのエミュレーション
int CVorbisStream::fclose_emulation(void* p)
{
	if(p==NULL) return(EOF);

	CVorbisStream* pThis = (CVorbisStream*)p;
	return (pThis->m_vFile.Close()==0) ? 0 : EOF;
}
// C-Runtimeのftellのエミュレーション
long CVorbisStream::ftell_emulation(void* p)
{
	if(p==NULL) return -1;

	CVorbisStream* pThis = (CVorbisStream*)p;
	return pThis->m_nPos;
}

LRESULT CVorbisStream::Open(const char* filename)
{
	Close();

	if(m_vFile.Read(filename)!=0){
		Close();  return 1;
	}

	// コールバックの設定
	ov_callbacks callbacks = {
		(size_t (*)(void *, size_t, size_t, void *))  fread_emulation,
		(int (*)(void *, ogg_int64_t, int))           fseek_emulation,
		(int (*)(void *))                             fclose_emulation,
		(long (*)(void *))                            ftell_emulation
	};
	// ファイルはOggVorbis形式かな？
	int ret = ov_open_callbacks((LPVOID)this, GetVorbisFile(), NULL, 0, callbacks);
	if(ret!=0){
		Close();  return 2;
	}

	// SeekableなOggVorbisストリームでないと困る
	if(ov_seekable(GetVorbisFile())==0){
		Close();  return 3;
	}

	// WAVEFORMATEX構造体の設定
	const vorbis_info* pVorbisInfo = ov_info(GetVorbisFile(), -1);
	m_WaveFormat.nChannels = pVorbisInfo->channels & 0xffff;
	m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat.nSamplesPerSec = pVorbisInfo->rate;
	m_WaveFormat.wBitsPerSample = 16;
	m_WaveFormat.nBlockAlign = m_WaveFormat.nChannels * (m_WaveFormat.wBitsPerSample/8);
	m_WaveFormat.nAvgBytesPerSec = m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign;

	return 0;
}

LRESULT CVorbisStream::Close()
{
	ov_clear(GetVorbisFile());

	memset(&m_WaveFormat, 0, sizeof(m_WaveFormat));
	memset(&m_vVorbisFile, 0, sizeof(m_vVorbisFile));

	m_vFile.Close();
	m_nPos = 0;

	return 0;
}

WAVEFORMATEX* CVorbisStream::GetFormat() const
{
	return &const_cast<CVorbisStream*>(this)->m_WaveFormat;
}

//[msec]
LRESULT CVorbisStream::SetCurrentPos(LONG lPosition)
{
	if(m_vFile.GetMemory()==NULL){
		// んなもんでReadさすなボケがーヽ(`Д´)ノ
		return 1;
	}
	const int IsSuccess = ov_time_seek(GetVorbisFile(), (double)lPosition/1000);
	return (IsSuccess!=0)?2:0;
}

//[msec]
LONG CVorbisStream::GetCurrentPos() const
{
	if(m_vFile.GetMemory()==NULL){
		// んなもんでReadさすなボケがーヽ(`Д´)ノ
		return -1;
	}
	const double CurrentTime = ov_time_tell(GetVorbisFile()) * 1000;
	return (LONG)CurrentTime;
}

//[msec]
LONG CVorbisStream::GetLength() const
{
	if(m_vFile.GetMemory()==NULL){
		// んなもんでReadさすなボケがーヽ(`Д´)ノ
		return 0;
	}
	const double TotalTime = ov_time_total(GetVorbisFile(), -1) * 1000;
	return (DWORD)TotalTime;
}

DWORD CVorbisStream::Read(BYTE* lpDest, DWORD dwSize)
{
	if(lpDest==NULL||dwSize==0||m_vFile.GetMemory()==NULL){
		// んなもんでReadさすなボケがーヽ(`Д´)ノ
		return 0;
	}

/* [syntax]
	ov_read(GetVorbisFile(), (char*)lpDest, dwSize,
			0,	// little endian
			2,	// sizeof(short)
			1,	// signed
			NULL);
*/
	// OggVorbisのデコーダはパケット毎にデコードするため、
	// 必要なバッファ分になるまで、何回かデコードしなければいけない。
	long read = 0;
	for(;read<dwSize;){
		const long ret = ov_read(GetVorbisFile(), (char*)lpDest+read, dwSize-read, 0, 2, 1, NULL);
//		const long ret = ov_read_l2s(GetVorbisFile(), lpDestBuf+read, destSize-read);
		// デコード済みサイズを更新して～
		read += ret;
		if(ret==0) break;
	}

	return read;
}

//[msec->byte]
DWORD CVorbisStream::GetPosToByte(LONG lPos)
{
	const DWORD dwByte = (DWORD)( (DWORDLONG)lPos * m_WaveFormat.nAvgBytesPerSec / 1000);
	return dwByte;
}

//[byte->msec]
LONG CVorbisStream::GetByteToPos(DWORD dwByte)
{
	const LONG lPos = (LONG)( (DWORDLONG)dwByte * 1000 / m_WaveFormat.nAvgBytesPerSec);
	return lPos;
}

#ifdef OGGVORBISPLUGIN_EXPORTS
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

void YaneRegistPlugIn(IObjectCreater*p){
	p->RegistClass("CVorbisStream",new factory<CVorbisStream>);
}

#endif // OGGVORBISPLUGIN_EXPORTS

