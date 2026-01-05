// Roguelike Mode - Main Implementation
// MiniSlug Roguelike - Wave survival mode

#include "includes.h"
#include <SDL2/SDL.h>
#include "roguelike.h"

// External functions from main.c

// External functions from main.c
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
// Monster Pools - Different monsters for different difficulty levels
//=============================================================================

// Easy monsters (Waves 1-5)
static u8 gpMonsterPool_Easy[] = {
    e_Mst2_Enemy1,
    e_Mst7_Zombie1,
};
#define POOL_EASY_SIZE (sizeof(gpMonsterPool_Easy) / sizeof(gpMonsterPool_Easy[0]))

// Medium monsters (Waves 6-10)
static u8 gpMonsterPool_Medium[] = {
    e_Mst2_Enemy1,
    e_Mst7_Zombie1,
    e_Mst14_RebelSoldier0,
    e_Mst6_RShobu,
};
#define POOL_MEDIUM_SIZE (sizeof(gpMonsterPool_Medium) / sizeof(gpMonsterPool_Medium[0]))

// Hard monsters (Waves 11-15)
static u8 gpMonsterPool_Hard[] = {
    e_Mst2_Enemy1,
    e_Mst7_Zombie1,
    e_Mst14_RebelSoldier0,
    e_Mst6_RShobu,
    e_Mst25_RocketDiver0,
    e_Mst26_Girida0,
};
#define POOL_HARD_SIZE (sizeof(gpMonsterPool_Hard) / sizeof(gpMonsterPool_Hard[0]))

// Insane monsters (Waves 16-20)
static u8 gpMonsterPool_Insane[] = {
    e_Mst14_RebelSoldier0,
    e_Mst6_RShobu,
    e_Mst25_RocketDiver0,
    e_Mst26_Girida0,
    e_Mst28_Masknell0,
    e_Mst43_FlyingTara0,
};
#define POOL_INSANE_SIZE (sizeof(gpMonsterPool_Insane) / sizeof(gpMonsterPool_Insane[0]))

// Nightmare monsters (Waves 21+)
static u8 gpMonsterPool_Nightmare[] = {
    e_Mst14_RebelSoldier0,
    e_Mst25_RocketDiver0,
    e_Mst26_Girida0,
    e_Mst28_Masknell0,
    e_Mst43_FlyingTara0,
};
#define POOL_NIGHTMARE_SIZE (sizeof(gpMonsterPool_Nightmare) / sizeof(gpMonsterPool_Nightmare[0]))

//=============================================================================
// Utility Functions
//=============================================================================

// Get difficulty level based on wave number
u8 Roguelike_GetDifficulty(u32 nWave)
{
    if (nWave <= 5)  return e_Rogue_Diff_Easy;
    if (nWave <= 10) return e_Rogue_Diff_Medium;
    if (nWave <= 15) return e_Rogue_Diff_Hard;
    if (nWave <= 20) return e_Rogue_Diff_Insane;
    return e_Rogue_Diff_Nightmare;
}

// Calculate monsters needed for a wave
u32 Roguelike_GetMonstersForWave(u32 nWave)
{
    u32 nMonsters = ROGUE_WAVE_START_MONSTERS + (nWave / 3);
    if (nMonsters > ROGUE_WAVE_MAX_MONSTERS)
        nMonsters = ROGUE_WAVE_MAX_MONSTERS;
    return nMonsters;
}

// Calculate spawn interval for a wave (gets faster as waves progress)
u32 Roguelike_GetSpawnInterval(u32 nWave)
{
    s32 nInterval = ROGUE_SPAWN_INTERVAL_BASE - ((nWave / 5) * 10);
    if (nInterval < ROGUE_SPAWN_INTERVAL_MIN)
        nInterval = ROGUE_SPAWN_INTERVAL_MIN;
    return (u32)nInterval;
}

//=============================================================================
// Core Functions
//=============================================================================

