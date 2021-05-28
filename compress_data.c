// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include <stdio.h>
#include <string.h>
#include <stdlib.h>



struct LABEL_STRUCT {char *label; char *text;};



int TotalUncompressed;
int TotalCompressed;



//from compression.c
int CompressText(char *text_in, int size_in, char *text_out);



#define NUM_ROOMS  111  //including null room 0



// Label  Room Name^Room Desc

struct LABEL_STRUCT RoomDesc[NUM_ROOMS] =
{
  { "ROOM_NULL"                , "" },
  { "ROOM_WEST_OF_HOUSE"       , "West of House" },
  { "ROOM_STONE_BARROW"        , "Stone Barrow^You are standing in front of a massive barrow of stone. In the east face is a huge stone door which is open. You cannot see into the dark of the tomb." },
  { "ROOM_NORTH_OF_HOUSE"      , "North of House^You are facing the north side of a white house. There is no door here, and all the windows are boarded up. To the north a narrow path winds through the trees." },
  { "ROOM_SOUTH_OF_HOUSE"      , "South of House^You are facing the south side of a white house. There is no door here, and all the windows are boarded." },
  { "ROOM_EAST_OF_HOUSE"       , "Behind House" },
  { "ROOM_FOREST_1"            , "Forest^This is a forest, with trees in all directions. To the east, there appears to be sunlight." },
  { "ROOM_FOREST_2"            , "Forest^This is a dimly lit forest, with large trees all around." },
  { "ROOM_MOUNTAINS"           , "Forest^The forest thins out, revealing impassable mountains." },
  { "ROOM_FOREST_3"            , "Forest^This is a dimly lit forest, with large trees all around." },
  { "ROOM_PATH"                , "Forest Path^This is a path winding through a dimly lit forest. The path heads north-south here. One particularly large tree with some low branches stands at the edge of the path." },
  { "ROOM_UP_A_TREE"           , "Up a Tree" },
  { "ROOM_GRATING_CLEARING"    , "Clearing" },
  { "ROOM_CLEARING"            , "Clearing^You are in a small clearing in a well marked forest path that extends to the east and west." },
  { "ROOM_KITCHEN"             , "Kitchen" },
  { "ROOM_ATTIC"               , "Attic^This is the attic. The only exit is a stairway leading down." },
  { "ROOM_LIVING_ROOM"         , "Living Room" },
  { "ROOM_CELLAR"              , "Cellar^You are in a dark and damp cellar with a narrow passageway leading north, and a crawlway to the south. On the west is the bottom of a steep metal ramp which is unclimbable." },
  { "ROOM_TROLL_ROOM"          , "The Troll Room^This is a small room with passages to the east and south and a forbidding hole leading west. Bloodstains and deep scratches (perhaps made by an axe) mar the walls." },
  { "ROOM_EAST_OF_CHASM"       , "East of Chasm^You are on the east edge of a chasm, the bottom of which cannot be seen. A narrow passage goes north, and the path you are on continues to the east." },
  { "ROOM_GALLERY"             , "Gallery^This is an art gallery. Most of the paintings have been stolen by vandals with exceptional taste. The vandals left through either the north or west exits." },
  { "ROOM_STUDIO"              , "Studio^This appears to have been an artist's studio. The walls and floors are splattered with paints of 69 different colors. Strangely enough, nothing of value is hanging here. At the south end of the room is an open door (also covered with paint). A dark and narrow chimney leads up from a fireplace; although you might be able to get up it, it seems unlikely you could get back down." },
  { "ROOM_MAZE_1"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_2"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_3"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_4"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_DEAD_END_1"          , "Dead End^You have come to a dead end in the maze." },
  { "ROOM_MAZE_5"              , "Maze^This is part of a maze of twisty little passages, all alike. A skeleton, probably the remains of a luckless adventurer, lies here." },
  { "ROOM_DEAD_END_2"          , "Dead End^You have come to a dead end in the maze." },
  { "ROOM_MAZE_6"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_7"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_8"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_DEAD_END_3"          , "Dead End^You have come to a dead end in the maze." },
  { "ROOM_MAZE_9"              , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_10"             , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_11"             , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_GRATING_ROOM"        , "Grating Room" },
  { "ROOM_MAZE_12"             , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_DEAD_END_4"          , "Dead End^You have come to a dead end in the maze." },
  { "ROOM_MAZE_13"             , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_14"             , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_MAZE_15"             , "Maze^This is part of a maze of twisty little passages, all alike." },
  { "ROOM_CYCLOPS_ROOM"        , "Cyclops Room" },
  { "ROOM_STRANGE_PASSAGE"     , "Strange Passage^This is a long passage. To the west is one entrance. On the east there is an old wooden door, with a large opening in it (about cyclops sized)." },
  { "ROOM_TREASURE_ROOM"       , "Treasure Room^This is a large room, whose east wall is solid granite. A number of discarded bags, which crumble at your touch, are scattered about on the floor. There is an exit down a staircase." },
  { "ROOM_RESERVOIR_SOUTH"     , "Reservoir South" },
  { "ROOM_RESERVOIR"           , "Reservoir" },
  { "ROOM_RESERVOIR_NORTH"     , "Reservoir North" },
  { "ROOM_STREAM_VIEW"         , "Stream View^You are standing on a path beside a gently flowing stream. The path follows the stream, which flows from west to east." },
  { "ROOM_IN_STREAM"           , "Stream^You are on the gently flowing stream. The upstream route is too narrow to navigate, and the downstream route is invisible due to twisting walls. There is a narrow beach to land on." },
  { "ROOM_MIRROR_ROOM_1"       , "Mirror Room" },
  { "ROOM_MIRROR_ROOM_2"       , "Mirror Room" },
  { "ROOM_SMALL_CAVE"          , "Cave^This is a tiny cave with entrances west and north, and a staircase leading down." },
  { "ROOM_TINY_CAVE"           , "Cave^This is a tiny cave with entrances west and north, and a dark, forbidding staircase leading down." },
  { "ROOM_COLD_PASSAGE"        , "Cold Passage^This is a cold and damp corridor where a long east-west passageway turns into a southward path." },
  { "ROOM_NARROW_PASSAGE"      , "Narrow Passage^This is a long and narrow corridor where a long north-south passageway briefly narrows even further." },
  { "ROOM_WINDING_PASSAGE"     , "Winding Passage^This is a winding passage. It seems that there are only exits on the east and north." },
  { "ROOM_TWISTING_PASSAGE"    , "Twisting Passage^This is a winding passage. It seems that there are only exits on the east and north." },
  { "ROOM_ATLANTIS_ROOM"       , "Atlantis Room^This is an ancient room, long under water. There is an exit to the south and a staircase leading up." },
  { "ROOM_EW_PASSAGE"          , "East-West Passage^This is a narrow east-west passageway. There is a narrow stairway leading down at the north end of the room." },
  { "ROOM_ROUND_ROOM"          , "Round Room^This is a circular stone room with passages in all directions. Several of them have unfortunately been blocked by cave-ins." },
  { "ROOM_DEEP_CANYON"         , "Deep Canyon" },
  { "ROOM_DAMP_CAVE"           , "Damp Cave^This cave has exits to the west and east, and narrows to a crack toward the south. The earth is particularly damp here." },
  { "ROOM_LOUD_ROOM"           , "Loud Room" },
  { "ROOM_NS_PASSAGE"          , "North-South Passage^This is a high north-south passage, which forks to the northeast." },
  { "ROOM_CHASM_ROOM"          , "Chasm^A chasm runs southwest to northeast and the path follows it. You are on the south side of the chasm, where a crack opens into a passage." },
  { "ROOM_ENTRANCE_TO_HADES"   , "Entrance to Hades^You are outside a large gateway, on which is inscribed\n\n  Abandon every hope all ye who enter here!\n\nThe gate is open; through it you can see a desolation, with a pile of mangled bodies in one corner. Thousands of voices, lamenting some hideous fate, can be heard." },
  { "ROOM_LAND_OF_LIVING_DEAD" , "Land of the Dead^You have entered the Land of the Living Dead. Thousands of lost souls can be heard weeping and moaning. In the corner are stacked the remains of dozens of previous adventurers less fortunate than yourself. A passage exits to the north." },
  { "ROOM_ENGRAVINGS_CAVE"     , "Engravings Cave^You have entered a low cave with passages leading northwest and east." },
  { "ROOM_EGYPT_ROOM"          , "Egyptian Room^This is a room which looks like an Egyptian tomb. There is an ascending staircase to the west." },
  { "ROOM_DOME_ROOM"           , "Dome Room" },
  { "ROOM_TORCH_ROOM"          , "Torch Room" },
  { "ROOM_NORTH_TEMPLE"        , "Temple^This is the north end of a large temple. On the east wall is an ancient inscription, probably a prayer in a long-forgotten language. Below the prayer is a staircase leading down. The west wall is solid granite. The exit to the north end of the room is through huge marble pillars." },
  { "ROOM_SOUTH_TEMPLE"        , "Altar^This is the south end of a large temple. In front of you is what appears to be an altar. In one corner is a small hole in the floor which leads into darkness. You probably could not get back up it." },
  { "ROOM_DAM_ROOM"            , "Dam" },
  { "ROOM_DAM_LOBBY"           , "Dam Lobby^This room appears to have been the waiting room for groups touring the dam. There are open doorways here to the north and east marked \"Private\", and there is a path leading south over the top of the dam." },
  { "ROOM_MAINTENANCE_ROOM"    , "Maintenance Room^This is what appears to have been the maintenance room for Flood Control Dam #3. Apparently, this room has been ransacked recently, for most of the valuable equipment is gone. On the wall in front of you is a group of buttons colored blue, yellow, brown, and red. There are doorways to the west and south." },
  { "ROOM_DAM_BASE"            , "Dam Base^You are at the base of Flood Control Dam #3, which looms above you and to the north. The river Frigid is flowing by here. Along the river are the White Cliffs which seem to form giant walls stretching from north to south along the shores of the river as it winds its way downstream." },
  { "ROOM_RIVER_1"             , "Frigid River^You are on the Frigid River in the vicinity of the Dam. The river flows quietly here. There is a landing on the west shore." },
  { "ROOM_RIVER_2"             , "Frigid River^The river turns a corner here making it impossible to see the Dam. The White Cliffs loom on the east bank and large rocks prevent landing on the west." },
  { "ROOM_RIVER_3"             , "Frigid River^The river descends here into a valley. There is a narrow beach on the west shore below the cliffs. In the distance a faint rumbling can be heard." },
  { "ROOM_WHITE_CLIFFS_NORTH"  , "White Cliffs Beach^You are on a narrow strip of beach which runs along the base of the White Cliffs. There is a narrow path heading south along the Cliffs and a tight passage leading west into the cliffs themselves." },
  { "ROOM_WHITE_CLIFFS_SOUTH"  , "White Cliffs Beach^You are on a rocky, narrow strip of beach beside the Cliffs. A narrow path leads north along the shore." },
  { "ROOM_RIVER_4"             , "Frigid River^The river is running faster here and the sound ahead appears to be that of rushing water. On the east shore is a sandy beach. A small area of beach can also be seen below the cliffs on the west shore." },
  { "ROOM_RIVER_5"             , "Frigid River^The sound of rushing water is nearly unbearable here. On the east shore is a large landing area." },
  { "ROOM_SHORE"               , "Shore^You are on the east shore of the river. The water here seems somewhat treacherous. A path travels from north to south here, the south end quickly turning around a sharp corner." },
  { "ROOM_SANDY_BEACH"         , "Sandy Beach^You are on a large sandy beach on the east shore of the river, which is flowing quickly by. A path runs beside the river to the south here, and a passage is partially buried in sand to the northeast." },
  { "ROOM_SANDY_CAVE"          , "Sandy Cave^This is a sand-filled cave whose exit is to the southwest." },
  { "ROOM_ARAGAIN_FALLS"       , "Aragain Falls" },
  { "ROOM_ON_RAINBOW"          , "On the Rainbow^You are on top of a rainbow (I bet you never thought you would walk on a rainbow), with a magnificent view of the Falls. The rainbow travels east-west here." },
  { "ROOM_END_OF_RAINBOW"      , "End of Rainbow^You are on a small, rocky beach on the continuation of the Frigid River past the Falls. The beach is narrow due to the presence of the White Cliffs. The river canyon opens here and sunlight shines in from above. A rainbow crosses over the falls to the east and a narrow path continues to the southwest." },
  { "ROOM_CANYON_BOTTOM"       , "Canyon Bottom^You are beneath the walls of the river canyon which may be climbable here. The lesser part of the runoff of Aragain Falls flows by below. To the north is a narrow path." },
  { "ROOM_CLIFF_MIDDLE"        , "Rocky Ledge^You are on a ledge about halfway up the wall of the river canyon. You can see from here that the main flow from Aragain Falls twists along a passage which it is impossible for you to enter. Below you is the canyon bottom. Above you is more cliff, which appears climbable." },
  { "ROOM_CANYON_VIEW"         , "Canyon View^You are at the top of the Great Canyon on its west wall. From here there is a marvelous view of the canyon and parts of the Frigid River upstream. Across the canyon, the walls of the White Cliffs join the mighty ramparts of the Flathead Mountains to the east. Following the Canyon upstream to the north, Aragain Falls may be seen, complete with rainbow. The mighty Frigid River flows out from a great dark cavern. To the west and south can be seen an immense forest, stretching for miles around. A path leads northwest. It is possible to climb down into the canyon from here." },
  { "ROOM_MINE_ENTRANCE"       , "Mine Entrance^You are standing at the entrance of what might have been a coal mine. The shaft enters the west wall, and there is another exit on the south end of the room." },
  { "ROOM_SQUEEKY_ROOM"        , "Squeaky Room^You are in a small room. Strange squeaky sounds may be heard coming from the passage at the north end. You may also escape to the east." },
  { "ROOM_BAT_ROOM"            , "Bat Room^You are in a small room which has doors only to the east and south." },
  { "ROOM_SHAFT_ROOM"          , "Shaft Room^This is a large room, in the middle of which is a small shaft descending through the floor into darkness below. To the west and the north are exits from this room. Constructed over the top of the shaft is a metal framework to which a heavy iron chain is attached." },
  { "ROOM_SMELLY_ROOM"         , "Smelly Room^This is a small nondescript room. However, from the direction of a small descending staircase a foul odor can be detected. To the south is a narrow tunnel." },
  { "ROOM_GAS_ROOM"            , "Gas Room^This is a small room which smells strongly of coal gas. There is a short climb up some stairs and a narrow tunnel leading east." },
  { "ROOM_LADDER_TOP"          , "Ladder Top^This is a very small room. In the corner is a rickety wooden ladder, leading downward. It might be safe to descend. There is also a staircase leading upward." },
  { "ROOM_LADDER_BOTTOM"       , "Ladder Bottom^This is a rather wide room. On one side is the bottom of a narrow wooden ladder. To the west and the south are passages leaving the room." },
  { "ROOM_DEAD_END_5"          , "Dead End^You have come to a dead end in the mine." },
  { "ROOM_TIMBER_ROOM"         , "Timber Room^This is a long and narrow passage, which is cluttered with broken timbers. A wide passage comes from the east and turns at the west end of the room into a very narrow passageway. From the west comes a strong draft." },
  { "ROOM_LOWER_SHAFT"         , "Drafty Room^This is a small drafty room in which is the bottom of a long shaft. To the south is a passageway and to the east a very narrow passage. In the shaft can be seen a heavy iron chain." },
  { "ROOM_MACHINE_ROOM"        , "Machine Room" },
  { "ROOM_MINE_1"              , "Coal Mine^This is a nondescript part of a coal mine." },
  { "ROOM_MINE_2"              , "Coal Mine^This is a nondescript part of a coal mine." },
  { "ROOM_MINE_3"              , "Coal Mine^This is a nondescript part of a coal mine." },
  { "ROOM_MINE_4"              , "Coal Mine^This is a nondescript part of a coal mine." },
  { "ROOM_SLIDE_ROOM"          , "Slide Room^This is a small chamber, which appears to have been part of a coal mine. On the south wall of the chamber the letters \"Granite Wall\" are etched in the rock. To the east is a long passage, and there is a steep metal slide twisting downward. To the north is a small opening." }
};



#define NUM_BLOCK_MESSAGES  23



struct LABEL_STRUCT BlockMsg[NUM_BLOCK_MESSAGES] =
{
  { "BL0 = (256-23)", "You can't go that way." },
  { "BL1", "The door is boarded and you can't remove the boards." },
  { "BL2", "The windows are all boarded." },
  { "BL3", "You would need a machete to go further west." },
  { "BL4", "There is no tree here suitable for climbing." },
  { "BL5", "The forest becomes impenetrable to the north." },
  { "BL6", "The mountains are impassable." },
  { "BL7", "Storm-tossed trees block your way." },
  { "BL8", "The rank undergrowth prevents eastward movement." },
  { "BL9", "You cannot climb any higher." },
  { "BLA", "You try to ascend the ramp, but it is impossible, and you slide back down." },
  { "BLB", "The chasm probably leads straight to the infernal regions." },
  { "BLC", "The dam blocks your way." },
  { "BLD", "The stream emerges from a spot too small for you to enter." },
  { "BLE", "The channel is too narrow." },
  { "BLF", "It is too narrow for most insects." },
  { "BLG", "Are you out of your mind?" },
  { "BLH", "You cannot reach the rope." },
  { "BLI", "The White Cliffs prevent your landing here." },
  { "BLJ", "You cannot go upstream due to strong currents." },
  { "BLK", "Just in time you steer away from the rocks." },
  { "BLL", "It's a long way..." },
  { "BLM", "You wouldn't fit and would die if you could." }
};



#define NUM_OBJECTS  76



// Label  Name^Desc Before Pickup^Desc After Pickup

struct LABEL_STRUCT ObjectDesc[NUM_OBJECTS] =
{
  { "OBJ_NOTHING"            , "nothing^Nothing is here.^Nothing is here." },
  { "OBJ_YOU"                , "you^You are here.^You are here." },
  { "OBJ_CYCLOPS"            , "" },
  { "OBJ_GHOSTS"             , "" },
  { "OBJ_BAT"                , "" },
  { "OBJ_THIEF"              , "" },
  { "OBJ_TROLL"              , "" },
  { "OBJ_LOWERED_BASKET"     , "a basket^From the chain is suspended a basket.^From the chain is suspended a basket." },
  { "OBJ_RAISED_BASKET"      , "a basket^At the end of the chain is a basket.^At the end of the chain is a basket." },
  { "OBJ_TROPHY_CASE"        , "a trophy case^There is a trophy case here.^There is a trophy case here." },
  { "OBJ_MACHINE"            , "a machine^There is a machine here.^There is a machine here." },
  { "OBJ_MAILBOX"            , "a small mailbox^There is a small mailbox here.^There is a small mailbox here." },
  { "OBJ_WATER"              , "a quantity of water^There is a quantity of water here.^There is a quantity of water here." },
  { "OBJ_SKULL"              , "a crystal skull^Lying in one corner of the room is a beautifully carved crystal skull. It appears to be grinning at you rather nastily.^There is a crystal skull here." },
  { "OBJ_TIMBERS"            , "a broken timber^There is a broken timber here.^There is a broken timber here." },
  { "OBJ_LUNCH"              , "a lunch^A hot pepper sandwich is here.^A hot pepper sandwich is here." },
  { "OBJ_BELL"               , "a brass bell^There is a brass bell here.^There is a brass bell here." },
  { "OBJ_HOT_BELL"           , "a red hot brass bell^On the ground is a red hot bell.^On the ground is a red hot bell." },
  { "OBJ_BOOK"               , "a black book^On the altar is a large black book, open to page 569.^There is a black book here." },
  { "OBJ_AXE"                , "a bloody axe^There is a bloody axe here.^There is a bloody axe here." },
  { "OBJ_BROKEN_LAMP"        , "a broken lantern^There is a broken lantern here.^There is a broken lantern here." },
  { "OBJ_SCEPTRE"            , "a sceptre^A sceptre, possibly that of ancient Egypt itself, is in the coffin. The sceptre is ornamented with colored enamel, and tapers to a sharp point.^An ornamented sceptre, tapering to a sharp point, is here." },
  { "OBJ_SANDWICH_BAG"       , "a brown sack^On the table is an elongated brown sack, smelling of hot peppers.^There is a brown sack here." },
  { "OBJ_CHALICE"            , "a chalice^There is a silver chalice, intricately engraved, here.^There is a silver chalice, intricately engraved, here." },
  { "OBJ_GARLIC"             , "a clove of garlic^There is a clove of garlic here.^There is a clove of garlic here." },
  { "OBJ_TRIDENT"            , "a crystal trident^On the shore lies Poseidon's own crystal trident.^There is a crystal trident here." },
  { "OBJ_BOTTLE"             , "a glass bottle^A bottle is sitting on the table.^There is a glass bottle here." },
  { "OBJ_COFFIN"             , "a gold coffin^The solid-gold coffin used for the burial of Ramses II is here.^The solid-gold coffin used for the burial of Ramses II is here." },
  { "OBJ_PUMP"               , "a hand-held air pump^There is a hand-held air pump here.^There is a hand-held air pump here." },
  { "OBJ_DIAMOND"            , "a huge diamond^There is an enormous diamond (perfectly cut) here.^There is an enormous diamond (perfectly cut) here." },
  { "OBJ_JADE"               , "a jade figurine^There is an exquisite jade figurine here.^There is an exquisite jade figurine here." },
  { "OBJ_KNIFE"              , "a nasty knife^On a table is a nasty-looking knife.^There is a nasty knife here." },
  { "OBJ_BURNED_OUT_LANTERN" , "a burned-out lantern^The deceased adventurer's useless lantern is here.^There is a burned-out lantern here." },
  { "OBJ_BAG_OF_COINS"       , "a leather bag of coins^An old leather bag, bulging with coins, is here.^An old leather bag, bulging with coins, is here." },
  { "OBJ_LAMP"               , "a brass lantern^A battery-powered brass lantern is on the trophy case.^There is a brass lantern (battery-powered) here." },
  { "OBJ_EMERALD"            , "a large emerald^There is a large emerald here.^There is a large emerald here." },
  { "OBJ_ADVERTISEMENT"      , "a leaflet^A small leaflet is on the ground.^A small leaflet is on the ground." },
  { "OBJ_INFLATED_BOAT"      , "" },
  { "OBJ_MATCH"              , "a matchbook^There is a matchbook whose cover says \"Visit Beautiful FCD#3\" here.^There is a matchbook whose cover says \"Visit Beautiful FCD#3\" here." },
  { "OBJ_PAINTING"           , "a painting^Fortunately, there is still one chance for you to be a vandal, for on the far wall is a painting of unparalleled beauty.^A painting by a neglected genius is here." },
  { "OBJ_CANDLES"            , "a pair of candles^On the two ends of the altar are burning candles.^There is a pair of candles here." },
  { "OBJ_GUNK"               , "a small piece of vitreous slag^There is a small piece of vitreous slag here.^There is a small piece of vitreous slag here." },
  { "OBJ_LEAVES"             , "a pile of leaves^On the ground is a pile of leaves.^On the ground is a pile of leaves." },
  { "OBJ_PUNCTURED_BOAT"     , "a punctured boat^There is a punctured boat here.^There is a punctured boat here." },
  { "OBJ_INFLATABLE_BOAT"    , "a pile of plastic^There is a folded pile of plastic here which has a small valve attached.^There is a folded pile of plastic here which has a small valve attached." },
  { "OBJ_BAR"                , "a platinum bar^On the ground is a large platinum bar.^On the ground is a large platinum bar." },
  { "OBJ_POT_OF_GOLD"        , "a pot of gold^At the end of the rainbow is a pot of gold.^There is a pot of gold here." },
  { "OBJ_BUOY"               , "a red buoy^There is a red buoy here (probably a warning).^There is a red buoy here." },
  { "OBJ_ROPE"               , "a rope^A large coil of rope is lying in the corner.^There is a rope here." },
  { "OBJ_RUSTY_KNIFE"        , "a rusty knife^Beside the skeleton is a rusty knife.^There is a rusty knife here." },
  { "OBJ_BRACELET"           , "a sapphire-encrusted bracelet^There is a sapphire-encrusted bracelet here.^There is a sapphire-encrusted bracelet here." },
  { "OBJ_TOOL_CHEST"         , "a group of tool chests^There is a group of tool chests here.^There is a group of tool chests here." },
  { "OBJ_SCREWDRIVER"        , "a screwdriver^There is a screwdriver here.^There is a screwdriver here." },
  { "OBJ_KEYS"               , "a skeleton key^There is a skeleton key here.^There is a skeleton key here." },
  { "OBJ_SHOVEL"             , "a shovel^There is a shovel here.^There is a shovel here." },
  { "OBJ_COAL"               , "a small pile of coal^There is a small pile of coal here.^There is a small pile of coal here." },
  { "OBJ_SCARAB"             , "a beautiful jeweled scarab^There is a beautiful jeweled scarab here.^There is a beautiful jeweled scarab here." },
  { "OBJ_LARGE_BAG"          , "a large bag^There is a large bag here.^There is a large bag here." },
  { "OBJ_STILETTO"           , "a stiletto^There is a stiletto here.^There is a stiletto here." },
  { "OBJ_SWORD"              , "a sword^Above the trophy case hangs an elvish sword of great antiquity.^There is a sword here." },
  { "OBJ_MAP"                , "an ancient map^In the trophy case is an ancient parchment which appears to be a map.^There is an ancient map here." },
  { "OBJ_BOAT_LABEL"         , "a tan label^There is a tan label here.^There is a tan label here." },
  { "OBJ_TORCH"              , "a torch^Sitting on the pedestal is a flaming torch, made of ivory.^There is a torch here." },
  { "OBJ_GUIDE"              , "a tour guidebook^Some guidebooks entitled \"Flood Control Dam #3\" are on the reception desk.^There is a tour guidebook here." },
  { "OBJ_TRUNK"              , "a trunk of jewels^Lying half buried in the mud is an old trunk, bulging with jewels.^There is an old trunk here, bulging with assorted jewels." },
  { "OBJ_TUBE"               , "a tube^There is an object which looks like a tube of toothpaste here.^There is an object which looks like a tube of toothpaste here." },
  { "OBJ_PUTTY"              , "a viscous material^There is a viscous material here.^There is a viscous material here." },
  { "OBJ_OWNERS_MANUAL"      , "a ZORK owner's manual^Loosely attached to a wall is a small piece of paper.^There is a ZORK owner's manual here." },
  { "OBJ_WRENCH"             , "a wrench^There is a wrench here.^There is a wrench here." },
  { "OBJ_NEST"               , "a bird's nest^Beside you on the branch is a small bird's nest.^There is a bird's nest here." },
  { "OBJ_EGG"                , "a jewel-encrusted egg^In the bird's nest is a large egg encrusted with precious jewels, apparently scavenged by a childless songbird. The egg is covered with fine gold inlay, and ornamented in lapis lazuli and mother-of-pearl. Unlike most eggs, this one is hinged and closed with a delicate looking clasp. The egg appears extremely fragile.^There is a jewel-encrusted egg here." },
  { "OBJ_BROKEN_EGG"         , "a broken jewel-encrusted egg^There is a somewhat ruined egg here.^There is a somewhat ruined egg here." },
  { "OBJ_BAUBLE"             , "a beautiful brass bauble^There is a beautiful brass bauble here.^There is a beautiful brass bauble here." },
  { "OBJ_CANARY"             , "a golden clockwork canary^There is a golden clockwork canary nestled in the egg. It has ruby eyes and a silver beak. Through a crystal window below its left wing you can see intricate machinery inside. It appears to have wound down.^There is a golden clockwork canary here." },
  { "OBJ_BROKEN_CANARY"      , "a broken clockwork canary^There is a golden clockwork canary nestled in the egg. It seems to have recently had a bad experience. The mountings for its jewel-like eyes are empty, and its silver beak is crumpled. Through a cracked crystal window below its left wing you can see the remains of intricate machinery. It is not clear what result winding it would have, as the mainspring seems sprung.^There is a broken clockwork canary here." },
  { "OBJ_ENGRAVINGS"         , "a wall with engravings^There are old engravings on the walls here.^There are old engravings on the walls here." }
};



void CompressPrintList(FILE *f, char *listname, struct LABEL_STRUCT list[], int listsize)
{
  int i, j, uncompressed_size, compressed_size;
  char *uncompressed_text, *compressed_text;
  char *hex = "0123456789abcdef";

  fprintf(f, "unsigned char *%s[%i] =\n{\n", listname, listsize);
  for (i=0; i<listsize; i++)
  {
    uncompressed_text = list[i].text;
    uncompressed_size = strlen(uncompressed_text)+1;

    compressed_text = malloc(uncompressed_size);
    compressed_size = CompressText(uncompressed_text, uncompressed_size, compressed_text);

    TotalUncompressed += uncompressed_size;
    TotalCompressed   += compressed_size;

    fputs("  \"", f);
    for (j=0; j<compressed_size; j++)
    {
      unsigned char ch = compressed_text[j];

      fputs("\\x", f);
      fputc(hex[ch >> 4], f);
      fputc(hex[ch & 15], f);
    }
    fputs((i < listsize-1) ? "\",\n" : "\"\n", f);

    free(compressed_text);
  }
  fputs("};\n\n", f);
}



void PrintLabels(FILE *f, struct LABEL_STRUCT list[], int listsize)
{
  int i;

  fputs("enum\n{\n", f);
  for (i=0; i<listsize; i++)
  {
    fputs("  ", f);
    fputs(list[i].label, f);
    if (i < listsize-1) fputs(",\n", f); else fputs("\n", f);
  }
  fputs("};\n\n", f);
}



void main(void)
{
  FILE *f;
  int i;

  TotalUncompressed = 0;
  TotalCompressed   = 0;

  f = fopen("_data.c", "w");
  CompressPrintList(f, "RoomDesc",   RoomDesc,   NUM_ROOMS);
  CompressPrintList(f, "BlockMsg",   BlockMsg,   NUM_BLOCK_MESSAGES);
  CompressPrintList(f, "ObjectDesc", ObjectDesc, NUM_OBJECTS);
  fclose(f);

  printf("%i%%\n", 100 * TotalCompressed / TotalUncompressed);

  f = fopen("_data.h", "w");
  PrintLabels(f, RoomDesc,   NUM_ROOMS);
  PrintLabels(f, BlockMsg,   NUM_BLOCK_MESSAGES);
  PrintLabels(f, ObjectDesc, NUM_OBJECTS);
  fclose(f);
}
