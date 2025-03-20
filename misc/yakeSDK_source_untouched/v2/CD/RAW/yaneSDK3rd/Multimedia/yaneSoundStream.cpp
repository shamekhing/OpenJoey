// Sound Stream Management

#include "stdafx.h"
#include "../Auxiliary/yaneFile.h"
#include "../Auxiliary/yaneStringMap.h"
#include "../AppFrame/yaneObjectCreater.h"

// Stream Family
#include "yaneWaveStream.h"
#include "yaneACMStream.h"


// CSoundStreamManager
CSoundStreamFactory::CSoundStreamFactory()
{
	// plugin map ‚Ì\’z
	m_vPlugInMap.Add(new CStringMap);
}

CSoundStreamFactory::~CSoundStreamFactory()
{
}

smart_ptr<ISoundStream> CSoundStreamFactory::Open(const string& filename)
{
	// ‚Ü‚¸Šg’£q‚ğ“¾‚é
	string ext = CFile::GetSuffixOf(filename);
	CFile::ToLower(ext);

	// Šg’£q‚É‚æ‚Á‚Ä¶¬‚·‚é
	smart_ptr<ISoundStream> p;
	if (ext=="wav") {	// .wav
		p.Add(new CWaveStream);
		if (p->Open(filename.c_str())==0) return p;
	}

	// map‚É•·‚¢‚Ä‚İ‚æ‚©
	map<string, string>::const_iterator it = GetPlugInMap()->GetMap()->begin();
	while (it!=GetPlugInMap()->GetMap()->end()){
		string registerd_ext = it->first;  CFile::ToLower(registerd_ext);
		if (registerd_ext==ext){
			p.Add((ISoundStream*)CObjectCreater::GetObj()->Create(it->second));
			if (!p.isNull() && p->Open(filename.c_str())==0) return p;
		}
		it++;
	}

	// ACM‚É•·‚¢‚Ä‚İ‚æ‚©
	{
		p.Add(new CACMStream);
		if (p->Open(filename.c_str())==0) return p;
	}

	// –³‚¢‚Ë
	p.Add(new CNullSoundStream);

	return p;
}
