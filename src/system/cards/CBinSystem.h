#ifndef CBINSYSTEM_H
#define CBINSYSTEM_H

#include "../../stdafx.h"

// Card property enums
enum MonsterType {
    TYPE_NONE = 0,
    TYPE_DRAGON = 1,
    TYPE_ZOMBIE = 2,
    TYPE_FIEND = 3,
    TYPE_PYRO = 4,
    TYPE_SEASERPENT = 5,
    TYPE_ROCK = 6,
    TYPE_MACHINE = 7,
    TYPE_FISH = 8,
    TYPE_DINOSAUR = 9,
    TYPE_INSECT = 0xA,
    TYPE_BEAST = 0xB,
    TYPE_BEASTWARRIOR = 0xC,
    TYPE_PLANT = 0xD,
    TYPE_WARRIOR = 0xF
};

enum CardCategory {
    CATEGORY_NORMAL_0 = 0,
    CATEGORY_NORMAL_1 = 1,
    CATEGORY_NORMAL_2 = 2,
    CATEGORY_NORMAL_3 = 3,
    CATEGORY_EFFECT_4 = 4,
    CATEGORY_EFFECT_5 = 5,
    CATEGORY_EFFECT_6 = 6,
    CATEGORY_EFFECT_7 = 7,
    CATEGORY_FUSION_8 = 8,
    CATEGORY_FUSION_9 = 9,
    CATEGORY_FUSION_A = 0xA,
    CATEGORY_FUSION_B = 0xB,
    CATEGORY_RITUAL_C = 0xC,
    CATEGORY_RITUAL_D = 0xD,
    CATEGORY_RITUAL_E = 0xE,
    CATEGORY_RITUAL_F = 0xF
};

enum MonsterAttribute {
    ATTR_DIVINE_BEAST_NO_TRIBUTE = 0,
    ATTR_DIVINE_BEAST_2_TRIBUTE = 1,
    ATTR_LIGHT_NO_TRIBUTE = 2,
    ATTR_LIGHT_2_TRIBUTE = 3,
    ATTR_DARK_NO_TRIBUTE = 4,
    ATTR_DARK_2_TRIBUTE = 5,
    ATTR_WATER_NO_TRIBUTE = 6,
    ATTR_WATER_2_TRIBUTE = 7,
    ATTR_FIRE_NO_TRIBUTE = 8,
    ATTR_FIRE_2_TRIBUTE = 9,
    ATTR_EARTH_NO_TRIBUTE = 0xA,
    ATTR_EARTH_2_TRIBUTE = 0xB,
    ATTR_WIND_NO_TRIBUTE = 0xC,
    ATTR_WIND_2_TRIBUTE = 0xD
};

enum MonsterStars {
    STARS_1 = 0xA,   // 1 Star
    STARS_2 = 0xC,   // 2 Stars
    STARS_3 = 0xE,   // 3 Stars
    STARS_8 = 0x0,   // 8 Stars
    STARS_9 = 0x2,   // 9 Stars
    STARS_10 = 0x4,  // 10 Stars
    STARS_11 = 0x6,  // 11 Stars
    STARS_12 = 0x8   // 12 Stars
};

struct CardPropertiesBytes {
    BYTE defenseModifiers;   // Position 1-2: DEF
    BYTE attackModifiers;    // Position 3-4: ATK
    BYTE typeAndCategory;    // Position 5-6: Type & Category
    BYTE attrAndStars;      // Position 7-8: Attribute & Stars
};

