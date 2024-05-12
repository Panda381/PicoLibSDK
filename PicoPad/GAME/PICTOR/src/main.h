
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

/* Levels: background ... actor ... attack ... enemy1, enemy2, enemy3
 0                  Jill .............. Shurigen
 1 Meadow ......... Bird .............. Seed ................ Fly, Wasp, Hornet
 2 Jungle ......... Parrot ............ Berry ............... Dragonfly, Moth, Kestrel
 3 Sandy Beach .... Scarabeus ......... Sand Ball ........... Maggot, Eagle, Antman
 4 Underwater ..... Stingray .......... Lightning ........... Fish, Anglerfish, Shark
 5 Ice Land ....... Ice Bird .......... Ice Crystal ......... Penguin, Polar Bear, Snowflake
 6 Fiery Land ..... Dragon ............ Fire Ball ........... Phoenix, Fireball, Evil
 7 Haunted Hill ... Ghost ............. Screaming ........... Pumpkin, Zombie, Death
 8 Spacecraft ..... R2-D2 ............. Laser ............... Camera, Explorer, War Dron
 9 Galaxy ......... UFO ............... Phaser .............. Starship Enterprise, Klingon Warship, Death Star
10 Alien Planet ... Rocket Squirrel ... Rocket .............. Alien, Xenomorph, Meteor
11 Candy Land ..... Gingerbread Man ... Sugar Bomb .......... Candy Cane, Lollipop, Fat Man
12 Surreal Land ... Devil Bird ........ Hole of Nothingness . Cloud, Star, Moon
*/

#ifndef _MAIN_H
#define _MAIN_H

// Debug
#define DEBUG		0		// 1=debug mode

#if DEBUG
#define STARTLEVEL	0		// start level-1 (default 0 = Sector 1)
#define DEB_ENDLESS	0		// 1=endless level
#define DEB_SOUND	0		// initial sound mode 0=off, 1=sound, 2=all
#define DEB_ALLACTORS	0		// 1=all actors are available, 0=normal mode
#endif

#define TOPLIST_NAME	"PICTOR.TOP"	// top list filename
#define HOMEPATH	"/GAME"		// default home path

// Game difficulty settings
#define ENEMY_GEN_LEVEL	1.06f	// increase number of enemies per level (relative float number)
#define GAMELEN		700	// game length in number of frames (equals to approx. 1 minute)

// One level settings
#define ENEMY_GEN_BEG	8000	// probability of generating enemies at the beginning of the scene (number 0..65535)
#define ENEMY_GEN_END	14000	// probability of generating enemies at the end of the scene (number 0..65535)
#define ENEMY_GEN_BUL	1200	// probability of generating enemy bullet (number 0..65536)

#define MUSIC_VOL	0.4f	// music volume (float number, 1.0f = max.)

// main setup
#define BG_NUM		12		// number of backgrounds
#define ACT_NUM		(BG_NUM+1)	// number of actors, including Jill (= 13)
#define ENEMY_LEVNUM	3		// number of enemies per level
#define ENEMY_NUM	(BG_NUM*ENEMY_LEVNUM) // number of enemy templates (= 36)
#define ENEMY_MAX	20		// max. enemies on the screen

#endif // _MAIN_H
