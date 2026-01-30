//
// Effect-related blitting (ported from yaneSDK2nd by derplayer)
//

#ifndef __yanePlaneEffectBlt_h__
#define __yanePlaneEffectBlt_h__

// sqrt code was written for VC6. But VC7+ is not that easygoing anymore, so this wrapper exists.
#define VC6_SQRT(x) (sqrt((double)(x)))

/////////////////////////////////////////////////////////////////
// For scrolling text (used in RevolutionX opening)

class ISurfaceFadeBlt {
public:
    LRESULT FadeBlt(ISurface*, ISurface*, int x, int y);

    int* GetFadeTable(void)    { return& m_nFadeTable[0]; }
    int* GetRasterTable(void)  { return& m_nRasterTable[0]; }

    ISurfaceFadeBlt(void);
    virtual ~ISurfaceFadeBlt();

protected:
    int m_nFadeTable[600];     // Fade value (0-256) for each raster (800x600)
    int m_nRasterTable[600];   // Raster offset value for each raster
};

/////////////////////////////////////////////////////////////////
// Generic blitter for transitions

// Listener
class ISurfaceTransBltListener {
public:
    virtual LRESULT Blt(ISurface*lpDst,ISurface*lpSrc,int x,int y,int nPhase,int nTransMode=0,BYTE byFadeRate=255,LPRECT lpDstClipRect=NULL) = 0;
    virtual ~ISurfaceTransBltListener() {}
};

#define TransBltFunc(FuncName) \
LRESULT FuncName(ISurface*lpDst,ISurface*lpSrc,int x,int y,int nPhase,int nTransMode=0,BYTE byFadeRate=255,LPRECT lpDstClipRect=NULL);
    // nTransMode: 0 = No transparency, 1 = With transparency, 2 = With alpha (CDIB32 only)
    //             3 = Transfer with BltNatural

class ISurfaceTransBlt {
public:
    static LRESULT Blt(int nTransNo,ISurface*lpDst,ISurface*lpSrc,int x,int y,int nPhase,int nTransMode=0,BYTE byFadeRate=255,LPRECT lpDstClipRect=NULL); // Can be called by number

    static TransBltFunc(MirrorBlt1);    // Left-right mirror
    static TransBltFunc(MirrorBlt2);    // Up-down mirror
    static TransBltFunc(MirrorBlt3);    // Horizontal blur
    static TransBltFunc(MirrorBlt4);    // Vertical blur

    static TransBltFunc(CutInBlt1);     // Random horizontal raster
    static TransBltFunc(CutInBlt2);     // Random rectangle
    static TransBltFunc(CutInBlt3);     // 4*2 dot rectangle random fade
    static TransBltFunc(CutInBlt4);     // 4*2 dot rectangle random fade from top
    static TransBltFunc(CutInBlt5);     // 4*2 dot rectangle random fade from bottom
    static TransBltFunc(CutInBlt6);     // 4*2 dot rectangle random fade from left
    static TransBltFunc(CutInBlt7);     // 4*2 dot rectangle random fade from right
    static TransBltFunc(CutInBlt8);     // Rectangle from center
    static TransBltFunc(CutInBlt9);     // Rectangle from top left
    static TransBltFunc(CutInBlt10);    // Rectangle from top right
    static TransBltFunc(CutInBlt11);    // Rectangle from bottom left
    static TransBltFunc(CutInBlt12);    // Appear with left-right split
    static TransBltFunc(CutInBlt13);    // Appear with up-down split
    static TransBltFunc(CutInBlt14);    // Appear with 2-stage left-right split
    static TransBltFunc(CutInBlt15);    // Appear with 2-stage up-down split
    static TransBltFunc(CutInBlt16);    // Random fade from center 4*2 rectangle
    static TransBltFunc(CutInBlt17);    // Random fade from center 2*2 rectangle
    static TransBltFunc(CutInBlt18);    // Shuffling top to bottom
    static TransBltFunc(CutInBlt19);    // Shuffling bottom to top

    static TransBltFunc(WaveBlt1);      // Wave effect
    static TransBltFunc(WaveBlt2);      // Wave effect 180 degree phase difference
    static TransBltFunc(WaveBlt3);      // Small wave effect
    static TransBltFunc(WaveBlt4);      // Small wave effect 180 degree phase difference

    static TransBltFunc(WhorlBlt1);     // Spiral from outside
    static TransBltFunc(WhorlBlt2);     // Spiral from inside
    static TransBltFunc(WhorlBlt3);     // Spiral from outside 2*2
    static TransBltFunc(WhorlBlt4);     // Spiral from inside 2*2
    static TransBltFunc(WhorlBlt5);     // Spiral from outside 4*4
    static TransBltFunc(WhorlBlt6);     // Spiral from inside 4*4
    static TransBltFunc(WhorlBlt7);     // Spiral from outside 8*8
    static TransBltFunc(WhorlBlt8);     // Spiral from inside 8*8

    static TransBltFunc(CircleBlt1);    // Circle from top
    static TransBltFunc(CircleBlt2);    // Circle from bottom
    static TransBltFunc(CircleBlt3);    // Circle from left
    static TransBltFunc(CircleBlt4);    // Circle from right
    static TransBltFunc(CircleBlt5);    // Circle from center

