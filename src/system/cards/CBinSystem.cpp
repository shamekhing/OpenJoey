#include "../../stdafx.h"
#include "CBinSystem.h"
#include "CBinDecompress.h"

CBinSystem::CBinSystem()
    : m_cards(NULL)
    , m_dialogs(NULL)
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
    if(m_cards) delete[] m_cards;
    if(m_dialogs) delete[] m_dialogs;
    if(m_cardDescriptions) delete[] m_cardDescriptions;
    if(m_cardDescIndexes) delete[] m_cardDescIndexes;
    if(m_dialogTexts) delete[] m_dialogTexts;
    if(m_dialogIndexes) delete[] m_dialogIndexes;
}

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
    
    // Allocate the unified card array
    m_cards = new Card[m_cardCount];
    memset(m_cards, 0, m_cardCount * sizeof(Card));

    // Copy properties into each card
    for(DWORD i = 0; i < m_cardCount; i++) {
        memcpy(&m_cards[i].properties, &((CardProperties*)data)[i], sizeof(CardProperties));
    }

    delete[] data;
    return TRUE;
}

BOOL CBinSystem::LoadCardNames(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    if(size != m_cardCount * sizeof(CardName))
    {
        delete[] data;
        return FALSE;
    }

    // Copy names into each card
    for(DWORD i = 0; i < m_cardCount; i++) {
        memcpy(&m_cards[i].name, &((CardName*)data)[i], sizeof(CardName));
    }

    delete[] data;
    return TRUE;
}

BOOL CBinSystem::LoadCardRealIds(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    if(size != m_cardCount * sizeof(CardRealId))
    {
        delete[] data;
        return FALSE;
    }

    // Copy real IDs into each card
    for(DWORD i = 0; i < m_cardCount; i++) {
        memcpy(&m_cards[i].realId, &((CardRealId*)data)[i], sizeof(CardRealId));
    }

    delete[] data;
    return TRUE;
}

BOOL CBinSystem::LoadCardInternalIds(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    // Set first entry to 0xFFFF
    m_cards[0].internalId.id = 0xFFFF;

    // Copy remaining internal IDs
    const size_t bytesToCopy = (m_cardCount - 1) * sizeof(CardInternalId);
    if(size < bytesToCopy)
    {
        delete[] data;
        return FALSE;
    }
    
    for(DWORD i = 1; i < m_cardCount; i++) {
        memcpy(&m_cards[i].internalId, &((CardInternalId*)data)[i-1], sizeof(CardInternalId));
    }

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

    // Set description pointers for each card
    for(DWORD i = 0; i < m_cardCount; i++) {
        m_cards[i].description = &m_cardDescriptions[m_cardDescIndexes[i]];
    }

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

    m_dialogCount = indexSize / sizeof(DWORD);
    m_dialogIndexes = (DWORD*)indexData;
    m_dialogTexts = (char*)textData;

    // Allocate and setup dialog entries
    m_dialogs = new DialogEntry[m_dialogCount];
    for(DWORD i = 0; i < m_dialogCount; i++) {
        m_dialogs[i].text = &m_dialogTexts[m_dialogIndexes[i]];
    }

    return TRUE;
}

BOOL CBinSystem::LoadCardPacks(const char* path)
{
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    if(size != 2230)
    {
        delete[] data;
        return FALSE;
    }

    // Copy pack data into each card
    for(DWORD i = 0; i < m_cardCount; i++) {
        memcpy(&m_cards[i].pack, &((CardPack*)data)[i], sizeof(CardPack));
    }

    delete[] data;
    return TRUE;
}

// Getter implementations now use m_cards directly
WORD CBinSystem::GetDefenseValue(DWORD cardId)
{
    if(cardId >= m_cardCount) return 0;
    return m_cards[cardId].properties.GetDefenseValue();
}

WORD CBinSystem::GetAttackValue(DWORD cardId)
{
    if(cardId >= m_cardCount) return 0;
    return m_cards[cardId].properties.GetAttackValue();
}

MonsterType CBinSystem::GetMonsterType(DWORD cardId)
{
    if(cardId >= m_cardCount) return TYPE_NONE;
    return m_cards[cardId].properties.GetMonsterType();
}

CardCategory CBinSystem::GetCardCategory(DWORD cardId)
{
    if(cardId >= m_cardCount) return CATEGORY_NORMAL_0;
    return m_cards[cardId].properties.GetCardCategory();
}

MonsterAttribute CBinSystem::GetMonsterAttribute(DWORD cardId)
{
    if(cardId >= m_cardCount) return ATTR_DIVINE_BEAST_NO_TRIBUTE;
    return m_cards[cardId].properties.GetMonsterAttribute();
}

MonsterStars CBinSystem::GetMonsterStars(DWORD cardId)
{
    if(cardId >= m_cardCount) return STARS_1;
    return m_cards[cardId].properties.GetMonsterStars();
}

BOOL CBinSystem::RequiresTwoTributes(DWORD cardId)
{
    if(cardId >= m_cardCount) return FALSE;
    return m_cards[cardId].properties.RequiresTwoTributes();
}

const char* CBinSystem::GetCardName(DWORD cardId)
{
    if(cardId >= m_cardCount) return NULL;
    return m_cards[cardId].name.name;
}

WORD CBinSystem::GetCardRealId(DWORD internalId)
{
    if(internalId >= m_cardCount) return 0xFFFF;
    return m_cards[internalId].realId.id;
}

WORD CBinSystem::GetCardInternalId(DWORD cardId)
{
    if(cardId >= m_cardCount) return 0;
    return m_cards[cardId].internalId.id;
}

const char* CBinSystem::GetCardDescription(DWORD cardId)
{
    if(cardId >= m_cardCount) return NULL;
    return m_cards[cardId].description;
}

const char* CBinSystem::GetDialogText(DWORD dialogId)
{
    if(dialogId >= m_dialogCount) return NULL;
    return m_dialogs[dialogId].text;
}

BOOL CBinSystem::IsCardAvailable(DWORD cardId, BYTE gameId)
{
    if(cardId >= m_cardCount) return FALSE;
    return (m_cards[cardId].pack.gameFlags & gameId) != 0;
}