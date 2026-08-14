// Roguelike Mode - Main Implementation
// MiniSlug Roguelike - Wave survival mode with Perks, Bosses, and Dynamic Combat

#include "includes.h"
#include <SDL2/SDL.h>
#include "roguelike.h"

// External functions
void RenderFlip(u32 nSync);
int EventHandler(u32 nInGame);
void SprDisplayAll_Pass1(void);
void SprDisplayAll_Pass2(void);
u32 Menu(void (*pFctInit)(void), u32 (*pFctMain)(void));

//=============================================================================
// Global Variables
//=============================================================================

struct SRogueState gRogue;

//=============================================================================
// Monster Pools - Categorized by Difficulty and Enemy Roles
//=============================================================================

// Easy monsters (Waves 1-3): Infantry, Shield Soldiers, Running Zombies, Flying Tara
static u8 gpMonsterPool_Easy[] = {
    e_Mst14_RebelSoldier0,
    e_Mst7_Zombie1,
    e_Mst14_RebelSoldier0,
    e_Mst43_FlyingTara0,
};
#define POOL_EASY_SIZE (sizeof(gpMonsterPool_Easy) / sizeof(gpMonsterPool_Easy[0]))

// Medium monsters (Waves 4-7): Rebel Soldiers, Zombies, R-Shobu Choppers, Flying Tara, Girida Tanks
static u8 gpMonsterPool_Medium[] = {
    e_Mst14_RebelSoldier0,
    e_Mst7_Zombie1,
    e_Mst6_RShobu,
    e_Mst43_FlyingTara0,
    e_Mst26_Girida0,
};
#define POOL_MEDIUM_SIZE (sizeof(gpMonsterPool_Medium) / sizeof(gpMonsterPool_Medium[0]))

// Hard monsters (Waves 8-11): Tanks, Choppers, Masknell, Rebel Squads
static u8 gpMonsterPool_Hard[] = {
    e_Mst14_RebelSoldier0,
    e_Mst26_Girida0,
    e_Mst6_RShobu,
    e_Mst28_Masknell0,
    e_Mst43_FlyingTara0,
};
#define POOL_HARD_SIZE (sizeof(gpMonsterPool_Hard) / sizeof(gpMonsterPool_Hard[0]))

// Insane monsters (Waves 12-15): Armor Squads, Air Strike, Gunship Ribert
static u8 gpMonsterPool_Insane[] = {
    e_Mst26_Girida0,
    e_Mst28_Masknell0,
    e_Mst43_FlyingTara0,
    e_Mst6_RShobu,
    e_Mst46_HairBusterRibert0,
};
#define POOL_INSANE_SIZE (sizeof(gpMonsterPool_Insane) / sizeof(gpMonsterPool_Insane[0]))

// Nightmare monsters (Waves 16+): Elite Boss / Air / Tank Assault
static u8 gpMonsterPool_Nightmare[] = {
    e_Mst26_Girida0,
    e_Mst28_Masknell0,
    e_Mst43_FlyingTara0,
    e_Mst46_HairBusterRibert0,
    e_Mst20_Boss,
};
#define POOL_NIGHTMARE_SIZE (sizeof(gpMonsterPool_Nightmare) / sizeof(gpMonsterPool_Nightmare[0]))

// Forward declarations
u32  Roguelike_GetActiveEnemyCount(void);
u32  Roguelike_GetSpawnPosX(void);
u32  Roguelike_GetRandomMonster(u8 nDifficulty);
void Roguelike_SpawnMonster(void);

//=============================================================================
// Utility Functions
//=============================================================================

u8 Roguelike_GetDifficulty(u32 nWave)
{
    if (nWave <= 3)  return e_Rogue_Diff_Easy;
    if (nWave <= 7)  return e_Rogue_Diff_Medium;
    if (nWave <= 11) return e_Rogue_Diff_Hard;
    if (nWave <= 15) return e_Rogue_Diff_Insane;
    return e_Rogue_Diff_Nightmare;
}

u32 Roguelike_GetMonstersForWave(u32 nWave)
{
    u32 nMonsters = ROGUE_WAVE_START_MONSTERS + (nWave * 2);
    if (nMonsters > ROGUE_WAVE_MAX_MONSTERS)
        nMonsters = ROGUE_WAVE_MAX_MONSTERS;
    return nMonsters;
}

u32 Roguelike_GetSpawnInterval(u32 nWave)
{
    s32 nInterval = ROGUE_SPAWN_INTERVAL_BASE - ((nWave / 3) * 12);
    if (nInterval < ROGUE_SPAWN_INTERVAL_MIN)
        nInterval = ROGUE_SPAWN_INTERVAL_MIN;
    return (u32)nInterval;
}

//=============================================================================
// Core Functions
//=============================================================================

void Roguelike_Init(void)
{
    memset(&gRogue, 0, sizeof(struct SRogueState));
    gRogue.nActive = 1;
    Roguelike_InitLeaderboard();
    
    gRogue.wave.nWaveNo = 0;
    gRogue.wave.nDifficulty = e_Rogue_Diff_Easy;
    
    gRogue.stats.nTotalKills = 0;
    gRogue.stats.nSurvivalTime = 0;
    gRogue.stats.nHighestWave = 0;
    gRogue.stats.nFinalScore = 0;
    
    gRogue.nItemDropTimerAmmo = ROGUE_ITEM_DROP_AMMO_INTERVAL;
    gRogue.nItemDropTimerBomb = ROGUE_ITEM_DROP_BOMB_INTERVAL;
    
    gRogue.nPhase = e_Rogue_Phase_Init;
    gRogue.nPhaseTimer = 90;
    
    gRogue.combo.nComboCount = 0;
    gRogue.combo.nComboTimer = 0;
    gRogue.combo.nMaxCombo = 0;
    
    memset(&gRogue.perks, 0, sizeof(struct SRoguePerks));
}

