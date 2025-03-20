#include "stdafx.h"
#include "CApp.h"
#include <queue>

class CTaskControlBase {
/**
	タスク制御のための基底クラス
*/
public:
	void	setPriority(int nPriority) { nPriority_ = nPriority;}
	int		getPriority() { return nPriority_; }
	/*
		プライオリティ（タスクの優先度）の設定／取得
		プライオリティの設定自体は、class CTaskController の側で行なうので
		この派生クラスが行なう必要はない。
	*/

	virtual void Task(void*) = 0;
	///	呼び出されるべきタスク。これを派生クラスでオーバーライドする
	///	この引数には、CTaskController::callTaskで渡したものがそのまま入ってくる

protected:
	int		nPriority_;
};

class CTaskController {
/**
	タスクのコントローラー
	CTaskControlBase*のlistを持っていて管理する
*/
public:
	typedef list<CTaskControlBase*>	tasklist;
	tasklist* getTaskList() { return& tasklist_; }

	///	すべてのタスクを呼び出す
	void	callTask(void*p){
		tasklist::iterator it = getTaskList()->begin();
		bKillTask_ = false;
		while (it!=getTaskList()->end()){
			(*it)->Task(p);
			if (bKillTask_) {
				//	このタスクは自爆しよった
				bKillTask_ = false;
				delete *it;
				it = getTaskList()->erase(it);
			} else {
				it++;
			}
		}
	}

	/**
		生成したタスクをタスクリストに登録する
		new したCTaskControlBase派生クラスを渡してチョ
		プライオリティは、タスクの優先度。0だと一番優先順位が高い。
		万が一に備えてマイナスの値を渡しても良いようにはなっている。
	*/
	void	addTask(CTaskControlBase*p,int nPriority)
	{
		p->setPriority(nPriority);
		tasklist::iterator it = getTaskList()->begin();
		while (it!=getTaskList()->end()){
			if ((*it)->getPriority() > nPriority) break;
			it++;
		}
		getTaskList()->insert(it,p);
	}

	/**
		優先度を指定して、そのタスクを消滅させる
		(ただし、自分で自分のタスクを削除することは出来ない)
	*/
	void	killTask(int nPriority){
		tasklist::iterator it = getTaskList()->begin();
		while (it!=getTaskList()->end()){
			int nMyPriority = (*it)->getPriority();
			if (nMyPriority > nPriority) break;
			// priorityに関して整順を仮定できるので超えていればそこでおしまい
			if (nMyPriority == nPriority) {
				delete *it;
				it = getTaskList()->erase(it);
			}
		}
	}

	/**
		優先度を指定してタスクを一括削除する
		このメソッドを呼び出すとき、自分のタスクが含まれてはいけない
	*/
	void	killTask(int nStartPriority,int nEndPriority){
		tasklist::iterator it = getTaskList()->begin();
		while (it!=getTaskList()->end()
			&& ((*it)->getPriority()<nStartPriority)) ++it;
		while (it!=getTaskList()->end()
			&& ((*it)->getPriority()<nEndPriority)) {
			delete *it;
			it = getTaskList()->erase(it);
		}
	}

	/**
		callTaskで呼び出されているタスクが
		自分自身を削除するためのメソッド
	*/
	void	killMe() { bKillTask_ = true; }

protected:
	tasklist tasklist_;
	bool	bKillTask_;
};

//	↑↑↑ここまでは汎用的なゲームタスクとゲームタスクコントローラー
//	適当にぶっこ抜いて使ってチョ
/////////////////////////////////////////////////////////////////
//	↓↓↓は、このゲーム専用のゲームタスクコントローラーとinfo

/**
	タスク優先順位としてはタスクごとに異なる値を用意する
*/
namespace taskPriority {
	enum TaskPriority__ {
		background,
		scene,
		normal,
		effect,
	};
}

struct CGameTaskInfo {
	CApp* getApp() { return pApp;}
	void  setApp(CApp*p){ pApp = p; }

	//	描画するコンテキストを取得
	CFastPlane* getDrawContext()
	{ return getApp()->GetDraw()->GetSecondary(); }

	CTaskController* getTaskController() { return pTaskController;}
	void setTaskController(CTaskController*tc) { pTaskController=tc;}

	bool bMove;	//	移動するのか
	bool bDraw;	//	描画するのか

protected:
	CTaskController* pTaskController;
	CApp*		pApp;	//	自分のアプリクラス
};

