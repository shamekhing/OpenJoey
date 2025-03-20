// yaneMacro.h :
//	簡単なマクロ集

#ifndef __yaneMacro__h__
#define __yaneMacro__h__

// ゼロで埋めるマクロ
#define ZERO(var) ZeroMemory(&var,sizeof(var));

// ポインタが非NULLならばReleaseを掛けるマクロ。
#define RELEASE_SAFE(var)	\
	if (var != NULL) {		\
		var->Release();		\
		var = NULL;			\
	}

// ポインタが非NULLならばdeleteを掛けるマクロ。
#define DELETE_SAFE(var)	\
	if (var != NULL) {		\
		delete var;			\
		var = NULL;			\
	}

// 配列のポインタをdeleteするマクロ
#define DELETEPTR_SAFE(var) \
	if (var != NULL) {		\
		delete [] var;		\
		var = NULL;			\
	}

//	else ifマクロ（邪道ぎみ=p）
#define ef else if

//	WARNINGマクロ
#ifdef _DEBUG
	#define WARNING(b,s)				\
		if(!(b)) {						\
			CHAR buf[256];				\
			wsprintf(buf,"%sファイルの%d行\n%s",__FILE__,__LINE__,s);	\
			MessageBox(NULL,buf,"WARNING!!",MB_OK);	\
			* LPLONG(0xcdcdcdcd) = 0;	\
		}
	//	最後の* LPLONG(0xcdcdcdcd) = 0;の部分は、メモリエラーをわざと起こすコード
#else
	#define WARNING(var,s) {}
	//	{}にしているのは、if文のあとWARNINGを入れているときなどのため。
#endif

#endif
