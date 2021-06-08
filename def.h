// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include <stdio.h>
#include <string.h>
#include <stdlib.h> // malloc free exit atexit
#include <time.h>
#include <ctype.h>



#include "_data.h"



#define NUM_ROOMS  111 //including null room 0

#define NUM_OBJECTS  80

#define SCORE_MAX  350

#define CURE_WAIT  30

#define LOAD_MAX  100

#define MAX_INVENTORY_ITEMS  7
#define INV_LIMIT_CHANCE     8

#define STRENGTH_MIN  2
#define STRENGTH_MAX  7



#define INSIDE  2048



//room properties bit flags

#define R_DESCRIBED     1  // set when room description already printed
#define R_BODYOFWATER   2
#define R_LIT           4  // set when there is natural light or a light fixture
#define R_WATERHERE     8
#define R_SACRED       16  // set when thief not allowed in room
#define R_MAZE         32
#define R_ALWAYSDESC   64



//object bit flags

#define PROP_OPENABLE        1
#define PROP_OPEN            2
#define PROP_LIT             4
#define PROP_NODESC          8
#define PROP_NOTTAKEABLE    16
#define PROP_MOVEDDESC      32  // set when object is first taken
#define PROP_INSIDEDESC     64  // set for objects that are initially described as inside another object
#define PROP_SACRED        128  // set for objects that aren't allowed to be taken by thief
#define PROP_EVERYWHERE    256
#define PROP_WEAPON        512
#define PROP_ACTOR        1024
#define PROP_TOOL         2048
#define PROP_INFLAMMABLE  4096
#define PROP_SURFACE      8192



#define NUM_TREASURESCORES  19
#define NUM_ROOMSCORES      5



//move directions

enum
{
  DIR_NULL,

  DIR_N,
  DIR_S,
  DIR_E,
  DIR_W,
  DIR_NE,
  DIR_NW,
  DIR_SE,
  DIR_SW,
  DIR_U,
  DIR_D,
  DIR_IN,
  DIR_OUT
};



//actions

enum
{
  A_NOTHING,

  A_NORTH, //direction actions must be grouped together in this order
  A_SOUTH,
  A_EAST,
  A_WEST,
  A_NORTHEAST,
  A_NORTHWEST,
  A_SOUTHEAST,
  A_SOUTHWEST,
  A_UP,
  A_DOWN,
  A_IN,
  A_OUT,

  A_ACTIVATE,
  A_ATTACK,
  A_BREAK,
  A_BRIEF,
  A_BRUSH,
  A_CLIMB,
  A_CLIMBDOWN,
  A_CLIMBTHROUGH,
  A_CLIMBUP,
  A_CLOSE,
  A_COUNT,
  A_CROSS,
  A_DEACTIVATE,
  A_DEFLATE,
  A_DIAGNOSE,
  A_DIG,
  A_DISEMBARK,
  A_DISMOUNT,
  A_DRINK,
  A_DROP,
  A_EAT,
  A_ECHO,
  A_EMPTY,
  A_ENTER,
  A_EXAMINE,
  A_EXIT,
  A_EXORCISE,
  A_FILL,
  A_FIX,
  A_GIVE,
  A_GO,
  A_GREET,
  A_INFLATE,
  A_INVENTORY,
  A_JUMP,
  A_KNOCK,
  A_LAND,
  A_LAUNCH,
  A_LISTENTO,
  A_LOCK,
  A_LOOK,
  A_LOOKBEHIND,
  A_LOOKIN,
  A_LOOKON,
  A_LOOKTHROUGH,
  A_LOOKUNDER,
  A_LOWER,
  A_MOUNT,
  A_MOVE,
  A_ODYSSEUS,
  A_OIL,
  A_OPEN,
  A_PLAY,
  A_POUR,
  A_PRAY,
  A_PRY,
  A_PULL,
  A_PUSH,
  A_PUT,
  A_QUIT,
  A_RAISE,
  A_READ,
  A_REMOVE,
  A_RESTART,
  A_RESTORE,
  A_RING,
  A_SAVE,
  A_SAY,
  A_SCORE,
  A_SLEEP,
  A_SLEEPON,
  A_SLIDEDOWN,
  A_SLIDEUP,
  A_SMELL,
  A_SQUEEZE,
  A_SUPERBRIEF,
  A_SWIM,
  A_TAKE,
  A_TALKTO,
  A_THROW,
  A_TIE,
  A_TOUCH,
  A_TURN,
  A_UNLOCK,
  A_UNTIE,
  A_VERBOSE,
  A_VERSION,
  A_WAIT,
  A_WAVE,
  A_WEAR,
  A_WHEREIS,
  A_WIND
};



