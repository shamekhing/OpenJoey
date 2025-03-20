//
//	yaneSurfaceBltter.h
//		サーフェースの転送補助
//

#ifndef __yaneSurfaceBltter_h__
#define __yaneSurfaceBltter_h__

#include "yaneSurface.h"
#include "yaneGTL.h"

class CSurfaceBltter {
/**
	CSurfaceInfoのGeneralBltの転送の実装のためのヘルパ関数

	Blt1 : ほぼすべてのサーフェース間での転送をサポートする
			WORD型のサーフェース(RGB565等)に対しては
			CFastPlaneEffect::BltDoubleを呼び出す。
			※　CFastPlaneCopySrcで使用。
	Blt2 : Blt1の縮退版
			サポート：
			同じサーフェース間
			非αサーフェース⇒αサーフェース
			αサーフェース⇒非αサーフェース
	Blt3 : Blt2の縮退版
			サポート：
			同じサーフェース間
			αサーフェース⇒非αサーフェース
			※　CFastPlaneCopyMulAlphaで使用

	Effect1（未実装） : サーフェースに対して、エフェクトをかける


	＜マクロ＞
		※　Fnは、DWORD dwで与えられた抜き色情報等が代入される
	CSurfaceBltter_BltHelper1 : 未実装（面倒なので）
	CSurfaceBltter_BltHelper2 : Blt2コンパチ
			※	CFastPlaneCopySrcColorKeyで使用。
	CSurfaceBltter_EffectHelper1 : Effect1とコンパチ
			※	CFastPlaneFillColorで使用。

		例：FnがCFastPlaneCopySrcColorKeyDoubleならば
		Fn ← CFastPlaneCopySrcColorKeyDouble<CFastPlaneRGB565565> rgb(dword);
		とバインドして、Blt2を呼び出すなど。

		C++のテンプレート機構では、
			F<T> というような関数のF<T>ではなくFだけを引数として渡すことが出来ない。
			そこで、マクロが必要になる。

		BltHelper系は、マクロとして実装されているので呼び出し箇所にて
		インライン展開されるので、その点に注意すること。


	ToDo:やっとyaneSDK2ndのyaneFastPlane1,2の実装終了。
		残りは、3,4,5..
*/
public:

