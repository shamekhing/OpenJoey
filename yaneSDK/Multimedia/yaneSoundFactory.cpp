#include "stdafx.h"

//	Sound??C???^?[?t?F?[?X
#include "yaneSound.h"

//	DirectSound, DirectMusic
#include "yaneDirectMusic.h"
#include "yaneDirectSound.h"
#include "yaneWaveOutput.h"
#include "yaneSoundParameter.h"

//	Sound?????
#include "yaneMIDIOutputMCI.h"
#if USE_DirectMusic
#include "yaneMIDIOutputDM.h"
#endif
#include "yaneWaveSound.h"

#include "yaneSoundFactory.h"

namespace yaneuraoGameSDK3rd {
namespace Multimedia {

CSoundFactory::CSoundFactory()
{
	m_nMIDIFactoryFirst	 = 2;
	m_nMIDIFactorySecond = 1;

	// default parameter
	m_pSoundParameter.Add();

	// ?f?t?H???g??X?g???[???????
	m_bStreamPlay = false;

	//	???????factory???????????????
	CSound::SetFactory(smart_ptr<ISoundFactory>(this,false));
}

CSoundFactory::~CSoundFactory()
{
}

smart_ptr<ISound> CSoundFactory::Create(const string& filename)
{
	string ext = CFile::GetSuffixOf(filename);
	CFile::ToLower(ext);

	// ???????????(??)
	smart_ptr<ISound> p;
	if (ext=="mid") {
		p = CreateMIDI();
	} else {
		p = CreateSound();
	}
	if (p->Open(filename)!=0)
		{ p.Add(new CSoundNullDevice); }
	//	????????Open????????AOpen???????????????
	//	Loader??Open???????

	return p;
}
	
smart_ptr<ISound> CSoundFactory::CreateSound(){
	return InnerCreate(3);
}

smart_ptr<ISound>	CSoundFactory::Create(const string& filename,bool bStreamPlay){
	return InnerCreate(4 + (bStreamPlay?1:0));
}

smart_ptr<ISound> CSoundFactory::CreateMIDI(){
	smart_ptr<ISound> p(InnerCreate(m_nMIDIFactoryFirst));
	if (p->GetType()==0 /*isNull()*/ ){
		p = InnerCreate(m_nMIDIFactorySecond);
	}
	return p;
}

smart_ptr<ISound> CSoundFactory::InnerCreate(int nDevice){
	ISound* pSound = NULL;
	smart_ptr<ISound> p;

	switch (nDevice){
	case 0 : pSound = new CSoundNullDevice; break;
	case 1 : if (CMIDIOutputMCI::CanUseMCIMIDI()) {
				pSound = new CMIDIOutputMCI();
			}
			break;
#if USE_DirectMusic
	case 2 : if (CDirectMusic::CanUseDirectMusic()) {
				// DirectMusic?g????[
				IncRefDirectMusic();
				pSound = new CMIDIOutputDM(GetDirectMusic());

				// DirectMusic??Q??J?E???g??f?N???????g??????
				// ???????I?u?W?F?N?g??s?????Asmart_ptr????????
				smart_ptr<function_callback>
					fn(function_callback_v::Create(&CSoundFactory::InnerDeleteChainForDM, this, pSound));
				p.Add(pSound, new nonarray_callback_ref_object<ISound>(pSound, fn));
				//	chain????????
				GetSoundList()->insert(pSound);
				return p;
			}
			break;
#endif
	case 3 :
	case 4 :
	case 5 :
		{
			if (CDirectSound::CanUseDirectSound()) {
				// WaveOutput?g????[
				IncRefWaveOutput();
				GetWaveOutput()->SetSoundParameter(GetSoundParameter());
				pSound = new CWaveSound(GetWaveOutput());

				bool bStreamPlay = false;
				if (nDevice==3) bStreamPlay=IsStreamPlay();
				else if (nDevice==4) bStreamPlay = false;
				else if (nDevice==5) bStreamPlay = true;

				// ?D?L????????????
				((CWaveSound*)pSound)->SetStreamPlay(bStreamPlay);

				// WaveOutput??Q??J?E???g??f?N???????g??????
				// ???????I?u?W?F?N?g??s?????Asmart_ptr????????
				smart_ptr<function_callback>
					fn(function_callback_v::Create(&CSoundFactory::InnerDeleteChainForWO, this, pSound));
				p.Add(pSound, new nonarray_callback_ref_object<ISound>(pSound, fn));
				//	chain????????
				GetSoundList()->insert(pSound);
				return p;
			}
		}
			break;
//	case 6 : break;	//	?????
	}

	if (pSound!=NULL){
		// ???I?u?W?F?N?g??s?????Asmart_ptr????????
		smart_ptr<function_callback>
			fn(function_callback_r<bool>::Create(&CSoundFactory::InnerDeleteChain,this,pSound));
		p.Add(pSound,new nonarray_callback_ref_object<ISound>(pSound,fn));
		// chain????????
		GetSoundList()->insert(pSound);
	} else {
		p.Add(new CSoundNullDevice);
	}
	return p;
}

bool CSoundFactory::InnerDeleteChain(ISound* p){
	return GetSoundList()->erase(p);
}

void CSoundFactory::InnerDeleteChainForWO(ISound* p){
	// DirectSound?g???I???????[
	if (InnerDeleteChain(p)) {
		DecRefWaveOutput();
	}
}

#if USE_DirectMusic
void CSoundFactory::InnerDeleteChainForDM(ISound* p){
	// DirectMusic?g???I???????[
	if (InnerDeleteChain(p)) {
		DecRefDirectMusic();
	}
}
#endif

void CSoundFactory::IncRefDirectSound()
{
	m_vDirectSound.inc_ref();
}
CDirectSound* CSoundFactory::GetDirectSound()
{
	return m_vDirectSound.get();
}
void CSoundFactory::DecRefDirectSound()
{
	m_vDirectSound.dec_ref();
}

#if USE_DirectMusic
void CSoundFactory::IncRefDirectMusic()
{
	m_vDirectSound.inc_ref();
	m_vDirectMusic.SetOutClass(GetDirectSound());
	m_vDirectMusic.inc_ref();
}
CDirectMusic* CSoundFactory::GetDirectMusic()
{
	return m_vDirectMusic.get();
}
void CSoundFactory::DecRefDirectMusic()
{
	m_vDirectMusic.dec_ref();
	m_vDirectSound.dec_ref();
}
#else
void CSoundFactory::IncRefDirectMusic() {}
void CSoundFactory::DecRefDirectMusic() {}
CDirectMusic* CSoundFactory::GetDirectMusic() { return nullptr; }
#endif

void CSoundFactory::IncRefWaveOutput()
{
	m_vDirectSound.inc_ref();
	m_vWaveOutputDS.SetOutClass(GetDirectSound());
	m_vWaveOutputDS.inc_ref();
}
IWaveOutput* CSoundFactory::GetWaveOutput()
{
	return m_vWaveOutputDS.get();
}
void CSoundFactory::DecRefWaveOutput()
{
	m_vWaveOutputDS.dec_ref();
	m_vDirectSound.dec_ref();
}

} // end of namespace Multimedia
} // end of namespace yaneuraoGameSDK3rd