//fixed (unmoving) objects

enum
{
  FOBJ_SCENERY_VIS = 2048, //some anonymous scenery object, visible
  FOBJ_SCENERY_NOTVIS,     //                               not visible

  FOBJ_NOTVIS,             //fixed object not visible

  FOBJ_AMB,                //amibiguous (ask for clarification)

  //game-specific data follows

  FOBJ_SLIDE,
  FOBJ_BOARD,
  FOBJ_SONGBIRD,
  FOBJ_WHITE_HOUSE,
  FOBJ_FOREST,
  FOBJ_TREE,
  FOBJ_KITCHEN_WINDOW,
  FOBJ_CHIMNEY,
  FOBJ_BOARDED_WINDOW,
  FOBJ_CRACK,
  FOBJ_GRATE,
  FOBJ_CLIMBABLE_CLIFF,
  FOBJ_WHITE_CLIFF,
  FOBJ_BODIES,
  FOBJ_RAINBOW,
  FOBJ_RIVER,
  FOBJ_LADDER,
  FOBJ_TRAP_DOOR,
  FOBJ_STAIRS,
  FOBJ_MOUNTAIN_RANGE,
  FOBJ_BOLT,
  FOBJ_BUBBLE,
  FOBJ_ALTAR,
  FOBJ_YELLOW_BUTTON,
  FOBJ_BROWN_BUTTON,
  FOBJ_RED_BUTTON,
  FOBJ_BLUE_BUTTON,
  FOBJ_RUG,
  FOBJ_DAM,
  FOBJ_FRONT_DOOR,
  FOBJ_BARROW_DOOR,
  FOBJ_BARROW,
  FOBJ_BONES,
  FOBJ_LEAK,
  FOBJ_MIRROR2,
  FOBJ_MIRROR1,
  FOBJ_PRAYER,
  FOBJ_RAILING,
  FOBJ_SAND,
  FOBJ_MACHINE_SWITCH,
  FOBJ_WOODEN_DOOR,
  FOBJ_PEDESTAL,
  FOBJ_CONTROL_PANEL,
  FOBJ_NAILS,
  FOBJ_GRANITE_WALL,
  FOBJ_CHAIN,
  FOBJ_GATE,
  FOBJ_STUDIO_DOOR,
  FOBJ_CHASM,
  FOBJ_LAKE,
  FOBJ_STREAM,
  FOBJ_GAS
};



enum
{
  VILLAIN_TROLL,
  VILLAIN_THIEF,
  VILLAIN_CYCLOPS,

  NUM_VILLAINS
};



struct ROOM_STRUCT
{
  const char *scenery;
  unsigned short      prop;
  const unsigned short init_prop;
};



struct ROOM_PASSAGES_STRUCT
{
  const unsigned char passage[10];
};



struct OBJ_STRUCT
{
  const unsigned short init_loc;
  unsigned short            loc;
  const unsigned short size;
  const unsigned short capacity;
  unsigned short order;
  unsigned short prop;
  const unsigned char init_thiefvalue;
  unsigned char            thiefvalue;
};



struct VERBTOACTION_STRUCT
{
  const char *phrase;
  const unsigned short action;
};



