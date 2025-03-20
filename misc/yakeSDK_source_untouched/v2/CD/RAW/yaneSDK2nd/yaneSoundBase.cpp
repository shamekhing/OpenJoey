#include "stdafx.h"

#include "yaneSoundBase.h"

LRESULT CSoundBase::ConvertToMealTime(LONG nPos,int&nHour,int&nMin,int&nSec,int&nMS){
	if (nPos<0) return -1;
	nMS = nPos % 1000;
	nPos /= 1000;
	nSec = nPos % 60;
	nPos /= 60;
	nMin = nPos % 60;
	nPos /= 60;
	nHour = nPos;
	return 0;
}

LRESULT CSoundBase::ConvertFromMealTime(LONG&nPos,int nHour,int nMin,int nSec,int nMS){
	if ( nHour < 0 || nMin < 0 || nSec < 0 || nMS < 0 ) return -1;
	nPos = nMS;
	nPos += nSec * 1000;
	nPos += nMin * 60 * 1000;
	nPos += nHour * 24*60*1000;
	return 0;
}
