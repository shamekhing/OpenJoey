#include "stdafx.h"
#include "yaneSinTable.h"
#include <math.h>

const double PI = 3.1415926535897932384626433832795;

CSinTable::CSinTable(){
	for(int i=0;i<512;i++){
		m_lTable[i] = (LONG) (cos(i*PI/256) * 65536);
	}
}