// Initialize Roguelike mode
void Roguelike_Init(void)
{
    // Clear all state
    memset(&gRogue, 0, sizeof(struct SRogueState));
    
    // Mark as active
    gRogue.nActive = 1;
    Roguelike_InitLeaderboard();
    
    // Initialize wave state
    gRogue.wave.nWaveNo = 0;
    gRogue.wave.nDifficulty = e_Rogue_Diff_Easy;
    
    // Initialize stats
    gRogue.stats.nTotalKills = 0;
    gRogue.stats.nSurvivalTime = 0;
    gRogue.stats.nHighestWave = 0;
    gRogue.stats.nFinalScore = 0;
    
    // Initialize item drop timers
    gRogue.nItemDropTimerAmmo = ROGUE_ITEM_DROP_AMMO_INTERVAL;
    gRogue.nItemDropTimerBomb = ROGUE_ITEM_DROP_BOMB_INTERVAL;
    
    // Set initial phase
    gRogue.nPhase = e_Rogue_Phase_Init;
    gRogue.nPhaseTimer = 120;  // 2 second intro
    
    // Reset combo
    gRogue.combo.nComboCount = 0;
    gRogue.combo.nComboTimer = 0;
    gRogue.combo.nMaxCombo = 0;
    
    // Clear perks
    memset(&gRogue.perks, 0, sizeof(struct SRoguePerks));
    
    // No active power-up
    gRogue.powerUp.nType = 0;
    gRogue.powerUp.nTimer = 0;
}

// Main roguelike game loop (called every frame)
void Roguelike_Main(void)
{
    if (!gRogue.nActive) return;
    
    // Update survival time
    gRogue.stats.nSurvivalTime++;
    
    // Update combo system
    Roguelike_ComboUpdate();
    
    // Update power-up timer
    if (gRogue.powerUp.nTimer > 0)
        gRogue.powerUp.nTimer--;
    
    // Phase state machine
    switch (gRogue.nPhase)
    {
    case e_Rogue_Phase_Init:
        // Initial delay before first wave
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
        // Show "WAVE X" message
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
        // Spawn monsters
        if (gRogue.wave.nMonstersSpawned < gRogue.wave.nMonstersTotal)
        {
            if (gRogue.wave.nSpawnTimer > 0)
            {
                gRogue.wave.nSpawnTimer--;
            }
            else
            {
                // Check if we can spawn more (limit active monsters)
                u32 nActiveMonsters = MstOnScreenNb(e_Mst2_Enemy1, 10) + 
                                      MstOnScreenNb(e_Mst7_Zombie1, 10) +
                                      MstOnScreenNb(e_Mst14_RebelSoldier0, 10);
                
                if (nActiveMonsters < ROGUE_MAX_ACTIVE_MONSTERS)
                {
                    Roguelike_SpawnMonster();
                    gRogue.wave.nSpawnTimer = gRogue.wave.nSpawnInterval;
                }
            }
        }
        
        // Check wave completion
        if (Roguelike_IsWaveComplete())
        {
            Roguelike_WaveComplete();
        }
        
        // Item drops
        Roguelike_DropItem();
        break;
        
    case e_Rogue_Phase_WaveComplete:
        // Show wave complete message
        Roguelike_DrawWaveComplete();
        if (gRogue.nPhaseTimer > 0)
        {
            gRogue.nPhaseTimer--;
        }
        else
        {
            // Check if perk selection
            if (gRogue.wave.nWaveNo % ROGUE_PERK_SELECT_WAVES == 0)
            {
                Roguelike_InitPerkSelection();
                Roguelike_InitPerkSelection();
                gRogue.nPhase = e_Rogue_Phase_PerkSelect;
                gRogue.nPhaseTimer = 30; // Delay for 30 frames (0.5s) to prevent accidental skip
            }
            else
            {
                // Start next wave
                Roguelike_WaveStart(gRogue.wave.nWaveNo + 1);
            }
        }
        break;
        
    case e_Rogue_Phase_PerkSelect:
        Roguelike_ShowPerkSelection();
        break;
        
    case e_Rogue_Phase_GameOver:
        Roguelike_DrawGameOver();
        break;
    }
    
    // Draw HUD
    if (gRogue.nPhase == e_Rogue_Phase_Playing)
    {
        Roguelike_DrawHUD();
    }
}

