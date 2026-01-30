#include "stdafx.h"
#include "CBinDecompress.h"

bool CBinDecompress::IsCompressed(const BYTE* data, DWORD size, const char* filename) {
    return CBinDecompressDetector::NeedsDecompression(data, size, filename);
}

BYTE* CBinDecompress::Decompress(const BYTE* data, DWORD size, DWORD& outSize) {
    if (size <= 1) return NULL;  // Need at least flag byte + some data

    BYTE* decompressedData = NULL;
    outSize = size;  // Use input size as initial estimate

    // Create LZSS decompressor
    yaneuraoGameSDK3rd::Auxiliary::CLZSS lzss;
    
    // Skip the flag byte for decompression
    LRESULT result = lzss.Decode(
        (BYTE*)data + 1,  // Skip flag byte
        decompressedData,
        outSize,
        true  // Allocate buffer in Decode
    );

    if (result != 0) {
        if (decompressedData) delete[] decompressedData;
        return NULL;
    }

    return decompressedData;
}

CBinDecompressDetector::BinType CBinDecompressDetector::GetBinType(const char* filename) {
    // MSVC 2003: Use basic string functions instead of std::string
    char tempName[MAX_PATH];
    strcpy_s(tempName, sizeof(tempName), filename);
    
    // Simple lowercase conversion
    char* p = tempName;
    while (*p) {
        *p = (char)tolower(*p);
        p++;
    }

    if (strstr(tempName, "card_id.bin") != NULL)
        return CARD_ID;
    if (strstr(tempName, "dlg_text") != NULL)
        return DLG_TEXT;
    
    return GENERIC;
}

bool CBinDecompressDetector::NeedsDecompression(const BYTE* data, DWORD size, const char* filename) {
    if (size <= 0) return false;

    BYTE flag = data[0];
    BinType type = GetBinType(filename);

    // Special cases first
    if (type == CARD_ID)
        return false;  // Never compressed
    if (type == DLG_TEXT)
        return flag == 0xAA;  // Must be exactly 0xAA

    // For all other files, check against known flags
    switch (flag) {
        case 0xFF:  // Generic JTP
        case 0xDF:  // PoC Joey
        case 0x7F:  // Card pack
        case 0x5F:  // Other PoC variants
        case 0xAF:  // YGO Online 2005+
        case 0xFE:  // Card properties
        case 0xF0:  // Card names
            return true;
    }

    return false;
}