	template <class Fn>
	static LRESULT Blt1(CSurfaceInfo* pSrc,CSurfaceInfo* pDst,CSurfaceInfo::CBltInfo *pInfo,Fn fn){
	//	このようにコーディングしておけば、この関数を抜けるときに自動的にデストラクタが呼び出され、
	//	正常にUnlockされる：
	if (pDst->Lock()!=0) return 1;
	CSurfaceLockerGuard g1(pDst); 
	if (pSrc->Lock()!=0) return 1;
	CSurfaceLockerGuard g2(pSrc); 

	int nS1 = pDst->GetSurfaceType();
	int nS2 = pSrc->GetSurfaceType();

	//	BltFastだけは、AutoRestoreで使用するため、
	//	ほぼすべての変換をサポート！

	switch (nS2){
	case 3:
		switch (nS1){
		//	RGB565 ->565
		case 3: {
			CFastPlaneEffect::BltDouble(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneRGB565(),pDst,
				CFastPlaneXRGB8888(),	//	DWORD扱いする
				CFastPlaneXRGB8888(),	//	DWORD扱いする
				fn,
				fn,
				pInfo);
			break; }
		//	RGB565->RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB565->RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB565->BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB565->XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB565->XRGB8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB565 -> ARGB4565はサポート
		case 10: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneARGB4565(),pDst,
				fn,
				pInfo);
			break; }
		default:
		//	残念ながら非サポートでした＾＾；
			return 2;
		} break;
	case 4:
		switch (nS1){
		//	RGB555 -> 8bpp はサポート
		case 2: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneBytePal(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB555 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		case 4: {
			CFastPlaneEffect::BltDouble(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneRGB555(),pDst,
				CFastPlaneXRGB8888(),	//	DWORD扱いする
				CFastPlaneXRGB8888(),	//	DWORD扱いする
				fn,
				fn,
				pInfo);
			break; }
		//	RGB555->RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB555->BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break;}
		//	RGB555->XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB555->XRGB8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		case 11: {
		//	RGB555 -> ARGB4555はサポート
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneARGB4555(),pDst,
				fn,
				pInfo);
			break; }
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;
	case 5:
		switch (nS1){
		//	RGB888 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888->RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888->BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break;}
		//	RGB888->XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888->XRGB8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888/BGR888/XRGB8888/ARGB8888 -> ARGB8888はサポート
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break;}
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;
	case 6:
		switch (nS1){
		//	BGR888 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	BGR888 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	BGR888->RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	BGR888->BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break;}
		//	BGR888->XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	BGR888->XRGB8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888/BGR888/XRGB8888/ARGB8888 -> ARGB8888はサポート
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break;}
		//	BGR -> ABGRサポート
		case 13: {
			CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,
				pInfo);
			break;}
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;
	case 7:
		switch (nS1){
		//	XRGB8888 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	XRGB8888 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	XRGB8888->RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	XRGB8888->BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break;}
		//	XRGB8888->XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	XRGB8888->XRGB8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888/BGR888/XRGB8888/ARGB8888 -> ARGB8888はサポート
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break;}
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;
	case 8:
		switch (nS1){
		//	XBGR8888 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	XBGR8888 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	XBGR8888->RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	XBGR8888->BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break;}
		//	XBGR8888->XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	XBGR8888->XBGR8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	RGB888/BGR888/XRGB8888/ARGB8888 -> ARGB8888はサポート
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break;}
		//	BGR->ABGRサポート
		case 13: {
			CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,
				pInfo);
			break;}
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;

	//	αサーフェース間の変換
	case 10:
		switch (nS1){
		//	ARGB4565 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> XBGR8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> ARGB4565
		case 10: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneARGB4565(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> ARGB4555
		case 11: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneARGB4555(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> ARGB8888
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4565 -> ABGR8888
		case 13: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,
				pInfo);
			break; }
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;

	case 11:
		switch (nS1){
		//	ARGB4555 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> XBGR8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> ARGB4565
		case 10: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneARGB4565(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> ARGB4555
		case 11: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneARGB4555(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> ARGB8888
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB4555 -> ABGR8888
		case 13: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,
				pInfo);
			break; }
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;

	case 12:
		switch (nS1){
		//	ARGB8888 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> XBGR8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> ARGB4565
		case 10: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneARGB4565(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> ARGB4555
		case 11: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneARGB4555(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> ARGB8888
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ARGB8888 -> ABGR8888
		case 13: {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,
				pInfo);
			break; }
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;

	case 13:
		switch (nS1){
		//	ABGR8888 -> RGB565
		case 3: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> RGB555
		case 4: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> RGB888
		case 5: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> BGR888
		case 6: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> XRGB8888
		case 7: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> XBGR8888
		case 8: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> ARGB4565
		case 10: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneARGB4565(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> ARGB4555
		case 11: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneARGB4555(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> ARGB8888
		case 12: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,
				pInfo);
			break; }
		//	ABGR8888 -> ABGR8888
		case 13: {
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,
				pInfo);
			break; }
		default:
			//	残念ながら非サポートでした＾＾；
			return 2;
		} break;

	//	このソースサーフェースタイプはなんやねん？
	default:
		//	残念ながら非サポートでした＾＾；
		;
	}

	return 0;
	}	//	the end of function Blt1

	/////////////////////////////////////////////////////////////////////////////

	template <class Fn>
	static LRESULT Blt2(CSurfaceInfo* pSrc,CSurfaceInfo* pDst,CSurfaceInfo::CBltInfo *pInfo,Fn fn){
	//	このようにコーディングしておけば、この関数を抜けるときに自動的にデストラクタが呼び出され、
	//	正常にUnlockされる：
	if (pDst->Lock()!=0) return 1;
	CSurfaceLockerGuard g1(pDst); 
	if (pSrc->Lock()!=0) return 1;
	CSurfaceLockerGuard g2(pSrc); 

	int nS1 = pDst->GetSurfaceType();
	int nS2 = pSrc->GetSurfaceType();

	//	異なるサーフェース間の転送は基本的にサポートしない！
	if (nS1!=nS2) {

		//	特別サポート！＾＾；
		//	αサーフェースからの転送
		//	αプレーンとの親和性
		if (nS1==3 && nS2==10) {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4565(),pSrc,
				CFastPlaneRGB565(),pDst,
				fn,pInfo);
		}ef(nS1==4 && nS2==11) {
			CFastPlaneEffect::Blt(
				CFastPlaneARGB4555(),pSrc,
				CFastPlaneRGB555(),pDst,
				fn,pInfo);
		}ef(nS1==5 && nS2==12){
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneRGB888(),pDst,
				fn,pInfo);
		}ef(nS1==6 && nS2==13){
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneBGR888(),pDst,
				fn,pInfo);
		}ef(nS1==7 && nS2==12){
			CFastPlaneEffect::Blt(
				CFastPlaneARGB8888(),pSrc,
				CFastPlaneXRGB8888(),pDst,
				fn,pInfo);
		}ef(nS1==8 && nS2==13){
			CFastPlaneEffect::Blt(
				CFastPlaneABGR8888(),pSrc,
				CFastPlaneXBGR8888(),pDst,
				fn,pInfo);
		}
		
		//	おまけでサポート＾＾；
		//	αサーフェースへの転送

		//	RGB565 -> ARGB4565はサポート
		ef (nS2==3 && nS1==10){
			CFastPlaneEffect::Blt(
				CFastPlaneRGB565(),pSrc,
				CFastPlaneARGB4565(),pDst,
				fn,pInfo);
		}ef(nS2==4 && nS1==11){
		//	RGB555 -> ARGB4555はサポート
			CFastPlaneEffect::Blt(
				CFastPlaneRGB555(),pSrc,
				CFastPlaneARGB4555(),pDst,
				fn,pInfo);
		//	RGB888/BGR888/XRGB8888/ARGB8888 -> ARGB8888はサポート
		}ef(nS1==12){
			switch (nS2){
			case 5: {
				CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,pInfo);
				break;
					}
			case 6: {
				CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,pInfo);
				break;
					}
			case 7: {
				CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,pInfo);
				break;
					}
			case 8: {
				CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneARGB8888(),pDst,
				fn,pInfo);
				break;
					}
			default:
				//	残念ながら非サポートでした＾＾；
				return 2;
			}
		//	RGB888/BGR888/XRGB8888/ARGB8888 -> ABGR8888はサポート
		}ef(nS1==13){
			switch (nS2){
			case 5: {
				CFastPlaneEffect::Blt(
				CFastPlaneRGB888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,pInfo);
				break;
					}
			case 6: {
				CFastPlaneEffect::Blt(
				CFastPlaneBGR888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,pInfo);
				break;
					}
			case 7: {
				CFastPlaneEffect::Blt(
				CFastPlaneXRGB8888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,pInfo);
				break;
					}
			case 8: {
				CFastPlaneEffect::Blt(
				CFastPlaneXBGR8888(),pSrc,
				CFastPlaneABGR8888(),pDst,
				fn,pInfo);
				break;
					}
			default:
				//	残念ながら非サポートでした＾＾；
				return 2;
			}
		} else {
			//	残念ながら非サポートでした＾＾；
			return 2;
		}
		return 0;
	}

	switch( nS1 ) {
	case 3:	{
		CFastPlaneEffect::Blt(
			CFastPlaneRGB565(),pSrc,
			CFastPlaneRGB565(),pDst,
				fn,pInfo);
				break;
		}
	case 4: {
		CFastPlaneEffect::Blt(
			CFastPlaneRGB555(),pSrc,
			CFastPlaneRGB555(),pDst,
				fn,pInfo);
				break;
		}
	case 5:	{
		CFastPlaneEffect::Blt(
			CFastPlaneRGB888(),pSrc,
			CFastPlaneRGB888(),pDst,
				fn,pInfo);
				break;
		}
	case 6: {
		CFastPlaneEffect::Blt(
			CFastPlaneBGR888(),pSrc,
			CFastPlaneBGR888(),pDst,
				fn,pInfo);
				break;
		}
	case 7: {
		CFastPlaneEffect::Blt(
			CFastPlaneXRGB8888(),pSrc,
			CFastPlaneXRGB8888(),pDst,
				fn,pInfo);
				break;
		}
	case 8: {
		CFastPlaneEffect::Blt(
			CFastPlaneXBGR8888(),pSrc,
			CFastPlaneXBGR8888(),pDst,
				fn,pInfo);
				break;
		}
	case 10: {
		CFastPlaneEffect::Blt(
			CFastPlaneARGB4565(),pSrc,
			CFastPlaneARGB4565(),pDst,
				fn,pInfo);
				break;
		}
	case 11: {
		CFastPlaneEffect::Blt(
			CFastPlaneARGB4555(),pSrc,
			CFastPlaneARGB4555(),pDst,
				fn,pInfo);
				break;
		}
	case 12: {
		CFastPlaneEffect::Blt(
			CFastPlaneARGB8888(),pSrc,
			CFastPlaneARGB8888(),pDst,
				fn,pInfo);
				break;
		}
	case 13: {
		CFastPlaneEffect::Blt(
			CFastPlaneABGR8888(),pSrc,
			CFastPlaneABGR8888(),pDst,
				fn,pInfo);
				break;
		}
	}

	return 0;
	}	//	the end of function Blt2

