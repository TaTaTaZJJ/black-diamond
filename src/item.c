#include "global.h"
#include "item.h"
#include "party_menu.h"
#include "berry.h"
#include "string_util.h"
#include "text.h"
#include "event_data.h"
#include "malloc.h"
#include "secret_base.h"
#include "item_menu.h"
#include "strings.h"
#include "load_save.h"
#include "pokemon_summary_screen.h"
#include "item_use.h"
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "constants/items.h"
#include "constants/hold_effects.h"

extern u16 gUnknown_0203CF30[];

// this file's functions
static bool8 CheckPyramidBagHasItem(u16 itemId, u16 count);
static bool8 CheckPyramidBagHasSpace(u16 itemId, u16 count);

// EWRAM variables
EWRAM_DATA struct BagPocket gBagPockets[POCKETS_COUNT] = {0};
EWRAM_DATA struct ItemSlot gKeyItemSlots[BAG_KEYITEMS_COUNT] = {0};
EWRAM_DATA struct ItemSlot gExplorationKitSlots[BAG_EXPLORATIONKIT_COUNT] = {0};
EWRAM_DATA struct ItemSlot gTmHmItemSlots[BAG_TMHM_COUNT] = {0};
EWRAM_DATA struct ItemSlot gCostumeSlots[BAG_COSTUMES_COUNT] = {0};
EWRAM_DATA struct ItemSlot gZCrystalSlots[BAG_ZCRYSTALS_COUNT] = {0};

// rodata
#include "data/text/item_descriptions.h"
#include "data/items.h"

// 背包8位slots定义
static const u16 sKeyItemList[BAG_KEYITEMS_COUNT] = {
    ITEM_MEGA_BRACELET,
    ITEM_SHINY_CHARM,
    ITEM_OVAL_CHARM,
    ITEM_Z_RING,
    ITEM_GRACIDEA,
    // 船票
    ITEM_SS_TICKET,
    ITEM_MYSTIC_TICKET,
    ITEM_AURORA_TICKET,
    ITEM_EON_TICKET,
    ITEM_CONTEST_PASS,
    ITEM_TRI_PASS,
    ITEM_RAINBOW_PASS,
    ITEM_OLD_SEA_MAP,
    // 钥匙
    ITEM_BASEMENT_KEY,
    ITEM_ROOM_1_KEY,
    ITEM_ROOM_2_KEY,
    ITEM_ROOM_4_KEY,
    ITEM_ROOM_6_KEY,
    ITEM_STORAGE_KEY,
    ITEM_CARD_KEY, 
    ITEM_LIFT_KEY, 
    ITEM_SECRET_KEY,
    // 交换用
    ITEM_DEVON_GOODS,
    ITEM_BIKE_VOUCHER,
    ITEM_LETTER, 
    ITEM_SCANNER,
    ITEM_METEORITE,
    ITEM_TEA, 
};

static const u16 sExplorationKitList[BAG_EXPLORATIONKIT_COUNT] = {
    ITEM_ESCAPE_ROPE,
    ITEM_WAILMER_PAIL,
    ITEM_MACH_BIKE,
    ITEM_ACRO_BIKE,
    ITEM_OLD_ROD,
    ITEM_GOOD_ROD,
    ITEM_SUPER_ROD,
    ITEM_ITEMFINDER,
    ITEM_DEVON_SCOPE,
    ITEM_SILPH_SCOPE,
    ITEM_SOOT_SACK,
    ITEM_GO_GOGGLES,
    ITEM_VS_SEEKER,
    ITEM_FAME_CHECKER,
    ITEM_POKE_FLUTE,
    ITEM_POKEBLOCK_CASE,
    ITEM_POWDER_JAR,
    ITEM_COIN_CASE,
};

static const u16 sCostumeList[BAG_COSTUMES_COUNT] = {};

static const u16 sZCrystalList[BAG_ZCRYSTALS_COUNT] = {
    ITEM_NORMALIUM_Z,
    ITEM_FIGHTINIUM_Z,
    ITEM_FLYINIUM_Z,
    ITEM_POISONIUM_Z,
    ITEM_GROUNDIUM_Z,
    ITEM_ROCKIUM_Z,
    ITEM_BUGINIUM_Z,
    ITEM_GHOSTIUM_Z,
    ITEM_STEELIUM_Z,
    ITEM_FIRIUM_Z,
    ITEM_WATERIUM_Z,
    ITEM_GRASSIUM_Z,
    ITEM_ELECTRIUM_Z,
    ITEM_PSYCHIUM_Z,
    ITEM_ICIUM_Z,
    ITEM_DRAGONIUM_Z,
    ITEM_DARKINIUM_Z,
    ITEM_FAIRIUM_Z,
    ITEM_ALORAICHIUM_Z,
    ITEM_DECIDIUM_Z,
    ITEM_EEVIUM_Z,
    ITEM_INCINIUM_Z,
    ITEM_KOMMONIUM_Z,
    ITEM_LUNALIUM_Z,
    ITEM_LYCANIUM_Z,
    ITEM_MARSHADIUM_Z,
    ITEM_MEWNIUM_Z,
    ITEM_MIMIKIUM_Z,
    ITEM_PIKANIUM_Z,
    ITEM_PIKASHUNIUM_Z,
    ITEM_PRIMARIUM_Z,
    ITEM_SNORLIUM_Z,
    ITEM_SOLGANIUM_Z,
    ITEM_TAPUNIUM_Z,
    ITEM_ULTRANECROZIUM_Z,
};

// code
static u16 GetBagItemQuantity(u8 *quantity)
{
    // return gSaveBlock2Ptr->encryptionKey ^ *quantity; // 原版的加密机制
    return *quantity;
}

