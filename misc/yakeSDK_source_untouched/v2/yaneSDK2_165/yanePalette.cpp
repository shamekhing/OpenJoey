#include "stdafx.h"
#include "yanePalette.h"

//////////////////////////////////////////////////////////////////////////////

CPalette::CPalette(void){
}

CPalette::~CPalette(){
}


LPPALETTEENTRY CPalette::GetPalette(void) { return &m_pal[0]; }

//////////////////////////////////////////////////////////////////////////////

LRESULT	CPalette::Get(HDC hdc){	//	PaletteÇéÊìæ
//	if (::GetSystemPaletteEntries(hdc,0,256,&m_pal[0])==0) return 1;
	return 0;
}

LRESULT	CPalette::Set(HDC hdc){	//	PaletteÇê›íË
//	if (::SetPaletteEntries(hdc,0,256,&m_pal[0])==0) return 1;
	return 0;
}

/*
HPALETTE SelectPalette(
  HDC hdc, // handle of device context
  HPALETTE hpal, // handle of logical color palette
  BOOL bForceBackground // foreground/background mode
);
*/

//////////////////////////////////////////////////////////////////////////////

