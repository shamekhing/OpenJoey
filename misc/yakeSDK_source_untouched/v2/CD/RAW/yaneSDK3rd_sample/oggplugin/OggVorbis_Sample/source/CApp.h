#ifndef __CApp_h__
#define __CApp_h__

class CApp : public CAppFrame {
public:
protected:
	//	こいつがメインね
	void MainThread(void);		  //  これが実行される
};

//	これがmain windowのためのクラス。
class CAppMainWindow : public CAppBase {	//	アプリケーションクラスから派生
	virtual void MainThread(void){			//	これがワーカースレッド
		CApp().Start();						//	CApp app; app.Start();の意味ね
	}
};


#endif
