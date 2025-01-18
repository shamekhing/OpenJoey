using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

using YD = Yanesdk.Draw;

namespace Sample1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        int x = 0;
        int y = 0;
        int dx = 1;
        int dy = 1;
        YD.Win32Window window;
        YD.GlTexture texture;

        private void Form1_Load(object sender, EventArgs e)
        {
            // 初期処理

            // 描画サイズは320x240
            ClientSize = new Size(320, 240);
            // PictureBoxをDockStyle.Fillで貼り付けている
            // PictureBox上に描画
            window = new YD.Win32Window(pictureBox1.Handle);
            // Texture作成時はScreen.Selectを呼ぶ
            window.Screen.Select();
            // ゴシック(0がゴシック、明朝は1)、32ポイントのフォントを読み込み
            YD.Font font = new YD.Font();
            font.Load(0, 32);
            // フォント色は赤
            font.SetColor(255, 0, 0);
            // 描画用のテクスチャを用意
            texture = new YD.GlTexture();
            // カラーキーは座標(0, 0)の色とする
            texture.SetColorKeyPos(0, 0);
            // Hello, Worldの文字をテクスチャに貼り付ける
            texture.SetSurface(font.DrawBlended("Hello, World"));
        }

        private void OnTick(object sender, EventArgs e)
        {
            // TimerクラスのTickイベント
            // 定期的に呼ばれる

            YD.Screen scr = window.Screen;
            // 描画はScreen.Select〜Updateで挟む
            scr.Select();

            // 画面を白でクリア
            scr.SetClearColor(255, 255, 255);
            scr.Clear();

            // アルファ転送ON
            scr.BlendSrcAlpha();
            // テクスチャを描画(Hello, World)
            scr.Blt(texture, x, y);

            // ウィンドウの端で反射
            x += dx;
            y += dy;

            if (x < 0 || x > pictureBox1.Width - texture.Width)
                dx = -dx;
            if (y < 0 || y > pictureBox1.Height - texture.Height)
                dy = -dy;

            // 描画終了
            scr.Update();
        }
    }
}