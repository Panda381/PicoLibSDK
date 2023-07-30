
// ****************************************************************************
//
//                                 Game engine
//
// ****************************************************************************

#ifndef _GAME_H
#define _GAME_H

#define USBTRANS_VER	100	// USB protocol version

#define TRANSCOL	0x86			// transparent color
#define CASTLEMAX	100			// castle and fence max. height
#define CASTLEY		(HEIGHT-CARDH-16)	// castle Y coordinate of bottom (96 is card, 16 is grass)
#define CARDNUM		8			// number of cards of one player
#define CARDTYPES	30			// number of card types
#define CARD_BACK	CARDTYPES		// card background
#define CARD_SHADOW	(CARDTYPES+1)		// card shadow
#define CARD_DISCARD	(CARDTYPES+2)		// card discard
#define MIDX1		(WIDTH/2-CARDW)		// middle card 1
#define MIDX2		(WIDTH/2)		// middle card 2

// sound mode
#define SOUND_OFF	0		// sound is OFF
#define SOUND_SOME	1		// some sound
#define SOUND_ALL	2		// all sound
extern int SoundMode;

// player's parameters (order must be preserved - used to display status)
enum {
	PAR_BUILDERS = 0,	// 0: number of builders
	PAR_BRICKS,		// 1: number of bricks
	PAR_SOLDIERS,		// 2: number of soldiers
	PAR_WEAPONS,		// 3: number of weapons
	PAR_WIZARDS,		// 4: number of wizards
	PAR_CRYSTALS,		// 5: number of crystals
	PAR_CASTLE,		// 6: height of castle
	PAR_FENCE,		// 7: height of fence

	PAR_NUM			// 8: number of parameters
};

// player type
#define PLAYER_HUMAN	0	// human
#define PLAYER_COMP	1	// computer
#define PLAYER_USB	2	// USB multiplayer

// player descriptor
typedef struct {
	int	player;		// player type PLAYER_*
	int	par[PAR_NUM];	// parameter
	int	add[PAR_NUM];	// parameter addition
	int	cards[CARDNUM]; // cards (-1 none)
	Bool	disable[CARDNUM]; // cards disable
} sPlayer;

// card descriptor
typedef struct {
	int type;	// card type PAR_*
	int costs;	// card value "costs"
	int param;	// parameter to change PAR_* (PAR_NUM = extra)
	int value;	// change value (+ own, - enemy)
} sCard;

// game state USB packet (56 bytes)
#define USBPORT_CMD_STATE	(USBPORT_CMD_APP+0) // command: game state
typedef struct {
	u16	crc;		// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;		// 2: (1) total packet length (= 56)
	u8	cmd;		// 3: (1) command (= USBPORT_CMD_STATE)
	u8	flags;		// 4: (1) flags
				//	B0: 0=active player 1, 1=active player 2
				//	B1: 1=last card was discarded
	s8	last;		// 5: (1) last laid card
	u8	disable1;	// 6: (1) mask of disabled cards of player 1
	u8	disable2;	// 7: (1) mask of disabled cards of player 2
	s8	cards1[CARDNUM]; // 8: (8) cards of player 1
	s8	cards2[CARDNUM]; // 16: (8) cards of player 2
	s16	par1[PAR_NUM];	// 24: (16) parameters of player 1
	s16	par2[PAR_NUM];	// 40: (16) parameters of player 2
} sUsbPortPktState;

// player's turn USB packet (6 bytes)
#define USBPORT_CMD_TURN	(USBPORT_CMD_APP+1) // command: player's turn
typedef struct {
	u16	crc;		// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;		// 2: (1) total packet length (= 6)
	u8	cmd;		// 3: (1) command (= USBPORT_CMD_TURN)
	u8	turn;		// 4: (1) selected card
	u8	discard;	// 5: (1) flag 1=discard card
} sUsbPortPktTurn;

// game
void Game(int player1, int player2, Bool autodemo);

#endif // _GAME_H
