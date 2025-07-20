#ifndef CBINSYSTEM_H
#define CBINSYSTEM_H

#include "../../stdafx.h"

// Card property enums
enum MonsterType {
    TYPE_WINGED_BEAST		= 0x00,
    TYPE_DRAGON				= 0x01,
    TYPE_ZOMBIE				= 0x02,
    TYPE_FIEND				= 0x03,
    TYPE_PYRO				= 0x04,
    TYPE_SEA_SERPENT		= 0x05,
    TYPE_ROCK				= 0x06,
    TYPE_MACHINE			= 0x07,
    TYPE_FISH				= 0x08,
    TYPE_DINOSAUR			= 0x09,
    TYPE_INSECT				= 0x0A,
    TYPE_BEAST				= 0x0B,
    TYPE_BEAST_WARRIOR		= 0x0C,
    TYPE_PLANT				= 0x0D,
    TYPE_AQUA				= 0x0E,
    TYPE_WARRIOR			= 0x0F,
    // Additional types when useSecondary flag is set
    TYPE_UNUSED				= 0x10,
    TYPE_FAIRY				= 0x11,
    TYPE_SPELLCASTER		= 0x12,
    TYPE_THUNDER			= 0x13,
    TYPE_REPTILE			= 0x14,
    TYPE_TRAPCARD			= 0x15,
    TYPE_SPELLCARD			= 0x16,
	TYPE_NON_GAME_CARD		= 0x17,
	TYPE_DIVINE_BEAST		= 0x18
};

enum CardCategory {
    CATEGORY_NORMAL = 0x00,		// Includes 0,1,2,3
    CATEGORY_EFFECT = 0x04,		// Includes 4,5,6,7
    CATEGORY_FUSION = 0x08,		// Includes 8,9,A,B
    CATEGORY_RITUAL = 0x0C		// Includes C,D,E,F
};

enum MonsterAttribute {
    ATTR_DIVINE_BEAST_NO_TRIBUTE	= 0x00,
    ATTR_DIVINE_BEAST_2_TRIBUTE		= 0x01,
    ATTR_LIGHT_NO_TRIBUTE			= 0x02,
    ATTR_LIGHT_2_TRIBUTE			= 0x03,
    ATTR_DARK_NO_TRIBUTE			= 0x04,
    ATTR_DARK_2_TRIBUTE				= 0x05,
    ATTR_WATER_NO_TRIBUTE			= 0x06,
    ATTR_WATER_2_TRIBUTE			= 0x07,
    ATTR_FIRE_NO_TRIBUTE			= 0x08,
    ATTR_FIRE_2_TRIBUTE				= 0x09,
    ATTR_EARTH_NO_TRIBUTE			= 0x0A,
    ATTR_EARTH_2_TRIBUTE			= 0x0B,
    ATTR_WIND_NO_TRIBUTE			= 0x0C,
    ATTR_WIND_2_TRIBUTE				= 0x0D,
	ATTR_UNUSED_NO_TRIBUTE			= 0x0E,
    ATTR_UNUSED_2_TRIBUTE			= 0x0F
};

struct CardPropertiesBytes {
    BYTE defenseModifiers;   // Position 1-2: DEF
    BYTE attackModifiers;    // Position 3-4: ATK
    BYTE typeAndCategory;    // Position 5-6: Type & Category
    BYTE attrAndStars;       // Position 7-8: Attribute & Stars
};

struct CardProperties {
	// Special thanks to @github.com/kiemkhach for his spreadsheet table with correct formulas
	// https://github.com/derplayer/YuGiOh-PoC-ModTools/discussions/2#discussioncomment-5973506
	CardPropertiesBytes bytes;