// この関数の引数はBlt2とコンパチ
//	ただし、fnは、dwを食わせてから実体化
#define	CSurfaceBltter_BltHelper2(pSrc,pDst,pInfo,fn,dw){		\
	if (pDst->Lock()!=0) return 1;						\
	CSurfaceLockerGuard g1(pDst);						\
	if (pSrc->Lock()!=0) return 1;						\
	CSurfaceLockerGuard g2(pSrc);						\
	int nS1 = pDst->GetSurfaceType();					\
	int nS2 = pSrc->GetSurfaceType();					\
	if (nS1!=nS2) {										\
		if (nS1==3 && nS2==10) {						\
			CFastPlaneARGB4565 rgb;						\
			rgb.SetRGB(dw);								\
			rgb.SetA(0);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneARGB4565(),pSrc,				\
				CFastPlaneRGB565(),pDst,				\
				fn<CFastPlaneARGB4565>(rgb),pInfo);		\
		}ef(nS1==4 && nS2==11) {						\
			CFastPlaneARGB4555 rgb;						\
			rgb.SetRGB(dw);								\
			rgb.SetA(0);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneARGB4555(),pSrc,				\
				CFastPlaneRGB555(),pDst,				\
				fn<CFastPlaneARGB4555>(rgb),pInfo);		\
		}ef(nS1==5 && nS2==12){							\
			CFastPlaneARGB8888 rgb;						\
			rgb.SetRGB(dw);								\
			rgb.SetA(0);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneARGB8888(),pSrc,				\
				CFastPlaneRGB888(),pDst,				\
				fn<CFastPlaneARGB8888>(rgb),pInfo);		\
		}ef(nS1==6 && nS2==13){							\
			CFastPlaneABGR8888 rgb;						\
			rgb.SetRGB(dw);								\
			rgb.SetA(0);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneABGR8888(),pSrc,				\
				CFastPlaneBGR888(),pDst,				\
				fn<CFastPlaneABGR8888>(rgb),pInfo);		\
		}ef(nS1==7 && nS2==12){							\
			CFastPlaneARGB8888 rgb;						\
			rgb.SetRGB(dw);								\
			rgb.SetA(0);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneARGB8888(),pSrc,				\
				CFastPlaneXRGB8888(),pDst,				\
				fn<CFastPlaneARGB8888>(rgb),pInfo);		\
		}ef(nS1==8 && nS2==13){							\
			CFastPlaneABGR8888 rgb;						\
			rgb.SetRGB(dw);								\
			rgb.SetA(0);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneABGR8888(),pSrc,				\
				CFastPlaneXBGR8888(),pDst,				\
				fn<CFastPlaneABGR8888>(rgb),pInfo);		\
		}												\
														\
		ef (nS2==3 && nS1==10){							\
			CFastPlaneRGB565 rgb;						\
			rgb.SetRGB(dw);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneRGB565(),pSrc,				\
				CFastPlaneARGB4565(),pDst,				\
				fn<CFastPlaneRGB565>(rgb),pInfo);		\
		}ef(nS2==4 && nS1==11){							\
			CFastPlaneRGB555 rgb;						\
			rgb.SetRGB(dw);								\
			CFastPlaneEffect::Blt(						\
				CFastPlaneRGB555(),pSrc,				\
				CFastPlaneARGB4555(),pDst,				\
				fn<CFastPlaneRGB555>(rgb),pInfo);		\
		}ef(nS1==12){									\
			switch (nS2){								\
			case 5: {									\
				CFastPlaneRGB888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneRGB888(),pSrc,				\
				CFastPlaneARGB8888(),pDst,				\
				fn<CFastPlaneRGB888>(rgb),pInfo);		\
				break;									\
					}									\
			case 6: {									\
				CFastPlaneBGR888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneBGR888(),pSrc,				\
				CFastPlaneARGB8888(),pDst,				\
				fn<CFastPlaneBGR888>(rgb),pInfo);		\
				break;									\
					}									\
			case 7: {									\
				CFastPlaneXRGB8888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneXRGB8888(),pSrc,				\
				CFastPlaneARGB8888(),pDst,				\
				fn<CFastPlaneXRGB8888>(rgb),pInfo);		\
				break;									\
					}									\
			case 8: {									\
				CFastPlaneXBGR8888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneXBGR8888(),pSrc,				\
				CFastPlaneARGB8888(),pDst,				\
				fn<CFastPlaneXBGR8888>(rgb),pInfo);		\
				break;									\
					}									\
			default:									\
				return 2;								\
			}											\
		}ef(nS1==13){									\
			switch (nS2){								\
			case 5: {									\
				CFastPlaneRGB888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneRGB888(),pSrc,				\
				CFastPlaneABGR8888(),pDst,				\
				fn<CFastPlaneRGB888>(rgb),pInfo);		\
				break;									\
					}									\
			case 6: {									\
				CFastPlaneBGR888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneBGR888(),pSrc,				\
				CFastPlaneABGR8888(),pDst,				\
				fn<CFastPlaneBGR888>(rgb),pInfo);		\
				break;									\
					}									\
			case 7: {									\
				CFastPlaneXRGB8888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneXRGB8888(),pSrc,				\
				CFastPlaneABGR8888(),pDst,				\
				fn<CFastPlaneXRGB8888>(rgb),pInfo);		\
				break;									\
					}									\
			case 8: {									\
				CFastPlaneXBGR8888 rgb;					\
				rgb.SetRGB(dw);							\
				CFastPlaneEffect::Blt(					\
				CFastPlaneXBGR8888(),pSrc,				\
				CFastPlaneABGR8888(),pDst,				\
				fn<CFastPlaneXBGR8888>(rgb),pInfo);		\
				break;									\
					}									\
			default:									\
				return 2;								\
			}											\
		} else {										\
			return 2;									\
		}												\
		return 0;										\
	}													\
														\
	switch( nS1 ) {										\
	case 3:	{											\
		CFastPlaneRGB565 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneRGB565(),pSrc,					\
			CFastPlaneRGB565(),pDst,					\
			fn<CFastPlaneRGB565>(rgb),pInfo);			\
				break;									\
		}												\
	case 4: {											\
		CFastPlaneRGB555 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneRGB555(),pSrc,					\
			CFastPlaneRGB555(),pDst,					\
			fn<CFastPlaneRGB555>(rgb),pInfo);			\
				break;									\
		}												\
	case 5:	{											\
		CFastPlaneRGB888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneRGB888(),pSrc,					\
			CFastPlaneRGB888(),pDst,					\
			fn<CFastPlaneRGB888>(rgb),pInfo);			\
				break;									\
		}												\
	case 6: {											\
		CFastPlaneBGR888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneBGR888(),pSrc,					\
			CFastPlaneBGR888(),pDst,					\
			fn<CFastPlaneBGR888>(rgb),pInfo);			\
				break;									\
		}												\
	case 7: {											\
		CFastPlaneXRGB8888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneXRGB8888(),pSrc,					\
			CFastPlaneXRGB8888(),pDst,					\
			fn<CFastPlaneXRGB8888>(rgb),pInfo);			\
				break;									\
		}												\
	case 8: {											\
		CFastPlaneXBGR8888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneXBGR8888(),pSrc,					\
			CFastPlaneXBGR8888(),pDst,					\
			fn<CFastPlaneXBGR8888>(rgb),pInfo);			\
				break;									\
		}												\
	case 10: {											\
		CFastPlaneARGB4565 rgb;							\
		rgb.SetRGB(dw);									\
		rgb.SetA(0);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneARGB4565(),pSrc,					\
			CFastPlaneARGB4565(),pDst,					\
			fn<CFastPlaneARGB4565>(rgb),pInfo);			\
				break;									\
		}												\
	case 11: {											\
		CFastPlaneARGB4555 rgb;							\
		rgb.SetRGB(dw);									\
		rgb.SetA(0);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneARGB4555(),pSrc,					\
			CFastPlaneARGB4555(),pDst,					\
			fn<CFastPlaneARGB4555>(rgb),pInfo);			\
				break;									\
		}												\
	case 12: {											\
		CFastPlaneARGB8888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneARGB8888(),pSrc,					\
			CFastPlaneARGB8888(),pDst,					\
			fn<CFastPlaneARGB8888>(rgb),pInfo);			\
				break;									\
		}												\
	case 13: {											\
		CFastPlaneABGR8888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Blt(							\
			CFastPlaneABGR8888(),pSrc,					\
			CFastPlaneABGR8888(),pDst,					\
			fn<CFastPlaneABGR8888>(rgb),pInfo);			\
				break;									\
		}												\
	}													\
														\
	return 0;											\
														\
	}	//	the end of function BltHelper2

