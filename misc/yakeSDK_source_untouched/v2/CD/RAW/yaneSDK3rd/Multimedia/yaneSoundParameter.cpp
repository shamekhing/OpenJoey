#include "stdafx.h"
#include "yaneSoundStream.h"
#include "yaneSoundParameter.h"

CSoundParameter::CSoundParameter()
{
	// デフォルトで持たない
	m_bGlobalFocus		= false;
	m_bPanControl		= false;
	m_bFrequencyControl	= false;
	// デフォルトでCSoundStreamFactoru
	m_pStreamFactory.Add(new CSoundStreamFactory);
}