//	ユーザー定義タスク基底クラス
class CMyTask : public CTaskControlBase {
public:
	virtual void Task(void*p){
		CGameTaskInfo* pInfo = (CGameTaskInfo*)p;
		if (pInfo->bMove) OnMove(pInfo);
		if (pInfo->bDraw) OnDraw(pInfo);
	}
	virtual void OnMove(CGameTaskInfo* p){}
	virtual void OnDraw(CGameTaskInfo* p){}
	/*
		ユーザーはこのクラスを派生させ、
		OnMoveとOnDrawをオーバーライドして使う
	*/

	//	自分自身のタスクを消す(タスクコントローラーの力を借りて)
	virtual void killMe(CGameTaskInfo* p){
		p->getTaskController()->killMe();
	}

	//	ほか、タスクコントローラーへのアクセス等は
	//	このクラスで適当にwrapすると良い
};



///////////////////////////////////////////////////////////////////////////
/// ここから

namespace {
	///	無名ネームスペース
	///	この空間へは、他のファイルからはアクセスできない。
	int sgn(int a){
		///	富豪・・・もとい符号を返す
		if(0<a)return 1;
		if(a<0)return -1;
		return 0;
	}
}
struct CFadeInTask : public CMyTask {
	/**
		フェードイン・タスク。
		これを作れば、そのあとnターンかけてフェードインしてくれる。
		真っ暗な状態から徐々に明るくなっていく。
	*/
	CFadeInTask(int n) : n_(-1), n2_(n) {}
	virtual void OnMove(CGameTaskInfo* p){
		n_++;
		if (n2_<=n_){
			killMe(p);
		}
	}
	virtual void OnDraw(CGameTaskInfo* p){
		p->getDrawContext()->FadeEffect((255*(n_*2+1))/(n2_*2));
	}
private:
	int n_, n2_;
};
struct CFadeOutTask : public CMyTask {
	/**
		フェードイン・タスク。
		これを作れば、そのあとnターンかけてフェードアウトしてくれる。
		真っ暗な状態から徐々に明るくなっていく。
	*/
	CFadeOutTask(int n) : n_(-1), n2_(n) {}
	virtual void OnMove(CGameTaskInfo* p){
		n_++;
		if (n2_<=n_){
			killMe(p);
		}
	}
	virtual void OnDraw(CGameTaskInfo* p){
		p->getDrawContext()->FadeEffect((255*((n2_-n_)*2-1))/(n2_*2));
	}
private:
	int n_, n2_;
};

///	↓ロゴを表示するシーンタスク。
///		テンプレート仕様で、次のタスクを指定できる。
template<class NextTask>
struct CLogoTask : public CMyTask {
	CLogoTask()	: n_(-1)
	{
		pl_.SetReadDir("image\\");
		pl_.Set("image\\logo.txt");
		pl_.SetColorKeyPos(0, 0);
	}
	virtual void OnDraw(CGameTaskInfo* p){
		p->getDrawContext()->Blt(pl_.GetPlane(0), 320, 240, NULL, NULL, NULL, 4);	///表示。
	}
	virtual void OnMove(CGameTaskInfo* p){
		n_++;	/// タスクカウンタを動かす
		if(n_==0){
			p->getTaskController()->addTask(new CFadeInTask(15),taskPriority::effect);
		}else if(n_==35){
			p->getTaskController()->addTask(new CFadeOutTask(15),taskPriority::effect);
		}else if(n_==50){
			killMe(p);
			p->getTaskController()->addTask(new NextTask,taskPriority::scene);
		}
	}
private:
	int n_;
	CPlaneLoader pl_;
};


///	↓ボードオブジェクト。
///		リバーシゲーム画面の状態の保存および画面描画を担当する。
struct CBoardObj{
private:
	enum{
		sx_=18-221,
		sy_=15-225,
		dx_=50,
		dy_=50,
	};
public:
	bool check_put(int x, int y){
		if(data_[x][y]!=0)return false;
		for(int ax=-1;ax<2;ax++)for(int ay=-1;ay<2;ay++){
			if(ax==0 && ay==0)continue;
			int mx=x+ax;
			int my=y+ay;
			int mm=0;

			while(0<=mx && mx<8 && 0<=my && my<8 && sgn(data_[mx][my])==sgn(-turn_)){
				mm++;
				mx+=ax;
				my+=ay;
			}
			if(mx<0 || mx>=8 || my<0 || my>=8 || data_[mx][my]==0)continue;
			if(0<mm){return true;}
		}
		return false;
	}
	void que_add(int x, int y){
		if(que_p_.size()<120){
			que_p_.push(x);
			que_p_.push(y);
		}
	}
	bool que_empty(){
		return que_p_.empty();
	}
	///アクセッサ
	bool GetErrFlag(){	return flag_err_;	}
	void SetErrFlag(bool f){	flag_err_=f;	}
	bool GetEndFlag(){	return flag_end_;	}
	void SetEndFlag(bool f){	flag_end_=f;	}
	bool GetWaitFlag(){	return flag_wait_;	}
	void SetWaitFlag(bool f){	flag_wait_=f;	}
	int GetTurn(){	return turn_;	}
	int GetSubBW(){	return dif_;	}
	int GetX(){return x_;}
	int GetY(){return y_;}
	void SetX(int x){x_=x;}
	void SetY(int y){y_=y;}

