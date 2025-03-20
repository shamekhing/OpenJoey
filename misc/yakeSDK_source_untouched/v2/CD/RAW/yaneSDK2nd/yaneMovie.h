#ifndef __yaneMovie_h__
#define __yaneMovie_h__



#include "yaneMovieBase.h"

class CMovie : public CMovieBase {
public:
	CMovie(void);
	virtual ~CMovie();

	// CSoundBase Ç∆ã§í 
	virtual LRESULT Open(LPCTSTR pFileName) { return m_lpMovie->Open(pFileName); }
	virtual LRESULT Close(void) { return m_lpMovie->Close();  }
	virtual LRESULT Play(void)	{ return m_lpMovie->Play();   }
	virtual LRESULT Replay(void){ return m_lpMovie->Replay(); }
	virtual LRESULT Stop(void)	{ return m_lpMovie->Stop();   }
	virtual LRESULT Pause(void) { return m_lpMovie->Pause();  }
	virtual bool IsPlay(void)	{ return m_lpMovie->IsPlay(); }
	virtual bool IsLoopPlay(void) { return m_lpMovie->IsLoopPlay(); }
	virtual LRESULT SetLoopMode(bool bLoop) { return m_lpMovie->SetLoopMode(bLoop); }
	virtual LONG	GetCurrentPos(void) { return m_lpMovie->GetCurrentPos(); }
	virtual LRESULT	SetCurrentPos(LONG lPos){ return m_lpMovie->SetCurrentPos(lPos);}

	virtual LRESULT	SetVolume(LONG lv){ return m_lpMovie->SetVolume(lv);}
	virtual LONG	GetVolume(void) { return m_lpMovie->GetVolume();}
//	virtual void	SetMute(bool bf){ m_lpMovie->SetMute(bf);}
	virtual LONG	GetLength(void){ return m_lpMovie->GetLength();}
	// CMovieBase 
//	virtual void	SetAspect(bool b){ m_lpMovie->SetAspect(b);}
	virtual bool*	GetAspect(void){ return m_lpMovie->GetAspect();}
	virtual void	SetPos(int x,int y){ m_lpMovie->SetPos(x,y);}
//	virtual void	SetAudioOutput(bool bFlag){ m_lpMovie->SetAudioOutput(bFlag);}
//	virtual bool	IsAudioOutput(void) { return m_lpMovie->IsAudioOutput();}
//	virtual void	SetVisible(bool bFlag){ m_lpMovie->SetVisible(bFlag);}
	virtual bool*	GetVisible(void) {return m_lpMovie->GetVisible();}
	virtual bool*	GetCenter(void) { return m_lpMovie->GetCenter();}
	virtual bool	IsVideo(void) { return m_lpMovie->IsVideo();}
	virtual bool	IsAudio(void) { return m_lpMovie->IsAudio();}
#ifdef USE_FastDraw
    virtual CFastPlane* GetFastPlane(void){ return m_lpMovie->GetFastPlane();}
#else 
  #ifdef USE_DirectDraw
      virtual CPlane*	GetPlane(void){ return m_lpMovie->GetPlane();}
  #endif // end USE_DirectDraw
#endif//end USE_FastDraw

#ifdef USE_DIB32
	virtual CDIB32*	GetDIB(void){ return m_lpMovie->GetDIB();}
#endif
	virtual void	SetSurfaceSize(int w,int h,bool bResize=false){ m_lpMovie->SetSurfaceSize(w,h,bResize);}

protected:
	auto_ptrEx<CMovieBase>	m_lpMovie;
private:
	static void	CreateInstance(CMovie* t);

	static int m_nMovieType;	
								//	0 : çƒê∂ÇµÇ»Ç¢?
								//	1 : DirectShow
								//	2 :	AVIStream
								//	3 : MCI
};								


#endif