static void SetBagItemQuantity(u8 *quantity, u8 newValue)
{
    // *quantity =  newValue ^ gSaveBlock2Ptr->encryptionKey;
    *quantity = newValue;
}

static u16 GetPCItemQuantity(u16 *quantity)
{
    return *quantity;
}

static void SetPCItemQuantity(u16 *quantity, u16 newValue)
{
    *quantity = newValue;
}

void ApplyNewEncryptionKeyToBagItems(u32 newKey) // 原版的加密机制
{
    // u32 pocket, item;
    // for (pocket = 0; pocket < POCKETS_COUNT; pocket++)
    // {
    //     for (item = 0; item < gBagPockets[pocket].capacity; item++)
    //         ApplyNewEncryptionKeyToHword(&(gBagPockets[pocket].itemSlots[item].quantity), newKey);
    // }
}

void ApplyNewEncryptionKeyToBagItems_(u32 newKey) // really GF?
{
    ApplyNewEncryptionKeyToBagItems(newKey);
}

void Deserialize8BitItemSlots(void) // 解压8位储存的道具
{
    int i;
    u8 bit;
    u16 itemId;

    // TM
    for (i = 0; i < BAG_TMHM_COUNT; ++i)
    {
        gTmHmItemSlots[i].itemId = 0;
        SetBagItemQuantity(&(gTmHmItemSlots[i].quantity), 0);
    }
    for (i = 0; i < TM_COUNT; ++i)
    {
        bit = i % 8;
        if (gSaveBlock1Ptr->bagPocket_TMHMs[i / 8] & (1<<bit))
        {    
            AddBagItem(i + ITEM_TM_START, 1);
        }
    }

    // 重要道具
    for (i = 0; i < BAG_KEYITEMS_COUNT; i++)
    {
        gKeyItemSlots[i].itemId = 0;
        SetBagItemQuantity(&(gKeyItemSlots[i].quantity), 0);
        itemId = sKeyItemList[i];
        if (itemId != 0)
        {
            bit = i % 8;
            if (gSaveBlock1Ptr->bagPocket_KeyItems[i / 8] & (1<<bit))
            {
                AddBagItem(itemId, 1);
            }    
        }
    }

    // 探索道具
    for (i = 0; i < BAG_EXPLORATIONKIT_COUNT; i++)
    {
        gExplorationKitSlots[i].itemId = 0;
        SetBagItemQuantity(&(gExplorationKitSlots[i].quantity), 0);
        itemId = sExplorationKitList[i];
        if (itemId != 0)
        {
            bit = i % 8;
            if (gSaveBlock1Ptr->bagPocket_ExplorationKit[i / 8] & (1<<bit))
            {
                AddBagItem(itemId, 1);
            }    
        }
    }

    // 服装
    for (i = 0; i < BAG_COSTUMES_COUNT; i++)
    {
        gCostumeSlots[i].itemId = 0;
        SetBagItemQuantity(&(gCostumeSlots[i].quantity), 0);
        itemId = sCostumeList[i];
        if (itemId != 0)
        {
            bit = i % 8;
            if (gSaveBlock1Ptr->bagPocket_Costume[i / 8] & (1<<bit))
            {
                AddBagItem(itemId, 1);
            }    
        }
    }

    // z纯晶
    for (i = 0; i < BAG_ZCRYSTALS_COUNT; i++)
    {
        gZCrystalSlots[i].itemId = 0;
        SetBagItemQuantity(&(gZCrystalSlots[i].quantity), 0);
        itemId = sZCrystalList[i];
        if (itemId != 0)
        {
            bit = i % 8;
            if (gSaveBlock1Ptr->bagPocket_ZCrystal[i / 8] & (1<<bit))
            {
                AddBagItem(itemId, 1);
            }    
        }
    }
}

void SetBagItemsPointers(void)
{
    gBagPockets[MEDICINE_POCKET].itemSlots = gSaveBlock1Ptr->bagPocket_Medicine;
    gBagPockets[MEDICINE_POCKET].capacity = BAG_MEDICINE_COUNT;

    gBagPockets[BALLS_POCKET].itemSlots = gSaveBlock1Ptr->bagPocket_PokeBalls;
    gBagPockets[BALLS_POCKET].capacity = BAG_POKEBALLS_COUNT;

    gBagPockets[BATTLEITEMS_POCKET].itemSlots = gSaveBlock1Ptr->bagPocket_BattleItems;
    gBagPockets[BATTLEITEMS_POCKET].capacity = BAG_BATTLEITEMS_COUNT;

    gBagPockets[EXPLORATIONKIT_POCKET].itemSlots = &gExplorationKitSlots[0];
    gBagPockets[EXPLORATIONKIT_POCKET].capacity = BAG_EXPLORATIONKIT_COUNT;

    gBagPockets[BERRIES_POCKET].itemSlots = gSaveBlock1Ptr->bagPocket_Berries;
    gBagPockets[BERRIES_POCKET].capacity = BAG_BERRIES_COUNT;

    gBagPockets[KEYITEMS_POCKET].itemSlots = &gKeyItemSlots[0];
    gBagPockets[KEYITEMS_POCKET].capacity = BAG_KEYITEMS_COUNT;

    gBagPockets[COLLECTION_POCKET].itemSlots = gSaveBlock1Ptr->bagPocket_Collection;
    gBagPockets[COLLECTION_POCKET].capacity = BAG_COLLECTION_COUNT;

    gBagPockets[TMHM_POCKET].itemSlots = &gTmHmItemSlots[0];
    gBagPockets[TMHM_POCKET].capacity = BAG_TMHM_COUNT;

    gBagPockets[COSTUMES_POCKET].itemSlots = &gCostumeSlots[0];
    gBagPockets[COSTUMES_POCKET].capacity = BAG_COSTUMES_COUNT;

    gBagPockets[ZCRYSTALS_POCKET].itemSlots = &gZCrystalSlots[0];
    gBagPockets[ZCRYSTALS_POCKET].capacity = BAG_ZCRYSTALS_COUNT;

    gBagPockets[MEGASTONES_POCKET].itemSlots = gSaveBlock1Ptr->bagPocket_MegaStones;
    gBagPockets[MEGASTONES_POCKET].capacity = BAG_MEGASTONE_COUNT;
}

