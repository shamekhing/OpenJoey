#include "stdafx.h"
#include "CBinSystem.h"
#include "CBinDecompress.h"

CBinSystem::CBinSystem()
    : m_cards(NULL)
    , m_dialogs(NULL)
    , m_cardDescriptions(NULL)
    , m_cardDescIndexes(NULL)
    , m_dialogTexts(NULL)
    , m_dialogIndexes(NULL)
	, m_cardList(NULL)
	, m_cardListMini(NULL)
    , m_cardCount(0)
    , m_dialogCount(0)
{
	m_lastErrorFile[0] = '\0';
}

CBinSystem::~CBinSystem()
{
    if(m_cards) delete[] m_cards;
    if(m_dialogs) delete[] m_dialogs;
    if(m_cardDescriptions) delete[] m_cardDescriptions;
    if(m_cardDescIndexes) delete[] m_cardDescIndexes;
    if(m_dialogTexts) delete[] m_dialogTexts;
    if(m_dialogIndexes) delete[] m_dialogIndexes;
	if(m_cardList) delete[] m_cardList;
	if(m_cardListMini) delete[] m_cardListMini;
}

void CBinSystem::Reset()
{
	if(m_cards) { delete[] m_cards; m_cards = NULL; }
	if(m_dialogs) { delete[] m_dialogs; m_dialogs = NULL; }
	if(m_cardDescriptions) { delete[] m_cardDescriptions; m_cardDescriptions = NULL; }
	if(m_cardDescIndexes) { delete[] m_cardDescIndexes; m_cardDescIndexes = NULL; }
	if(m_dialogTexts) { delete[] m_dialogTexts; m_dialogTexts = NULL; }
	if(m_dialogIndexes) { delete[] m_dialogIndexes; m_dialogIndexes = NULL; }
	if(m_cardList) { delete[] m_cardList; m_cardList = NULL; }
	if(m_cardListMini) { delete[] m_cardListMini; m_cardListMini = NULL; }
	m_cardCount = 0;
	m_dialogCount = 0;
	m_lastErrorFile[0] = '\0';
}

BOOL CBinSystem::Initialize(const char* basePath, const char* language)
{
    char fullPath[MAX_PATH];
    char indexPath[MAX_PATH];
	m_lastErrorFile[0] = '\0';

    // Load card properties (no language suffix)
    sprintf_s(fullPath, sizeof(fullPath), "%s\\card_prop.bin", basePath);
    if(!LoadCardProperties(fullPath)) { sprintf_s(m_lastErrorFile, "%s", fullPath); return FALSE; }

    // Load card names with language (### -> eng for example)
    sprintf_s(fullPath, sizeof(fullPath), "%s\\card_name%s.bin", basePath, language);
    if(!LoadCardNames(fullPath)) { sprintf_s(m_lastErrorFile, "%s", fullPath); return FALSE; }

    // Load real card IDs (no language suffix)
    sprintf_s(fullPath, sizeof(fullPath), "%s\\card_id.bin", basePath);
    if(!LoadCardRealIds(fullPath)) { sprintf_s(m_lastErrorFile, "%s", fullPath); return FALSE; }

    // Load internal card IDs (no language suffix)
    sprintf_s(fullPath, sizeof(fullPath), "%s\\card_intid.bin", basePath);
    if(!LoadCardInternalIds(fullPath)) { sprintf_s(m_lastErrorFile, "%s", fullPath); return FALSE; }

    // Load card descriptions and indices with language
    sprintf_s(fullPath, sizeof(fullPath), "%s\\card_desc%s.bin", basePath, language);
    sprintf_s(indexPath, sizeof(indexPath), "%s\\card_indx%s.bin", basePath, language);
    if(!LoadCardDescriptions(fullPath, indexPath)) { sprintf_s(m_lastErrorFile, "%s (or %s)", fullPath, indexPath); return FALSE; }

    // Load dialog texts and indices with language
    sprintf_s(fullPath, sizeof(fullPath), "%s\\dlg_text%s.bin", basePath, language);
    sprintf_s(indexPath, sizeof(indexPath), "%s\\dlg_indx%s.bin", basePath, language);
    if(!LoadDialogTexts(fullPath, indexPath)) { sprintf_s(m_lastErrorFile, "%s (or %s)", fullPath, indexPath); return FALSE; }

    // Load card pack data (no language suffix)
    sprintf_s(fullPath, sizeof(fullPath), "%s\\card_pack.bin", basePath);
    if(!LoadCardPacks(fullPath)) { sprintf_s(m_lastErrorFile, "%s", fullPath); return FALSE; }

	// Load card list
    sprintf_s(fullPath, sizeof(fullPath), "%s\\..\\card\\list_card.txt", basePath);
    if(!LoadCardList(fullPath, FALSE)) { sprintf_s(m_lastErrorFile, "%s", fullPath); return FALSE; }

	// Load card list (mini)
    sprintf_s(fullPath, sizeof(fullPath), "%s\\..\\mini\\list_card.txt", basePath);
    if(!LoadCardList(fullPath, TRUE)) { sprintf_s(m_lastErrorFile, "%s", fullPath); return FALSE; }

    return TRUE;
}