struct NOUNPHRASETOOBJ_STRUCT
{
  const char *phrase;
  const unsigned short obj;
};



struct NOUNPHRASETOFIXEDOBJ_STRUCT
{
  const char *phrase;
  const unsigned short room;
  const unsigned short fobj;
};



struct DOMISCWITH_STRUCT
{
  const int action;
  const int obj; //can be obj or fobj
  void (*f)(int);
};



struct DOMISC_STRUCT
{
  const int action;
  const int obj; //can be obj or fobj
  void (*f)(void);
};



struct DOMISCTO_STRUCT
{
  const int action;
  const int to; //can be obj or fobj
  void (*f)(int);
};



struct GOFROM_STRUCT
{
  const int room;
  const int action; //go-direction action (but technically could be any action)
  int (*f)(void); //function returns 1 if action completed; otherwise fall through
};



struct OVERRIDEROOMDESC_STRUCT
{
  const int room;
  void (*f)(void);
};



struct OVERRIDEOBJECTDESC_STRUCT
{
  const int obj;
  void (*f)(int);
};



struct TREASURESCORE_STRUCT
{
  const int obj;
  const unsigned char take_value;
  const unsigned char case_value;
  unsigned char flags; // &1 taken  &2 put in case
};



struct ROOMSCORE_STRUCT
{
  const int room;
  const unsigned char value;
  unsigned char flag;
};



//parser.c
void PrintText(char *p);
void PrintLine(char *p);
void PrintCompText(char *comp_text);
void PrintCompLine(char *p);
void PrintNewLine(void);
void PrintInteger(int num);
void PrintObjectDesc(int obj, int desc_flag);
void PrintContents(int obj, char *heading, int print_empty);
void PrintBlockMsg(int newroom);
void GetWords(char *prompt);
int IsObjVisible(int obj);
int IsPlayerInDarkness(void);
int GetNumObjectsInLocation(int loc);
void MoveObjOrderToLast(int obj);
void PrintPlayerRoomDesc(int force_description);
void PrintPresentObjects(int location, char *heading, int list_flag);
int MatchCurWord(const char *match);
int GetAllObjFromInput(int room);
int TakeRoutine(int obj, char *msg);
int GetWith(void);

//game.c
int PercentChance(int x, int x_not_lucky);
void YoureDead(void);
void DoJump(void);
void DoSleep(void);
void DoDisembark(void);
void DoLaunch(void);
void DoLand(void);
void DoEcho(void);
void DoPray(void);
void DoVersion(void);
void DoDiagnose(void);
void DoOdysseus(void);
void DoSwim(void);
void DoIntro(void);
void DoCommandActor(int obj);
void DoTalkTo(void);
void DoGreet(void);
void DoSay(void);
int ActionDirectionRoutine(int newroom);
int InterceptAction(int action);
int InterceptTakeObj(int obj);
int GetPlayersVehicle(void);
int InterceptTakeFixedObj(int obj);
int InterceptTakeOutOf(int container);
int InterceptDropPutObj(int obj, int container, int test, int multi);
void ThrowObjRoutine(int obj, int to);
void RunEventRoutines(void);
int CountLoot(void);
int GetScore(void);
int GetMaxScore(void);
char *GetRankName(void);
int GetSaveStateSize(void);
void ReadSaveState(char *p);
void WriteSaveState(char *p);
void InitGameState(void);

//villains.c
int PlayerFightStrength(int adjust);
void ThiefRecoverStiletto(void);
void VillainDead(int i);
void VillainConscious(int i);
void VillainsRoutine(void);
void PlayerBlow(int obj, int player_weapon);
void ThiefProtectsTreasure(void);

//compress.c
int GetDecompressTextSize(char *text_in, int size_in);
int DecompressText(char *text_in, int size_in, char *text_out);

//mt.c
void InitRandom(unsigned int seed);
int GetRandom(int range);