void CopyItemName(u16 itemId, u8 *dst)
{
    StringCopy(dst, ItemId_GetName(itemId));
}

void CopyItemNameHandlePlural(u16 itemId, u8 *dst, u32 quantity)
{
    if (itemId == ITEM_POKE_BALL)
    {
        if (quantity < 2)
            StringCopy(dst, ItemId_GetName(ITEM_POKE_BALL));
        else
            StringCopy(dst, gText_PokeBalls);
    }
    else
    {
        if (itemId >= FIRST_BERRY_INDEX && itemId <= LAST_BERRY_INDEX)
            GetBerryCountString(dst, gBerries[itemId - FIRST_BERRY_INDEX].name, quantity);
        else
            StringCopy(dst, ItemId_GetName(itemId));
    }
}

void GetBerryCountString(u8 *dst, const u8 *berryName, u32 quantity)
{
    const u8 *berryString;
    u8 *txtPtr;

    if (quantity < 2)
        berryString = gText_Berry;
    else
        berryString = gText_Berries;

    txtPtr = StringCopy(dst, berryName);
    *txtPtr = CHAR_SPACE;
    StringCopy(txtPtr + 1, berryString);
}

bool8 IsBagPocketNonEmpty(u8 pocket)
{
    u16 i;

    for (i = 0; i < gBagPockets[pocket - 1].capacity; i++)
    {
        if (gBagPockets[pocket - 1].itemSlots[i].itemId != 0)
            return TRUE;
    }
    return FALSE;
}

bool8 CheckBagHasItem(u16 itemId, u16 count)
{
    u16 i;
    u8 pocket;

    if (ItemId_GetPocket(itemId) == 0)
        return FALSE;
    if (InBattlePyramid() || FlagGet(FLAG_STORING_ITEMS_IN_PYRAMID_BAG) == TRUE)
        return CheckPyramidBagHasItem(itemId, count);
    pocket = ItemId_GetPocket(itemId) - 1;
    // Check for item slots that contain the item
    for (i = 0; i < gBagPockets[pocket].capacity; i++)
    {
        if (gBagPockets[pocket].itemSlots[i].itemId == itemId)
        {
            u16 quantity;
            // Does this item slot contain enough of the item?
            quantity = GetBagItemQuantity(&gBagPockets[pocket].itemSlots[i].quantity);
            if (quantity >= count)
                return TRUE;
            count -= quantity;
            // Does this item slot and all previous slots contain enough of the item?
            if (count == 0)
                return TRUE;
        }
    }
    return FALSE;
}

bool8 HasAtLeastOneBerry(void)
{
    u16 i;

    for (i = FIRST_BERRY_INDEX; i < ITEM_BRIGHT_POWDER; i++)
    {
        if (CheckBagHasItem(i, 1) == TRUE)
        {
            gSpecialVar_Result = TRUE;
            return TRUE;
        }
    }
    gSpecialVar_Result = FALSE;
    return FALSE;
}

bool8 CheckBagHasSpace(u16 itemId, u16 count)
{
    u16 i;
    u8 pocket;
    u16 slotCapacity;
    u16 ownedCount;

    if (ItemId_GetPocket(itemId) == POCKET_NONE)
        return FALSE;

    if (InBattlePyramid() || FlagGet(FLAG_STORING_ITEMS_IN_PYRAMID_BAG) == TRUE)
    {
        return CheckPyramidBagHasSpace(itemId, count);
    }

    pocket = ItemId_GetPocket(itemId) - 1;
    if (pocket != BERRIES_POCKET)
        slotCapacity = MAX_BAG_ITEM_CAPACITY;
    else
        slotCapacity = MAX_BERRY_CAPACITY;

    // Check space in any existing item slots that already contain this item
    for (i = 0; i < gBagPockets[pocket].capacity; i++)
    {
        if (gBagPockets[pocket].itemSlots[i].itemId == itemId)
        {
            ownedCount = GetBagItemQuantity(&gBagPockets[pocket].itemSlots[i].quantity);
            if (ownedCount + count <= slotCapacity)
                return TRUE;
            if (pocket == TMHM_POCKET 
                || pocket == BERRIES_POCKET 
                || pocket == EXPLORATIONKIT_POCKET 
                || pocket == COSTUMES_POCKET
                || pocket == ZCRYSTALS_POCKET
                || pocket == KEYITEMS_POCKET)
                return FALSE;
            count -= (slotCapacity - ownedCount);
            if (count == 0)
                return TRUE;
        }
    }

    // Check space in empty item slots
    if (count > 0)
    {
        for (i = 0; i < gBagPockets[pocket].capacity; i++)
        {
            if (gBagPockets[pocket].itemSlots[i].itemId == 0)
            {
                if (count > slotCapacity)
                {
                    if (pocket == TMHM_POCKET || pocket == BERRIES_POCKET)
                        return FALSE;
                    count -= slotCapacity;
                }
                else
                {
                    count = 0; //should be return TRUE, but that doesn't match
                    return TRUE;
                }
            }
        }
        if (count > 0)
            return FALSE; // No more item slots. The bag is full
    }

    return TRUE;
}

