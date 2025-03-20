// enraAudioPluginReader.h
//	"IkmPCMstream Plugin" Wrapper for CPCMReaderFactory
//
//		programmed by enra '01/12/13
//

#ifndef __enraAudioPlugin_h__
#define __enraAudioPlugin_h__

class CAudioPlugin
{
public:
	CAudioPlugin(string PluginFilename);
	virtual ~CAudioPlugin();

	// PluginのReaderインターフェースの取得
	mtknLib::IkmPCMstream* QueryInterface();

	// std::set用のoperator
	friend bool operator < (smart_ptr<CAudioPlugin> l, smart_ptr<CAudioPlugin> r);

	string GetFilename() { return m_strFilename; }

protected:
	// Plugin DLL の読み込み
	void InitPlugin(string PluginFilename);
	// Plugin DLL の開放
	void ReleasePlugin();

private:
	HINSTANCE m_hDll;
	string m_strFilename;
};
#endif // __enraAudioPlugin_h__
