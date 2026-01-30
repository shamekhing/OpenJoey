
//	TEhÖWÌt@CÌinclude

#include "yaneSound.h"			//	for Sound-Base class

//	MIDI,WAV..
#include "yaneMIDIOutputMCI.h"	//	for MIDIOutput using MCI
#include "yaneDirectMusic.h"	//	for DirectMusic (stub when USE_DirectMusic 0)
#if USE_DirectMusic
#include "yaneMIDIOutputDM.h"	//	for MIDIOutput using DirectMusic
#endif
#include "yaneDirectSound.h"	//	for DirectSound
#include "yaneSoundBuffer.h"	//	for SoundBuffer Creater
#include "yaneSoundStream.h"	//	for SoundStream Creater
#include "yaneSoundParameter.h"	//	for Sound Parameter
#include "yaneWaveOutput.h"		//	for Sound Device
#include "yaneWaveSound.h"		//	for Sound Player

//	sound manager
#include "yaneSoundFactory.h"	//	for	Sound Creater

//	volume control
#include "yaneAudioMixer.h"		//	for Volume Control
#include "yaneVolumeFader.h"	//	for Volume Fader

#include "yaneCDDA.h"			//	for CDDA 

#include "yaneSoundLoader.h"	//	for Sound Cache
#include "yaneSELoader.h"		//	for SE Cache