static void set1BitItemOwned(u16 itemId, u8 pocket)
{   
    int i;
    u8* flagByte;
    switch (pocket)
    {
    case EXPLORATIONKIT_POCKET:
        for (i = 0; i < BAG_EXPLORATIONKIT_COUNT; i++)
        {
            if (sExplorationKitList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_ExplorationKit[i / 8];
                *flagByte |= (1 << (i % 8));
                break;
            }
        }
        break;
    case TMHM_POCKET:
        i = itemId - ITEM_TM_START;
        flagByte = &gSaveBlock1Ptr->bagPocket_TMHMs[i / 8];
        *flagByte |= (1 << (i % 8));
        break;
    case KEYITEMS_POCKET:
        for (i = 0; i < BAG_KEYITEMS_COUNT; i++)
        {
            if (sKeyItemList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_KeyItems[i / 8];
                *flagByte |= (1 << (i % 8));
                break;
            }
        }
        break;
    case COSTUMES_POCKET:
        for (i = 0; i < BAG_COSTUMES_COUNT; i++)
        {
            if (sCostumeList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_Costume[i / 8];
                *flagByte |= (1 << (i % 8));
                break;
            }
        }
        break;
    case ZCRYSTALS_POCKET:
        for (i = 0; i < BAG_ZCRYSTALS_COUNT; i++)
        {
            if (sZCrystalList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_ZCrystal[i / 8];
                *flagByte |= (1 << (i % 8));
                break;
            }
        }
        break;
    }
}

static void unset1BitItemOwned(u16 itemId, u8 pocket)
{   
    int i;
    u8* flagByte;
    switch (pocket)
     {
    case EXPLORATIONKIT_POCKET:
        for (i = 0; i < BAG_EXPLORATIONKIT_COUNT; i++)
        {
            if (sExplorationKitList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_ExplorationKit[i / 8];
                *flagByte &= ~(1 << (i % 8));
                break;
            }
        }
        break;
    case TMHM_POCKET:
        i = itemId - ITEM_TM_START;
        flagByte = &gSaveBlock1Ptr->bagPocket_TMHMs[i / 8];
        *flagByte &= ~(1 << (i % 8));
        break;
    case KEYITEMS_POCKET:
        for (i = 0; i < BAG_KEYITEMS_COUNT; i++)
        {
            if (sKeyItemList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_KeyItems[i / 8];
                *flagByte &= ~(1 << (i % 8));
                break;
            }
        }
        break;
    case COSTUMES_POCKET:
        for (i = 0; i < BAG_COSTUMES_COUNT; i++)
        {
            if (sCostumeList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_Costume[i / 8];
                *flagByte &= ~(1 << (i % 8));
                break;
            }
        }
        break;
    case ZCRYSTALS_POCKET:
        for (i = 0; i < BAG_ZCRYSTALS_COUNT; i++)
        {
            if (sZCrystalList[i] == itemId)
            {
                flagByte = &gSaveBlock1Ptr->bagPocket_ZCrystal[i / 8];
                *flagByte &= ~(1 << (i % 8));
                break;
            }
        }
        break;
    }
}

bool8 AddBagItem(u16 itemId, u16 count)
{
    u16 i;
    u8 pocket = ItemId_GetPocket(itemId) - 1;

    if (ItemId_GetPocket(itemId) == POCKET_NONE)
        return FALSE;

    // check Battle Pyramid Bag
    if (InBattlePyramid() || FlagGet(FLAG_STORING_ITEMS_IN_PYRAMID_BAG) == TRUE)
    {
        return AddPyramidBagItem(itemId, count);
    }
    else
    {
        struct BagPocket *itemPocket;
        struct ItemSlot *newItems;
        u16 slotCapacity;
        u16 ownedCount;

        itemPocket = &gBagPockets[pocket];
        newItems = AllocZeroed(itemPocket->capacity * sizeof(struct ItemSlot));
        memcpy(newItems, itemPocket->itemSlots, itemPocket->capacity * sizeof(struct ItemSlot));

        switch(pocket)
        {
            case BERRIES_POCKET:
                slotCapacity = 999;
            break;
            case TMHM_POCKET:
                slotCapacity = 1;
            break;
            default:
                slotCapacity = 99;
            break;
        }

        for (i = 0; i < itemPocket->capacity; i++)
        {
            if (newItems[i].itemId == itemId)
            {
                ownedCount = GetBagItemQuantity(&newItems[i].quantity);
                // check if won't exceed max slot capacity
                if (ownedCount + count <= slotCapacity)
                {
                    // successfully added to already existing item's count
                    SetBagItemQuantity(&newItems[i].quantity, ownedCount + count);
                    memcpy(itemPocket->itemSlots, newItems, itemPocket->capacity * sizeof(struct ItemSlot));
                    Free(newItems);
                    return TRUE;
                }
                else
                {
                    // try creating another instance of the item if possible
                    if (pocket == TMHM_POCKET 
                        || pocket == BERRIES_POCKET 
                        || pocket == EXPLORATIONKIT_POCKET 
                        || pocket == COSTUMES_POCKET
                        || pocket == ZCRYSTALS_POCKET
                        || pocket == KEYITEMS_POCKET)
                    {
                        Free(newItems);
                        return FALSE;
                    }
                    else
                    {
                        count -= slotCapacity - ownedCount;
                        SetBagItemQuantity(&newItems[i].quantity, slotCapacity);
                        // don't create another instance of the item if it's at max slot capacity and count is equal to 0
                        if (count == 0)
                        {
                            break;
                        }
                    }
                }
            }
        }

        // we're done if quantity is equal to 0
        if (count > 0)
        {
            // either no existing item was found or we have to create another instance, because the capacity was exceeded
            for (i = 0; i < itemPocket->capacity; i++)
            {
                if (newItems[i].itemId == ITEM_NONE)
                {
                    newItems[i].itemId = itemId;
                    if (count > slotCapacity)
                    {
                        // try creating a new slot with max capacity if duplicates are possible
                        if (pocket == TMHM_POCKET 
                            || pocket == BERRIES_POCKET 
                            || pocket == EXPLORATIONKIT_POCKET 
                            || pocket == COSTUMES_POCKET
                            || pocket == ZCRYSTALS_POCKET
                            || pocket == KEYITEMS_POCKET)
                        {
                            Free(newItems);
                            return FALSE;
                        }
                        count -= slotCapacity;
                        SetBagItemQuantity(&newItems[i].quantity, slotCapacity);
                    }
                    else
                    {
                        // created a new slot and added quantity
                        SetBagItemQuantity(&newItems[i].quantity, count);
                        if (pocket == TMHM_POCKET 
                            || pocket == EXPLORATIONKIT_POCKET 
                            || pocket == COSTUMES_POCKET
                            || pocket == ZCRYSTALS_POCKET
                            || pocket == KEYITEMS_POCKET)
                        {
                            set1BitItemOwned(itemId, pocket);
                        }
                        count = 0;
                        break;
                    }
                }
            }

            if (count > 0)
            {
                Free(newItems);
                return FALSE;
            }
        }
        memcpy(itemPocket->itemSlots, newItems, itemPocket->capacity * sizeof(struct ItemSlot));
        Free(newItems);
        return TRUE;
    }
}