	WORD GetAttackValue() const {
		// Split bytes into same nibbles as Excel
		BYTE P = bytes.attackModifiers >> 4;     // High ATK nibble
		BYTE Q = bytes.attackModifiers & 0x0F;   // Low ATK nibble
		BYTE S = bytes.typeAndCategory & 0x0F;   // Low type nibble

		// Excel formula: P*80 + (Q - REST(Q,2))*5 + 1280*REST(S,4)
		return (P * 80) +                   // 14 * 80 = 1120
			((Q - (Q % 2)) * 5) +			// (0 - (0 % 2)) * 5 = 0
			(1280 * (S % 4));				// 1280 * (5 % 4) = 1280 * 1 = 1280
		// Example Total for id 428: 1120 + 0 + 1280 = 2400
	}

	WORD GetDefenseValue() const {
		// Split bytes into same nibbles as Excel
		BYTE M = bytes.defenseModifiers >> 4;     // High DEF nibble
		BYTE N = bytes.defenseModifiers & 0x0F;   // Low DEF nibble
		BYTE Q = bytes.attackModifiers & 0x0F;    // Low ATK nibble

		// Excel formula: M*160 + N*10 + REST(Q,2)*2560
		return (M * 160) +                  // 9 * 160 = 1440
			(N * 10) +						// 6 * 10 = 60
			((Q % 2) * 2560);				// (0 % 2) * 2560 = 0
		// Example Total id 428: 1440 + 60 + 0 = 1500
	}

	MonsterType GetMonsterType() const {
		BYTE typeIdx = (bytes.typeAndCategory >> 4);     // Get upper nibble for type
		BYTE y = (bytes.attrAndStars & 0x0F);            // Get Y value (using lower nibble)
		BOOL useSecondary = ((y % 2) + 2) == 3;          // REST(Y,2) + 2 == 3 means secondary
	    
		if(useSecondary) {
			return (MonsterType)(0x10 | typeIdx);
		}
		return (MonsterType)typeIdx;
	}

	CardCategory GetCardCategory() const {
		// Get low nibble (category data)
		BYTE categoryNibble = (bytes.typeAndCategory & 0x0F);

		// Convert from nibble value to actual category
		if (categoryNibble >= 0x4 && categoryNibble <= 0x7)
			return CATEGORY_EFFECT;
		else if (categoryNibble >= 0x8 && categoryNibble <= 0xB)
			return CATEGORY_FUSION;
		else if (categoryNibble >= 0xC && categoryNibble <= 0xF)
			return CATEGORY_RITUAL;
		else // 0x0-0x3
			return CATEGORY_NORMAL;
	}

	MonsterAttribute GetMonsterAttribute() const {
		BYTE attr = ((bytes.attrAndStars >> 4) & 0x0F);
		return (MonsterAttribute)attr;  // Get upper nibble of attrAndStars
	}

	BYTE GetMonsterStars() const {
		BYTE Y = bytes.attrAndStars & 0x0F;     // Low nibble
		BYTE X = bytes.attrAndStars >> 4;       // High nibble
		return (Y / 2) + ((X % 2) * 8);
	}

	BOOL RequiresTwoTributes() const {
		MonsterAttribute attr = GetMonsterAttribute();
		return (attr == ATTR_DIVINE_BEAST_2_TRIBUTE ||
			attr == ATTR_LIGHT_2_TRIBUTE ||
			attr == ATTR_DARK_2_TRIBUTE ||
			attr == ATTR_WATER_2_TRIBUTE ||
			attr == ATTR_FIRE_2_TRIBUTE ||
			attr == ATTR_EARTH_2_TRIBUTE ||
			attr == ATTR_WIND_2_TRIBUTE ||
			attr == ATTR_UNUSED_2_TRIBUTE);
	}
};

// Simple structures for BIN files
struct CardName {
    char name[0x40];  // Fixed size of 64 bytes
};

struct CardRealId {
    WORD id;  // 2 bytes per entry, first entry is 0xFFFF (dummy for backside)
};

struct CardInternalId {
    WORD id;  // 2 bytes per entry
};

struct CardDescIndex {
    DWORD pointer;  // 4 byte pointer to description
};

