#include "../../stdafx.h"
#include "CBinSystem.h"
#include "CBinDecompress.h"

CBinSystem::CBinSystem()
    : m_cardProps(NULL)
    , m_cardNames(NULL)
    , m_cardRealIds(NULL)
    , m_cardInternalIds(NULL)
    , m_cardPacks(NULL)
    , m_cardDescriptions(NULL)
    , m_cardDescIndexes(NULL)
    , m_dialogTexts(NULL)
    , m_dialogIndexes(NULL)
    , m_cardCount(0)
    , m_dialogCount(0)
{
}

CBinSystem::~CBinSystem()
{
    if(m_cardProps) delete[] m_cardProps;
    if(m_cardNames) delete[] m_cardNames;
    if(m_cardRealIds) delete[] m_cardRealIds;
    if(m_cardInternalIds) delete[] m_cardInternalIds;
    if(m_cardPacks) delete[] m_cardPacks;
    if(m_cardDescriptions) delete[] m_cardDescriptions;
    if(m_cardDescIndexes) delete[] m_cardDescIndexes;
    if(m_dialogTexts) delete[] m_dialogTexts;
    if(m_dialogIndexes) delete[] m_dialogIndexes;
}

// In CBinSystem.cpp, update the loading functions to use language suffix

BOOL CBinSystem::Initialize(const char* basePath, const char* language)
{
    char fullPath[MAX_PATH];

    // Load card properties (no language suffix)
    sprintf(fullPath, "%s\\card_prop.bin", basePath);
    if(!LoadCardProperties(fullPath)) return FALSE;

    // Load card names with language (### -> eng for example)
    sprintf(fullPath, "%s\\card_name%s.bin", basePath, language);
    if(!LoadCardNames(fullPath)) return FALSE;

    // Load real card IDs (no language suffix)
    sprintf(fullPath, "%s\\card_id.bin", basePath);
    if(!LoadCardRealIds(fullPath)) return FALSE;

    // Load internal card IDs (no language suffix)
    sprintf(fullPath, "%s\\card_intid.bin", basePath);
    if(!LoadCardInternalIds(fullPath)) return FALSE;

    // Load card descriptions and indices with language
    sprintf(fullPath, "%s\\card_desc%s.bin", basePath, language);
    char indexPath[MAX_PATH];
    sprintf(indexPath, "%s\\card_indx%s.bin", basePath, language);
    if(!LoadCardDescriptions(fullPath, indexPath)) return FALSE;

    // Load dialog texts and indices with language
    sprintf(fullPath, "%s\\dlg_text%s.bin", basePath, language);
    sprintf(indexPath, "%s\\dlg_indx%s.bin", basePath, language);
    if(!LoadDialogTexts(fullPath, indexPath)) return FALSE;

    // Load card pack data (no language suffix)
    sprintf(fullPath, "%s\\card_pack.bin", basePath);
    if(!LoadCardPacks(fullPath)) return FALSE;

    return TRUE;
}

BYTE* CBinSystem::DecompressFile(const char* path, DWORD& outSize)
{
    FILE* file = fopen(path, "rb");
    if(!file) return NULL;

    // Get file size
    fseek(file, 0, SEEK_END);
    DWORD fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read file data
    BYTE* fileData = new BYTE[fileSize];
    fread(fileData, 1, fileSize, file);
    fclose(file);

    // Get filename for compression check
    const char* filename = strrchr(path, '\\');
    if(!filename) filename = path;
    else filename++; // Skip backslash

    // Check if file needs decompression
    if(CBinDecompress::IsCompressed(fileData, fileSize, filename))
    {
        BYTE* decompressedData = CBinDecompress::Decompress(fileData, fileSize, outSize);
        delete[] fileData;
        return decompressedData;
    }

    outSize = fileSize;
    return fileData;
}

BOOL CBinSystem::LoadCardProperties(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    m_cardCount = size / sizeof(CardProperties);
    m_cardProps = new CardProperties[m_cardCount];
    memcpy(m_cardProps, data, size);

    delete[] data;
    return TRUE;
}

BOOL CBinSystem::LoadCardNames(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    // Verify size matches card count
    if(size != m_cardCount * sizeof(CardName))
    {
        delete[] data;
        return FALSE;
    }

    m_cardNames = new CardName[m_cardCount];
    memcpy(m_cardNames, data, size);

    delete[] data;
    return TRUE;
}

BOOL CBinSystem::LoadCardRealIds(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    // Verify size matches card count
    if(size != m_cardCount * sizeof(CardRealId))
    {
        delete[] data;
        return FALSE;
    }

    m_cardRealIds = new CardRealId[m_cardCount];
    memcpy(m_cardRealIds, data, size);

    delete[] data;
    return TRUE;
}

