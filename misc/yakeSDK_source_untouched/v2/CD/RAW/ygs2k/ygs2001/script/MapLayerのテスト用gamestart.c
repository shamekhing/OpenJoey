void main(){
	int x,y;	//	マップの表示位置。ワールド座標で指定する

	//	マップレイヤーを設置
	MapLayerOn();

	//	マップの読み込み
	MapLoad("test.map");

//	MapLoad後、マップ全体のドット数を取得できる
//	MapGetSize(&x,&y);
//	Dbg("%d,%d",x,y);

//	画面に部分描画するのは、まだデバッグ中
//	MapSetView(100,100,300,300);
//	↑この引数は(left,top,right,bottom)

	//	マップの表示箇所は(0,0)から
	x = 0; y = 0;

	loop {
		//	マップの描画
		MapOnDraw(x,y,0);

		//	斜めにスクロールしてみる
		x++; y=y+2;

		halt;
	}

	//	マップレイヤーを除去
	MapLayerOff();
}
