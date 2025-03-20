
/*

	yaneSDK configulation

	使わないものについてはここでdefineをコメントアウト
	すればコンパイルされない。

*/

#ifndef __yaneConfig_h__
#define __yaneConfig_h__

// -=-=-=-=-=-=-=-=- 使わない機能の排除 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// --- メモリマネージメント -----------------------------------------------

//#define USE_MEMORY_CHECK		//	new,deleteごとにログ記録を残すか？
//#define USE_MEMORY_STATE		//	メモリのデバッグクラス(CMemoryState)を利用するか
								//	（このクラスは、高速なnew/deleteの役目も果す）
								//	↑このクラスは作りかけ。使わないでね！！

#define USE_EXCEPTION			//	例外処理を使う

//	↑もし、上記オプションで例外を使用するとして、
#ifdef _DEBUG
  #define USE_STOP_EXCEPTION
	//	YTL/exceptions.hで定義されている
	//	例外が発生したときにダイアログを表示して
	//	メモリエラーで停止するための措置(デバッグ時のみ)
#endif

// --- MIDI出力系  --------------------------------------------------------
//

// --- ストリーム再生  ----------------------------------------------------

#define USE_StreamSound		//	CStreamSoundを使うか

// --- 描画	 --------------------------------------------------------------

//	#define USE_Direct3D		//	CDirect3DPlane,CDirect3DDrawを使うか

#define USE_FastDraw		//	CFastPlane,CFastDrawを使うか
#define USE_DIB32			//	←これはFastPlaneの実装のために仮に利用している
							//	のちのバージョンでは消す予定
#define USE_YGA				//	YGAを使うか

// --- JoyStick関連 -------------------------------------------------------

#define USE_JOYSTICK		///	JoyStickを使用するのか

// --- ムービー再生系 -----------------------------------------------------

#define USE_MovieDS			// DirectShowを使う（推奨） 
							// DirectShowに対応したフォーマットで有ればだいたい再生出来る
							// ただしDirectX6.1以上が入っている必要がある
							// DirectShowが入っていない場合、下のUSE_MovieAVIが定義されていれば
							// そちらで再生を試みるようになっている
#define USE_MovieAVI		// AVIStream関数を用いる。ただしAVIファイルしか再生できない。

// --- スクリーンセーバー系 ----------------------------------------------------
//#define USE_SAVER

// --- CErrorLog出力系 ----------------------------------------------------

#define USE_ErrorLog		//	CErrorLogクラスを有効にする。
//	これをdefineしなければ、CErrorLogクラスは空のクラスになり、デバッグ用のエラー
//	文字列等はVC++の最適化によって消滅する。

#endif
