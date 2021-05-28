// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



extern struct ROOM_STRUCT Room[NUM_ROOMS];
extern char *SceneryNouns;
extern struct ROOM_PASSAGES_STRUCT RoomPassages[NUM_ROOMS];
extern struct OBJ_STRUCT Obj[NUM_OBJECTS];
extern struct VERBTOACTION_STRUCT VerbToAction[];
extern struct NOUNPHRASETOOBJ_STRUCT NounPhraseToObj[];
extern struct NOUNPHRASETOFIXEDOBJ_STRUCT NounPhraseToFixedObj[];
extern struct TREASURESCORE_STRUCT TreasureScore[NUM_TREASURESCORES];
extern struct ROOMSCORE_STRUCT RoomScore[NUM_ROOMSCORES];
