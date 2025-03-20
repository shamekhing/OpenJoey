// enraPCMReaderFactory.h
//	"kmPCMstream" Factory
//
//		programmed by enra '01/12/13
//

#ifndef __enraPluginFactory_h__
#define __enraPluginFactory_h__

#include "mtknPCMreader.h"
#include "enraAudioPlugin.h"

class CPCMReaderFactory
{
public:
	CPCMReaderFactory(){};
	virtual ~CPCMReaderFactory(){};

	// kmPCMstreamÇÃê∂ê¨
	virtual mtknLib::IkmPCMstream* CreateReader(const string& filename);

	// CAudioPluginÇÃí«â¡
	virtual void AddPlugin(const string& PluginFilename);
	virtual void AddPlugin(const smart_ptr<CAudioPlugin>& lpPlugin);

protected:
	static mtknLib::IkmPCMstream* CreateRAW();
	static mtknLib::IkmPCMstream* CreateDirectShow(IMultiMediaStream *a_pMMStream=NULL);
	static mtknLib::IkmPCMstream* CreateACM();

private:
	static set< smart_ptr<CAudioPlugin> > m_lpAudioPlugin_List;
};
#endif // __enraPluginFactory_h__