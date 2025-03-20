[ enraogg.dll - OggVorbis to PCM Decoder Library  version 1.0.0 ]

１．はじめに
　　このDLLファイルはyaneSDK3rdでOggVorbis形式の圧縮ファイルを再生するための
　　プラグインです。

２．使用方法
　　まずWinmain関数内で、DLLを登録する必要があります。
　　[例]
　　　CObjectCreater* pCreater = CObjectCreater::GetObj();
　　　// CAppBaseより外側で登録
　　　pCreater->LoadPlugIn("yaneSDK/plugin/enraogg.dll");
　　　CSingleApp sapp;
　　　if (sapp.IsValid()) {
　　　　　CAppMainWindow().Run();
　　　}
　　　// CAppBaseより外側で解放
　　　pCreater->ReleasePlugIn("yaneSDK/plugin/enraogg.dll");

　　CSoundParameterクラスのGetStreamFactory関数で、ISoundStreamFactory派生クラスを得ます。
　　そのクラスのGetPlugInMap関数で、拡張子"ogg"とクラス名"CVorbisStream"を登録します。
　　[例]
　　　CSoundFactory manage;
　　　manage.GetSoundParameter()->GetStreamFactory()->GetPlugInMap()->Write("ogg", "CVorbisStream");
　　　smart_ptr<ISound> bgm = manage.CreateSound();
　　　bgm->Open("hanya-n.ogg");
　　　bgm->Play();

３．使用上の注意
　　CAppFrame派生クラスに、ISound派生クラスをメンバとして持たせる場合、
　　WinMain関数内でPluginをロード・リリースしないと、不正解放によりアプリケーションが
　　異常終了します。

４．アンインストール
　　ファイルを手動で削除してください。

５．連絡先・著作権表示
　　このDLLは、libvorbis, vorbisfile を使用しています。
　　ソースをビルドする際はhttp://www.vorbis.comより、ライブラリを
　　別途ダウンロードする必要があります。