	void OnMoveBoard(){
		{	///アニメーション用演算
			bool flag=false;
			for(int yy=0;yy<8;yy++)for(int xx=0;xx<8;xx++){
				char c=data_[xx][yy];
				if(1<c){c--;flag=true;}
				else if(c<-1){c++;flag=true;}
				data_[xx][yy]=c;
			}
			flag_wait_=!flag;
		}
		if(flag_wait_ && !flag_err_ && (!que_p_.empty()))
		{	///アニメなしの場合、次の手を読み込む
			int x=que_p_.front();		que_p_.pop();
			int y=que_p_.front();		que_p_.pop();
			put(x,y);
			flag_wait_=false;
		}
	}
	void OnDrawBoard(ISurface * surface, int x=0, int y=0){
		int ssx=sx_+x, ssy=sy_+y;
		surface->BltFast(pl_rev_.GetPlane(0), x-221, y-225);
		for(int yy=0;yy<8;yy++)for(int xx=0;xx<8;xx++){
			char c=data_[xx][yy];
			if(c!=0){
				char cc=c;
				if(cc<0){cc=-cc;}
				cc--;
				if(19<=cc){cc=18;}
				if(0<c){
					surface->Blt(pl_b_.GetPlane(cc), ssx+xx*dx_, ssy+yy*dy_);
				}
				else{
					surface->Blt(pl_w_.GetPlane(cc), ssx+xx*dx_, ssy+yy*dy_);
				}
			}
		}
		surface->Blt(pl_rev_.GetPlane(1), ssx+3+x_*dx_, ssy+10+y_*dy_);
	}

	///コンストラクタ
	CBoardObj()
		: turn_(1), dif_(0)
		, x_(4), y_(4)
		, flag_err_(false), flag_end_(false), flag_wait_(false)
	{
		{//データの初期化
			int x, y;
			for(y=0;y<8;y++)for(x=0;x<8;x++){
				data_[x][y]=0;
			}
			data_[3][3]=1;
			data_[4][3]=-1;
			data_[3][4]=-1;
			data_[4][4]=1;
		}
		{//画像の読み込み
			pl_rev_.SetReadDir("image\\");
			pl_rev_.Set("image\\rev.txt");
			pl_rev_.SetColorKeyPos(0, 0);
			pl_b_.SetReadDir("image\\");
			pl_b_.Set("image\\black.txt");
			pl_b_.SetColorKeyPos(0, 0);
			pl_w_.SetReadDir("image\\");
			pl_w_.Set("image\\white.txt");
			pl_w_.SetColorKeyPos(0, 0);
		}
	}
protected:
	void put(int x, int y){
		if(x<0 || x>=8 || y<0 || y>=8)return;
		if(data_[x][y]!=0){flag_err_=true;return;}

		int rev=0;
		for(int ax=-1;ax<2;ax++)for(int ay=-1;ay<2;ay++){
			if(ax==0 && ay==0)continue;
			/// 9-1の、8方向を調べるでー
			int mx=x+ax;
			int my=y+ay;
			int mm=0;

			while(mx>=0 && mx<8 && my>=0 && my<8 && sgn(data_[mx][my])==sgn(-turn_)){
				mm++;
				mx+=ax;
				my+=ay;
			}
			if(mx<0 || mx>=8 || my<0 || my>=8 || sgn(data_[mx][my])==0)continue;
			if(0<mm){rev+=mm;}
			mx-=ax;	my-=ay;
			while(sgn(data_[mx][my])==sgn(-turn_)){
				mm--;
				data_[mx][my]=turn_*(mm*3+19);
				mx-=ax;	my-=ay;
			}
		}
		if(rev<=0){flag_err_=true;return;}
		dif_+=rev*turn_*2;

		data_[x][y]=turn_;
		dif_+=turn_;
		turn_=-turn_;
		x_=x;
		y_=y;
		check_turn();
	}
	void check_turn(bool flag=false){
		for(int x=0;x<8;x++)for(int y=0;y<8;y++){
			if(check_put(x,y))return;
		}
		turn_=-turn_;
		if(flag){
			flag_end_=true;
		} else {
			check_turn(true);
		}
	}
private:
	std::queue<int> que_p_;
	CPlaneLoader pl_rev_;
	CPlaneLoader pl_b_;
	CPlaneLoader pl_w_;
	int data_[8][8];		///データ
	int turn_;				///ターン	
	int dif_;				///点差
	int x_, y_;				///最後の座標
	bool flag_err_;			///ゲームエラーフラグ
	bool flag_end_;			///ゲーム終了フラグ
	bool flag_wait_;		///アニメーションストップ
};

