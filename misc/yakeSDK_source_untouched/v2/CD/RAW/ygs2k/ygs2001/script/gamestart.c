void main(){

	//	シナリオレイヤーを設置
	ScenarioLayerOn();

	//	シナリオの読み込み
	ScenarioLoad("grp/kari/scenario01.html");

	//	シナリオの早送りボタンを有効にする
	//	ScenarioEnableSkip(1);

	loop {
		//	キーとマウスの入力は、こちらで行なう
		KeyInput(); MouseInput();

		//	シナリオの描画
		if (ScenarioOnDraw()!=0) break;
		halt;
	}

	//	シナリオレイヤーを除去
	ScenarioLayerOff();
}