void Roguelike_Main(void)
{
    if (!gRogue.nActive) return;
    
    gRogue.stats.nSurvivalTime++;
    Roguelike_ComboUpdate();
    
    if (gRogue.powerUp.nTimer > 0)
        gRogue.powerUp.nTimer--;
    
    switch (gRogue.nPhase)
    {
    case e_Rogue_Phase_Init:
        if (gRogue.nPhaseTimer > 0)
        {
            gRogue.nPhaseTimer--;
        }
        else
        {
            Roguelike_WaveStart(1);
        }
        break;
        
    case e_Rogue_Phase_WaveStart:
        Roguelike_DrawWaveStart();
        if (gRogue.nPhaseTimer > 0)
        {
            gRogue.nPhaseTimer--;
        }
        else
        {
            gRogue.nPhase = e_Rogue_Phase_Playing;
        }
        break;
        
    case e_Rogue_Phase_Playing:
        // Spawn Wave Monsters
        if (gRogue.wave.nMonstersSpawned < gRogue.wave.nMonstersTotal)
        {
            if (gRogue.wave.nSpawnTimer > 0)
            {
                gRogue.wave.nSpawnTimer--;
            }
            else
            {
                u32 nActiveMonsters = Roguelike_GetActiveEnemyCount();
                if (nActiveMonsters < ROGUE_MAX_ACTIVE_MONSTERS)
                {
                    Roguelike_SpawnMonster();
                    gRogue.wave.nSpawnTimer = gRogue.wave.nSpawnInterval;
                }
            }
        }
        
        // Check Wave Complete
        if (Roguelike_IsWaveComplete())
        {
            Roguelike_WaveComplete();
        }
        
        Roguelike_DropItem();
        break;
        
    case e_Rogue_Phase_WaveComplete:
        Roguelike_DrawWaveComplete();
        if (gRogue.nPhaseTimer > 0)
        {
            gRogue.nPhaseTimer--;
        }
        else
        {
            // Offer Perks every 2 waves!
            if (gRogue.wave.nWaveNo % 2 == 0)
            {
                Roguelike_InitPerkSelection();
                Roguelike_ShowPerkSelection();
            }
            else
            {
                Roguelike_WaveStart(gRogue.wave.nWaveNo + 1);
            }
        }
        break;
        
    case e_Rogue_Phase_GameOver:
        Roguelike_DrawGameOver();
        break;
    }
    
    // Draw In-Game Roguelike HUD
    if (gRogue.nPhase == e_Rogue_Phase_Playing || gRogue.nPhase == e_Rogue_Phase_WaveStart)
    {
        Roguelike_DrawHUD();
    }
}

void Roguelike_Exit(void)
{
    gRogue.nActive = 0;
    gRogue.stats.nFinalScore = Roguelike_CalculateFinalScore();
}

//=============================================================================
// Wave Management
//=============================================================================

void Roguelike_WaveStart(u32 nWave)
{
    gRogue.wave.nWaveNo = nWave;
    gRogue.wave.nMonstersKilled = 0;
    gRogue.wave.nMonstersTotal = Roguelike_GetMonstersForWave(nWave);
    gRogue.wave.nMonstersSpawned = 0;
    gRogue.wave.nSpawnTimer = 30;
    gRogue.wave.nSpawnInterval = Roguelike_GetSpawnInterval(nWave);
    gRogue.wave.nDifficulty = Roguelike_GetDifficulty(nWave);
    
    if (nWave > gRogue.stats.nHighestWave)
        gRogue.stats.nHighestWave = nWave;
    
    gRogue.nPhase = e_Rogue_Phase_WaveStart;
    gRogue.nPhaseTimer = 100;
    
    // Milestone item drops
    if (nWave > 1 && (nWave % 3 == 0))
    {
        Roguelike_DropWeaponCapsule();
    }
}

u32 Roguelike_GetActiveEnemyCount(void)
{
    u32 nCount = 0;
    for (int i = 0; i < MST_MAX_SLOTS; i++)
    {
        if (!gpMstSlots[i].nUsed) continue;
        u8 mstType = gpMstSlots[i].nMstNo;
        if (mstType == e_Mst2_Enemy1 ||
            mstType == e_Mst6_RShobu ||
            mstType == e_Mst7_Zombie1 ||
            mstType == e_Mst14_RebelSoldier0 ||
            mstType == e_Mst15_Truck0 ||
            mstType == e_Mst20_Boss ||
            mstType == e_Mst25_RocketDiver0 ||
            mstType == e_Mst26_Girida0 ||
            mstType == e_Mst27_HalfBoss ||
            mstType == e_Mst28_Masknell0 ||
            mstType == e_Mst43_FlyingTara0 ||
            mstType == e_Mst46_HairBusterRibert0)
        {
            nCount++;
        }
    }
    return nCount;
}

void Roguelike_WaveComplete(void)
{
    gRogue.nPhase = e_Rogue_Phase_WaveComplete;
    gRogue.nPhaseTimer = ROGUE_WAVE_COMPLETE_DELAY;
    
    u32 nWaveBonus = gRogue.wave.nWaveNo * 2000;
    Roguelike_AddScore(nWaveBonus);
    Sfx_PlaySfx(e_Sfx_Fx_ThankYou, e_SfxPrio_10);
}

u32 Roguelike_IsWaveComplete(void)
{
    if (gRogue.wave.nMonstersKilled >= gRogue.wave.nMonstersTotal &&
        gRogue.wave.nMonstersSpawned >= gRogue.wave.nMonstersTotal)
    {
        return 1;
    }
    // Safety check: all monsters spawned and 0 enemies alive
    if (gRogue.wave.nMonstersSpawned >= gRogue.wave.nMonstersTotal && Roguelike_GetActiveEnemyCount() == 0)
    {
        gRogue.wave.nMonstersKilled = gRogue.wave.nMonstersTotal;
        return 1;
    }
    return 0;
}

//=============================================================================
// Monster Spawning
//=============================================================================