bool8 RemoveBagItem(u16 itemId, u16 count)
{
    u16 i;
    u16 totalQuantity = 0;

    if (ItemId_GetPocket(itemId) == POCKET_NONE || itemId == ITEM_NONE)
        return FALSE;

    // check Battle Pyramid Bag
    if (InBattlePyramid() || FlagGet(FLAG_STORING_ITEMS_IN_PYRAMID_BAG) == TRUE)
    {
        return RemovePyramidBagItem(itemId, count);
    }
    else
    {
        u8 pocket;
        u8 var;
        u16 ownedCount;
        struct BagPocket *itemPocket;

        pocket = ItemId_GetPocket(itemId) - 1;
        itemPocket = &gBagPockets[pocket];

        for (i = 0; i < itemPocket->capacity; i++)
        {
            if (itemPocket->itemSlots[i].itemId == itemId)
                totalQuantity += GetBagItemQuantity(&itemPocket->itemSlots[i].quantity);
        }

        if (totalQuantity < count)
            return FALSE;   // We don't have enough of the item

        if (CurMapIsSecretBase() == TRUE)
        {
            VarSet(VAR_SECRET_BASE_LOW_TV_FLAGS, VarGet(VAR_SECRET_BASE_LOW_TV_FLAGS) | SECRET_BASE_USED_BAG);
            VarSet(VAR_SECRET_BASE_LAST_ITEM_USED, itemId);
        }

        var = GetItemListPosition(pocket);
        if (itemPocket->capacity > var
         && itemPocket->itemSlots[var].itemId == itemId)
        {
            ownedCount = GetBagItemQuantity(&itemPocket->itemSlots[var].quantity);
            if (ownedCount >= count)
            {
                SetBagItemQuantity(&itemPocket->itemSlots[var].quantity, ownedCount - count);
                count = 0;
            }
            else
            {
                count -= ownedCount;
                SetBagItemQuantity(&itemPocket->itemSlots[var].quantity, 0);
            }

            if (GetBagItemQuantity(&itemPocket->itemSlots[var].quantity) == 0)
                itemPocket->itemSlots[var].itemId = ITEM_NONE;

            if (pocket == TMHM_POCKET 
                || pocket == EXPLORATIONKIT_POCKET 
                || pocket == COSTUMES_POCKET
                || pocket == ZCRYSTALS_POCKET
                || pocket == KEYITEMS_POCKET)
            {
                unset1BitItemOwned(itemId, pocket);
            }

            if (count == 0)
                return TRUE;
        }

        for (i = 0; i < itemPocket->capacity; i++)
        {
            if (itemPocket->itemSlots[i].itemId == itemId)
            {
                ownedCount = GetBagItemQuantity(&itemPocket->itemSlots[i].quantity);
                if (ownedCount >= count)
                {
                    SetBagItemQuantity(&itemPocket->itemSlots[i].quantity, ownedCount - count);
                    count = 0;
                }
                else
                {
                    count -= ownedCount;
                    SetBagItemQuantity(&itemPocket->itemSlots[i].quantity, 0);
                }

                if (GetBagItemQuantity(&itemPocket->itemSlots[i].quantity) == 0)
                    itemPocket->itemSlots[i].itemId = ITEM_NONE;

                if (pocket == TMHM_POCKET 
                    || pocket == EXPLORATIONKIT_POCKET 
                    || pocket == COSTUMES_POCKET
                    || pocket == ZCRYSTALS_POCKET
                    || pocket == KEYITEMS_POCKET)
                {
                    unset1BitItemOwned(itemId, pocket);
                }
                
                if (count == 0)
                    return TRUE;
            }
        }
        return TRUE;
    }
}

u8 GetPocketByItemId(u16 itemId)
{
    return ItemId_GetPocket(itemId);
}

void ClearItemSlots(struct ItemSlot *itemSlots, u8 itemCount)
{
    u16 i;

    for (i = 0; i < itemCount; i++)
    {
        itemSlots[i].itemId = ITEM_NONE;
        SetBagItemQuantity(&itemSlots[i].quantity, 0);
    }
}

