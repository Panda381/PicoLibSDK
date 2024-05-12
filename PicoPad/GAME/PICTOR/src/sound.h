
// ****************************************************************************
//
//                                 Sounds
//
// ****************************************************************************

#define SOUNDMODE_OFF	0	// sound is OFF
#define SOUNDMODE_SOUND	1	// only sound
#define SOUNDMODE_MUSIC	2	// sound + music

#define SOUNDCHAN_MISSILE	0	// sound channel - missiles thrown by actor
#define SOUNDCHAN_ENEMYHIT	1	// sound channel - enemy hit
#define SOUNDCHAN_ACTORHIT	2	// sound channel - actor hit
#define SOUNDCHAN_MUSIC		3	// sound channel - music

// sound format: Intel IMA ADPCM mono 4-bit 22050Hz
#define IMA_SAMPBLOCK	1017	// number of samples per block
extern const u8 Music1Snd[77668];
extern const u8 Music2Snd[38064];
extern const u8 Music3Snd[39164];
extern const u8 Music4Snd[39752];
extern const u8 Music5Snd[37464];
extern const u8 Music6Snd[39172];
extern const u8 Music7Snd[38828];
extern const u8 Music8Snd[37784];
extern const u8 Music9Snd[39588];
extern const u8 Music10Snd[48444];
extern const u8 Music11Snd[38352];
extern const u8 Music12Snd[38356];

extern const u8 EnemyHitSnd[8340];	// enemy explosion
extern const u8 SwingSnd[1252];		// 00 Jill missile
extern const u8 MinigunSnd[2464];	// 01 Bird missile
extern const u8 GunSnd[3572];		// 02 Parrot missile
extern const u8 NoiseSnd[3332];		// 03 Scarabeus missile
extern const u8 ElectricitySnd[3944];	// 04 Stingray missile
extern const u8 GlassSnd[3856];		// 05 Ice Bird missile
extern const u8 FlameSnd[5480];		// 06 Dragon missile
extern const u8 ScreamingSnd[6668];	// 07 Ghost missile
extern const u8 LaserSnd[4852];		// 08 R2D2 missile
extern const u8 PhaserSnd[5992];	// 09 UFO missile
extern const u8 RocketSnd[23096];	// 10 Squirrel missile
extern const u8 ThrowSnd[1468];		// 11 Gingerbread Man missile
extern const u8 BlackholeSnd[11148];	// 12 Devil Bird missile
extern const u8 Blackhole2Snd[2548];	// 12 Devil Bird missile 2

extern const u8 FailSnd[2480];		// fail hit
extern const u8 ZingSnd[4712];		// get bonus
extern const u8 BigbonusSnd[7560];	// big bonus

extern Bool SoundMode; // sound mode

// music samples
#define SND_NUM BG_NUM
extern const u8* SndSamp[SND_NUM];
extern const int SndLen[SND_NUM];

// play music
void MusicStart();