u32 Roguelike_GetSpawnPosX(void)
{
    s32 nScrollX = gScrollPos.nPosX / 256;
    s32 nMaxMapX = (gMap.nMapLg * 16) - 48;
    s32 nMinMapX = 48;
    
    int bCanSpawnRight = (nScrollX + SCR_Width + 24 <= nMaxMapX);
    int bCanSpawnLeft = (nScrollX - 24 >= nMinMapX);
    
    s32 nSpawnX;
    if (bCanSpawnRight && bCanSpawnLeft)
    {
        if (rand() % 2 == 0)
            nSpawnX = nScrollX + SCR_Width + 20;
        else
            nSpawnX = nScrollX - 20;
    }
    else if (bCanSpawnLeft)
    {
        // Player is near right edge: spawn from the left!
        nSpawnX = nScrollX - 20 - (rand() % 40);
        if (nSpawnX < nMinMapX) nSpawnX = nMinMapX;
    }
    else if (bCanSpawnRight)
    {
        // Player is near left edge: spawn from the right!
        nSpawnX = nScrollX + SCR_Width + 20 + (rand() % 40);
        if (nSpawnX > nMaxMapX) nSpawnX = nMaxMapX;
    }
    else
    {
        // Fallback: spawn inside arena
        nSpawnX = nScrollX + (rand() % (SCR_Width - 40)) + 20;
    }
    
    return (u32)nSpawnX;
}

u32 Roguelike_GetRandomMonster(u8 nDifficulty)
{
    u8 *pPool;
    u32 nPoolSize;
    
    switch (nDifficulty)
    {
    case e_Rogue_Diff_Easy:
        pPool = gpMonsterPool_Easy;
        nPoolSize = POOL_EASY_SIZE;
        break;
    case e_Rogue_Diff_Medium:
        pPool = gpMonsterPool_Medium;
        nPoolSize = POOL_MEDIUM_SIZE;
        break;
    case e_Rogue_Diff_Hard:
        pPool = gpMonsterPool_Hard;
        nPoolSize = POOL_HARD_SIZE;
        break;
    case e_Rogue_Diff_Insane:
        pPool = gpMonsterPool_Insane;
        nPoolSize = POOL_INSANE_SIZE;
        break;
    case e_Rogue_Diff_Nightmare:
    default:
        pPool = gpMonsterPool_Nightmare;
        nPoolSize = POOL_NIGHTMARE_SIZE;
        break;
    }
    
    return pPool[rand() % nPoolSize];
}

void Roguelike_SpawnMonster(void)
{
    u32 nMstType = Roguelike_GetRandomMonster(gRogue.wave.nDifficulty);
    u32 nPosX = Roguelike_GetSpawnPosX();
    u32 nPosY = 150;
    u8 pData[8] = {0};
    u8 *pDataPtr = NULL;
    
    // Configure parameters for each enemy type
    switch (nMstType)
    {
    case e_Mst14_RebelSoldier0:
        // Type 0..5 (Rifle, Mortar, LRAC, Pistol, Grenade, Shield)
        pData[0] = (u8)(rand() % 6);
        // Bit 12 = 1 (nMove = 1 -> march towards player), Bit 15 = 1 (nJump = 1)
        pData[1] = 0x90;
        pDataPtr = pData;
        nPosY = 150;
        break;
        
    case e_Mst7_Zombie1:
        // CRITICAL FIX: MST7_NB is 4 (0=Zombie Teen, 1=Zombie Fat, 2=Mars People, 3=Brain Bot)
        // Bit 0..3: Type (0..3), Bit 4..7: Zone size (15 blocks)
        pData[0] = (u8)((rand() % 4) | (0xF << 4));
        pDataPtr = pData;
        nPosY = 150;
        break;
        
    case e_Mst26_Girida0:
        // Tank: ZoneMax = 30 blocks
        pData[0] = 0x3C;
        pDataPtr = pData;
        nPosY = 150;
        break;
        
    case e_Mst6_RShobu:
    case e_Mst28_Masknell0:
    case e_Mst43_FlyingTara0:
        // Airborne attackers in sky
        nPosY = 45 + (rand() % 40);
        break;
        
    default:
        nPosY = 150;
        break;
    }
    
    s32 nSlot = MstAdd(nMstType, nPosX, nPosY, pDataPtr, -1);
    if (nSlot != -1)
    {
        // Special override for R-Shobu to attack immediately
        if (nMstType == e_Mst6_RShobu)
        {
            gpMstSlots[nSlot].nPhase = 1; // e_Mst6_RShobu_Arrival
            gpMstSlots[nSlot].nPosY = 50 * 256;
        }
        gRogue.wave.nMonstersSpawned++;
    }
}

//=============================================================================
// Item Drops
//=============================================================================

void Roguelike_DropItem(void)
{
    if (gRogue.nItemDropTimerAmmo > 0)
    {
        gRogue.nItemDropTimerAmmo--;
    }
    else
    {
        Player_WeaponReload(0);
        gRogue.nItemDropTimerAmmo = ROGUE_ITEM_DROP_AMMO_INTERVAL;
    }
    
    if (gRogue.nItemDropTimerBomb > 0)
    {
        gRogue.nItemDropTimerBomb--;
    }
    else
    {
        Player_WeaponReload(3);
        gRogue.nItemDropTimerBomb = ROGUE_ITEM_DROP_BOMB_INTERVAL;
    }
}

void Roguelike_DropWeaponCapsule(void)
{
    u32 nWeapon = (rand() % (e_Player_Weapon_Max - 1)) + 1;
    Player_WeaponSet(nWeapon);
    Player_WeaponReload(0);
    Sfx_PlaySfx(e_Sfx_Fx_GunReload, e_SfxPrio_10);
}

//=============================================================================
// Combo System
//=============================================================================

void Roguelike_ComboRegisterKill(void)
{
    gRogue.wave.nMonstersKilled++;
    gRogue.stats.nTotalKills++;
    
    gRogue.combo.nComboCount++;
    gRogue.combo.nComboTimer = ROGUE_COMBO_TIMEOUT;
    
    if (gRogue.combo.nComboCount > gRogue.combo.nMaxCombo)
        gRogue.combo.nMaxCombo = gRogue.combo.nComboCount;
    
    u32 nBaseScore = 150;
    Roguelike_AddScore(nBaseScore);
}

void Roguelike_ComboUpdate(void)
{
    if (gRogue.combo.nComboTimer > 0)
    {
        gRogue.combo.nComboTimer--;
        if (gRogue.combo.nComboTimer == 0)
        {
            gRogue.combo.nComboCount = 0;
        }
    }
}

u32 Roguelike_GetComboMultiplier(void)
{
    u32 nMultiplier = 10;
    if (gRogue.combo.nComboCount >= 2)
    {
        nMultiplier = 10 + ((gRogue.combo.nComboCount - 1) * 5);
        if (nMultiplier > 80) nMultiplier = 80;
    }
    return nMultiplier;
}

//=============================================================================
// Scoring
//=============================================================================

void Roguelike_AddScore(u32 nBasePoints)
{
    u32 nMultiplier = Roguelike_GetComboMultiplier();
    u32 nPoints = (nBasePoints * nMultiplier) / 10;
    
    if (gRogue.perks.nDamageUp > 0)
    {
        nPoints = (nPoints * (100 + gRogue.perks.nDamageUp * 30)) / 100;
    }
    
    gShoot.nPlayerScore += nPoints;
}

u32 Roguelike_CalculateFinalScore(void)
{
    u32 nScore = gShoot.nPlayerScore;
    nScore += gRogue.stats.nHighestWave * 2000;
    nScore += gRogue.stats.nTotalKills * 100;
    nScore += gRogue.combo.nMaxCombo * 500;
    nScore += (gRogue.stats.nSurvivalTime / 60) * 10;
    gRogue.stats.nFinalScore = nScore;
    return nScore;
}

//=============================================================================
// Perks Information
//=============================================================================

char *Roguelike_GetPerkName(u8 nPerkType)
{
    switch (nPerkType)
    {
    case e_Perk_HeavyMachinegun: return "ปืนกลหนัก (HEAVY MACHINE GUN)";
    case e_Perk_Shotgun:         return "ปืนลูกซองพลังสูง (SHOTGUN)";
    case e_Perk_RocketLauncher:  return "จรวดนำวิถี (ROCKET LAUNCHER)";
    case e_Perk_Flamethrower:    return "ปืนพ่นไฟ (FLAMETHROWER)";
    case e_Perk_BombSupply:      return "คลังระเบิดเสริม (BOMB SUPPLY +10)";
    case e_Perk_SpeedBoost:      return "เพิ่มความเร็ว (SPEED BOOSTER)";
    case e_Perk_DamageUp:        return "เพิ่มพลังโจมตี (DAMAGE +30%)";
    case e_Perk_Armor:           return "เพิ่มชีวิตสำรอง (1UP EXTRA LIFE)";
    case e_Perk_SupplyDrop:      return "หน่วยเสบียงสนับสนุน (SUPPLY DROP)";
    default:                     return "ทักษะพิเศษ (SPECIAL PERK)";
    }
}

char *Roguelike_GetPerkDesc(u8 nPerkType)
{
    switch (nPerkType)
    {
    case e_Perk_HeavyMachinegun: return "ติดตั้งปืนกลหนัก HMG พร้อมกระสุนรัวต่อเนื่อง 150 นัด";
    case e_Perk_Shotgun:         return "ติดตั้งปืนลูกซองพลังทำลายล้างสูง กวาดล้างศัตรูทั้งหน้าจอ";
    case e_Perk_RocketLauncher:  return "ติดตั้งเครื่องยิงจรวดติดตามเป้าหมาย สร้างแรงระเบิดรุนแรง";
    case e_Perk_Flamethrower:    return "ติดตั้งปืนพ่นไฟระยะประชิด เผาผลาญกลุ่มศัตรูในพริบตา";
    case e_Perk_BombSupply:      return "เติมระเบิดมือเพิ่มทันที 10 ลูก สำหรับทำลายศัตรูกลุ่มใหญ่";
    case e_Perk_SpeedBoost:      return "เพิ่มความเร็วในการวิ่งและกระโดดคล่องตัวขึ้น 25%";
    case e_Perk_DamageUp:        return "เพิ่มพลังทำลายของกระสุนทุกชนิดและแต้มคะแนน +30%";
    case e_Perk_Armor:           return "ฟื้นฟูและมอบชีวิตสำรองให้ตัวละครทันที +1 ชีวิต";
    case e_Perk_SupplyDrop:      return "ปล่อยกล่องเสบียงอาวุธและเติมกระสุนให้เต็มทันที";
    default:                     return "เพิ่มขีดความสามารถในการต่อสู้";
    }
}

void Roguelike_InitPerkSelection(void)
{
    gRogue.nSelectedPerkIdx = 0;
    gRogue.bConfirmReady = 0;
    gRogue.nPhaseTimer = 35; // 35 frames delay to prevent immediate confirmation
    
    // Clear keyboard inputs so shooting in-game doesn't trigger confirm
    memset(gVar.pKeys, 0, sizeof(gVar.pKeys));
    
    // Choose 3 distinct random perks
    u8 used[e_Perk_MAX] = {0};
    for (int i = 0; i < 3; i++)
    {
        u8 p;
        do {
            p = rand() % e_Perk_MAX;
        } while (used[p]);
        used[p] = 1;
        gRogue.nOfferedPerks[i] = p;
    }
}

void Roguelike_ApplyPerk(u8 nPerkType)
{
    switch (nPerkType)
    {
    case e_Perk_HeavyMachinegun:
        Player_WeaponSet(e_Player_Weapon_Machinegun);
        Player_WeaponReload(0);
        gShoot.nAmmo += 100;
        break;
    case e_Perk_Shotgun:
        Player_WeaponSet(e_Player_Weapon_Shotgun);
        Player_WeaponReload(0);
        gShoot.nAmmo += 20;
        break;
    case e_Perk_RocketLauncher:
        Player_WeaponSet(e_Player_Weapon_Rocket);
        Player_WeaponReload(0);
        gShoot.nAmmo += 15;
        break;
    case e_Perk_Flamethrower:
        Player_WeaponSet(e_Player_Weapon_Flamethrower);
        Player_WeaponReload(0);
        gShoot.nAmmo += 30;
        break;
    case e_Perk_BombSupply:
        gShoot.nBombAmmo += 10;
        if (gRogue.perks.nBombSupply < 5) gRogue.perks.nBombSupply++;
        break;
    case e_Perk_SpeedBoost:
        if (gRogue.perks.nSpeedBoost < 3) gRogue.perks.nSpeedBoost++;
        break;
    case e_Perk_DamageUp:
        if (gRogue.perks.nDamageUp < 3) gRogue.perks.nDamageUp++;
        break;
    case e_Perk_Armor:
        gShoot.nPlayerLives++;
        gShoot.nHUDPlayerLivesBlink = 60;
        if (gRogue.perks.nArmor < 5) gRogue.perks.nArmor++;
        break;
    case e_Perk_SupplyDrop:
        Player_WeaponReload(5);
        gShoot.nBombAmmo += 5;
        break;
    }
}

void Roguelike_DrawPerkSelection(void)
{
    char pTitle[] = "- เลือกทักษะเสริม (CHOOSE PERK) -";
    u32 nLg = Font_Print(0, 8, pTitle, FONT_NoDisp);
    Font_Print((SCR_Width - nLg) / 2, 18, pTitle, FONT_Highlight);
    
    s32 nBaseX = 20;
    s32 nBaseY = 46;
    s32 nCardHeight = 48;
    
    for (int i = 0; i < 3; i++)
    {
        u8 nPerkType = gRogue.nOfferedPerks[i];
        char *pName = Roguelike_GetPerkName(nPerkType);
        char *pDesc = Roguelike_GetPerkDesc(nPerkType);
        s32 nY = nBaseY + (i * nCardHeight);
        
        char pHeader[128];
        char pDescBuf[128];
        if (i == gRogue.nSelectedPerkIdx)
        {
            snprintf(pHeader, sizeof(pHeader), ">>> [ %d ] %s <<<", i + 1, pName);
            Font_Print(nBaseX, nY, pHeader, FONT_Highlight);
            snprintf(pDescBuf, sizeof(pDescBuf), "  %s", pDesc);
            Font_Print(nBaseX + 10, nY + 16, pDescBuf, FONT_Highlight);
        }
        else
        {
            snprintf(pHeader, sizeof(pHeader), "    [ %d ] %s", i + 1, pName);
            Font_Print(nBaseX, nY, pHeader, 0);
            snprintf(pDescBuf, sizeof(pDescBuf), "  %s", pDesc);
            Font_Print(nBaseX + 10, nY + 16, pDescBuf, 0);
        }
    }
    
    char pFooter[] = "กด [ซ้าย / ขวา] เพื่อเลือก  |  กด [ENTER] เพื่อยืนยัน";
    u32 nFootLg = Font_Print(0, 8, pFooter, FONT_NoDisp);
    Font_Print((SCR_Width - nFootLg) / 2, SCR_Height - 20, pFooter, FONT_Highlight);
}

void Roguelike_ShowPerkSelection(void)
{
    SDL_Surface *pBkg = NULL;
    if (gVar.pScreen)
    {
        pBkg = SDL_CreateRGBSurface(0, gVar.pScreen->w, gVar.pScreen->h, gVar.pScreen->format->BitsPerPixel,
            gVar.pScreen->format->Rmask, gVar.pScreen->format->Gmask, gVar.pScreen->format->Bmask, gVar.pScreen->format->Amask);
        if (pBkg)
        {
            SDL_BlitSurface(gVar.pScreen, NULL, pBkg, NULL);
            // Dim arena background by 50%
            SDL_LockSurface(pBkg);
            u16 *pPix = (u16 *)pBkg->pixels;
            int total = (pBkg->pitch / 2) * pBkg->h;
            for (int i = 0; i < total; i++)
            {
                pPix[i] = (pPix[i] >> 1) & 0x7BEF;
            }
            SDL_UnlockSurface(pBkg);
        }
    }

    memset(gVar.pKeys, 0, sizeof(gVar.pKeys));
    u8 bReady = 0;
    u32 nLockout = 20;

    while (1)
    {
        FrameWait();
        EventHandler(0);

        if (pBkg)
        {
            SDL_BlitSurface(pBkg, NULL, gVar.pScreen, NULL);
        }
        else
        {
            gVar.pBackground = gVar.pBkg[0];
            Bkg1Scroll(-gnFrame >> 1, -gnFrame >> 1);
        }

        Roguelike_DrawPerkSelection();
        RenderFlip(1);

        // Check if confirm keys have been released
        if (!gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] && 
            !gVar.pKeys[SDL_SCANCODE_RETURN] && 
            !gVar.pKeys[SDL_SCANCODE_SPACE])
        {
            bReady = 1;
        }

        if (nLockout > 0)
        {
            nLockout--;
            continue;
        }

        // Left / Up (Previous)
        if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Left]] || gVar.pKeys[SDL_SCANCODE_LEFT] ||
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Up]]   || gVar.pKeys[SDL_SCANCODE_UP])
        {
            if (gRogue.nSelectedPerkIdx > 0) 
            {
                gRogue.nSelectedPerkIdx--;
            }
            else
            {
                gRogue.nSelectedPerkIdx = 2; // Wrap around
            }
            Sfx_PlaySfx(e_Sfx_MenuClic1, e_SfxPrio_10);
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Left]] = 0;
            gVar.pKeys[SDL_SCANCODE_LEFT] = 0;
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Up]] = 0;
            gVar.pKeys[SDL_SCANCODE_UP] = 0;
        }

        // Right / Down (Next)
        if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Right]] || gVar.pKeys[SDL_SCANCODE_RIGHT] ||
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Down]]  || gVar.pKeys[SDL_SCANCODE_DOWN])
        {
            if (gRogue.nSelectedPerkIdx < 2) 
            {
                gRogue.nSelectedPerkIdx++;
            }
            else
            {
                gRogue.nSelectedPerkIdx = 0; // Wrap around
            }
            Sfx_PlaySfx(e_Sfx_MenuClic1, e_SfxPrio_10);
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Right]] = 0;
            gVar.pKeys[SDL_SCANCODE_RIGHT] = 0;
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Down]] = 0;
            gVar.pKeys[SDL_SCANCODE_DOWN] = 0;
        }

        // Confirm
        if (bReady &&
            (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] || 
             gVar.pKeys[SDL_SCANCODE_RETURN] || 
             gVar.pKeys[SDL_SCANCODE_SPACE]))
        {
            Sfx_PlaySfx(e_Sfx_MenuClic2, e_SfxPrio_10);
            
            u8 nSelectedPerk = gRogue.nOfferedPerks[gRogue.nSelectedPerkIdx];
            Roguelike_ApplyPerk(nSelectedPerk);
            
            memset(gVar.pKeys, 0, sizeof(gVar.pKeys));
            break;
        }
    }

    if (pBkg) SDL_FreeSurface(pBkg);
    
    // Start Next Wave!
    Roguelike_WaveStart(gRogue.wave.nWaveNo + 1);
}

//=============================================================================
// UI Drawing
//=============================================================================

void Roguelike_DrawHUD(void)
{
    char pBuffer[128];
    
    // Wave & Difficulty (Top Right)
    snprintf(pBuffer, sizeof(pBuffer), "เวฟ %d", gRogue.wave.nWaveNo);
    Font_Print(250, 2, pBuffer, FONT_Highlight);
    
    // Monster Progress
    snprintf(pBuffer, sizeof(pBuffer), "ศัตรู: %d/%d", gRogue.wave.nMonstersKilled, gRogue.wave.nMonstersTotal);
    Font_Print(230, 14, pBuffer, 0);

    // Survival Time
    u32 nSeconds = gRogue.stats.nSurvivalTime / 60;
    u32 nMinutes = nSeconds / 60;
    nSeconds = nSeconds % 60;
    snprintf(pBuffer, sizeof(pBuffer), "เวลา: %02d:%02d", nMinutes, nSeconds);
    Font_Print(230, 26, pBuffer, 0);
    
    // Combo Indicator (Center Top)
    if (gRogue.combo.nComboCount >= 2)
    {
        snprintf(pBuffer, sizeof(pBuffer), "COMBO x%d!", gRogue.combo.nComboCount);
        u32 nLg = Font_Print(0, 8, pBuffer, FONT_NoDisp);
        Font_Print((SCR_Width - nLg) / 2, 34, pBuffer, FONT_Highlight);
    }
}

void Roguelike_DrawWaveStart(void)
{
    char pBuffer[128];
    snprintf(pBuffer, sizeof(pBuffer), "=== เริ่มเวฟที่ %d ===", gRogue.wave.nWaveNo);
    u32 nLg = Font_Print(0, 8, pBuffer, FONT_NoDisp);
    Font_Print((SCR_Width - nLg) / 2, 90, pBuffer, FONT_Highlight);
    
    if (gRogue.wave.nWaveNo % 5 == 0 && gRogue.wave.nWaveNo > 0)
    {
        char pBoss[] = "[ คำเตือน : ฝูงบินรบและบอสประชิด! (BOSS WAVE) ]";
        u32 nBossLg = Font_Print(0, 8, pBoss, FONT_NoDisp);
        Font_Print((SCR_Width - nBossLg) / 2, 110, pBoss, FONT_Highlight);
    }
}

void Roguelike_DrawWaveComplete(void)
{
    char pClear[] = "=== ผ่านเวฟสำเร็จ! (WAVE CLEAR) ===";
    u32 nLg = Font_Print(0, 8, pClear, FONT_NoDisp);
    Font_Print((SCR_Width - nLg) / 2, 90, pClear, FONT_Highlight);
    
    char pBuffer[128];
    snprintf(pBuffer, sizeof(pBuffer), "กำจัดศัตรูแล้วทั้งหมด %d ตัว", gRogue.stats.nTotalKills);
    u32 nSubLg = Font_Print(0, 8, pBuffer, FONT_NoDisp);
    Font_Print((SCR_Width - nSubLg) / 2, 110, pBuffer, 0);
}

void Roguelike_DrawGameOver(void)
{
    char pTitle[] = "- สรุปผลการเอาชีวิตรอด (SURVIVAL OVER) -";
    u32 nLg = Font_Print(0, 8, pTitle, FONT_NoDisp);
    Font_Print((SCR_Width - nLg) / 2, 30, pTitle, FONT_Highlight);
    
    char pBuffer[128];
    s32 nBaseY = 60;
    
    snprintf(pBuffer, sizeof(pBuffer), "เวฟที่รอดชีวิตสูงสุด:   เวฟ %d", gRogue.stats.nHighestWave);
    Font_Print(50, nBaseY, pBuffer, 0);
    
    snprintf(pBuffer, sizeof(pBuffer), "ศัตรูที่กำจัดทั้งหมด:   %d ตัว", gRogue.stats.nTotalKills);
    Font_Print(50, nBaseY + 20, pBuffer, 0);
    
    snprintf(pBuffer, sizeof(pBuffer), "คอมโบต่อเนื่องสูงสุด:   x%d คอมโบ", gRogue.combo.nMaxCombo);
    Font_Print(50, nBaseY + 40, pBuffer, 0);
    
    u32 nSeconds = gRogue.stats.nSurvivalTime / 60;
    u32 nMinutes = nSeconds / 60;
    nSeconds = nSeconds % 60;
    snprintf(pBuffer, sizeof(pBuffer), "เวลาในการเอาชีวิตรอด:  %02d:%02d", nMinutes, nSeconds);
    Font_Print(50, nBaseY + 60, pBuffer, 0);
    
    Roguelike_CalculateFinalScore();
    snprintf(pBuffer, sizeof(pBuffer), "คะแนนสรุปทั้งหมด:     %d แต้ม", gRogue.stats.nFinalScore);
    Font_Print(50, nBaseY + 80, pBuffer, FONT_Highlight);
    
    char pPrompt[] = "กดปุ่ม A หรือ SPACE เพื่อบันทึกสถิติและดูหอเกียรติยศ";
    u32 nPromptLg = Font_Print(0, 8, pPrompt, FONT_NoDisp);
    Font_Print((SCR_Width - nPromptLg) / 2, SCR_Height - 20, pPrompt, 0);
}

