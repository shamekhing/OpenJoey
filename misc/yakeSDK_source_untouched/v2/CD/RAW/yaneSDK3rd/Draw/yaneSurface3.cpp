#include "stdafx.h"
#include "yaneSurface.h"
#include "yaneSurfaceBltter.h"
//#include "yaneGTL.h"	//	↑が読み込むので、あえて指定せんでもええやろ

//	inline展開されるので、ファイル分割しておかないと、馬鹿でっかくなる

LRESULT CSurfaceInfo::GeneralEffect(EEffectType type,LPRECT prc,DWORD*pAdditionalParameter){

	switch (type){
	case eSurfaceEffectFill:
		CSurfaceBltter_EffectHelper1(this,prc,CFastPlaneFillColor,GetFillColor());
		///	↑こいつのなかでreturnしているので、breakしなくても、でんでん大丈夫○(≧∇≦)o
	default:
		return 1;	//	未実装？
	}

	return 0;
}