BYTE* CBinSystem::DecompressFile(const char* path, DWORD& outSize)
{
    FILE* file = NULL;
    if (fopen_s(&file, path, "rb") != 0 || !file) return NULL;

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
	// TODO: something is off with the data, could it be that its just garbage in game files? verify
	// this file doesnt make sense, it can be build at runtime
    DWORD size;
    BYTE* data = DecompressFile(path, size);
    if(!data) return FALSE;

    // Set first entry to 0xFFFF
    m_cards[0].internalId.id = 0xFFFF;

    // Copy remaining internal IDs
    //const size_t bytesToCopy = (m_cardCount - 1) * sizeof(CardInternalId);
    //if(size < bytesToCopy)
    //{
    //    delete[] data;
    //    return FALSE;
    //}
    
	// TODO: its faster this way, any undesired behaviour?
    for(DWORD i = 1; i < m_cardCount; i++) {
        //memcpy(&m_cards[i].internalId, &((CardInternalId*)data)[i-1], sizeof(CardInternalId));
		m_cards[i].internalId.id = i;
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

    //Allocate and setup dialog entries
	//char buf[512];
    m_dialogs = new DialogEntry[m_dialogCount];
    for(DWORD i = 0; i < m_dialogCount; i++) {
        m_dialogs[i].text = &m_dialogTexts[m_dialogIndexes[i]];
		// Debug dump ids to console
		//sprintf(buf, "%d DialogTextId: %s\n", i, m_dialogs[i].text);
		//OutputDebugStringA(buf);
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

BOOL CBinSystem::LoadCardList(const char* path, BOOL isMini)
{
    FILE* file = NULL;
    if (fopen_s(&file, path, "r") != 0 || !file) return FALSE;

    // Allocate array based on card count
    CardListEntry** targetArray = isMini ? &m_cardListMini : &m_cardList;
    *targetArray = new CardListEntry[m_cardCount];
    memset(*targetArray, 0, sizeof(CardListEntry) * m_cardCount);

    char line[MAX_PATH];
    WORD currentId = 0;

    while(fgets(line, sizeof(line), file))
    {
        // Skip comment lines that don't have .bmp
        if(strstr(line, ".bmp") == NULL)
            continue;

        // Remove newline if present
        char* newline = strchr(line, '\n');
        if(newline) *newline = '\0';

        // Store filename and update card reference
        if(currentId < m_cardCount) {
            strcpy_s((*targetArray)[currentId].imageFilename, MAX_PATH, line);
            
			// Update the appropriate pointer in the Card structure
			if(isMini) {
				m_cards[currentId].imageMiniFilename = (*targetArray)[currentId].imageFilename;
			} else {
				m_cards[currentId].imageFilename = (*targetArray)[currentId].imageFilename;
			}

			// No GFX in game files for the card but is specified (sometimes incomplete)
			if(strstr(line, "card_ura.bmp") != NULL && currentId != 0) {
				m_cards[currentId].hasValidGFX = false;
			} else {
				m_cards[currentId].hasValidGFX = true;
			}

            currentId++;
        }
    }

    fclose(file);
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
    if(cardId >= m_cardCount) return TYPE_WINGED_BEAST;
    return m_cards[cardId].properties.GetMonsterType();
}

CardCategory CBinSystem::GetCardCategory(DWORD cardId)
{
    if(cardId >= m_cardCount) return CATEGORY_NORMAL;
    return m_cards[cardId].properties.GetCardCategory();
}

MonsterAttribute CBinSystem::GetMonsterAttribute(DWORD cardId)
{
    if(cardId >= m_cardCount) return ATTR_DIVINE_BEAST_NO_TRIBUTE;
    return m_cards[cardId].properties.GetMonsterAttribute();
}

BYTE CBinSystem::GetMonsterStars(DWORD cardId)
{
    if(cardId >= m_cardCount) return 0;
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
    if(internalId >= m_cardCount) return 0;
    return m_cards[internalId].realId.id;
}

/// SLOW, try to not use. We dont have a lookup table for this relation because its rarely used.
WORD CBinSystem::GetCardInternalId(DWORD realId)
{
	for (DWORD rc = 0; rc < m_cardCount; ++rc)
	{
		if (m_cards[rc].realId.id == realId)
		{
			return m_cards[rc].internalId.id;
		}
	}
	return 0;
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

const char* CBinSystem::GetCardImageFilename(DWORD cardId)
{
    if(cardId >= m_cardCount) return NULL;
    return m_cards[cardId].imageFilename;
}

const char* CBinSystem::GetCardImageMiniFilename(DWORD cardId)
{
    if(cardId >= m_cardCount) return NULL;
    return m_cards[cardId].imageMiniFilename;
}