//=============================================================================
// Leaderboard Implementation
//=============================================================================

struct SRogueScoreEntry gRogueHighScores[ROGUE_HISC_MAX];
static u8 bLeaderboardInit = 0;

void Roguelike_InitLeaderboard(void)
{
    if (bLeaderboardInit) return;
    
    const char *defaultNames[ROGUE_HISC_MAX] = { "MS1", "MAR", "TAR", "ERI", "FIO", "TRE", "NAD", "RAL", "CLA", "CPU" };
    for (int i = 0; i < ROGUE_HISC_MAX; i++)
    {
        strcpy(gRogueHighScores[i].pName, defaultNames[i]);
        gRogueHighScores[i].nScore = (ROGUE_HISC_MAX - i) * 15000;
        gRogueHighScores[i].nKills = (ROGUE_HISC_MAX - i) * 18;
        gRogueHighScores[i].nWave = (ROGUE_HISC_MAX - i);
    }
    bLeaderboardInit = 1;
}

s32 Roguelike_CheckHighScore(u32 nScore)
{
    if (nScore == 0) return -1;
    for (int i = 0; i < ROGUE_HISC_MAX; i++)
    {
        if (nScore > gRogueHighScores[i].nScore)
        {
            return i;
        }
    }
    return -1;
}

void Roguelike_InsertHighScore(char *pName, u32 nScore, u32 nKills, u16 nWave)
{
    s32 nRank = Roguelike_CheckHighScore(nScore);
    if (nRank == -1) return;
    
    for (int i = ROGUE_HISC_MAX - 1; i > nRank; i--)
    {
        gRogueHighScores[i] = gRogueHighScores[i-1];
    }
    
    strncpy(gRogueHighScores[nRank].pName, pName, 3);
    gRogueHighScores[nRank].pName[3] = '\0';
    gRogueHighScores[nRank].nScore = nScore;
    gRogueHighScores[nRank].nKills = nKills;
    gRogueHighScores[nRank].nWave = nWave;
}

void Roguelike_DrawLeaderboard(void)
{
    char pTitle[] = "- หอเกียรติยศ (ROGUELIKE) -";
    u32 nLg = Font_Print(0, 8, pTitle, FONT_NoDisp);
    Font_Print((SCR_Width - nLg) / 2, 17, pTitle, 0);

    for (int i = 0; i < ROGUE_HISC_MAX; i++)
    {
        s32 nPosX = 16;
        s32 nPosY = 41 + (i * 18);
        char pStr[16];

        strcpy(pStr, "00");
        MyItoA(i + 1, pStr);
        Font_PrintSpc(nPosX, nPosY, pStr, 0, 9);

        Font_Print(nPosX + 28, nPosY, gRogueHighScores[i].pName, 0);

        char pWaveStr[32];
        snprintf(pWaveStr, sizeof(pWaveStr), "เวฟ %02d (%d ตัว)", gRogueHighScores[i].nWave, gRogueHighScores[i].nKills);
        Font_Print(nPosX + 68, nPosY, pWaveStr, 0);

        strcpy(pStr, "00000000");
        MyItoA(gRogueHighScores[i].nScore, pStr);
        Font_PrintSpc(nPosX + 210, nPosY, pStr, 0, 9);
    }
}

// Game Over Wait Loop
void Roguelike_GameOverWait(void)
{
    Roguelike_CalculateFinalScore();
    Roguelike_InsertHighScore("YOU", gRogue.stats.nFinalScore, gRogue.stats.nTotalKills, gRogue.stats.nHighestWave);
    
    Music_Start(e_YmMusic_GameOver, 1);
    gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] = 0;
    
    while (1)
    {
        FrameWait();
        EventHandler(1);
        
        if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] || gVar.pKeys[SDL_SCANCODE_RETURN] || gVar.pKeys[SDL_SCANCODE_SPACE]) break;
        
        gVar.pBackground = gVar.pBkg[0];
        Bkg1Scroll(-gnFrame >> 1, -gnFrame >> 1);

        Roguelike_DrawGameOver();
        
        SprDisplayAll_Pass1();
        SprDisplayAll_Pass2();
        
        RenderFlip(1);
    }
    Music_Start(e_YmMusic_NoMusic, 1);
}

void Roguelike_ShowLeaderboardLoop(void)
{
    Menu(MenuRoguelikeHighScores_Init, MenuRoguelikeHighScores_Main);
}

void Roguelike_OnPlayerDeath(void)
{
    if (gRogue.nActive)
    {
        gRogue.nPhase = e_Rogue_Phase_GameOver;
        gRogue.nPhaseTimer = 0;
        Roguelike_CalculateFinalScore();
    }
}

static u32 g_nAutoBombTimer = 0;
s32 g_nRogueAimDir = -1; // -1 = no target, 0 = Right, 1 = Left

void Roguelike_AutoAim_Update(void)
{
    if (!gRogue.nActive) return;
    if (gRogue.nPhase != e_Rogue_Phase_Playing) return;
    
    // Completely stop auto-aim, auto-fire, and bombs when player is dying or dead
    if (gShoot.nDeathFlag || 
        gShoot.nPlayerLives < 0 ||
        AnmGetKey(gShoot.nPlayerAnm) == e_AnmKey_Hero_Death ||
        AnmGetKey(gShoot.nPlayerAnm) == e_AnmKey_Hero_DeathAir ||
        gGameVar.nPhase == e_Game_PlayerDead ||
        gGameVar.nPhase == e_Game_GameOver)
    {
        g_nRogueAimDir = -1;
        gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] = 0;
        gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonC]] = 0;
        return;
    }

    // Auto replenish/switch to pistol if weapon ammo depleted
    if (gShoot.nAmmo == 0)
    {
        Player_WeaponSet(e_Player_Weapon_Gun);
    }

    // 1. Scan for nearest hostile enemy on screen
    s32 nBestDistSq = 99999999;
    s32 nBestDx = 0;
    s32 nBestDy = 0;
    u8 bFoundTarget = 0;
    u8 bHeavyTarget = 0;

    for (int i = 0; i < MST_MAX_SLOTS; i++)
    {
        if (!gpMstSlots[i].nUsed) continue;
        u8 mstType = gpMstSlots[i].nMstNo;
        
        // Hostile enemy types
        int bIsEnemy = (mstType == e_Mst2_Enemy1 ||
                        mstType == e_Mst6_RShobu ||
                        mstType == e_Mst7_Zombie1 ||
                        mstType == e_Mst14_RebelSoldier0 ||
                        mstType == e_Mst15_Truck0 ||
                        mstType == e_Mst20_Boss ||
                        mstType == e_Mst25_RocketDiver0 ||
                        mstType == e_Mst26_Girida0 ||
                        mstType == e_Mst27_HalfBoss ||
                        mstType == e_Mst28_Masknell0 ||
                        mstType == e_Mst43_FlyingTara0 ||
                        mstType == e_Mst46_HairBusterRibert0);
        if (!bIsEnemy) continue;

        s32 dx = (gpMstSlots[i].nPosX - gShoot.nPlayerPosX) >> 8;
        s32 dy = (gpMstSlots[i].nPosY - gShoot.nPlayerPosY) >> 8;

        // Expanded screen detection range
        if (ABS(dx) < 240 && ABS(dy) < 180)
        {
            s32 distSq = dx * dx + dy * dy;
            if (distSq < nBestDistSq)
            {
                nBestDistSq = distSq;
                nBestDx = dx;
                nBestDy = dy;
                bFoundTarget = 1;
                if (mstType == e_Mst26_Girida0 || mstType == e_Mst20_Boss ||
                    mstType == e_Mst27_HalfBoss || mstType == e_Mst46_HairBusterRibert0)
                {
                    bHeavyTarget = 1;
                }
            }
        }
    }

    if (bFoundTarget)
    {
        // 2. Lock Facing Direction towards enemy
        if (nBestDx < -4)
        {
            gShoot.nPlayerDir = 1; // Face Left
            g_nRogueAimDir = 1;
        }
        else if (nBestDx > 4)
        {
            gShoot.nPlayerDir = 0; // Face Right
            g_nRogueAimDir = 0;
        }

        // 3. Auto-Aim Up if enemy is overhead
        if (nBestDy < -28 && ABS(nBestDx) < 75)
        {
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Up]] = 1;
        }

        // 4. ALWAYS trigger shooting
        gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] = 1;

        // 5. Smart Bomb Launch
        if (gShoot.nBombAmmo > 0)
        {
            if (g_nAutoBombTimer > 0)
            {
                g_nAutoBombTimer--;
            }
            else
            {
                int bInBombRange = (ABS(nBestDx) >= 20 && ABS(nBestDx) <= 145 && nBestDy >= -50 && nBestDy <= 50);
                if (bInBombRange || (bHeavyTarget && ABS(nBestDx) <= 160))
                {
                    gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonC]] = 1;
                    g_nAutoBombTimer = 55; // ~0.9s cooldown
                }
            }
        }
    }
    else
    {
        g_nRogueAimDir = -1;
        // Continuous forward firing
        gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] = 1;
    }
}

//=============================================================================
// Roguelike Game Loop Entry
//=============================================================================

void RoguelikeGame(void)
{
    Roguelike_Init();
    g_nAutoBombTimer = 0;
    
    gCCodes.nCheat = 0;
    s32 nCredits = 1;
    
    u8 nOriginalScrollType = gMissionTb[MISSIONOFFS_LEVELS].nScrollType;
    gMissionTb[MISSIONOFFS_LEVELS].nScrollType = e_ScrollType_Free;
    
    ExgPlatformerInit(nCredits, MISSIONOFFS_LEVELS);
    
    gShoot.nPlayerLives = ROGUE_PLAYER_START_LIVES - 1;
    gShoot.nBombAmmo = ROGUE_PLAYER_START_BOMBS;
    
    Transit2D_Reset();
    
    FrameInit();
    #if CACHE_ON == 1
    CacheClear();
    #endif
    
    while (gGameVar.nExitCode == 0)
    {
        #ifdef DEBUG_KEYS
        if (EventHandler(1) != 0) { LevelRelease(); gGameVar.nExitCode = e_Game_Aborted; break; }
        #else
        EventHandler(1);
        #endif
        
        // Guard against inactivity abort and level completion triggers in Roguelike mode
        gpMstQuestItems[MST_QUEST_ITEM_NEXT_LEVEL] = 0;
        gShoot.nInactivityCnt = 0;
        if (gGameVar.nPhase == e_Game_MissionEnd || gGameVar.nPhase == e_Game_LevelCompleted || gGameVar.nPhase == e_Game_MissionEnd_2)
        {
            gGameVar.nPhase = e_Game_Normal;
        }

        // Auto-Aim, Auto-Fire, and Auto-Bomb Aimbot Engine
        Roguelike_AutoAim_Update();

        PlatformerGame();
        Roguelike_Main();
        
        if (gShoot.nPlayerLives < 0 && gRogue.nPhase != e_Rogue_Phase_GameOver)
        {
            gRogue.nPhase = e_Rogue_Phase_GameOver;
            gGameVar.nExitCode = e_Game_GameOver;
            break;
        }
        
        if (gGameVar.nPhase == e_Game_GameOver || gGameVar.nPhase == e_Game_PlayerDead)
        {
            gRogue.nPhase = e_Rogue_Phase_GameOver;
            gGameVar.nExitCode = e_Game_GameOver;
            break;
        }
        
        RenderFlip(1);
    }
    
    gMissionTb[MISSIONOFFS_LEVELS].nScrollType = nOriginalScrollType;
    
    Roguelike_Exit();
    Music_Start(e_YmMusic_NoMusic, 1);
    
    if (gGameVar.nExitCode != e_Game_Aborted)
    {
        Roguelike_GameOverWait();
        Menu(MenuRoguelikeHighScores_Init, MenuRoguelikeHighScores_Main);
    }
}
