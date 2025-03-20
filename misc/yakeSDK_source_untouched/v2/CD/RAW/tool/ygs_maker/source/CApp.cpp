#include "stdafx.h"
#include "CApp.h"

void	CApp::MainThread(void) {				 //	 これが実行される
	GetDraw()->SetDisplay(false);				//	Windowモード

	CFileDialog dlg;
	CDIB32 plane[3];
	int sx[2],sy[2];

	CHAR file[2][MAX_PATH];
	CMsgDlg msg;

	CFPSTimer t;
	t.SetFPS(30);

retry:;
	msg.Out("画像選択","黒画像→白画像の順番で選んでください");
	for(int i=0;i<2;i++){
		while (true){
			if (dlg.GetOpenFile(file[i])!=0) return ;
			if (!IsThreadValid()) return ;
			if (plane[i].Load(file[i])==0) break;
		}
		if (plane[i].IsYGA()) {
			//	YGA画像なのでそれを表示する
			for (int j=0;j<256;j+=8){
				if (!IsThreadValid()) return ;
				GetDraw()->Clear(DIB32RGB(j,j,j));
				GetDraw()->BltNatural(&plane[i],0,0);
				GetDraw()->OnDraw();
				t.WaitFrame();
			}
			goto retry;
		}
		plane[i].GetSize(sx[i],sy[i]);
		GetDraw()->Clear();
		GetDraw()->BltFast(&plane[i],0,0);
		GetDraw()->OnDraw();
	}
	if (sx[0]!=sx[1] || sy[0]!=sy[1]) {
		CMsgDlg msg;
		msg.Out("エラー！","２つの画像サイズが異なります");
		goto retry;
	}

	plane[2].CreateSurface(sx[0],sy[0]);
	DWORD nSize = sx[0]*sy[0]*4;
	DWORD* p[2];
	DWORD* pd;
	p[0] = plane[0].GetPtr(); p[1] = plane[1].GetPtr();
	pd	 = plane[2].GetPtr();

	for(int y=0;y<sy[0];y++){
		for(int x=0;x<sx[0];x++){
			DWORD r,g,b,alpha;
			b=*p[0] & 0xff;g=(*p[0]>>8) & 0xff;r=(*p[0]>>16) & 0xff;
			DWORD d = 255*3 + (b+g+r) - ((*p[1] & 0xff) + ((*p[1]>>8) & 0xff) + ((*p[1]>>16) & 0xff));

			if(d==0){
				*(pd++) = 0;
			}else{
				b = b * (255*3)/d;
				g = g * (255*3)/d;
				r = r * (255*3)/d;

//				alpha = 255 * d/(255*3);

			// 実際にαチャンネル付転送するときには、
			// dest += ((src-dest)*alpha)>>8;
			// とされていることを意識して、
				{
					DWORD tmp = (256*d)/(255*3);
					if (tmp >= 0xFF) alpha = 0xFF;	// 0xFFを超える危険性がある
					else alpha = tmp;
				}
				*(pd++) = (r<<16) + (g<<8) + b + (alpha<<24);
			}
			p[0]++; p[1]++;
		}
	}

	/*
	for(int y=0;y<sy[0];y++){
		for(int x=0;x<sx[0];x++){
			DWORD dw;
			dw = *p[0] & 0xffffff;		//	最上位バイトをマスク

			int alpha;
			alpha = 255 - ((*p[1]&0x000000ff) - (*p[0]&0x000000ff));
			if ( alpha > 255 ) {
				msg.Out("Warning","Alpha が 255を越えました\n画像に問題があります");
				goto retry;
			}

			alpha = 255 - ((*p[1]&0xff) - (*p[0]&0xff));
			dw += ((DWORD)alpha) << 24;	//	最上位バイトに持ってくる
			*(pd++) = dw;
			p[0]++; p[1]++;
	}
	*/

	CFile f;
	::strcat(file[0],".yga");
	/*	//	そのままデータを書き出す
		f.Write(file[0],(void*)(BYTE*)dst,nSize);
	*/

	plane[2].SaveYGA(file[0]);	//	YGAで保存

	msg.Out("正常終了","処理が終わりました");
	goto retry;
}

//	これがmain windowのためのクラス。
class CAppMainWindow : public CAppBase {	//	アプリケーションクラスから派生
	virtual void MainThread(void){			//	これがワーカースレッド
		CApp().Start();						//	CApp app; app.Start();の意味ね
	}
};

//	言わずと知れたWinMain
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	CAppInitializer::Init(hInstance,hPrevInstance,lpCmdLine,nCmdShow);	//	必ず書いてね
	CSingleApp sapp;
	if (sapp.IsValid()) {
		CAppMainWindow().Run();					//	上で定義したメインのウィンドゥを作成
	}
	return 0;
}