// Exit roguelike mode
void Roguelike_Exit(void)
{
    gRogue.nActive = 0;
    gRogue.stats.nFinalScore = Roguelike_CalculateFinalScore();
}

//=============================================================================
// Wave Management
//=============================================================================

// Start a new wave
void Roguelike_WaveStart(u32 nWave)
{
    gRogue.wave.nWaveNo = nWave;
    gRogue.wave.nMonstersKilled = 0;
    gRogue.wave.nMonstersTotal = Roguelike_GetMonstersForWave(nWave);
    gRogue.wave.nMonstersSpawned = 0;
    gRogue.wave.nSpawnTimer = 30;  // Half second before first spawn
    gRogue.wave.nSpawnInterval = Roguelike_GetSpawnInterval(nWave);
    gRogue.wave.nDifficulty = Roguelike_GetDifficulty(nWave);
    
    // Update highest wave stat
    if (nWave > gRogue.stats.nHighestWave)
        gRogue.stats.nHighestWave = nWave;
    
    // Set phase
    gRogue.nPhase = e_Rogue_Phase_WaveStart;
    gRogue.nPhaseTimer = 120;  // 2 seconds to show wave message
    
    // Drop weapon capsule at wave milestones
    if (nWave > 1 && (nWave % ROGUE_ITEM_DROP_WEAPON_WAVES == 0))
    {
        Roguelike_DropWeaponCapsule();
    }
}

// Wave completed
void Roguelike_WaveComplete(void)
{
    gRogue.nPhase = e_Rogue_Phase_WaveComplete;
    gRogue.nPhaseTimer = ROGUE_WAVE_COMPLETE_DELAY;
    
    // Bonus points for completing wave
    u32 nWaveBonus = gRogue.wave.nWaveNo * 1000;
    Roguelike_AddScore(nWaveBonus);
}

// Check if current wave is complete
u32 Roguelike_IsWaveComplete(void)
{
    return (gRogue.wave.nMonstersKilled >= gRogue.wave.nMonstersTotal &&
            gRogue.wave.nMonstersSpawned >= gRogue.wave.nMonstersTotal);
}

//=============================================================================
// Monster Spawning
//=============================================================================

// Get random spawn X position (off screen but near player view)
u32 Roguelike_GetSpawnPosX(void)
{
    // Get current scroll position
    s32 nScrollX = gScrollPos.nPosX / 256;
    
    // Spawn off-screen to the left or right
    s32 nSpawnX;
    if (rand() % 2 == 0)
    {
        // Spawn to the right of screen
        nSpawnX = nScrollX + SCR_Width + 32;
    }
    else
    {
        // Spawn to the left of screen
        nSpawnX = nScrollX - 32;
        if (nSpawnX < 16) nSpawnX = nScrollX + SCR_Width + 32;  // Fallback to right
    }
    
    return (u32)nSpawnX;
}

// Get random monster from pool based on difficulty
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

// Spawn a random monster
void Roguelike_SpawnMonster(void)
{
    u32 nMstType = Roguelike_GetRandomMonster(gRogue.wave.nDifficulty);
    u32 nPosX = Roguelike_GetSpawnPosX();
    
    // Get ground level for spawn (use a reasonable Y position)
    u32 nPosY = 160;  // Default ground level
    
    // Add the monster at spawn position with no additional data (NULL)
    s32 nResult = MstAdd(nMstType, nPosX, nPosY, NULL, -1);
    
    if (nResult != -1)
    {
        gRogue.wave.nMonstersSpawned++;
    }
}

//=============================================================================
// Item Drops
//=============================================================================