///	↓コンピュータが考えるタスク
///		サンプルプログラムにつき、本格的な思考ルーチンはつんでいない。
///		単純に、ランダムに置ける場所を見つけて置くだけのプログラムである。
struct CCpuTask : public CMyTask {
	CCpuTask(const smart_ptr<CBoardObj> & obj, int turn) : obj_(obj), turn_(turn) {
		rand_.Randomize();
	}
	virtual void OnMove(CGameTaskInfo* p){
		if(obj_->GetEndFlag()){
			///ゲームが終わったなら終了
			killMe(p);
		} else if((!(obj_->GetErrFlag())) && obj_->GetWaitFlag() && obj_->GetTurn()==turn_){
			int i;
			for(i=0;i<10;++i){
				int x=rand_.Get(8), y=rand_.Get(8);
				if(obj_->check_put(x, y)){
					obj_->que_add(x, y);
					break;
				}
			}
		}
	}
private:
	smart_ptr<CBoardObj> obj_;
	int turn_;
	CRand rand_;
};

///	↓ユーザーに操作を行わせるタスク
struct CPlayerTask : public CMyTask {
	CPlayerTask(const smart_ptr<CBoardObj> & obj, int turn) : obj_(obj), turn_(turn) {}
	virtual void OnMove(CGameTaskInfo* p){
		if(obj_->GetEndFlag()){
			///ゲームが終わったなら終了
			killMe(p);
		} else if((!(obj_->GetErrFlag())) && obj_->GetWaitFlag() && obj_->GetTurn()==turn_){
			if(p->getApp()->GetKey()->IsKeyPush(0)){
				///ESC
				killMe(p);
				p->getTaskController()->addTask(new CCpuTask(obj_, turn_),taskPriority::normal);
			}
			if(p->getApp()->GetKey()->IsKeyPush(1)){
				///↑
				obj_->SetY((obj_->GetY()+7)%8);
			}
			if(p->getApp()->GetKey()->IsKeyPush(2)){
				///↓
				obj_->SetY((obj_->GetY()+1)%8);
			}
			if(p->getApp()->GetKey()->IsKeyPush(3)){
				///←
				obj_->SetX((obj_->GetX()+7)%8);
			}
			if(p->getApp()->GetKey()->IsKeyPush(4)){
				///→
				obj_->SetX((obj_->GetX()+1)%8);
			}
			if(p->getApp()->GetKey()->IsKeyPush(5) || p->getApp()->GetKey()->IsKeyPush(6)){
				//Space or Enter
				obj_->que_add(obj_->GetX(), obj_->GetY());
			}
		}
	}
private:
	smart_ptr<CBoardObj> obj_;
	int turn_;
};

///	↓リバーシ・ゲーム画面のシーンタスク
///	テンプレート仕様で、１つ目のクラスは次に呼び出すタスク、
///						２つ目のクラスは操作タスク１
///						３つ目のクラスは操作タスク２
template <class NextTask, class PlayerTask1, class PlayerTask2>
struct CBoardTask : public CMyTask {
	CBoardTask()
		: n_(-1)
	{
		plane_.SetSize(442, 450);
	}
	virtual void OnDraw(CGameTaskInfo* p){
		if(15<=n_){
			obj_->OnDrawBoard(p->getDrawContext(), 320, 240);	///普通に表示。
		} else if(0<=n_){
			obj_->OnDrawBoard(&plane_, 221, 225);	///普通に表示。
			p->getDrawContext()->RotateBltFast(&plane_, 99, 15, ((15-n_)*2-1)*256/30, (n_+1)<<12, 4);
		}
	}
	virtual void OnMove(CGameTaskInfo* p){
		if(n_==-1){
			n_++;
			///初期化
			obj_=smart_ptr<CBoardObj>(new CBoardObj);
			p->getTaskController()->addTask(new PlayerTask1(obj_, 1),taskPriority::normal);
			p->getTaskController()->addTask(new PlayerTask2(obj_, -1),taskPriority::normal);
			p->getTaskController()->addTask(new CFadeInTask(15),taskPriority::effect);
		} else if(n_<=15){
			if(n_!=15)n_++;
			else if(obj_->GetErrFlag())obj_->SetErrFlag(false);
			else if(obj_->GetEndFlag())n_++;
			obj_->OnMoveBoard();
		} else if(n_==25){
			obj_->OnMoveBoard();
			n_++;
			p->getTaskController()->addTask(new CFadeOutTask(15),taskPriority::effect);
		} else if(n_==40){
			obj_->OnMoveBoard();
			killMe(p);
			p->getTaskController()->addTask(new NextTask,taskPriority::scene);
		} else {
			obj_->OnMoveBoard();
			n_++;
		}
	}
private:
	int n_;
	smart_ptr<CBoardObj> obj_;
	CFastPlane plane_;
};