    static TransBltFunc(RectBlt1);      // Rectangle type 4 dots
    static TransBltFunc(RectBlt2);      // Rectangle type 8 dots
    static TransBltFunc(RectBlt3);      // Rectangle type 16 dots
    
    static TransBltFunc(BlindBlt1);     // Vertical blinds from left 8 dots
    static TransBltFunc(BlindBlt2);     // Vertical blinds from right 8 dots
    static TransBltFunc(BlindBlt3);     // Vertical blinds from left 16 dots
    static TransBltFunc(BlindBlt4);     // Vertical blinds from right 16 dots
    static TransBltFunc(BlindBlt5);     // Horizontal blinds from top 8 dots
    static TransBltFunc(BlindBlt6);     // Horizontal blinds from bottom 8 dots
    static TransBltFunc(BlindBlt7);     // Horizontal blinds from top 16 dots
    static TransBltFunc(BlindBlt8);     // Horizontal blinds from bottom 16 dots
    static TransBltFunc(BlindBlt9);     // Diagonal blinds right 8 dots
    static TransBltFunc(BlindBlt10);    // Diagonal blinds right 16 dots

    static TransBltFunc(BlendBlt1);     // Normal blend blit

    // Thanks! > TearDrop_Stone
    static TransBltFunc(MosaicBlt1);		 // Mosaic
    static TransBltFunc(FlushBlt1);			 // Negative-Positive inversion
    static TransBltFunc(SlitCurtainBlt1);    // Curtain from left 16 dots
    static TransBltFunc(SlitCurtainBlt2);    // Curtain from right 16 dots
    static TransBltFunc(SlitCurtainBlt3);    // Curtain from left 8 dots
    static TransBltFunc(SlitCurtainBlt4);    // Curtain from right 8 dots
    static TransBltFunc(SlitCurtainBlt5);    // Curtain from top 16 dots
    static TransBltFunc(SlitCurtainBlt6);    // Curtain from bottom 16 dots
    static TransBltFunc(SlitCurtainBlt7);    // Curtain from top 8 dots
    static TransBltFunc(SlitCurtainBlt8);    // Curtain from bottom 8 dots
    static TransBltFunc(TensileBlt1);        // Stretch from left edge -> original image
    static TransBltFunc(TensileBlt2);        // Stretch from right edge -> original image
    static TransBltFunc(TensileBlt3);        // Stretch from top edge -> original image
    static TransBltFunc(TensileBlt4);        // Stretch from bottom edge -> original image

    // Effects from yaneSDK1st
    static TransBltFunc(DiagonalDiffusionBlt);       // Diagonal fade in
    static TransBltFunc(DiffusionCongeriesBlt1);     // Random diffusion gathering 4*2
    static TransBltFunc(DiffusionCongeriesBlt2);     // Random diffusion gathering 2*2
    static TransBltFunc(DiffusionCongeriesBlt3);     // Random diffusion gathering
    static TransBltFunc(SquashBlt);                  // Squash effect
    static TransBltFunc(ForwardRollBlt);            // Forward rotation
    static TransBltFunc(RotationBlt1);              // Center screen rotation
    static TransBltFunc(RotationBlt2);              // Center screen rotation
    static TransBltFunc(RotationBlt3);              // Center screen rotation
    static TransBltFunc(RotationBlt4);              // Center screen rotation
    static TransBltFunc(EnterUpBlt1);               // 8*8 division from bottom
    static TransBltFunc(EnterUpBlt2);               // 16*16 division from bottom
    static TransBltFunc(CellGatherBlt1);            // Cell gathering 1
    static TransBltFunc(CellGatherBlt2);            // Cell gathering 2

    static vector< smart_ptr<ISurfaceTransBltListener> >* GetBltListener() {
        return &m_avListener;
    }

protected:
    // Callback functions
    static vector< smart_ptr<ISurfaceTransBltListener> > m_avListener;

    // Helper functions for SlitCurtainBlt and TensileBlt
    static LRESULT BltTransHelper1(ISurface* pSrc, ISurface* pDest, int x, int y,
                                  int nPhase, bool Direction, int nTransMode, BYTE byFadeRate,
                                  int WidthNum, LPRECT lpDstClipRect);
    static LRESULT BltTransHelper2(ISurface* pSrc, ISurface* pDest, int x, int y,
                                  int nPhase, bool Direction, int nTransMode, BYTE byFadeRate,
                                  int WidthNum, LPRECT lpDstClipRect);
    static LRESULT TensileBltHelper1(ISurface* pSrc, ISurface* pDest, int x, int y,
                                    int nPhase, bool Direction, int nTransMode, BYTE byFadeRate, LPRECT lpDstClipRect);
    static LRESULT TensileBltHelper2(ISurface* pSrc, ISurface* pDest, int x, int y,
                                    int nPhase, bool Direction, int nTransMode, BYTE byFadeRate, LPRECT lpDstClipRect);

    // Required for randomization
    static void MakeBltTable();

    class BltTransTable {
    public:
        BltTransTable();
        BYTE RandTable[256];    // Random pattern for mosaic
    };

    static smart_ptr<CSinTable> m_sin_table;
    static smart_ptr<BltTransTable> m_blt_table;
};

#endif