BOOL CBinSystem::LoadCardInternalIds(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    // Allocate exactly m_cardCount entries since that's already the correct count
    m_cardInternalIds = new CardInternalId[m_cardCount];
    
    // Set first entry to 0xFFFF
    m_cardInternalIds[0].id = 0xFFFF;

    // Copy the actual card data starting at index 1
    const size_t bytesToCopy = (m_cardCount - 1) * sizeof(CardInternalId);
    if(size < bytesToCopy)
    {
        delete[] data;
        return FALSE;
    }
    
    memcpy(&m_cardInternalIds[1], data, bytesToCopy); // void* moved to entry [1]

    delete[] data;
    return TRUE;
}

BOOL CBinSystem::LoadCardDescriptions(const char* path, const char* indexPath)
{
    // Load description index first
    DWORD indexSize;
    BYTE* indexData = DecompressFile(indexPath, indexSize);
    if(!indexData) return FALSE;

    // Load actual descriptions
    DWORD textSize;
    BYTE* textData = DecompressFile(path, textSize);
    if(!textData)
    {
        delete[] indexData;
        return FALSE;
    }

    m_cardDescIndexes = (DWORD*)indexData;
    m_cardDescriptions = (char*)textData;

    return TRUE;
}

BOOL CBinSystem::LoadDialogTexts(const char* path, const char* indexPath)
{
    // Load dialog index first
    DWORD indexSize;
    BYTE* indexData = DecompressFile(indexPath, indexSize);
    if(!indexData) return FALSE;

    // Load actual dialog texts
    DWORD textSize;
    BYTE* textData = DecompressFile(path, textSize);
    if(!textData)
    {
        delete[] indexData;
        return FALSE;
    }

    m_dialogIndexes = (DWORD*)indexData;
    m_dialogTexts = (char*)textData;
    m_dialogCount = indexSize / sizeof(DWORD);

    return TRUE;
}

BOOL CBinSystem::LoadCardPacks(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    // Card packs have 1115 pairs of 2 bytes each (2230 bytes total)
    const size_t CARD_PACK_COUNT = 1115;
    
    // Verify size is exactly 2230 bytes
    if(size != 2230)
    {
        delete[] data;
        return FALSE;
    }

    m_cardPacks = new CardPack[CARD_PACK_COUNT];
    memcpy(m_cardPacks, data, size);

    delete[] data;
    return TRUE;
}

// Getter implementations
WORD CBinSystem::GetDefenseValue(DWORD cardId)
{
    if(cardId >= m_cardCount) return 0;
    return m_cardProps[cardId].GetDefenseValue();
}

WORD CBinSystem::GetAttackValue(DWORD cardId)
{
    if(cardId >= m_cardCount) return 0;
    return m_cardProps[cardId].GetAttackValue();
}

MonsterType CBinSystem::GetMonsterType(DWORD cardId)
{
    if(cardId >= m_cardCount) return TYPE_NONE;
    return m_cardProps[cardId].GetMonsterType();
}

CardCategory CBinSystem::GetCardCategory(DWORD cardId)
{
    if(cardId >= m_cardCount) return CATEGORY_NORMAL_0;
    return m_cardProps[cardId].GetCardCategory();
}

MonsterAttribute CBinSystem::GetMonsterAttribute(DWORD cardId)
{
    if(cardId >= m_cardCount) return ATTR_DIVINE_BEAST_NO_TRIBUTE;
    return m_cardProps[cardId].GetMonsterAttribute();
}

MonsterStars CBinSystem::GetMonsterStars(DWORD cardId)
{
    if(cardId >= m_cardCount) return STARS_1;
    return m_cardProps[cardId].GetMonsterStars();
}

BOOL CBinSystem::RequiresTwoTributes(DWORD cardId)
{
    if(cardId >= m_cardCount) return FALSE;
    return m_cardProps[cardId].RequiresTwoTributes();
}

const char* CBinSystem::GetCardName(DWORD cardId)
{
    if(cardId >= m_cardCount) return NULL;
    return m_cardNames[cardId].name;
}

WORD CBinSystem::GetCardRealId(DWORD internalId)
{
    if(internalId >= m_cardCount) return 0xFFFF;
    return m_cardRealIds[internalId].id;
}

WORD CBinSystem::GetCardInternalId(DWORD cardId)
{
    if(cardId >= m_cardCount) return 0;
    return m_cardInternalIds[cardId].id;
}

const char* CBinSystem::GetCardDescription(DWORD cardId)
{
    if(cardId >= m_cardCount) return NULL;
    return &m_cardDescriptions[m_cardDescIndexes[cardId]];
}

const char* CBinSystem::GetDialogText(DWORD dialogId)
{
    if(dialogId >= m_dialogCount) return NULL;
    return &m_dialogTexts[m_dialogIndexes[dialogId]];
}

BOOL CBinSystem::IsCardAvailable(DWORD cardId, BYTE gameId)
{
    if(cardId >= m_cardCount) return FALSE;
    return (m_cardPacks[cardId].gameFlags & gameId) != 0;
}