void ClearPcItemSlots(struct PcItemSlot *itemSlots, u8 itemCount)
{
    u16 i;

    for (i = 0; i < itemCount; i++)
    {
        itemSlots[i].itemId = ITEM_NONE;
        SetPCItemQuantity(&itemSlots[i].quantity, 0);
    }
}

static s32 FindFreePCItemSlot(void)
{
    s8 i;

    for (i = 0; i < PC_ITEMS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->pcItems[i].itemId == ITEM_NONE)
            return i;
    }
    return -1;
}

u8 CountUsedPCItemSlots(void)
{
    u8 usedSlots = 0;
    u8 i;

    for (i = 0; i < PC_ITEMS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->pcItems[i].itemId != ITEM_NONE)
            usedSlots++;
    }
    return usedSlots;
}

bool8 CheckPCHasItem(u16 itemId, u16 count)
{
    u8 i;

    for (i = 0; i < PC_ITEMS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->pcItems[i].itemId == itemId && GetPCItemQuantity(&gSaveBlock1Ptr->pcItems[i].quantity) >= count)
            return TRUE;
    }
    return FALSE;
}

bool8 AddPCItem(u16 itemId, u16 count)
{
    u8 i;
    s8 freeSlot;
    u16 ownedCount;
    struct PcItemSlot *newItems;

    // Copy PC items
    newItems = AllocZeroed(sizeof(gSaveBlock1Ptr->pcItems));
    memcpy(newItems, gSaveBlock1Ptr->pcItems, sizeof(gSaveBlock1Ptr->pcItems));

    // Use any item slots that already contain this item
    for (i = 0; i < PC_ITEMS_COUNT; i++)
    {
        if (newItems[i].itemId == itemId)
        {
            ownedCount = GetPCItemQuantity(&newItems[i].quantity);
            if (ownedCount + count <= MAX_PC_ITEM_CAPACITY)
            {
                SetPCItemQuantity(&newItems[i].quantity, ownedCount + count);
                memcpy(gSaveBlock1Ptr->pcItems, newItems, sizeof(gSaveBlock1Ptr->pcItems));
                Free(newItems);
                return TRUE;
            }
            count += ownedCount - MAX_PC_ITEM_CAPACITY;
            SetPCItemQuantity(&newItems[i].quantity, MAX_PC_ITEM_CAPACITY);
            if (count == 0)
            {
                memcpy(gSaveBlock1Ptr->pcItems, newItems, sizeof(gSaveBlock1Ptr->pcItems));
                Free(newItems);
                return TRUE;
            }
        }
    }

    // Put any remaining items into a new item slot.
    if (count > 0)
    {
        freeSlot = FindFreePCItemSlot();
        if (freeSlot == -1)
        {
            Free(newItems);
            return FALSE;
        }
        else
        {
            newItems[freeSlot].itemId = itemId;
            SetPCItemQuantity(&newItems[freeSlot].quantity, count);
        }
    }

    // Copy items back to the PC
    memcpy(gSaveBlock1Ptr->pcItems, newItems, sizeof(gSaveBlock1Ptr->pcItems));
    Free(newItems);
    return TRUE;
}

void RemovePCItem(u8 index, u16 count)
{
    gSaveBlock1Ptr->pcItems[index].quantity -= count;
    if (gSaveBlock1Ptr->pcItems[index].quantity == 0)
    {
        gSaveBlock1Ptr->pcItems[index].itemId = ITEM_NONE;
        CompactPCItems();
    }
}

void CompactPCItems(void)
{
    u16 i;
    u16 j;

    for (i = 0; i < PC_ITEMS_COUNT - 1; i++)
    {
        for (j = i + 1; j < PC_ITEMS_COUNT; j++)
        {
            if (gSaveBlock1Ptr->pcItems[i].itemId == 0)
            {
                struct PcItemSlot temp = gSaveBlock1Ptr->pcItems[i];
                gSaveBlock1Ptr->pcItems[i] = gSaveBlock1Ptr->pcItems[j];
                gSaveBlock1Ptr->pcItems[j] = temp;
            }
        }
    }
}

void SwapRegisteredBike(void)
{
    switch (gSaveBlock1Ptr->registeredItem)
    {
    case ITEM_MACH_BIKE:
        gSaveBlock1Ptr->registeredItem = ITEM_ACRO_BIKE;
        break;
    case ITEM_ACRO_BIKE:
        gSaveBlock1Ptr->registeredItem = ITEM_MACH_BIKE;
        break;
    }
}

u16 BagGetItemIdByPocketPosition(u8 pocketId, u16 pocketPos)
{
    return gBagPockets[pocketId - 1].itemSlots[pocketPos].itemId;
}

u16 BagGetQuantityByPocketPosition(u8 pocketId, u16 pocketPos)
{
    return GetBagItemQuantity(&gBagPockets[pocketId - 1].itemSlots[pocketPos].quantity);
}

static void SwapItemSlots(struct ItemSlot *a, struct ItemSlot *b)
{
    struct ItemSlot temp;
    SWAP(*a, *b, temp);
}

void CompactItemsInBagPocket(struct BagPocket *bagPocket)
{
    u16 i, j;

    for (i = 0; i < bagPocket->capacity - 1; i++)
    {
        for (j = i + 1; j < bagPocket->capacity; j++)
        {
            if (GetBagItemQuantity(&bagPocket->itemSlots[i].quantity) == 0)
                SwapItemSlots(&bagPocket->itemSlots[i], &bagPocket->itemSlots[j]);
        }
    }
}

