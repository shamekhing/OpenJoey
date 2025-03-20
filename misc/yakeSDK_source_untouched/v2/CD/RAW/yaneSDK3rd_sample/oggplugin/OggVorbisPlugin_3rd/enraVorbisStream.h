//	enraVorbisStream.h :
//		programmed by ENRA	 '02/04/03

#ifndef __enraVorbisStream_h__
#define __enraVorbisStream_h__

#include "vorbis/vorbisfile.h"
#include "../../yaneSDK3rdTest/yaneSDK/Multimedia/yaneSoundStream.h"

class CVorbisStream : public ISoundStream
{
public:
	CVorbisStream();
	virtual ~CVorbisStream();

	/// override from class ISoundStream
	virtual LRESULT	Open(const char* filename);
	virtual LRESULT	Close();
	virtual LONG	GetLength() const;
	virtual LRESULT	SetCurrentPos(LONG lPosition);
	virtual LONG	GetCurrentPos() const;
	virtual DWORD 	Read(BYTE* lpDest, DWORD dwSize);
	virtual DWORD	GetPacketSize() const { return 0; }
	virtual WAVEFORMATEX* GetFormat() const;

protected:
	void	Init();
	OggVorbis_File* GetVorbisFile() const { return &const_cast<CVorbisStream*>(this)->m_vVorbisFile; }

	static size_t	fread_emulation(void*, size_t, size_t, void*);
	static int		fseek_emulation(void*, ogg_int64_t, int);
	static int		fclose_emulation(void*);
	static long		ftell_emulation(void*);

	//[msec->byte]
	DWORD	GetPosToByte(LONG lPos);
	//[byte->msec]
	LONG	GetByteToPos(DWORD dwByte);

private:
	WAVEFORMATEX	m_WaveFormat;
	OggVorbis_File	m_vVorbisFile;
	CFile			m_vFile;
	long			m_nPos;
};

#endif // __enraVorbisStream_h__