struct DialogIndex {
    DWORD pointer;  // 4 byte pointer to dialog text
};

struct CardPack {
    WORD gameFlags;  // 2 bytes for game availability
    WORD padding;    // 2 bytes padding
};

// Not part of the BIN system, but used by it exclusively
struct CardListEntry {
    char imageFilename[MAX_PATH];
};

// Unified card structure
struct Card {
	DWORD cardId;
    CardProperties properties;
    CardName name;
    CardRealId realId;
    CardInternalId internalId;
    CardPack pack;
    const char* description;  // Points to text in m_cardDescriptions buffer
	const char* imageFilename;		// Points to regular image filename
    const char* imageMiniFilename;	// Points to mini image filename
	BOOL hasValidGFX;
};

// Dialog entry structure
struct DialogEntry {
    const char* text;        // Points to text in m_dialogTexts buffer
};

class CBinSystem {
public:
    CBinSystem();
    ~CBinSystem();

    BOOL Initialize(const char* basePath, const char* language);

    // Property getters - keep these the same but modify their implementation
    WORD GetDefenseValue(DWORD cardId);
    WORD GetAttackValue(DWORD cardId);
    MonsterType GetMonsterType(DWORD cardId);
    CardCategory GetCardCategory(DWORD cardId);
    MonsterAttribute GetMonsterAttribute(DWORD cardId);
    BYTE GetMonsterStars(DWORD cardId);
    BOOL RequiresTwoTributes(DWORD cardId);

    // File data getters - keep these the same but modify their implementation
    const char* GetCardName(DWORD cardId);
    WORD GetCardRealId(DWORD internalId);
    WORD GetCardInternalId(DWORD cardId);
    const char* GetCardDescription(DWORD cardId);
    const char* GetDialogText(DWORD dialogId);
    BOOL IsCardAvailable(DWORD cardId, BYTE gameId);
	const char* GetCardImageFilename(DWORD cardId);
	const char* GetCardImageMiniFilename(DWORD cardId);

    const Card* GetCard(DWORD cardId) const {
        if(cardId >= m_cardCount) return NULL;
        return &m_cards[cardId];
    }

	const DialogEntry* GetDialog(DWORD dialogId) const {
        if(dialogId >= m_dialogCount) return NULL;
        return &m_dialogs[dialogId];
    }

    // Alternative if you want direct array access (be careful with this!)
    const Card* GetCards() const { return m_cards; }
    DWORD GetCardCount() const { return m_cardCount; }
	const DialogEntry* GetDialogs() const { return m_dialogs; }
    DWORD GetDialogCount() const { return m_dialogCount; }

private:
    // Loading functions stay the same
    BOOL LoadCardProperties(const char* path);
    BOOL LoadCardNames(const char* path);
    BOOL LoadCardRealIds(const char* path);
    BOOL LoadCardInternalIds(const char* path);
    BOOL LoadCardDescriptions(const char* path, const char* indexPath);
    BOOL LoadDialogTexts(const char* path, const char* indexPath);
    BOOL LoadCardPacks(const char* path);
    BOOL LoadCardList(const char* path, BOOL isMini);

    BYTE* DecompressFile(const char* path, DWORD& outSize);

    // New data storage
    Card* m_cards;					// Main array of cards [m_cardCount]
	CardListEntry* m_cardList;		// card_list (for GFX assignment)
	CardListEntry* m_cardListMini;  // card_list entries but for mini preview GFX
    DialogEntry* m_dialogs;			// Array of dialog entries [m_dialogCount]
    
    // Keep these buffers for text storage
    char* m_cardDescriptions;   // Buffer for all card descriptions
    char* m_dialogTexts;        // Buffer for all dialog texts
    DWORD* m_cardDescIndexes;   // Indexes into m_cardDescriptions
    DWORD* m_dialogIndexes;     // Indexes into m_dialogTexts
    
    // Counters
    DWORD m_cardCount;
    DWORD m_dialogCount;
};

#endif // CBINSYSTEM_H