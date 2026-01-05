// Roguelike Mode header
// MiniSlug Roguelike - Wave survival mode

#ifndef ROGUELIKE_H
#define ROGUELIKE_H

#include "ctypes.h"

//=============================================================================
// Constants
//=============================================================================

#define ROGUE_WAVE_START_MONSTERS       3       // Starting monsters in Wave 1
#define ROGUE_WAVE_MAX_MONSTERS         15      // Maximum monsters per wave
#define ROGUE_SPAWN_INTERVAL_BASE       90      // Base spawn interval (frames) - 1.5 seconds
#define ROGUE_SPAWN_INTERVAL_MIN        30      // Minimum spawn interval
#define ROGUE_WAVE_COMPLETE_DELAY       180     // 3 seconds between waves
#define ROGUE_MAX_ACTIVE_MONSTERS       20      // Maximum concurrent monsters
#define ROGUE_COMBO_TIMEOUT             180     // 3 seconds combo timeout (frames)
#define ROGUE_COMBO_MAX                 10      // Maximum combo multiplier

#define ROGUE_ITEM_DROP_AMMO_INTERVAL   (45 * 60)   // Every 45 seconds
#define ROGUE_ITEM_DROP_BOMB_INTERVAL   (60 * 60)   // Every 60 seconds
#define ROGUE_ITEM_DROP_WEAPON_WAVES    5           // Drop weapon every X waves

#define ROGUE_PERK_SELECT_WAVES         5       // Select perk every X waves
#define ROGUE_BOSS_WAVES                10      // Boss appears every X waves
#define ROGUE_MINIBOSS_WAVES            5       // Mini-boss appears every X waves (after wave 20)

#define ROGUE_PLAYER_START_LIVES        1       // Permadeath - 1 life
#define ROGUE_PLAYER_START_BOMBS        5       // Starting grenades

//=============================================================================
// Enums
//=============================================================================

// Roguelike game phases
enum
{
    e_Rogue_Phase_Init = 0,
    e_Rogue_Phase_WaveStart,
    e_Rogue_Phase_Playing,
    e_Rogue_Phase_WaveComplete,
    e_Rogue_Phase_PerkSelect,
    e_Rogue_Phase_BossWarning,
    e_Rogue_Phase_GameOver,
};

// Difficulty levels for monster pools
enum
{
    e_Rogue_Diff_Easy = 0,      // Waves 1-5
    e_Rogue_Diff_Medium,        // Waves 6-10
    e_Rogue_Diff_Hard,          // Waves 11-15
    e_Rogue_Diff_Insane,        // Waves 16-20
    e_Rogue_Diff_Nightmare,     // Waves 21+
    e_Rogue_Diff_MAX
};

// Perk types
enum
{
    e_Perk_SpeedBoost = 0,      // +10% player speed
    e_Perk_DamageUp,            // +15% damage
    e_Perk_LuckyDrop,           // +20% item drop chance
    e_Perk_Armor,               // -10% damage taken
    e_Perk_ExtendedClip,        // +25% max ammo
    e_Perk_MAX
};

// Temporary power-up types
enum
{
    e_PowerUp_Invincibility = 0,
    e_PowerUp_DoubleDamage,
    e_PowerUp_RapidFire,
    e_PowerUp_MAX
};

//=============================================================================
// Structures
//=============================================================================

// Wave state
struct SRogueWave
{
    u32     nWaveNo;            // Current wave number
    u32     nMonstersKilled;    // Monsters killed this wave
    u32     nMonstersTotal;     // Total monsters to spawn this wave
    u32     nMonstersSpawned;   // Monsters already spawned
    u32     nSpawnTimer;        // Timer for next spawn
    u32     nSpawnInterval;     // Current spawn interval (frames)
    u8      nDifficulty;        // Current difficulty level
};

// Combo system
struct SRogueCombo
{
    u32     nComboCount;        // Current combo kills
    u32     nComboTimer;        // Timer since last kill
    u32     nMaxCombo;          // Best combo this run
};

