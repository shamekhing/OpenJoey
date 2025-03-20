
//	描画

#include "yaneDirtyRect.h"			//	for Dirty Rect Management
#include "yaneDIBitmap.h"			//	simple DIBSection wrapper
#include "yaneSurface.h"			//	for Surface Base
//	GTLはでかいのでincludeちましぇん

#include "yaneDirectDraw.h"			//	for DirectDraw(COM wrapper)
#include "yaneFastDraw.h"			//	for FastDraw(FastPlaneControler)
#include "yaneFastPlane.h"			//	for FastPlane(ISurface derived class)

#include "yaneScene.h"				//	for	Scene Control

///	↓これは、CFastPlaneの実装のために一時的に利用しているだけ
///		のちのバージョンでは消す予定
#include "yaneDIB32.h"
