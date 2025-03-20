//
//	yaneSoundLoader.h :
//
//		サウンドの統括的ローディング
//

#ifndef __yaneSoundLoader_h__
#define __yaneSoundLoader_h__

#include "../Auxiliary/yaneLoadCache.h"

class ISound;
class ISoundFactory;

class CSoundLoader : public CLoadCache {
/**
	class ISound 派生クラスのための LoadCache
*/
public:
	virtual smart_ptr<ISound> GetSound(int nNo);		//	サウンドの取得

	void	SetSoundFactory(const smart_ptr<ISoundFactory>& sf) { m_sf = sf; }

protected:
	smart_vector_ptr<ISound> m_apSound;
	smart_ptr<ISoundFactory> m_sf;

	virtual	LRESULT InnerLoad(int nNo);
	virtual LRESULT InnerRelease(int nNo);
	virtual void	InnerCreate(int nMax);
	virtual void	InnerDelete();
};

#endif