/** この関数の引数はBlt2とコンパチ
	ただし、fnは、dwを食わせてから実体化

		CSurfaceInfo* pDst	:	エフェクトを施す対象
		RECT*		lpRect	:	エフェクトを施す範囲
		template<FN>	Fn	:	エフェクター
		DWORD			dw	:	エフェクターのためのパラメータ
*/
#define CSurfaceBltter_EffectHelper1(pDst,lpRect,fn,dw){			\
	if (pDst->Lock()!=0) return 1;						\
	CSurfaceLockerGuard g(pDst);						\
	int nS1 = pDst->GetSurfaceType();					\
	switch( nS1 ) {										\
	case 3: {											\
		CFastPlaneRGB565 rgb;							\
		rgb.SetRGB((WORD)dw);							\
		CFastPlaneXRGB8888 rgb2;						\
		rgb2.SetRGB(rgb.GetRGB_DWORD());				\
		CFastPlaneEffect::EffectDouble(					\
			CFastPlaneRGB565(),pDst,					\
			CFastPlaneXRGB8888(),						\
			fn<CFastPlaneRGB565>(rgb),					\
			fn<CFastPlaneXRGB8888>(rgb2),				\
			lpRect); } break;							\
	case 4: {											\
		CFastPlaneRGB555 rgb;							\
		CFastPlaneXRGB8888 rgb2;						\
		rgb.SetRGB((WORD)dw);							\
		rgb2.SetRGB(rgb.GetRGB_DWORD());				\
		CFastPlaneEffect::EffectDouble(					\
			CFastPlaneRGB555(),pDst,					\
			CFastPlaneXRGB8888(),						\
			fn<CFastPlaneRGB555>(rgb),					\
			fn<CFastPlaneXRGB8888>(rgb2),				\
			lpRect); } break;							\
	case 5: {											\
		CFastPlaneRGB888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneRGB888(),pDst,					\
			fn<CFastPlaneRGB888>(rgb),					\
			lpRect); } break;							\
	case 6: {											\
		CFastPlaneBGR888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneBGR888(),pDst,					\
			fn<CFastPlaneBGR888>(rgb),					\
			lpRect); } break;							\
	case 7: {											\
		CFastPlaneXRGB8888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneXRGB8888(),pDst,					\
			fn<CFastPlaneXRGB8888>(rgb),				\
			lpRect); } break;							\
	case 8: {											\
		CFastPlaneXBGR8888 rgb;							\
		rgb.SetRGB(dw);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneXBGR8888(),pDst,					\
			fn<CFastPlaneXBGR8888>(rgb),				\
			lpRect); } break;							\
	case 10: {											\
		CFastPlaneARGB4565 rgb;							\
		rgb.SetRGB(dw);									\
		rgb.SetA(0);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneARGB4565(),pDst,					\
			fn<CFastPlaneARGB4565>(rgb),				\
			lpRect); } break;							\
	case 11: {											\
		CFastPlaneARGB4555 rgb;							\
		rgb.SetRGB(dw);									\
		rgb.SetA(0);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneARGB4555(),pDst,					\
			fn<CFastPlaneARGB4555>(rgb),				\
			lpRect); } break;							\
	case 12: {											\
		CFastPlaneARGB8888 rgb;							\
		rgb.SetRGB(dw);									\
		rgb.SetA(0);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneARGB8888(),pDst,					\
			fn<CFastPlaneARGB8888>(rgb),				\
			lpRect); } break;							\
	case 13: {											\
		CFastPlaneABGR8888 rgb;							\
		rgb.SetRGB(dw);									\
		rgb.SetA(0);									\
		CFastPlaneEffect::Effect(						\
			CFastPlaneABGR8888(),pDst,					\
			fn<CFastPlaneABGR8888>(rgb),				\
			lpRect); } break;							\
	}													\
	return 0;											\
	}	//	end of CSurfaceBltter_EffectHelper1

};

#endif