// Handle periodic item drops
void Roguelike_DropItem(void)
{
    // Ammo drops
    if (gRogue.nItemDropTimerAmmo > 0)
    {
        gRogue.nItemDropTimerAmmo--;
    }
    else
    {
        // Reload weapon
        Player_WeaponReload(0);
        gRogue.nItemDropTimerAmmo = ROGUE_ITEM_DROP_AMMO_INTERVAL;
        
        // Lucky drop perk reduces interval
        if (gRogue.perks.nLuckyDrop > 0)
        {
            gRogue.nItemDropTimerAmmo = (gRogue.nItemDropTimerAmmo * (100 - gRogue.perks.nLuckyDrop * 20)) / 100;
        }
    }
    
    // Bomb drops
    if (gRogue.nItemDropTimerBomb > 0)
    {
        gRogue.nItemDropTimerBomb--;
    }
    else
    {
        // Add bombs
        Player_WeaponReload(5);
        gRogue.nItemDropTimerBomb = ROGUE_ITEM_DROP_BOMB_INTERVAL;
        
        // Lucky drop perk reduces interval
        if (gRogue.perks.nLuckyDrop > 0)
        {
            gRogue.nItemDropTimerBomb = (gRogue.nItemDropTimerBomb * (100 - gRogue.perks.nLuckyDrop * 20)) / 100;
        }
    }
}

// Drop a weapon capsule
void Roguelike_DropWeaponCapsule(void)
{
    // Random weapon (not gun)
    u32 nWeapon = (rand() % (e_Player_Weapon_Max - 1)) + 1;
    Player_WeaponSet(nWeapon);
}

//=============================================================================
// Combo System
//=============================================================================

// Register a kill for combo
void Roguelike_ComboRegisterKill(void)
{
    gRogue.wave.nMonstersKilled++;
    gRogue.stats.nTotalKills++;
    
    // Increase combo
    gRogue.combo.nComboCount++;
    gRogue.combo.nComboTimer = ROGUE_COMBO_TIMEOUT;
    
    // Update max combo
    if (gRogue.combo.nComboCount > gRogue.combo.nMaxCombo)
        gRogue.combo.nMaxCombo = gRogue.combo.nComboCount;
    
    // Add score with combo multiplier
    u32 nBaseScore = 100;
    Roguelike_AddScore(nBaseScore);
}

// Update combo timer (call every frame)
void Roguelike_ComboUpdate(void)
{
    if (gRogue.combo.nComboTimer > 0)
    {
        gRogue.combo.nComboTimer--;
        if (gRogue.combo.nComboTimer == 0)
        {
            // Combo expired
            gRogue.combo.nComboCount = 0;
        }
    }
}

// Get current combo multiplier (1.0 to 10.0, returned as 10-100 for integer math)
u32 Roguelike_GetComboMultiplier(void)
{
    u32 nMultiplier = 10;  // Base 1.0x (expressed as 10)
    
    if (gRogue.combo.nComboCount >= 2)
    {
        nMultiplier = 10 + ((gRogue.combo.nComboCount - 1) * 5);  // +0.5 per combo kill
        if (nMultiplier > 100)
            nMultiplier = 100;  // Cap at 10x
    }
    
    return nMultiplier;
}

//=============================================================================
// Scoring
//=============================================================================

// Add score with combo multiplier
void Roguelike_AddScore(u32 nBasePoints)
{
    u32 nMultiplier = Roguelike_GetComboMultiplier();
    u32 nPoints = (nBasePoints * nMultiplier) / 10;
    
    // Add damage perk bonus
    if (gRogue.perks.nDamageUp > 0)
    {
        nPoints = (nPoints * (100 + gRogue.perks.nDamageUp * 15)) / 100;
    }
    
    gShoot.nPlayerScore += nPoints;
}

// Calculate final score
u32 Roguelike_CalculateFinalScore(void)
{
    u32 nScore = gShoot.nPlayerScore;
    
    // Wave bonus
    nScore += gRogue.stats.nHighestWave * 500;
    
    // Kill bonus
    nScore += gRogue.stats.nTotalKills * 50;
    
    // Max combo bonus
    nScore += gRogue.combo.nMaxCombo * 200;
    
    // Survival time bonus (1 point per second)
    nScore += gRogue.stats.nSurvivalTime / 60;
    
    gRogue.stats.nFinalScore = nScore;
    return nScore;
}

//=============================================================================
// Perks
//=============================================================================

// Helper to get Perk Name
char *Roguelike_GetPerkName(u8 nPerkType)
{
    switch (nPerkType)
    {
    case e_Perk_SpeedBoost: return "SPEED BOOST";
    case e_Perk_DamageUp: return "DAMAGE UP";
    case e_Perk_LuckyDrop: return "LUCKY DROP";
    case e_Perk_Armor: return "ARMOR";
    case e_Perk_ExtendedClip: return "EXTENDED CLIP";
    default: return "UNKNOWN";
    }
}

// Show perk selection screen and handle input
// Show perk selection screen and handle input
void Roguelike_ShowPerkSelection(void)
{
    // Use Menu Background
    gVar.pBackground = gVar.pBkg[0];
    Bkg1Scroll(-gnFrame >> 1, -gnFrame >> 1);

    // Draw the UI
    Roguelike_DrawPerkSelection();
    
    SprDisplayAll_Pass1();
    SprDisplayAll_Pass2();
    
    // Input Delay to prevent accidental skip
    if (gRogue.nPhaseTimer > 0)
    {
        gRogue.nPhaseTimer--;
        return;
    }

    // Input Handling
    
    // Up (Previous)
    if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Up]])
    {
        if (gRogue.nSelectedPerkIdx > 0) 
        {
            gRogue.nSelectedPerkIdx--;
            Sfx_PlaySfx(e_Sfx_MenuClic1, e_SfxPrio_10);
        }
        gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Up]] = 0; // Debounce
    }
    
    // Down (Next)
    if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Down]])
    {
        if (gRogue.nSelectedPerkIdx < 2) 
        {
            gRogue.nSelectedPerkIdx++;
            Sfx_PlaySfx(e_Sfx_MenuClic1, e_SfxPrio_10);
        }
        gVar.pKeys[gMSCfg.pKeys[e_CfgKey_Down]] = 0; // Debounce
    }
    
    // Select (Button A, Enter, Space)
    if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] || 
        gVar.pKeys[SDL_SCANCODE_RETURN] || 
        gVar.pKeys[SDL_SCANCODE_SPACE])
    {
        Sfx_PlaySfx(e_Sfx_MenuClic2, e_SfxPrio_10);
        
        // Apply selected perk
        u8 nSelectedPerk = gRogue.nOfferedPerks[gRogue.nSelectedPerkIdx];
        Roguelike_ApplyPerk(nSelectedPerk);
        
        // Clear buttons
        gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] = 0;
        gVar.pKeys[SDL_SCANCODE_RETURN] = 0;
        gVar.pKeys[SDL_SCANCODE_SPACE] = 0;
        
        // Move to next wave
        Roguelike_WaveStart(gRogue.wave.nWaveNo + 1);
    }
}

// Apply a perk
void Roguelike_ApplyPerk(u8 nPerkType)
{
    switch (nPerkType)
    {
    case e_Perk_SpeedBoost:
        if (gRogue.perks.nSpeedBoost < 3) gRogue.perks.nSpeedBoost++;
        break;
    case e_Perk_DamageUp:
        if (gRogue.perks.nDamageUp < 3) gRogue.perks.nDamageUp++;
        break;
    case e_Perk_LuckyDrop:
        if (gRogue.perks.nLuckyDrop < 3) gRogue.perks.nLuckyDrop++;
        break;
    case e_Perk_Armor:
        if (gRogue.perks.nArmor < 3) gRogue.perks.nArmor++;
        break;
    case e_Perk_ExtendedClip:
        if (gRogue.perks.nExtendedClip < 3) gRogue.perks.nExtendedClip++;
        break;
    }
}

//=============================================================================
// UI Drawing
//=============================================================================

// Draw roguelike HUD
void Roguelike_DrawHUD(void)
{
    char pBuffer[32];
    
    // Survival time (top right)
    u32 nSeconds = gRogue.stats.nSurvivalTime / 60;
    u32 nMinutes = nSeconds / 60;
    nSeconds = nSeconds % 60;
    sprintf(pBuffer, "%02d:%02d", nMinutes, nSeconds);
    Font_Print(270, 10, pBuffer, 0);

    // Wave number (Below Timer)
    sprintf(pBuffer, "WAVE %d", gRogue.wave.nWaveNo);
    Font_Print(230, 24, pBuffer, 0);
    
    // Kill counter (Below Wave)
    sprintf(pBuffer, "KILLS %d/%d", gRogue.wave.nMonstersKilled, gRogue.wave.nMonstersTotal);
    Font_Print(210, 36, pBuffer, 0);
    
    // Combo (Centered Top)
    if (gRogue.combo.nComboCount >= 2)
    {
        sprintf(pBuffer, "x%d COMBO!", gRogue.combo.nComboCount);
        Font_Print(130, 50, pBuffer, 0);
    }
}

// Draw wave start message
void Roguelike_DrawWaveStart(void)
{
    char pBuffer[32];
    sprintf(pBuffer, "WAVE %d", gRogue.wave.nWaveNo);
    Font_Print(140, 100, pBuffer, 0);
    
    if (gRogue.wave.nWaveNo % ROGUE_BOSS_WAVES == 0 && gRogue.wave.nWaveNo > 0)
    {
        Font_Print(120, 120, "BOSS WAVE!", 0);
    }
}

// Draw wave complete message
void Roguelike_DrawWaveComplete(void)
{
    Font_Print(110, 100, "WAVE COMPLETE!", 0);
    
    char pBuffer[32];
    sprintf(pBuffer, "KILLS: %d", gRogue.wave.nMonstersKilled);
    Font_Print(130, 120, pBuffer, 0);
}

// Draw game over screen
void Roguelike_DrawGameOver(void)
{
    Font_Print(110, 80, "SURVIVAL ENDED", 0);
    
    char pBuffer[48];
    
    sprintf(pBuffer, "WAVE: %d", gRogue.stats.nHighestWave);
    Font_Print(120, 110, pBuffer, 0);
    
    sprintf(pBuffer, "KILLS: %d", gRogue.stats.nTotalKills);
    Font_Print(120, 125, pBuffer, 0);
    
    u32 nSeconds = gRogue.stats.nSurvivalTime / 60;
    u32 nMinutes = nSeconds / 60;
    nSeconds = nSeconds % 60;
    sprintf(pBuffer, "TIME: %02d:%02d", nMinutes, nSeconds);
    Font_Print(120, 140, pBuffer, 0);
    
    Roguelike_CalculateFinalScore();
    sprintf(pBuffer, "SCORE: %d", gRogue.stats.nFinalScore);
    Font_Print(120, 155, pBuffer, 0);
}

// Initialize perk selection
void Roguelike_InitPerkSelection(void)
{
    // Reset selection index
    gRogue.nSelectedPerkIdx = 0;
    
    // Generate 3 unique random perks
    for (int i = 0; i < 3; i++)
    {
        gRogue.nOfferedPerks[i] = rand() % e_Perk_MAX;
    }
}

// Draw perk selection screen
void Roguelike_DrawPerkSelection(void)
{
    char pBuffer[64];
    
    // Clear screen removed (Overlay on pause)
    // if (gpScreen) SDL_FillRect(gpScreen, NULL, 0);
    
    Font_Print(100, 40, "CHOOSE A PERK", 0);
    
    s32 nBaseX = 100;
    s32 nBaseY = 80;
    s32 nSpacingY = 30;
    
    for (int i = 0; i < 3; i++)
    {
        u8 nPerkType = gRogue.nOfferedPerks[i];
        char *pName = Roguelike_GetPerkName(nPerkType);
        s32 nY = nBaseY + (i * nSpacingY);
        
        // Highlight selected
        if (i == gRogue.nSelectedPerkIdx)
        {
             sprintf(pBuffer, "> %s <", pName);
             Font_Print(nBaseX - 20, nY, pBuffer, 0); 
        }
        else
        {
            Font_Print(nBaseX, nY, pName, 0);
        }
    }
}


//=============================================================================
// Player Death Hook (call from game.c when player dies)
//=============================================================================

void Roguelike_OnPlayerDeath(void)
{
    if (gRogue.nActive)
    {
        gRogue.nPhase = e_Rogue_Phase_GameOver;
        gRogue.nPhaseTimer = 0;
        Roguelike_CalculateFinalScore();
    }
}

//=============================================================================
// Main Entry Point - Called from main.c
//=============================================================================

// Special mission entry for Roguelike mode using Level 1 (Desert)
#define ROGUE_MISSIONTB_OFFSET  100  // Special offset for roguelike

// Roguelike game entry point
void RoguelikeGame(void)
{
    // Initialize roguelike state
    Roguelike_Init();
    
    // Use Level 1 (Desert) as base with Free scroll
    // We reuse the normal game loop but with roguelike modifications
    
    // Initialize the game with level 1
    // Note: We need to set up special variables for roguelike mode
    gCCodes.nCheat = 0;  // No cheats in roguelike
    
    // Set credits to 1 (permadeath)
    s32 nCredits = 1;
    
    // START ROGUELIKE OVERRIDES
    // Save original scroll type of Level 1 (Desert)
    u8 nOriginalScrollType = gMissionTb[MISSIONOFFS_LEVELS].nScrollType;
    // Force Free Scroll for Roguelike
    gMissionTb[MISSIONOFFS_LEVELS].nScrollType = e_ScrollType_Free;
    
    // Start with level 1 (index 4 in mission table is M1-1 Desert)
    ExgPlatformerInit(nCredits, MISSIONOFFS_LEVELS);
    
    // Override some settings for roguelike
    gShoot.nPlayerLives = ROGUE_PLAYER_START_LIVES - 1;  // 1 life only
    gShoot.nBombAmmo = ROGUE_PLAYER_START_BOMBS;
    
    Transit2D_Reset();
    
    // Main game loop
    FrameInit();
    #if CACHE_ON == 1
    CacheClear();
    #endif
    
    while (gGameVar.nExitCode == 0)
    {
        // Event handling
        #ifdef DEBUG_KEYS
        if (EventHandler(1) != 0) { LevelRelease(); gGameVar.nExitCode = e_Game_Aborted; break; }
        #else
        EventHandler(1);
        #endif
        
        // Normal game processing
        // Pause game logic during perk selection
        if (gRogue.nPhase != e_Rogue_Phase_PerkSelect)
        {
            PlatformerGame();
        }
        
        // Roguelike update (wave spawning, HUD, etc.)
        if (gGameVar.nPhase == e_Game_Normal)
        {
            Roguelike_Main();
        }
        
        // Check for player death -> trigger roguelike game over
        if (gGameVar.nPhase == e_Game_PlayerDead || gGameVar.nPhase == e_Game_GameOver)
        {
            Roguelike_OnPlayerDeath();
            gGameVar.nExitCode = e_Game_GameOver;
        }
        
        // Transition effects
        Transit2D_Manage();
        
        // Render
        #ifdef DEBUG_KEYS
        RenderFlip(gVar.pKeys[SDL_SCANCODE_U] ? 0 : 1);
        #else
        RenderFlip(1);
        #endif
        
        // Check for game over in roguelike phase
        if (gRogue.nPhase == e_Rogue_Phase_GameOver)
        {
            // Wait for player input to exit
            if (gVar.pKeys[SDL_SCANCODE_RETURN] || gVar.pKeys[SDL_SCANCODE_SPACE] ||
                gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] || gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonB]])
            {
                gGameVar.nExitCode = e_Game_GameOver;
            }
        }
    }
    
    // RESTORE overrides
    gMissionTb[MISSIONOFFS_LEVELS].nScrollType = nOriginalScrollType;
    
    // Cleanup
    Roguelike_Exit();
    Music_Start(e_YmMusic_NoMusic, 1);
    
    // Show Game Over screen with roguelike stats
    if (gGameVar.nExitCode != e_Game_Aborted)
    {
        Roguelike_GameOverWait();
    }
}

