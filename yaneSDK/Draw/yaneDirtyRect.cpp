#include "stdafx.h"
#include "yaneDirtyRect.h"
#include <limits.h>

namespace yaneuraoGameSDK3rd {
namespace Draw {

//	ÍÍwè^Cv
#pragma warning(disable:4701)	//	««««« lÌèÄçêÄ¢È¢Â\«Ì é[JÏÌãüwarning

void	CDirtyRect::AddRect(int left,int top,int right,int bottom){
	int nRectLast = m_nRectLast;
	::SetRect(&m_arcRect[nRectLast],left,top,right,bottom);
	m_anRectArea[nRectLast] = (right-left)*(bottom-top);

	if (nRectLast!=nRectSize){
		//	Ü¾üèÜµãÌÅA»ÌÜÜÁZµ½ÌÅIíèÅµã
			//	ÊÏðãüµÄ¨­
		m_nRectLast++;
		return ;
	}

	//	à¤±êÈãAüèÜµ¥ñB
	//	K[xWµÜµã

	//	sûjt
	//		Ý¢É·é±ÆÉæÁÄAÁ·éÊÏªÅ¬ÆÈéàÌðIðµAí·é
	int min_area = INT_MAX;			//	ÉæÁÄÁ·éÊÏÌÅ¬l
	int	min_a;						//	µ½Æ«ÌÊÏ
	int min_i,min_j;				// »ÌÆ«Ìi,jÌl
	int	nLeft,nTop,nRight,nBottom;	// »ÌÆ«Ìé`

	for(int i=0;i<nRectLast;i++){
		for(int j=i+1;j<nRectLast+1;j++){
			//	é`iÆé`jðµÄÝé

			//	OÌÊÏ
			int nAI = m_anRectArea[i];
			int nAJ = m_anRectArea[j];
			//	é`ÌO¤ðæ¾
			int mergeLeft	= m_arcRect[i].left	   < m_arcRect[j].left	 ? m_arcRect[i].left   : m_arcRect[j].left;
			int mergeTop		= m_arcRect[i].top	   < m_arcRect[j].top	 ? m_arcRect[i].top	   : m_arcRect[j].top;
			int mergeRight	= m_arcRect[i].right   > m_arcRect[j].right	 ? m_arcRect[i].right  : m_arcRect[j].right;
			int mergeBottom	= m_arcRect[i].bottom  > m_arcRect[j].bottom ? m_arcRect[i].bottom : m_arcRect[j].bottom;
			//	³ê½ÊÏðæ¾
			int nAC = (mergeRight - mergeLeft) * (mergeBottom - mergeTop);
			//	·é±ÆÉæÁÄÏ»·é·ªðæ¾
			int nADiff = nAC - (nAI + nAJ);
			if (nADiff < min_area) {
				min_a	 = nAC;
				min_area = nADiff;
				min_i = i; min_j=j;
				nLeft = mergeLeft; nTop = mergeTop; nRight = mergeRight; nBottom = mergeBottom;
			}
		}
	}

	//	min_iÆmin_jð·é
		//	min_iÉãü
	m_anRectArea[min_i] = min_a;
	::SetRect(&m_arcRect[min_i],nLeft,nTop,nRight,nBottom);
		//	min_jðÁ
	if (min_j!=nRectLast) {
		//	RséKvª éÆ«¾¯Rsé
		m_anRectArea[min_j] = m_anRectArea[nRectLast];
		m_arcRect[min_j] = m_arcRect[nRectLast];
	}
}
#pragma warning(default:4701)	//	ªªªªª

void	CDirtyRect::Refresh(){
	//	sûjt
	//		PDÝ¢É·é±ÆÉæÁÄAÁ·éÊÏªOÈºÈçÎA
	//			µÄí·é
	//		QDêûªà¤êûÉüèñÅ¢éÌÈçÎA»±ðØf·é
	
	int nRectLast = m_nRectLast;

	for(int i=0;i<nRectLast-1;i++){
		for(int j=i+1;j<nRectLast;){
			//	é`iÆé`jðµÄÝé

			//	OÌÊÏ
			int nAI = m_anRectArea[i];
			int nAJ = m_anRectArea[j];
			//	é`ÌO¤ðæ¾
			int left	= m_arcRect[i].left	   < m_arcRect[j].left	 ? m_arcRect[i].left   : m_arcRect[j].left;
			int top		= m_arcRect[i].top	   < m_arcRect[j].top	 ? m_arcRect[i].top	   : m_arcRect[j].top;
			int right	= m_arcRect[i].right   > m_arcRect[j].right	 ? m_arcRect[i].right  : m_arcRect[j].right;
			int bottom	= m_arcRect[i].bottom  > m_arcRect[j].bottom ? m_arcRect[i].bottom : m_arcRect[j].bottom;
			//	³ê½ÊÏðæ¾
			int nAC = (right - left) * (bottom-top);
			//	·é±ÆÉæÁÄÏ»·é·ªðæ¾
			int nADiff = nAC - (nAI + nAJ);
			if (nADiff <= 0) {	//	·é±ÆÉæÁÄ¬³­Èé
				//	iÆjÍ·é
				m_anRectArea[i] = nAC;
				::SetRect(&m_arcRect[i],left,top,right,bottom);
				//	jðÁ
				if (j!=nRectLast-1) {
				//	RséKvª éÆ«¾¯Rsé
					m_anRectArea[j] = m_anRectArea[nRectLast-1];
					m_arcRect[j] = m_arcRect[nRectLast-1];
				}
				nRectLast--;
			} else {
				//	íª­¶µÈ©Á½Æ«ÌÝjðÁZ
				j++;
			}
		}
	}
	m_nRectLast = nRectLast;
}

} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd
