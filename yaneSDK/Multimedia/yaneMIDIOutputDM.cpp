
#include "stdafx.h"

#if USE_DirectMusic

#include "yaneMIDIOutputDM.h"
#include "yaneDirectMusic.h"

namespace yaneuraoGameSDK3rd {
namespace Multimedia {

#define sign(val) (val>0?1:(val<0?-1:0))	//	???????? * yane

///////////////////////////////////////////////////////////////////////////////

CMIDIOutputDM::CMIDIOutputDM(CDirectMusic* p) : m_pDirectMusic(p)
{
	m_lpDMSegment = NULL;
	m_lpDMSegmentState = NULL;
	m_bPaused = 0;
	m_bLoopPlay = false;
	m_mtPosition = 0;
}

CMIDIOutputDM::~CMIDIOutputDM()
{
	Close();
}

////////////////
//	?I?[?v??
///////////////
LRESULT CMIDIOutputDM::Open(const string& sFileName)
{
	if (!CDirectMusic::CanUseDirectMusic()) return -1;

	Close();

	//	????t?@?C?????????
	if(m_File.Read(sFileName)!=0)	{
		Err.Out("CMIDIOutputDM::Open???????t?@?C???????? %s", sFileName.c_str());
		return 1;
	}

	DMUS_OBJECTDESC desc;
	desc.dwSize = sizeof(DMUS_OBJECTDESC);
	desc.guidClass = CLSID_DirectMusicSegment;
	desc.pbMemData = (BYTE*)m_File.GetMemory(); // (BYTE*)m_alpMIDICache;
	desc.llMemLength = m_File.GetSize();
	desc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY | DMUS_OBJ_LOADED;

	HRESULT hr;
	hr = GetDirectMusic()->GetDirectMusicLoader()->get()->GetObject(
			&desc,IID_IDirectMusicSegment,(void**)&m_lpDMSegment);
	if (FAILED(hr)){
		m_lpDMSegment = NULL;
		return 2;
	}
	hr = m_lpDMSegment->SetParam(GUID_StandardMIDIFile,0xffffffff,0,0,
		(void*)GetDirectMusic()->GetDirectMusicPerformance()->get());
	if (FAILED(hr)) return 3;
	hr = m_lpDMSegment->SetParam(GUID_Download,0xffffffff,0,0,
		(void*)GetDirectMusic()->GetDirectMusicPerformance()->get());
	if (FAILED(hr)) return 4;

	return 0;
}

//////////
//	?N???[?Y
//////////
LRESULT CMIDIOutputDM::Close()
{
	m_File.Close();	//	delete temporary file
	Stop();	// stop???????Close????????[??

	RELEASE_SAFE(m_lpDMSegmentState);

	if (m_lpDMSegment!=NULL) {
		//	?Q?????
		//	GetDirectMusic()->GetDirectMusicLoader()->ReleaseObject((IDirectMusicObject*)m_lpDMSegment);
		//	??????????o?O??O?O?G
		GetDirectMusic()->GetDirectMusicLoader()->get()->ClearCache(CLSID_DirectMusicSegment);

		//	DLS??A?????[?h
		m_lpDMSegment->SetParam(GUID_Unload,0xffffffff,0,0,(void*)GetDirectMusic()->GetDirectMusicPerformance()->get());
		m_lpDMSegment->Release();
	}

	return 0;
}

//////////
//	???t????
/////////

LRESULT CMIDIOutputDM::Play()
{
	if (!CDirectMusic::CanUseDirectMusic()) return -1;
	if (m_lpDMSegment==NULL) return -2;

	m_bPaused = 0;

	Stop();

	//	????J?n
	if (!m_bLoopPlay) {
		m_lpDMSegment->SetRepeats(0);	//	?J???????
	} else {
		m_lpDMSegment->SetRepeats((DWORD)-1);	//	????
	}
	m_mtPosition = 0;
	m_lpDMSegment->SetStartPoint(m_mtPosition);

	HRESULT hr = GetDirectMusic()->GetDirectMusicPerformance()->get()->PlaySegment(
		m_lpDMSegment,DMUS_SEGF_DEFAULT,0,&m_lpDMSegmentState);

	if (FAILED(hr)) return 1;

	return 0;
}

LRESULT CMIDIOutputDM::Replay()
{
	if (!CDirectMusic::CanUseDirectMusic()) return -1;
	if (m_lpDMSegment==NULL) return -2;

	if (m_bPaused==0) return 0;		//	pause???????I
	if (--m_bPaused!=0) return 0;	//	?Q??J?E???g????

	//	???????????????????????A??
	if (IsPlay()) return 0;

	Stop();

	//	?????J
	m_lpDMSegment->SetStartPoint(m_mtPosition);	//	m_mtPosition??O??Pause??????u
	if (!m_bLoopPlay) {
		m_lpDMSegment->SetRepeats(0);	//	?J???????
	} else {
		m_lpDMSegment->SetRepeats((DWORD)-1);	//	????
	}
	HRESULT hr = GetDirectMusic()->GetDirectMusicPerformance()->
		get()->PlaySegment(
			m_lpDMSegment,DMUS_SEGF_DEFAULT,0,&m_lpDMSegmentState);

	if (FAILED(hr)) return 1;

	m_bPaused = 0;
	return 0;
}

LRESULT CMIDIOutputDM::Stop()
{
	if (!CDirectMusic::CanUseDirectMusic()) return -1;

	m_bPaused += sign(m_bPaused);	//	?K?E?Z:p
	if (!IsPlay()) return 0;			//	??????~???????

	//	????????pause????????
	m_bPaused = 1;

	//	?????????o?????~
	if (m_lpDMSegment!=NULL && m_lpDMSegmentState!=NULL) {
		GetDirectMusic()->GetDirectMusicPerformance()->
			get()->Stop(m_lpDMSegment,m_lpDMSegmentState,0,0);
		if (FAILED(m_lpDMSegmentState->GetSeek(&m_mtPosition))) {	//	m_mtPosition??O??Pause??????u
			m_mtPosition = 0;
		}
		RELEASE_SAFE(m_lpDMSegmentState);
	}

	return 0;
}

LONG	CMIDIOutputDM::GetCurrentPos() const {
	if (!CDirectMusic::CanUseDirectMusic()) return -1;

	if (m_lpDMSegmentState==NULL) return -2;

	// ????????|?W?V????????
	MUSIC_TIME mt;
	if (FAILED(m_lpDMSegmentState->GetSeek(&mt))) return -3;
	//	??????????????A?e???|????????????????????????K?v??????

	REFERENCE_TIME rt;
	if (FAILED(GetDirectMusic()->GetDirectMusicPerformance()->get()->MusicToReferenceTime(
		mt,&rt))) return -4;
	REFERENCE_TIME rt2;
	if (FAILED(GetDirectMusic()->GetDirectMusicPerformance()->get()->MusicToReferenceTime(
		0,&rt2))) return -4;
	rt-=rt2;	//	???????????A?x?[?X?????l????????
				//	?????????????K?v????DirectMusic??o?O?????C?????????????B

	return (LONG)(rt/10000);	//	?????[ms]?P??iREFERENCE_TIME?????\???????I?i??j?j
}

LRESULT CMIDIOutputDM::SetCurrentPos(LONG lPos){
	HRESULT hr;
	if (!CDirectMusic::CanUseDirectMusic()) return -1;
	if ( m_lpDMSegment == NULL ) return -1;
	MUSIC_TIME mt;
	LONG lLen = GetLength();
	hr = m_lpDMSegment->GetLength(&mt);
	if (lLen==0) return -2; // ????O????D?D

//	????????????c
//	REFERENCE_TIME rt = lPos*10000;
//	hr = GetDirectMusic()->GetDirectMusicPerformance()->ReferenceToMusicTime(rt,&mt);
	Pause();
	//m_mtPosition;
	m_mtPosition = MUSIC_TIME(lPos * mt / lLen);
	Replay();

	return 0;
}

LRESULT CMIDIOutputDM::SetVolume(LONG volume){
	if (!CDirectMusic::CanUseDirectMusic()) return -1;
	LONG v = volume;
	GetDirectMusic()->GetDirectMusicPerformance()->get()->SetGlobalParam(GUID_PerfMasterVolume,&v,sizeof(v));
	return 0;
}


LONG	CMIDIOutputDM::GetVolume() const
{
	if (!CDirectMusic::CanUseDirectMusic()) return -1;
	LONG v;
	GetDirectMusic()->GetDirectMusicPerformance()->get()->GetGlobalParam(GUID_PerfMasterVolume,(LPVOID)&v,sizeof(v));
	return v;
}

LRESULT CMIDIOutputDM::SetLoopPlay(bool bLoop){
	m_bLoopPlay = bLoop;
	return 0;
}

bool CMIDIOutputDM::IsPlay() const {
	if (!CDirectMusic::CanUseDirectMusic()) return false;
	if (m_lpDMSegmentState==NULL) return false;
	return GetDirectMusic()->GetDirectMusicPerformance()->get()->IsPlaying(m_lpDMSegment,m_lpDMSegmentState)==S_OK;
}

LRESULT	CMIDIOutputDM::LoopPlay(){
	if (m_bLoopPlay) {
		return Play();
	} else {
		return Stop();
	}
}

LONG	CMIDIOutputDM::GetLength() const {
	if (!CDirectMusic::CanUseDirectMusic()) return -1;

	if (m_lpDMSegment==NULL) return -2;

	MUSIC_TIME mt;
	if ( FAILED(m_lpDMSegment->GetLength(&mt))) return -3;

	REFERENCE_TIME rt;
	if (FAILED(
		GetDirectMusic()->GetDirectMusicPerformance()->get()->MusicToReferenceTime(mt,&rt)
		)) return -4;
	REFERENCE_TIME rt2;
	if (FAILED(GetDirectMusic()->GetDirectMusicPerformance()->get()->MusicToReferenceTime(
		0,&rt2))) return -4;
	rt-=rt2;
	return (LONG)(rt/10000);
}

} // end of namespace Multimedia
} // end of namespace yaneuraoGameSDK3rd

#endif // USE_DirectMusic
