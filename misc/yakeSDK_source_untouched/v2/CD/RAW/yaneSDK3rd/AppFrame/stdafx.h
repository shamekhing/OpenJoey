
#ifndef __STDAFX_H__
#define __STDAFX_H__

//	最初に設定を読み込んどこう＾＾；
#include "../config/yaneConfig.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// DirectSoundが使いよる。自分の使うヘッダーぐらい自分でincludeせーよ...
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

//	mpeg用の構造体があるのでプラットフォームSDKから最新のと差し替えてね！
#include <mmreg.h>

//	DirectX関係のGUIDの設定のため
#pragma comment(lib,"dxguid.lib")

// ＮＴで動作させるので、バージョンはDirectX3だ！！
#define DIRECTDRAW_VERSION	0x0300
#define DIRECTSOUND_VERSION 0x0300
//	↑DirectX7からDSBUFFERDESC構造体のサイズが大きくなっているのでその対策

#define DIRECTINPUT_VERSION 0x0300
#include <dinput.h>			// つかうんならヘッダー読んどくしー
#pragma comment(lib,"dinput.lib")
//	リンクしたくないけど、dinput.libのグローバル変数がキーデバイス取得に
//	必要なので、仕方がない。なんでこんな設計になってるんかなー。BCC5.5ならば外して。

#include <ddraw.h>			// DirectDraw header
#include <dsound.h>			// DirectSound header
#include <dmusicc.h>		// DirectMusic header(directX6以降のSDKが必要にょ)
#include <dmusici.h>

// The debugger can't handle symbols more than 255 characters long.
// STL often creates symbols longer than that.
// When symbols are longer than 255 characters, the warning is disabled.
#pragma warning(disable:4786)

// STL関連
#include <set>
#include <list>
#include <vector>
#include <string>
#include <stack>
#include <map>
using namespace std;
//	usingをする奴は死刑？？
//	ご、、ごめん(;´Д`)

#include <stdlib.h>		// _MAX_PATH,qsort,rand..

//	あったら便利かな～＾＾

#include "../YTL/index.h"		//	Yanurao Template Library
#include "../Thread/yaneThreadLocal.h"		//	ThreadLocal template

//	Err出力に必要
#include "../Auxiliary/yaneStream.h"

#endif
