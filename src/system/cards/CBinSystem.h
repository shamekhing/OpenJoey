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

// Card properties (4 bytes) - Split into individual bytes for MSVC 2003
struct CardPropertiesBytes {
    BYTE defenseModifiers;   // Both DEF modifiers in one byte
    BYTE attackModifiers;    // Both ATK modifiers in one byte
    BYTE typeAndCategory;    // Monster type and card category
    BYTE attrAndStars;      // Attribute/tribute and stars
};

// Main structure that contains the raw data plus helper functions
struct CardProperties {
    CardPropertiesBytes bytes;

    // DEF value (first byte)
    BYTE GetDefenseLow() {
        return bytes.defenseModifiers & 0x0F;
    }
    BYTE GetDefenseHigh() {
        return (bytes.defenseModifiers >> 4) & 0x0F;
    }
    WORD GetDefenseValue() {
        return bytes.defenseModifiers;
    }

    // ATK value (second byte)
    BYTE GetAttackLow() {
        return bytes.attackModifiers & 0x0F;
    }
    BYTE GetAttackHigh() {
        return (bytes.attackModifiers >> 4) & 0x0F;
    }
    WORD GetAttackValue() {
        return bytes.attackModifiers;
    }

    // Monster Type and Category (third byte)
    MonsterType GetMonsterType() {
        return (MonsterType)(bytes.typeAndCategory & 0x0F);
    }
    CardCategory GetCardCategory() {
        return (CardCategory)((bytes.typeAndCategory >> 4) & 0x0F);
    }

    // Attribute and Stars (fourth byte)
    MonsterAttribute GetMonsterAttribute() {
        return (MonsterAttribute)(bytes.attrAndStars & 0x0F);
    }
    MonsterStars GetMonsterStars() {
        return (MonsterStars)((bytes.attrAndStars >> 4) & 0x0F);
    }
    BOOL RequiresTwoTributes() {
        return (bytes.attrAndStars & 0x01) != 0;
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

class CBinSystem {
public:
    CBinSystem();
    ~CBinSystem();

    BOOL Initialize(const char* basePath, const char* language);

    // Property getters
    WORD GetDefenseValue(DWORD cardId);
    WORD GetAttackValue(DWORD cardId);
    MonsterType GetMonsterType(DWORD cardId);
    CardCategory GetCardCategory(DWORD cardId);
    MonsterAttribute GetMonsterAttribute(DWORD cardId);
    MonsterStars GetMonsterStars(DWORD cardId);
    BOOL RequiresTwoTributes(DWORD cardId);

    // File data getters
    const char* GetCardName(DWORD cardId);
    WORD GetCardRealId(DWORD internalId);
    WORD GetCardInternalId(DWORD cardId);
    const char* GetCardDescription(DWORD cardId);
    const char* GetDialogText(DWORD dialogId);
    BOOL IsCardAvailable(DWORD cardId, BYTE gameId); // gameId: 1=Yugi, 2=Kaiba, 4=Joey

private:
    // Loading functions
    BOOL LoadCardProperties(const char* path);
    BOOL LoadCardNames(const char* path);
    BOOL LoadCardRealIds(const char* path);
    BOOL LoadCardInternalIds(const char* path);
    BOOL LoadCardDescriptions(const char* path, const char* indexPath);
    BOOL LoadDialogTexts(const char* path, const char* indexPath);
    BOOL LoadCardPacks(const char* path);
    
    BYTE* DecompressFile(const char* path, DWORD& outSize);

    // Data storage
    CardProperties* m_cardProps;
    CardName* m_cardNames;
    CardRealId* m_cardRealIds;
    CardInternalId* m_cardInternalIds;
    CardPack* m_cardPacks;
    
    // Description and dialog data
    char* m_cardDescriptions;
    DWORD* m_cardDescIndexes;
    char* m_dialogTexts;
    DWORD* m_dialogIndexes;
    
    // Counters
    DWORD m_cardCount;
    DWORD m_dialogCount;
};

#endif // CBINSYSTEM_H