void SortBerriesOrTMHMs(struct BagPocket *bagPocket)
{
    u16 i, j;

    for (i = 0; i < bagPocket->capacity - 1; i++)
    {
        for (j = i + 1; j < bagPocket->capacity; j++)
        {
            if (GetBagItemQuantity(&bagPocket->itemSlots[i].quantity) != 0)
            {
                if (GetBagItemQuantity(&bagPocket->itemSlots[j].quantity) == 0)
                    continue;
                if (bagPocket->itemSlots[i].itemId <= bagPocket->itemSlots[j].itemId)
                    continue;
            }
            SwapItemSlots(&bagPocket->itemSlots[i], &bagPocket->itemSlots[j]);
        }
    }
}

void MoveItemSlotInList(struct ItemSlot* itemSlots_, u32 from, u32 to_)
{
    // dumb assignments needed to match
    struct ItemSlot *itemSlots = itemSlots_;
    u32 to = to_;

    if (from != to)
    {
        s16 i, count;
        struct ItemSlot firstSlot = itemSlots[from];

        if (to > from)
        {
            to--;
            for (i = from, count = to; i < count; i++)
                itemSlots[i] = itemSlots[i + 1];
        }
        else
        {
            for (i = from, count = to; i > count; i--)
                itemSlots[i] = itemSlots[i - 1];
        }
        itemSlots[to] = firstSlot;
    }
}

void MovePcItemSlotInList(struct PcItemSlot* itemSlots_, u32 from, u32 to_)
{
    // dumb assignments needed to match
    struct PcItemSlot *itemSlots = itemSlots_;
    u32 to = to_;

    if (from != to)
    {
        s16 i, count;
        struct PcItemSlot firstSlot = itemSlots[from];

        if (to > from)
        {
            to--;
            for (i = from, count = to; i < count; i++)
                itemSlots[i] = itemSlots[i + 1];
        }
        else
        {
            for (i = from, count = to; i > count; i--)
                itemSlots[i] = itemSlots[i - 1];
        }
        itemSlots[to] = firstSlot;
    }
}

void ClearBag(void)
{
    u16 i;

    for (i = 0; i < POCKETS_COUNT; i++)
    {
        ClearItemSlots(gBagPockets[i].itemSlots, gBagPockets[i].capacity);
    }
}

u16 CountTotalItemQuantityInBag(u16 itemId)
{
    u16 i;
    u16 ownedCount = 0;
    struct BagPocket *bagPocket = &gBagPockets[ItemId_GetPocket(itemId) - 1];

    for (i = 0; i < bagPocket->capacity; i++)
    {
        if (bagPocket->itemSlots[i].itemId == itemId)
            ownedCount += GetBagItemQuantity(&bagPocket->itemSlots[i].quantity);
    }

    return ownedCount;
}

static bool8 CheckPyramidBagHasItem(u16 itemId, u16 count)
{
    u8 i;
    u16 *items = gSaveBlock2Ptr->frontier.pyramidBag.itemId[gSaveBlock2Ptr->frontier.lvlMode];
    u8 *quantities = gSaveBlock2Ptr->frontier.pyramidBag.quantity[gSaveBlock2Ptr->frontier.lvlMode];

    for (i = 0; i < PYRAMID_BAG_ITEMS_COUNT; i++)
    {
        if (items[i] == itemId)
        {
            if (quantities[i] >= count)
                return TRUE;

            count -= quantities[i];
            if (count == 0)
                return TRUE;
        }
    }

    return FALSE;
}

static bool8 CheckPyramidBagHasSpace(u16 itemId, u16 count)
{
    u8 i;
    u16 *items = gSaveBlock2Ptr->frontier.pyramidBag.itemId[gSaveBlock2Ptr->frontier.lvlMode];
    u8 *quantities = gSaveBlock2Ptr->frontier.pyramidBag.quantity[gSaveBlock2Ptr->frontier.lvlMode];

    for (i = 0; i < PYRAMID_BAG_ITEMS_COUNT; i++)
    {
        if (items[i] == itemId || items[i] == ITEM_NONE)
        {
            if (quantities[i] + count <= MAX_BAG_ITEM_CAPACITY)
                return TRUE;

            count = (quantities[i] + count) - MAX_BAG_ITEM_CAPACITY;
            if (count == 0)
                return TRUE;
        }
    }

    return FALSE;
}

bool8 AddPyramidBagItem(u16 itemId, u16 count)
{
    u16 i;

    u16 *items = gSaveBlock2Ptr->frontier.pyramidBag.itemId[gSaveBlock2Ptr->frontier.lvlMode];
    u8 *quantities = gSaveBlock2Ptr->frontier.pyramidBag.quantity[gSaveBlock2Ptr->frontier.lvlMode];

    u16 *newItems = Alloc(PYRAMID_BAG_ITEMS_COUNT * sizeof(u16));
    u8 *newQuantities = Alloc(PYRAMID_BAG_ITEMS_COUNT * sizeof(u8));

    memcpy(newItems, items, PYRAMID_BAG_ITEMS_COUNT * sizeof(u16));
    memcpy(newQuantities, quantities, PYRAMID_BAG_ITEMS_COUNT * sizeof(u8));

    for (i = 0; i < PYRAMID_BAG_ITEMS_COUNT; i++)
    {
        if (newItems[i] == itemId && newQuantities[i] < MAX_BAG_ITEM_CAPACITY)
        {
            newQuantities[i] += count;
            if (newQuantities[i] > MAX_BAG_ITEM_CAPACITY)
            {
                count = newQuantities[i] - MAX_BAG_ITEM_CAPACITY;
                newQuantities[i] = MAX_BAG_ITEM_CAPACITY;
            }
            else
            {
                count = 0;
            }

            if (count == 0)
                break;
        }
    }

    if (count > 0)
    {
        for (i = 0; i < PYRAMID_BAG_ITEMS_COUNT; i++)
        {
            if (newItems[i] == ITEM_NONE)
            {
                newItems[i] = itemId;
                newQuantities[i] = count;
                if (newQuantities[i] > MAX_BAG_ITEM_CAPACITY)
                {
                    count = newQuantities[i] - MAX_BAG_ITEM_CAPACITY;
                    newQuantities[i] = MAX_BAG_ITEM_CAPACITY;
                }
                else
                {
                    count = 0;
                }

                if (count == 0)
                    break;
            }
        }
    }

    if (count == 0)
    {
        memcpy(items, newItems, PYRAMID_BAG_ITEMS_COUNT * sizeof(u16));
        memcpy(quantities, newQuantities, PYRAMID_BAG_ITEMS_COUNT * sizeof(u8));
        Free(newItems);
        Free(newQuantities);
        return TRUE;
    }
    else
    {
        Free(newItems);
        Free(newQuantities);
        return FALSE;
    }
}

