// enraAudioPlugin.cpp

#include "stdafx.h"
#include "mtknPcmreader.h"
#include "yaneFile.h"
#include "enraAudioPlugin.h"

bool operator < ( smart_ptr<CAudioPlugin> l, smart_ptr<CAudioPlugin> r)
{
	// filename‚Å”»•Ê‚µ‚æ‚¤^^;
	return (l->GetFilename()) < (r->GetFilename());
}

CAudioPlugin::CAudioPlugin(string PluginFilename)
{
	InitPlugin(PluginFilename);
}

CAudioPlugin::~CAudioPlugin()
{
	ReleasePlugin();
}

void CAudioPlugin::InitPlugin(string PluginFilename)
{
	// ‰Šú‰»
	ReleasePlugin();
	m_strFilename = CFile::MakeFullName(PluginFilename);
	// DLL‚ğ“Ç‚İ‚Ş
	m_hDll = ::LoadLibrary(m_strFilename.c_str());
	if(m_hDll==NULL){
		Err.Out("CAudioPlugin::InitPlugin \"%s\"‚Ìƒ[ƒh‚É¸”s", m_strFilename.c_str());
	}
}

void CAudioPlugin::ReleasePlugin()
{
	// Dll‚ÌŠJ•ú
	if(m_hDll!=NULL){
		::FreeLibrary(m_hDll);
		m_hDll = NULL;
	}
}

mtknLib::IkmPCMstream* CAudioPlugin::QueryInterface()
{
	// DLL‚ÌŠÖ”‚ğæ“¾‚·‚é
	if(m_hDll==NULL){ return NULL; }
	typedef void (WINAPI *Func)(LPVOID*,LPCSTR);
	Func CreateInstance = (Func)::GetProcAddress(m_hDll, "CreateInstance");
	if(CreateInstance==NULL){ return NULL; }

	// DLL“à‚ÌReader‚ğæ“¾‚·‚é
	LPVOID lp = NULL;
	CreateInstance(&lp, "AudioReaderPlugin");
	return (mtknLib::IkmPCMstream*)lp;
}