struct CardProperties {
	CardPropertiesBytes bytes;
	// Special thanks to @github.com/kiemkhach for his Excel table with correct formulas
	WORD GetAttackValue() const {
		// Split bytes into same nibbles as Excel
		BYTE P = bytes.attackModifiers >> 4;     // High ATK nibble (14 for Jinzo)
		BYTE Q = bytes.attackModifiers & 0x0F;   // Low ATK nibble (0 for Jinzo)
		BYTE S = bytes.typeAndCategory & 0x0F;   // Low type nibble (5 for Jinzo)

		// Excel formula: P*80 + (Q - REST(Q,2))*5 + 1280*REST(S,4)
		return (P * 80) +                   // 14 * 80 = 1120
			((Q - (Q % 2)) * 5) +			// (0 - (0 % 2)) * 5 = 0
			(1280 * (S % 4));				// 1280 * (5 % 4) = 1280 * 1 = 1280
		// Example Total for id 428: 1120 + 0 + 1280 = 2400
	}

	WORD GetDefenseValue() const {
		// Split bytes into same nibbles as Excel
		BYTE M = bytes.defenseModifiers >> 4;     // High DEF nibble (9 for Jinzo)
		BYTE N = bytes.defenseModifiers & 0x0F;   // Low DEF nibble (6 for Jinzo)
		BYTE Q = bytes.attackModifiers & 0x0F;    // Low ATK nibble (0 for Jinzo)

		// Excel formula: M*160 + N*10 + REST(Q,2)*2560
		return (M * 160) +                  // 9 * 160 = 1440
			(N * 10) +						// 6 * 10 = 60
			((Q % 2) * 2560);				// (0 % 2) * 2560 = 0
		// Example Total id 428: 1440 + 60 + 0 = 1500
	}

    // Rest stays the same
    MonsterType GetMonsterType() const {
        return (MonsterType)(bytes.typeAndCategory & 0x0F);
    }

    CardCategory GetCardCategory() const {
        return (CardCategory)((bytes.typeAndCategory >> 4) & 0x0F);
    }

    MonsterAttribute GetMonsterAttribute() const {
        return (MonsterAttribute)(bytes.attrAndStars & 0x0F);
    }

    MonsterStars GetMonsterStars() const {
        return (MonsterStars)((bytes.attrAndStars >> 4) & 0x0F);
    }

    BOOL RequiresTwoTributes() const {
        BYTE stars = (bytes.attrAndStars >> 4) & 0x0F;
        return stars >= 7;
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

// First part stays the same until the class declaration...

// New unified card structure
struct Card {
	DWORD cardId;
    CardProperties properties;
    CardName name;
    CardRealId realId;
    CardInternalId internalId;
    CardPack pack;
    const char* description;  // Points to text in m_cardDescriptions buffer
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
    MonsterStars GetMonsterStars(DWORD cardId);
    BOOL RequiresTwoTributes(DWORD cardId);

    // File data getters - keep these the same but modify their implementation
    const char* GetCardName(DWORD cardId);
    WORD GetCardRealId(DWORD internalId);
    WORD GetCardInternalId(DWORD cardId);
    const char* GetCardDescription(DWORD cardId);
    const char* GetDialogText(DWORD dialogId);
    BOOL IsCardAvailable(DWORD cardId, BYTE gameId);

    const Card* GetCard(DWORD cardId) const {
        if(cardId >= m_cardCount) return NULL;
        return &m_cards[cardId];
    }

    // Alternative if you want direct array access (be careful with this!)
    const Card* GetCards() const { return m_cards; }
    DWORD GetCardCount() const { return m_cardCount; }

private:
    // Loading functions stay the same
    BOOL LoadCardProperties(const char* path);
    BOOL LoadCardNames(const char* path);
    BOOL LoadCardRealIds(const char* path);
    BOOL LoadCardInternalIds(const char* path);
    BOOL LoadCardDescriptions(const char* path, const char* indexPath);
    BOOL LoadDialogTexts(const char* path, const char* indexPath);
    BOOL LoadCardPacks(const char* path);
    
    BYTE* DecompressFile(const char* path, DWORD& outSize);

    // New data storage
    Card* m_cards;              // Main array of cards [m_cardCount]
    DialogEntry* m_dialogs;     // Array of dialog entries [m_dialogCount]
    
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