bool8 RemovePyramidBagItem(u16 itemId, u16 count)
{
    u16 i;

    u16 *items = gSaveBlock2Ptr->frontier.pyramidBag.itemId[gSaveBlock2Ptr->frontier.lvlMode];
    u8 *quantities = gSaveBlock2Ptr->frontier.pyramidBag.quantity[gSaveBlock2Ptr->frontier.lvlMode];

    i = gPyramidBagMenuState.cursorPosition + gPyramidBagMenuState.scrollPosition;
    if (items[i] == itemId && quantities[i] >= count)
    {
        quantities[i] -= count;
        if (quantities[i] == 0)
            items[i] = ITEM_NONE;
        return TRUE;
    }
    else
    {
        u16 *newItems = Alloc(PYRAMID_BAG_ITEMS_COUNT * sizeof(u16));
        u8 *newQuantities = Alloc(PYRAMID_BAG_ITEMS_COUNT * sizeof(u8));

        memcpy(newItems, items, PYRAMID_BAG_ITEMS_COUNT * sizeof(u16));
        memcpy(newQuantities, quantities, PYRAMID_BAG_ITEMS_COUNT * sizeof(u8));

        for (i = 0; i < PYRAMID_BAG_ITEMS_COUNT; i++)
        {
            if (newItems[i] == itemId)
            {
                if (newQuantities[i] >= count)
                {
                    newQuantities[i] -= count;
                    count = 0;
                    if (newQuantities[i] == 0)
                        newItems[i] = ITEM_NONE;
                }
                else
                {
                    count -= newQuantities[i];
                    newQuantities[i] = 0;
                    newItems[i] = ITEM_NONE;
                }

                if (count == 0)
                    break;
            }
        }

        if (count == 0)
        {
            memcpy(items, newItems, PYRAMID_BAG_ITEMS_COUNT * sizeof(u16));
            memcpy(quantities, newQuantities, PYRAMID_BAG_ITEMS_COUNT * sizeof(u8));
            Free(newItems);
            Free(newQuantities);
            return TRUE;
        }
        else
        {
            Free(newItems);
            Free(newQuantities);
            return FALSE;
        }
    }
}

static u16 SanitizeItemId(u16 itemId)
{
    if (itemId >= ITEMS_COUNT)
        return ITEM_NONE;
    else
        return itemId;
}

const u8 *ItemId_GetName(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].name;
}

u16 ItemId_GetId(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].itemId;
}

u16 ItemId_GetPrice(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].price;
}

u8 ItemId_GetHoldEffect(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].holdEffect;
}

u8 ItemId_GetHoldEffectParam(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].holdEffectParam;
}

static u8 *Item_GetTmHmMoveDescription(u16 itemId)
{
    u16 move = gTMHMMoves[itemId - ITEM_TM_START];
    return (u8*)gMoveDescriptionPointers[move - 1];
}

const u8 *ItemId_GetDescription(u16 itemId)
{
    if (gItems[SanitizeItemId(itemId)].pocket == POCKET_TM_HM)
        return Item_GetTmHmMoveDescription(itemId);
    return gItems[SanitizeItemId(itemId)].description;
}

u8 ItemId_GetImportance(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].importance;
}

// unused
u8 ItemId_GetUnknownValue(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].unk19;
}

u8 ItemId_GetPocket(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].pocket;
}

u8 ItemId_GetType(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].type;
}

ItemUseFunc ItemId_GetFieldFunc(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].fieldUseFunc;
}

u8 ItemId_GetBattleUsage(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].battleUsage;
}

ItemUseFunc ItemId_GetBattleFunc(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].battleUseFunc;
}

u8 ItemId_GetSecondaryId(u16 itemId)
{
    return gItems[SanitizeItemId(itemId)].secondaryId;
}

bool32 IsPinchBerryItemEffect(u16 holdEffect)
{
    switch (holdEffect)
    {
    case HOLD_EFFECT_ATTACK_UP:
    case HOLD_EFFECT_DEFENSE_UP:
    case HOLD_EFFECT_SPEED_UP:
    case HOLD_EFFECT_SP_ATTACK_UP:
    case HOLD_EFFECT_SP_DEFENSE_UP:
    case HOLD_EFFECT_CRITICAL_UP:
    case HOLD_EFFECT_RANDOM_STAT_UP:
    #ifdef HOLD_EFFECT_CUSTAP_BERRY
    case HOLD_EFFECT_CUSTAP_BERRY:
    #endif
    #ifdef HOLD_EFFECT_MICLE_BERRY
    case HOLD_EFFECT_MICLE_BERRY:
    #endif
        return TRUE;
    }

    return FALSE;
}
