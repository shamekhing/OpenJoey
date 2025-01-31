#include "stdafx.h"
#include "yaneTextLayer.h"

void	CTextLayer::OnDraw(HDC hdc){
	m_Font.OnDraw(hdc,m_nX,m_nY);
}
