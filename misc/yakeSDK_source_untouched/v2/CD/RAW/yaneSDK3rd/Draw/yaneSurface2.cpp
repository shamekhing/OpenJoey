#include "stdafx.h"
#include "yaneSurface.h"
#include "yaneSurfaceBltter.h"
//#include "yaneGTL.h"	//	↑が読み込むので、あえて指定せんでもええやろ

//	inline展開されるので、ファイル分割しておかないと、馬鹿でっかくなる

LRESULT CSurfaceInfo::GeneralBlt(EBltType type,CSurfaceInfo*pSrc,CBltInfo* pInfo,DWORD*pAdditionalParameter){
	switch (type){
	case eSurfaceBltFast:
		return CSurfaceBltter::Blt1(pSrc,this,pInfo,CFastPlaneCopySrc());
	case eSurfaceBlt:
		CSurfaceBltter_BltHelper2(pSrc,this,pInfo,
			CFastPlaneCopySrcColorKey,pSrc->GetColorKey());
		///	↑こいつのなかでreturnしているので、breakしなくても、でんでん大丈夫○(≧∇≦)o
	default:
		return 1;	//	未実装？
	}

	return 0;
}
