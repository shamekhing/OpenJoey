#ifndef CBINDECOMPRESS_H
#define CBINDECOMPRESS_H

#include "../../stdafx.h"

// Forward declarations
class CBinDecompressDetector;

class CBinDecompress {
public:
    static bool IsCompressed(const BYTE* data, DWORD size, const char* filename);
    static BYTE* Decompress(const BYTE* data, DWORD size, DWORD& outSize);
};

class CBinDecompressDetector {
public:
    enum BinType {  // No class enum in 2003!
        CARD_ID,    // Never compressed despite flag
        DLG_TEXT,   // Special case with 0xAA
        GENERIC     // Everything else
    };

    static bool NeedsDecompression(const BYTE* data, DWORD size, const char* filename);

private:
    static BinType GetBinType(const char* filename);
};

#endif // CBINDECOMPRESS_H