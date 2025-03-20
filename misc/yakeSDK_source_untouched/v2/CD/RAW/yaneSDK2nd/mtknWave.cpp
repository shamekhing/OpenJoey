#include "stdafx.h"

#include "mtknwave.h"
namespace mtknLib {

IkmWaveFileReader *IkmWaveFileReader::create(const char *file)
{
	IkmWaveFileReader *f;


	f=createMMIO();
	if(f->open(file))
	{	return f;	}
	else
	{	delete f;	}

	f=createMP3();
	if(f->open(file))
	{	return f;	}
	else
	{	delete f;	}

	return NULL;
}

};