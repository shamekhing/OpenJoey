#include "stdafx.h"

#include "mtknPCMReader.h"
#include "enraAudioPlugin.h"
#include "enraPCMReaderFactory.h"
#include "yaneFile.h"

set< smart_ptr<CAudioPlugin> > CPCMReaderFactory::m_lpAudioPlugin_List;

mtknLib::IkmPCMstream* CPCMReaderFactory::CreateReader(const string& filename)
{
	// ２段階に分けないとテンポラリが破壊される
	const string strFullFilename = CFile::MakeFullName(filename);
	LPCSTR szFullFilename = strFullFilename.c_str();

	{	// PluginLoaderからReader interfaceを取得して、openする
		set< smart_ptr<CAudioPlugin> >::iterator it = m_lpAudioPlugin_List.begin();
		set< smart_ptr<CAudioPlugin> >::iterator itEnd = m_lpAudioPlugin_List.end();
		while (it!=itEnd){
			mtknLib::IkmPCMstream* f = (*it)->QueryInterface();
			if (f && f->Open(szFullFilename)){ return f; }
			// 追加 '02/03/01  by ENRA  自殺させるの忘れてた…リーク天国(;´Д`)
			if (f!=NULL){ f->DeleteSelf(); }
			it++;
		}
	}

	{	// mmioによるwaveファイルのopen
		mtknLib::IkmPCMstream *f=CreateRAW();
		if (f && f->Open(szFullFilename)){ return f; }
		if (f!=NULL){ f->DeleteSelf(); }
	}

	#if 1
	{	// DirectShowFilterによる各種ファイルのopen
		mtknLib::IkmPCMstream *f=CreateDirectShow();
		if (f && f->Open(szFullFilename)){ return f; }
		if (f!=NULL){ f->DeleteSelf(); }
	}
	#endif

	#if 1
	{	// ACMによる各種ファイル(mp3,aiff,...etc)のopen
		mtknLib::IkmPCMstream *f=CreateACM();
		if (f && f->Open(szFullFilename)){ return f; }
		if (f!=NULL){ f->DeleteSelf(); }
	}
	#endif

	Err.Out("CPCMReaderFactory::CreateReader OpenできるReaderが無い - %s", filename);
	return NULL;
}

void CPCMReaderFactory::AddPlugin(const string& PluginFilename)
{
	AddPlugin(smart_ptr<CAudioPlugin>(new CAudioPlugin(PluginFilename),true));
}

void CPCMReaderFactory::AddPlugin(const smart_ptr<CAudioPlugin>& lpPlugin)
{
	m_lpAudioPlugin_List.insert(lpPlugin);
}
