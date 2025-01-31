#ifndef __yaneMovieBase_h__
#define __yaneMovieBase_h__

#include "yaneSoundBase.h"

#ifdef USE_FastDraw
class CFastPlane;
#else
  #ifdef USE_DirectDraw
class CPlane;
  #endif //end USE_DirectDraw
#endif //end USE_FastDraw

#ifdef USE_DIB32
class CDIB32;
#endif

class CMovieBase : public CSoundBase {
public:
	CMovieBase(void){};
	virtual ~CMovieBase(){};

	// CMovieBase
	virtual void	SetPos(int x,int y)= 0;
//	virtual void	SetVisible(bool bFlag)=0;
	virtual bool*	GetVisible(void)=0;
#ifdef USE_FastDraw
    virtual CFastPlane* GetFastPlane(void)=0;
#else
  #ifdef USE_DirectDraw
	virtual CPlane*	GetPlane(void)=0;
  #endif //end USE_DirectDraw
#endif //end USE_FastDraw
      
#ifdef USE_DIB32
	virtual CDIB32*	GetDIB(void)=0;
#endif
	virtual void	SetSurfaceSize(int x,int y,bool bResize) = 0;
//	virtual void	SetAspect(bool b)=0;
	virtual bool*	GetAspect(void)=0;
	virtual bool*	GetCenter(void)=0;

	virtual bool	IsAudio(void)=0;
	virtual bool	IsVideo(void)=0;

	/* ‚â‚Á‚Ï‚è‚¢‚ç‚È‚¢‚©‚È
	virtual void	SetMute(bool bf)=0;
	virtual void	SetAudioOutput(bool bFlag)= 0;
	virtual bool	IsAudioOutput(void)= 0;
	*/


};

// •K—v‚È‚¢‚Æ‚ÍŽv‚¤‚ª‚Æ‚è‚ ‚¦‚¸
class CMovieNull : public CMovieBase {
public:
	virtual LRESULT Open(LPCTSTR pFileName) { return 0; }
	virtual LRESULT Close(void)	 { return 0; }
	virtual LRESULT Play(void)	 { return 0; }
	virtual LRESULT Replay(void) { return 0; }
	virtual LRESULT Stop(void)	 { return 0; }
	virtual LRESULT Pause(void)	 { return 0; }
	virtual bool IsPlay(void)	 { return false; }
	virtual bool IsLoopPlay(void){ return m_bLoop; }// ’Ç‰Á
	virtual LRESULT SetLoopMode(bool bLoop) { m_bLoop=bLoop; return 0; }
	virtual LONG	GetCurrentPos(void) { return -1; }
	virtual LRESULT SetCurrentPos(LONG lPos){ return -1;}

	virtual LRESULT	SetVolume(long lv){ return -1;}
	virtual LONG	GetVolume(void) { return DSBVOLUME_MIN;}
//	virtual void	SetMute(bool bf){}
	virtual LONG	GetLength(void){ return -1;}
	
	virtual void	SetPos(int x,int y){};
//	virtual void	SetVisible(bool bFlag){};
	virtual bool*	GetVisible(void) {return NULL;}
//	virtual void	SetAudioOutput(bool bFlag){};
//	virtual bool	IsAudioOutput(void) { return false;}
#ifdef USE_FastDraw
    virtual CFastPlane* GetFastPlane(void) { return NULL; }
#else
  #ifdef USE_DirectDraw
	virtual CPlane*	GetPlane(void) { return NULL;}
  #endif //end USE_DirectDraw
#endif //end USE_FastDraw
      
#ifdef USE_DIB32
	virtual CDIB32* GetDIB(void) { return NULL;} 
#endif
	virtual void	SetSurfaceSize(int x,int y,bool bResize){};
	virtual bool*	GetAspect(void){ return NULL;};
	virtual bool*	GetCenter(void){ return NULL;};
	virtual bool	IsVideo(void) { return false;}
	virtual bool	IsAudio(void) { return false;}

	CMovieNull(void) { 
		m_bLoop = false;
//		m_bAudioOutput = false;
	}
	virtual ~CMovieNull(){};

protected:
	bool m_bLoop;
//	bool m_bAudioOutput;
};


#endif