//=============================================================================
// Leaderboard Implementation
//=============================================================================

struct SRogueScoreEntry gRogueHighScores[ROGUE_HISC_MAX];
static u8 bLeaderboardInit = 0;

void Roguelike_InitLeaderboard(void)
{
    if (bLeaderboardInit) return;
    
    // Initialize with some default scores
    for (int i = 0; i < ROGUE_HISC_MAX; i++)
    {
        strcpy(gRogueHighScores[i].pName, "CPU");
        gRogueHighScores[i].nScore = (ROGUE_HISC_MAX - i) * 1000;
        gRogueHighScores[i].nKills = (ROGUE_HISC_MAX - i) * 10;
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
            return i; // Qualifies for this rank
        }
    }
    return -1;
}

void Roguelike_InsertHighScore(char *pName, u32 nScore, u32 nKills, u16 nWave)
{
    s32 nRank = Roguelike_CheckHighScore(nScore);
    if (nRank == -1) return;
    
    // Shift scores down
    for (int i = ROGUE_HISC_MAX - 1; i > nRank; i--)
    {
        gRogueHighScores[i] = gRogueHighScores[i-1];
    }
    
    // Insert new score
    strncpy(gRogueHighScores[nRank].pName, pName, 3);
    gRogueHighScores[nRank].pName[3] = '\0';
    
    gRogueHighScores[nRank].nScore = nScore;
    gRogueHighScores[nRank].nKills = nKills;
    gRogueHighScores[nRank].nWave = nWave;
}

void Roguelike_DrawLeaderboard(void)
{
    char pBuffer[64];
    
    Font_Print(100, 40, "HALL OF FAME", 0);
    Font_Print(40, 70, "RNK NAME SCORE  WAVE KILL", 0);
    
    for (int i = 0; i < ROGUE_HISC_MAX; i++)
    {
        s32 nY = 90 + (i * 15);
        sprintf(pBuffer, "%d.  %s  %06d  %02d   %03d", 
            i+1, 
            gRogueHighScores[i].pName, 
            gRogueHighScores[i].nScore,
            gRogueHighScores[i].nWave,
            gRogueHighScores[i].nKills);
        Font_Print(40, nY, pBuffer, 0);
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
        
        if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] || gVar.pKeys[SDL_SCANCODE_RETURN]) break;
        
        // Clear screen removed
        // if (gpScreen) SDL_FillRect(gpScreen, NULL, 0);
        
        // Use Menu Background
        gVar.pBackground = gVar.pBkg[0];
        Bkg1Scroll(-gnFrame >> 1, -gnFrame >> 1);

        Roguelike_DrawGameOver();
        // Roguelike_DrawLeaderboard(); // Removed: Don't show on death
        
        SprDisplayAll_Pass1();
        SprDisplayAll_Pass2();
        
        RenderFlip(1);
    }
    Music_Start(e_YmMusic_NoMusic, 1);
}

// Hall of Fame Loop
void Roguelike_ShowLeaderboardLoop(void)
{
    gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] = 0;
    
    while (1)
    {
        FrameWait();
        EventHandler(1);
        
        if (gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonA]] || 
            gVar.pKeys[gMSCfg.pKeys[e_CfgKey_ButtonB]] ||
            gVar.pKeys[SDL_SCANCODE_RETURN]) 
        {
             break;
        }
        
        // Use Menu Background
        gVar.pBackground = gVar.pBkg[0];
        Bkg1Scroll(-gnFrame >> 1, -gnFrame >> 1);

        Roguelike_DrawLeaderboard();
        
        SprDisplayAll_Pass1();
        SprDisplayAll_Pass2();
        
        RenderFlip(1);
    }
}