struct CFirstTask;
///	↓モード選択画面のシーンタスク
struct CSelectTask : public CMyTask {
	CSelectTask() : n_(-1), y_(0) {
		pl_.SetReadDir("image\\");
		pl_.Set("image\\select.txt");
		pl_.SetColorKeyPos(0, 0);
	}
	virtual void OnDraw(CGameTaskInfo* p){
		p->getDrawContext()->BltFast(pl_.GetPlane(0), 320, 240, NULL, NULL, NULL, 4);
		p->getDrawContext()->BltFast(pl_.GetPlane(1), 200, 240+48*(y_-1), NULL, NULL, NULL, 4);
	}
	virtual void OnMove(CGameTaskInfo* p){
		n_++;
		if(n_==0){
			///フェードイン
			p->getTaskController()->addTask(new CFadeInTask(15),taskPriority::effect);
		}
		if(n_==285){
			///フェードアウト
			p->getTaskController()->addTask(new CFadeOutTask(15),taskPriority::effect);
		}
		if(p->getApp()->GetKey()->IsKeyPush(1)){
			///↑
			y_=(y_+2)%3;
		}
		if(p->getApp()->GetKey()->IsKeyPush(2)){
			///↓
			y_=(y_+1)%3;
		}
		if(p->getApp()->GetKey()->IsKeyPush(5) || p->getApp()->GetKey()->IsKeyPush(6) || 300<=n_){
			///Space or Enter
			killMe(p);
			if(y_==0){
				p->getTaskController()->addTask(new CBoardTask<CFirstTask, CCpuTask, CCpuTask>, taskPriority::scene);
			} else if(y_==1){
				p->getTaskController()->addTask(new CBoardTask<CFirstTask, CPlayerTask, CCpuTask>, taskPriority::scene);
			} else {
				p->getTaskController()->addTask(new CBoardTask<CFirstTask, CPlayerTask, CPlayerTask>, taskPriority::scene);
			}
		}
	}
private:
	int n_, y_;
	CPlaneLoader pl_;
};

///	↓最初のシーンタスク。
struct CFirstTask : public CMyTask {
	virtual void OnMove(CGameTaskInfo* p){
		killMe(p);
		p->getTaskController()->addTask(new CLogoTask<CSelectTask>, taskPriority::scene);
	}
};

//////////////////////////////////////////////////////////////////

void	CApp::MainThread() {				 //	 これが実行される
	GetDraw()->SetDisplay(false, 640, 480);

	CPlaneFactoryFastPlane factory( GetDraw() );
	CPlane::SetFactory( smart_ptr< IPlaneFactory>( &factory, false ) );

	CTaskController tc;
	CGameTaskInfo info;
	CFPSTimer timer;
	timer.SetFPS(30);

	info.setApp(this);
	info.setTaskController(&tc);
	tc.addTask(new CFirstTask,taskPriority::scene);
	///	↑第１タスクを起動
	while (IsThreadValid()){
		GetKey()->Input();

		info.bMove = true;
		info.bDraw = false;
		tc.callTask(&info);
		///	↑これでタスクを呼び出す)（演算）
		if(!timer.ToBeSkip()){
			GetDraw()->GetSecondary()->Clear();
			info.bMove = false;
			info.bDraw = true;
			tc.callTask(&info);
			GetDraw()->OnDraw();
			///	↑これでタスクを呼び出す)（描画）
		}
		timer.WaitFrame();
	}
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
	CAppInitializer init(hInstance,hPrevInstance,lpCmdLine,nCmdShow);	//	必ず書いてね
	CSingleApp sapp;
	if (sapp.IsValid()) {
		CThreadManager::CreateThread(new CAppMainWindow());					//	上で定義したメインのウィンドゥを作成
	}
	return 0;
}