// Roguelike stats
struct SRogueStats
{
    u32     nTotalKills;        // Total kills this run
    u32     nSurvivalTime;      // Time survived (frames)
    u32     nHighestWave;       // Highest wave reached
    u32     nFinalScore;        // Final score
    u32     nItemsCollected;    // Items collected
};

// Player perks/upgrades
struct SRoguePerks
{
    u8      nSpeedBoost;        // Speed boost level (0-3)
    u8      nDamageUp;          // Damage up level (0-3)
    u8      nLuckyDrop;         // Lucky drop level (0-3)
    u8      nArmor;             // Armor level (0-3)
    u8      nExtendedClip;      // Extended clip level (0-3)
};

// Temporary power-up state
struct SRoguePowerUp
{
    u8      nType;              // Power-up type
    u32     nTimer;             // Remaining duration (frames)
};

// Main roguelike state
struct SRogueState
{
    u32     nPhase;             // Current phase
    u32     nPhaseTimer;        // Timer for phase transitions
    
    struct SRogueWave   wave;
    struct SRogueCombo  combo;
    struct SRogueStats  stats;
    struct SRoguePerks  perks;
    struct SRoguePowerUp powerUp;

    
    // Offered perks for selection
    u8      nOfferedPerks[3];   // Types of perk currently offered
    u8      nSelectedPerkIdx;   // Currently highlighted perk index (0-2)
    
    u32     nItemDropTimerAmmo;     // Timer for ammo drops
    u32     nItemDropTimerBomb;     // Timer for bomb drops
    
    u8      nActive;            // Is roguelike mode active?
};

extern struct SRogueState gRogue;

//=============================================================================
// Function Prototypes
//=============================================================================

// Core functions
void Roguelike_Init(void);
void Roguelike_Main(void);
void Roguelike_Exit(void);

// Wave management
void Roguelike_WaveStart(u32 nWave);
void Roguelike_WaveComplete(void);
u32  Roguelike_IsWaveComplete(void);

// Monster spawning
void Roguelike_SpawnMonster(void);
u32  Roguelike_GetRandomMonster(u8 nDifficulty);
u32  Roguelike_GetSpawnPosX(void);

// Item drops
void Roguelike_DropItem(void);
void Roguelike_DropWeaponCapsule(void);

// Combo system
void Roguelike_ComboRegisterKill(void);
void Roguelike_ComboUpdate(void);
u32  Roguelike_GetComboMultiplier(void);

// Scoring
void Roguelike_AddScore(u32 nBasePoints);
u32  Roguelike_CalculateFinalScore(void);

// Perks
void Roguelike_InitPerkSelection(void);
void Roguelike_ShowPerkSelection(void); // Updates state/logic
void Roguelike_ApplyPerk(u8 nPerkType);

// UI
void Roguelike_DrawHUD(void);
void Roguelike_DrawWaveStart(void);
void Roguelike_DrawWaveComplete(void);
void Roguelike_DrawPerkSelection(void); // Draws the selection screen
void Roguelike_DrawGameOver(void);
void Roguelike_GameOverWait(void); // Loops until exit
void Roguelike_ShowLeaderboardLoop(void); // Hall of Fame loop


// Leaderboard
#define ROGUE_HISC_MAX 5
struct SRogueScoreEntry
{
    char    pName[4];
    u32     nScore;
    u32     nKills;
    u16     nWave;
};

extern struct SRogueScoreEntry gRogueHighScores[ROGUE_HISC_MAX];

// Leaderboard Functions
void Roguelike_InitLeaderboard(void);
s32  Roguelike_CheckHighScore(u32 nScore);
void Roguelike_InsertHighScore(char *pName, u32 nScore, u32 nKills, u16 nWave);
void Roguelike_DrawLeaderboard(void);

// Game entry point (called from main.c)
void RoguelikeGame(void);

// Utility
u8   Roguelike_GetDifficulty(u32 nWave);
u32  Roguelike_GetMonstersForWave(u32 nWave);
u32  Roguelike_GetSpawnInterval(u32 nWave);

#endif // ROGUELIKE_H
