// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include "_def.h"
#include "_tables.h"



#define HERO  0  // never set



int VillainObj[NUM_VILLAINS] = {OBJ_TROLL, OBJ_THIEF, OBJ_CYCLOPS};
char *VillainName[NUM_VILLAINS] = {"troll", "thief", "cyclops"};
int VillainBestWeaponAgainst[NUM_VILLAINS] = {OBJ_SWORD, OBJ_KNIFE, 0};
int VillainBestWeaponAgainstAdvantage[NUM_VILLAINS] = {1, 1, 0};



extern unsigned char RopeTiedToRail;
extern unsigned char TrollAllowsPassage;
extern unsigned char YouAreDead;
extern unsigned char ThiefHere;
extern unsigned char ThiefEngrossed;
extern unsigned char YouAreStaggered;

extern int Score;
extern int LoadAllowed;
extern int PlayerStrength;
extern int TrollDescType;
extern int ThiefDescType;
extern int EnableCureRoutine;

extern unsigned char VillainAttacking[NUM_VILLAINS];
extern unsigned char VillainStaggered[NUM_VILLAINS];
extern int VillainWakingChance[NUM_VILLAINS];
extern int VillainStrength[NUM_VILLAINS];



//*****************************************************************************

void ThiefRecoverStiletto(void)
{
  if (Obj[OBJ_STILETTO].loc == Obj[OBJ_THIEF].loc)
  {
    Obj[OBJ_STILETTO].loc = INSIDE + OBJ_THIEF;
    Obj[OBJ_STILETTO].prop |= PROP_NODESC;
    Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;
  }
}



int ThiefRob(int loc, int prob)
{
  int flag = 0, obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == loc &&
        (Obj[obj].prop & PROP_NODESC) == 0 &&
        (Obj[obj].prop & PROP_SACRED) == 0 &&
        Obj[obj].thiefvalue > 0 &&
        (prob < 0 || PercentChance(prob, -1)))
  {
    flag = 1;
    Obj[obj].loc = INSIDE + OBJ_THIEF;
    Obj[obj].prop |= PROP_MOVEDDESC;
    Obj[obj].prop |= PROP_NODESC;
    Obj[obj].prop |= PROP_NOTTAKEABLE;
  }

  return flag;
}



int PlayerFightStrength(int adjust)
{
  int s =  STRENGTH_MIN + (STRENGTH_MAX - STRENGTH_MIN) * Score / SCORE_MAX;

  if (adjust)
    s += PlayerStrength;

  return s;
}



int VillainFightStrength(int i, int player_weapon)
{
  int obj, strength;

  obj = VillainObj[i];
  strength = VillainStrength[i];

  if (strength >= 0)
  {
    if (obj == OBJ_THIEF && ThiefEngrossed)
    {
      ThiefEngrossed = 0;
      if (strength > 2) strength = 2;
    }

    if (player_weapon &&
        (Obj[player_weapon].prop & PROP_WEAPON) &&
        player_weapon == VillainBestWeaponAgainst[i])
    {
      strength -= VillainBestWeaponAgainstAdvantage[i];
      if (strength < 1) strength = 1;
    }
  }

  return strength;
}



int ThiefWinning(void)
{
  int vs = VillainStrength[VILLAIN_THIEF];
  int ps = vs - PlayerFightStrength(1);

       if (ps >  3) return PercentChance(90, -1);
  else if (ps >  0) return PercentChance(75, -1);
  else if (ps == 0) return PercentChance(50, -1);
  else if (vs >  1) return PercentChance(25, -1);
  else              return PercentChance(10, -1);
}



int ThiefVsAdventurer(int here)
{
  int prev_darkness;
  int robbed = 0; // 1: player  2: room

  if (YouAreDead == 0 && Obj[OBJ_YOU].loc == ROOM_TREASURE_ROOM)
  {
  }
  else if (ThiefHere == 0)
  {
    if (YouAreDead == 0 && here == 0 && PercentChance(30, -1))
    {
      if (Obj[OBJ_STILETTO].loc == INSIDE + OBJ_THIEF)
      {
        Obj[OBJ_THIEF].prop &= ~PROP_NODESC;
        PrintLine("Someone carrying a large bag is casually leaning against one of the walls here. He does not speak, but it is clear from his aspect that the bag will be taken only over his dead body.");
        ThiefHere = 1;
        return 1;
      }
      else
      {
        Obj[OBJ_STILETTO].loc = INSIDE + OBJ_THIEF;
        Obj[OBJ_STILETTO].prop |= PROP_NODESC;
        Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;
        Obj[OBJ_THIEF].prop &= ~PROP_NODESC;
        PrintLine("You feel a light finger-touch, and turning, notice a grinning figure holding a large bag in one hand and a stiletto in the other.");
        ThiefHere = 1;
        return 1;
      }
    }
    else if (here && VillainAttacking[VILLAIN_THIEF] && ThiefWinning() == 0)
    {
      PrintLine("Your opponent, determining discretion to be the better part of valor, decides to terminate this little contretemps. With a rueful nod of his head, he steps backward into the gloom and disappears.");
      Obj[OBJ_THIEF].prop |= PROP_NODESC;
      VillainAttacking[VILLAIN_THIEF] = 0;
      ThiefRecoverStiletto();
      return 1;
    }
    else if (here && VillainAttacking[VILLAIN_THIEF] && PercentChance(90, -1))
      return 0;
    else if (here && PercentChance(30, -1))
    {
      PrintLine("The holder of the large bag just left, looking disgusted. Fortunately, he took nothing.");
      Obj[OBJ_THIEF].prop |= PROP_NODESC;
      ThiefRecoverStiletto();
      return 1;
    }
    else if (PercentChance(70, -1))
      return 0;
    else if (YouAreDead == 0)
    {
      prev_darkness = IsPlayerInDarkness();

           if (ThiefRob(Obj[OBJ_YOU].loc, 100))  robbed = 2; // room
      else if (ThiefRob(INSIDE + OBJ_YOU,  -1))  robbed = 1; // player

      ThiefHere = 1;

      if (robbed && here == 0)
      {
        PrintText("A seedy-looking individual with a large bag just wandered through the room. On the way through, he quietly abstracted some valuables from ");

        if (robbed == 2)
          PrintText("the room");
        else
          PrintText("your possession");

        PrintLine(", mumbling something about \"Doing unto others before...\"");

        if (IsPlayerInDarkness() != prev_darkness)
          PrintLine("The thief seems to have left you in the dark.");
      }
      else if (here)
      {
        ThiefRecoverStiletto();

        if (robbed)
        {
          PrintText("The thief just left, still carrying his large bag. You may not have noticed that he ");

          if (robbed == 2)
            PrintLine("appropriated the valuables in the room.");
          else
            PrintLine("robbed you blind first.");

          if (IsPlayerInDarkness() != prev_darkness)
            PrintLine("The thief seems to have left you in the dark.");
        }
        else
          PrintLine("The thief, finding nothing of value, left disgusted.");

        Obj[OBJ_THIEF].prop |= PROP_NODESC;
        here = 0;
        return 1;
      }
      else
      {
        PrintLine("A \"lean and hungry\" gentleman just wandered through, carrying a large bag. Finding nothing of value, he left disgruntled.");
        return 1;
      }
    }
  }
  else
  {
    if (here)
    {
      if (PercentChance(30, -1))
      {
        prev_darkness = IsPlayerInDarkness();

             if (ThiefRob(Obj[OBJ_YOU].loc, 100))  robbed = 2; // room
        else if (ThiefRob(INSIDE + OBJ_YOU,  -1))  robbed = 1; // player

        if (robbed)
        {
          PrintText("The thief just left, still carrying his large bag. You may not have noticed that he ");

          if (robbed == 2)
            PrintLine("appropriated the valuables in the room.");
          else
            PrintLine("robbed you blind first.");

          if (IsPlayerInDarkness() != prev_darkness)
            PrintLine("The thief seems to have left you in the dark.");
        }
        else
          PrintLine("The thief, finding nothing of value, left disgusted.");

        Obj[OBJ_THIEF].prop |= PROP_NODESC;
        here = 0;
        ThiefRecoverStiletto();
      }
    }
  }

  return 0;
}



int ThiefDepositBooty(int room)
{
  int flag = 0, obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_THIEF &&
        Obj[obj].thiefvalue > 0 &&
        obj != OBJ_STILETTO &&
        obj != OBJ_LARGE_BAG)
  {
    flag = 1;
    Obj[obj].loc = room;
    if (obj == OBJ_EGG)
      Obj[OBJ_EGG].prop |= PROP_OPEN;
  }

  return flag;
}



int ThiefDropJunk(int room)
{
  int flag = 0, obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == INSIDE + OBJ_THIEF &&
        Obj[obj].thiefvalue == 0 &&
        PercentChance(30, -1) &&
        obj != OBJ_STILETTO &&
        obj != OBJ_LARGE_BAG)
  {
    if (flag == 0 && room == Obj[OBJ_YOU].loc)
    {
      flag = 1;
      PrintLine("The robber, rummaging through his bag, dropped a few items he found valueless.");
    }
    Obj[obj].loc = room;
    Obj[obj].prop &= ~PROP_NODESC;
    Obj[obj].prop &= ~PROP_NOTTAKEABLE;
  }

  return flag;
}



void ThiefHackTreasures(void)
{
  int obj;

  ThiefRecoverStiletto();

  Obj[OBJ_THIEF].prop |= PROP_NODESC;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == ROOM_TREASURE_ROOM &&
        obj != OBJ_CHALICE &&
        obj != OBJ_THIEF)
  {
    Obj[obj].prop &= ~PROP_NODESC;
    Obj[obj].prop &= ~PROP_NOTTAKEABLE;
  }
}



void ThiefRobMaze(int room)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == room &&
        (Obj[obj].prop & PROP_NODESC) == 0 &&
        (Obj[obj].prop & PROP_NOTTAKEABLE) == 0 &&
        PercentChance(40, -1))
  {
    PrintLine("You hear, off in the distance, someone saying \"My, I wonder what this fine item is doing here.\"");
    if (PercentChance(60, 80))
    {
      Obj[obj].loc = INSIDE + OBJ_THIEF;
      Obj[obj].prop |= PROP_MOVEDDESC;
      Obj[obj].prop |= PROP_NODESC;
      Obj[obj].prop |= PROP_NOTTAKEABLE;
    }
    break;
  }
}



void ThiefStealJunk(int room)
{
  int obj;

  for (obj=2; obj<NUM_OBJECTS; obj++)
    if (Obj[obj].loc == room &&
        Obj[obj].thiefvalue == 0 &&
        (Obj[obj].prop & PROP_NODESC) == 0 &&
        (Obj[obj].prop & PROP_NOTTAKEABLE) == 0 &&
        (Obj[obj].prop & PROP_SACRED) == 0 &&
        (obj == OBJ_STILETTO || PercentChance(10, -1)))
  {
    Obj[obj].loc = INSIDE + OBJ_THIEF;
    Obj[obj].prop |= PROP_MOVEDDESC;
    Obj[obj].prop |= PROP_NODESC;
    Obj[obj].prop |= PROP_NOTTAKEABLE;

    if (obj == OBJ_ROPE) // will never happen because it's sacred
      RopeTiedToRail = 0;

    if (room == Obj[OBJ_YOU].loc)
      PrintLine("You suddenly notice that something vanished.");

    break;
  }
}



void ThiefRoutine(void)
{
  int room, here, once = 0;


  // if thief is dead or unconcious
  if (Obj[OBJ_THIEF].loc == 0 ||
      ThiefDescType == 1) // unconcious
    return;


  for (;;) // used only to allow use of break instead of goto
  {
    room = Obj[OBJ_THIEF].loc;
    here = ((Obj[OBJ_THIEF].prop & PROP_NODESC) == 0);

    if (here)
      room = Obj[OBJ_THIEF].loc;

    if (room == ROOM_TREASURE_ROOM &&
        room != Obj[OBJ_YOU].loc)
    {
      if (here)
      {
        here = 0;
        ThiefHackTreasures();
      }
      ThiefDepositBooty(ROOM_TREASURE_ROOM);
    }
    else if (Obj[OBJ_YOU].loc == room &&
             (Room[room].prop & R_LIT) == 0 &&
             Obj[OBJ_TROLL].loc != Obj[OBJ_YOU].loc)
    {
      if (ThiefVsAdventurer(here))
        break; // break out of for(;;)

      if (Obj[OBJ_THIEF].prop & PROP_NODESC)
        here = 0;
    }
    else
    {
      if (Obj[OBJ_THIEF].loc == room &&
          (Obj[OBJ_THIEF].prop & PROP_NODESC) == 0)
      {
        Obj[OBJ_THIEF].prop |= PROP_NODESC;
        here = 0;
      }

      if (Room[room].prop & R_DESCRIBED)
      {
        ThiefRob(room, 75);

        if ((Room[room].prop & R_MAZE) &&
            (Room[Obj[OBJ_YOU].loc].prop & R_MAZE))
          ThiefRobMaze(room);
        else
          ThiefStealJunk(room);
      }
    }

    once = 1-once;
    if (once && here == 0)
    {
      ThiefRecoverStiletto();

      for (;;)
      {
        room++;
        if (room == NUM_ROOMS) room = 1;

        if ((Room[room].prop & R_SACRED) == 0 &&
            (Room[room].prop & R_BODYOFWATER) == 0)
        {
          Obj[OBJ_THIEF].loc = room;
          Obj[OBJ_THIEF].prop |= PROP_NODESC;
          VillainAttacking[VILLAIN_THIEF] = 0;
          ThiefHere = 0;
          break;
        }
      }
    }

    break; // break out of for(;;)
  }

  if (room != ROOM_TREASURE_ROOM)
    ThiefDropJunk(room);
}

//*****************************************************************************



//*****************************************************************************

void PrintWeaponName(int weapon)
{
  switch (weapon)
  {
    case OBJ_STILETTO:    PrintText("stiletto");    break;
    case OBJ_AXE:         PrintText("bloody axe");  break;
    case OBJ_SWORD:       PrintText("sword");       break;
    case OBJ_KNIFE:       PrintText("nasty knife"); break;
    case OBJ_RUSTY_KNIFE: PrintText("rusty knife"); break;
  }
}



const short BlowMsgOffset[10 * 4] =
{
  0, 6, 11, 14, 18, 22, 27, 29, 30, 31,
  0, 4,  5,  8, 12, 15, 19, 22, 24, 25,
  0, 4,  6,  9, 13, 17, 20, 23, 26, 28,
  0, 2,  3,  4,  6,  8, 10, 12, 13, 14
};



// i:       0 - NUM_VILLAINS-1
// blow:    1 - 9
// weapon:  OBJ_*

void PrintBlowRemark(int player_flag, int i, int blow, int weapon)
{
  int j, index, num, msg;

  j = player_flag ? 0 : 1+i;
  index = 10*j + (blow-1);
  num = BlowMsgOffset[index+1] - BlowMsgOffset[index];
  msg = 100*j + BlowMsgOffset[index] + GetRandom(num);

  switch (msg)
  {
    case 100*0 +  0: PrintText("Your "); PrintWeaponName(weapon); PrintText(" misses the "); PrintText(VillainName[i]); PrintText(" by an inch."); break;
    case 100*0 +  1: PrintText("A good slash, but it misses the "); PrintText(VillainName[i]); PrintText(" by a mile."); break;
    case 100*0 +  2: PrintText("You charge, but the "); PrintText(VillainName[i]); PrintText(" jumps nimbly aside."); break;
    case 100*0 +  3: PrintText("Clang! Crash! The "); PrintText(VillainName[i]); PrintText(" parries."); break;
    case 100*0 +  4: PrintText("A quick stroke, but the "); PrintText(VillainName[i]); PrintText(" is on guard."); break;
    case 100*0 +  5: PrintText("A good stroke, but it's too slow; the "); PrintText(VillainName[i]); PrintText(" dodges."); break;
    case 100*0 +  6: PrintText("Your "); PrintWeaponName(weapon); PrintText(" crashes down, knocking the "); PrintText(VillainName[i]); PrintText(" into dreamland."); break;
    case 100*0 +  7: PrintText("The "); PrintText(VillainName[i]); PrintText(" is battered into unconsciousness."); break;
    case 100*0 +  8: PrintText("A furious exchange, and the "); PrintText(VillainName[i]); PrintText(" is knocked out!"); break;
    case 100*0 +  9: PrintText("The haft of your "); PrintWeaponName(weapon); PrintText(" knocks out the "); PrintText(VillainName[i]); PrintText("."); break;
    case 100*0 + 10: PrintText("The "); PrintText(VillainName[i]); PrintText(" is knocked out!"); break;
    case 100*0 + 11: PrintText("It's curtains for the "); PrintText(VillainName[i]); PrintText(" as your "); PrintWeaponName(weapon); PrintText(" removes his head."); break;
    case 100*0 + 12: PrintText("The fatal blow strikes the "); PrintText(VillainName[i]); PrintText(" square in the heart: He dies."); break;
    case 100*0 + 13: PrintText("The "); PrintText(VillainName[i]); PrintText(" takes a fatal blow and slumps to the floor dead."); break;
    case 100*0 + 14: PrintText("The "); PrintText(VillainName[i]); PrintText(" is struck on the arm; blood begins to trickle down."); break;
    case 100*0 + 15: PrintText("Your "); PrintWeaponName(weapon); PrintText(" pinks the "); PrintText(VillainName[i]); PrintText(" on the wrist, but it's not serious."); break;
    case 100*0 + 16: PrintText("Your stroke lands, but it was only the flat of the blade."); break;
    case 100*0 + 17: PrintText("The blow lands, making a shallow gash in the "); PrintText(VillainName[i]); PrintText("'s arm!"); break;
    case 100*0 + 18: PrintText("The "); PrintText(VillainName[i]); PrintText(" receives a deep gash in his side."); break;
    case 100*0 + 19: PrintText("A savage blow on the thigh! The "); PrintText(VillainName[i]); PrintText(" is stunned but can still fight!"); break;
    case 100*0 + 20: PrintText("Slash! Your blow lands! That one hit an artery, it could be serious!"); break;
    case 100*0 + 21: PrintText("Slash! Your stroke connects! This could be serious!"); break;
    case 100*0 + 22: PrintText("The "); PrintText(VillainName[i]); PrintText(" is staggered, and drops to his knees."); break;
    case 100*0 + 23: PrintText("The "); PrintText(VillainName[i]); PrintText(" is momentarily disoriented and can't fight back."); break;
    case 100*0 + 24: PrintText("The force of your blow knocks the "); PrintText(VillainName[i]); PrintText(" back, stunned."); break;
    case 100*0 + 25: PrintText("The "); PrintText(VillainName[i]); PrintText(" is confused and can't fight back."); break;
    case 100*0 + 26: PrintText("The quickness of your thrust knocks the "); PrintText(VillainName[i]); PrintText(" back, stunned."); break;
    case 100*0 + 27: PrintText("The "); PrintText(VillainName[i]); PrintText("'s weapon is knocked to the floor, leaving him unarmed."); break;
    case 100*0 + 28: PrintText("The "); PrintText(VillainName[i]); PrintText(" is disarmed by a subtle feint past his guard."); break;
    case 100*0 + 29: PrintText("Unused"); break;
    case 100*0 + 30: PrintText("Unused"); break;

    case 100*1 +  0: PrintText("The troll swings his axe, but it misses."); break;
    case 100*1 +  1: PrintText("The troll's axe barely misses your ear."); break;
    case 100*1 +  2: PrintText("The axe sweeps past as you jump aside."); break;
    case 100*1 +  3: PrintText("The axe crashes against the rock, throwing sparks!"); break;
    case 100*1 +  4: PrintText("The flat of the troll's axe hits you delicately on the head, knocking you out."); break;
    case 100*1 +  5: PrintText("The troll neatly removes your head."); break;
    case 100*1 +  6: PrintText("The troll's axe stroke cleaves you from the nave to the chops."); break;
    case 100*1 +  7: PrintText("The troll's axe removes your head."); break;
    case 100*1 +  8: PrintText("The axe gets you right in the side. Ouch!"); break;
    case 100*1 +  9: PrintText("The flat of the troll's axe skins across your forearm."); break;
    case 100*1 + 10: PrintText("The troll's swing almost knocks you over as you barely parry in time."); break;
    case 100*1 + 11: PrintText("The troll swings his axe, and it nicks your arm as you dodge."); break;
    case 100*1 + 12: PrintText("The troll charges, and his axe slashes you on your "); PrintWeaponName(weapon); PrintText(" arm."); break;
    case 100*1 + 13: PrintText("An axe stroke makes a deep wound in your leg."); break;
    case 100*1 + 14: PrintText("The troll's axe swings down, gashing your shoulder."); break;
    case 100*1 + 15: PrintText("The troll hits you with a glancing blow, and you are momentarily stunned."); break;
    case 100*1 + 16: PrintText("The troll swings; the blade turns on your armor but crashes broadside into your head."); break;
    case 100*1 + 17: PrintText("You stagger back under a hail of axe strokes."); break;
    case 100*1 + 18: PrintText("The troll's mighty blow drops you to your knees."); break;
    case 100*1 + 19: PrintText("The axe hits your "); PrintWeaponName(weapon); PrintText(" and knocks it spinning."); break;
    case 100*1 + 20: PrintText("The troll swings, you parry, but the force of his blow knocks your "); PrintWeaponName(weapon); PrintText(" away."); break;
    case 100*1 + 21: PrintText("The axe knocks your "); PrintWeaponName(weapon); PrintText(" out of your hand. It falls to the floor."); break;
    case 100*1 + 22: PrintText("The troll hesitates, fingering his axe."); break;
    case 100*1 + 23: PrintText("The troll scratches his head ruminatively:  Might you be magically protected, he wonders?"); break;
    case 100*1 + 24: PrintText("Conquering his fears, the troll puts you to death."); break;

    case 100*2 +  0: PrintText("The thief stabs nonchalantly with his stiletto and misses."); break;
    case 100*2 +  1: PrintText("You dodge as the thief comes in low."); break;
    case 100*2 +  2: PrintText("You parry a lightning thrust, and the thief salutes you with a grim nod."); break;
    case 100*2 +  3: PrintText("The thief tries to sneak past your guard, but you twist away."); break;
    case 100*2 +  4: PrintText("Shifting in the midst of a thrust, the thief knocks you unconscious with the haft of his stiletto."); break;
    case 100*2 +  5: PrintText("The thief knocks you out."); break;
    case 100*2 +  6: PrintText("Finishing you off, the thief inserts his blade into your heart."); break;
    case 100*2 +  7: PrintText("The thief comes in from the side, feints, and inserts the blade into your ribs."); break;
    case 100*2 +  8: PrintText("The thief bows formally, raises his stiletto, and with a wry grin, ends the battle and your life."); break;
    case 100*2 +  9: PrintText("A quick thrust pinks your left arm, and blood starts to trickle down."); break;
    case 100*2 + 10: PrintText("The thief draws blood, raking his stiletto across your arm."); break;
    case 100*2 + 11: PrintText("The stiletto flashes faster than you can follow, and blood wells from your leg."); break;
    case 100*2 + 12: PrintText("The thief slowly approaches, strikes like a snake, and leaves you wounded."); break;
    case 100*2 + 13: PrintText("The thief strikes like a snake! The resulting wound is serious."); break;
    case 100*2 + 14: PrintText("The thief stabs a deep cut in your upper arm."); break;
    case 100*2 + 15: PrintText("The stiletto touches your forehead, and the blood obscures your vision."); break;
    case 100*2 + 16: PrintText("The thief strikes at your wrist, and suddenly your grip is slippery with blood."); break;
    case 100*2 + 17: PrintText("The butt of his stiletto cracks you on the skull, and you stagger back."); break;
    case 100*2 + 18: PrintText("The thief rams the haft of his blade into your stomach, leaving you out of breath."); break;
    case 100*2 + 19: PrintText("The thief attacks, and you fall back desperately."); break;
    case 100*2 + 20: PrintText("A long, theatrical slash. You catch it on your "); PrintWeaponName(weapon); PrintText(", but the thief twists his knife, and the "); PrintWeaponName(weapon); PrintText(" goes flying."); break;
    case 100*2 + 21: PrintText("The thief neatly flips your "); PrintWeaponName(weapon); PrintText(" out of your hands, and it drops to the floor."); break;
    case 100*2 + 22: PrintText("You parry a low thrust, and your "); PrintWeaponName(weapon); PrintText(" slips out of your hand."); break;
    case 100*2 + 23: PrintText("The thief, a man of superior breeding, pauses for a moment to consider the propriety of finishing you off."); break;
    case 100*2 + 24: PrintText("The thief amuses himself by searching your pockets."); break;
    case 100*2 + 25: PrintText("The thief entertains himself by rifling your pack."); break;
    case 100*2 + 26: PrintText("The thief, forgetting his essentially genteel upbringing, cuts your throat."); break;
    case 100*2 + 27: PrintText("The thief, a pragmatist, dispatches you as a threat to his livelihood."); break;

    case 100*3 +  0: PrintText("The Cyclops misses, but the backwash almost knocks you over."); break;
    case 100*3 +  1: PrintText("The Cyclops rushes you, but runs into the wall."); break;
    case 100*3 +  2: PrintText("The Cyclops sends you crashing to the floor, unconscious."); break;
    case 100*3 +  3: PrintText("The Cyclops breaks your neck with a massive smash."); break;
    case 100*3 +  4: PrintText("A quick punch, but it was only a glancing blow."); break;
    case 100*3 +  5: PrintText("A glancing blow from the Cyclops' fist."); break;
    case 100*3 +  6: PrintText("The monster smashes his huge fist into your chest, breaking several ribs."); break;
    case 100*3 +  7: PrintText("The Cyclops almost knocks the wind out of you with a quick punch."); break;
    case 100*3 +  8: PrintText("The Cyclops lands a punch that knocks the wind out of you."); break;
    case 100*3 +  9: PrintText("Heedless of your weapons, the Cyclops tosses you against the rock wall of the room."); break;
    case 100*3 + 10: PrintText("The Cyclops grabs your "); PrintWeaponName(weapon); PrintText(", tastes it, and throws it to the ground in disgust."); break;
    case 100*3 + 11: PrintText("The monster grabs you on the wrist, squeezes, and you drop your "); PrintWeaponName(weapon); PrintText(" in pain."); break;
    case 100*3 + 12: PrintText("The Cyclops seems unable to decide whether to broil or stew his dinner."); break;
    case 100*3 + 13: PrintText("The Cyclops, no sportsman, dispatches his unconscious victim."); break;
  }

  PrintText("\n");
}

//-----------------------------------------------------------------------------

enum
{
  BLOW_NULL,
  BLOW_MISSED,         // attacker misses
  BLOW_UNCONSCIOUS,    // defender unconscious
  BLOW_KILLED,         // defender dead
  BLOW_LIGHT_WOUND,    // defender lightly wounded
  BLOW_SERIOUS_WOUND,  // defender seriously wounded
  BLOW_STAGGER,        // defender staggered (miss turn)
  BLOW_LOSE_WEAPON,    // defender loses weapon
  BLOW_HESITATE,       // hesitates (miss on free swing)
  BLOW_SITTING_DUCK    // sitting duck (crunch!)
};



const unsigned char BlowForDefense1[13] =
{
  BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER,
  BLOW_UNCONSCIOUS, BLOW_UNCONSCIOUS, BLOW_KILLED, BLOW_KILLED, BLOW_KILLED,
  BLOW_KILLED, BLOW_KILLED
};

const unsigned char BlowForDefense2[22] =
{
  BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER,
  BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED,
  BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND,
  BLOW_LIGHT_WOUND, BLOW_UNCONSCIOUS, BLOW_KILLED, BLOW_KILLED, BLOW_KILLED
};

const unsigned char BlowForDefense3[31] =
{
  BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER,
  BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_MISSED,
  BLOW_MISSED, BLOW_MISSED, BLOW_STAGGER, BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND,
  BLOW_LIGHT_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_MISSED,
  BLOW_STAGGER, BLOW_STAGGER, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND, BLOW_LIGHT_WOUND,
  BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND, BLOW_SERIOUS_WOUND
};



int GetBlow(int attack, int defense)
{
  int blow = 0;

  if (defense == 1)
  {
    int j = attack - 1, offset[3] = {0, 2, 4};

    if (j < 0) j = 0; else if (j > 2) j = 2;
    blow = BlowForDefense1[offset[j] + GetRandom(9)];
  }
  else if (defense == 2)
  {
    int j = attack - 1, offset[4] = {0, 9, 11, 13};

    if (j < 0) j = 0; else if (j > 3) j = 3;
    blow = BlowForDefense2[offset[j] + GetRandom(9)];
  }
  else if (defense > 2)
  {
    int j = attack - defense + 2, offset[5] = {0, 2, 11, 13, 22};

    if (j < 0) j = 0; else if (j > 4) j = 4;
    blow = BlowForDefense3[offset[j] + GetRandom(9)];
  }

  return blow;
}



// obj is player or villain obj
int FindWeapon(int obj)
{
  int i, weapon[5] = {OBJ_STILETTO, OBJ_AXE, OBJ_SWORD, OBJ_KNIFE, OBJ_RUSTY_KNIFE};

  for (i=0; i<5; i++)
    if (Obj[weapon[i]].loc == INSIDE + obj) return weapon[i];

  return 0;
}



int PlayerResult(int defense, int blow, int original_defense)
{
  PlayerStrength = (defense == 0) ? -10000 : (defense - original_defense);

  if (defense - original_defense < 0)
    EnableCureRoutine = CURE_WAIT;

  if (PlayerFightStrength(1) <= 0)
  {
    PlayerStrength = 1 - PlayerFightStrength(0);
    PrintLine("It appears that that last blow was too much for you. I'm afraid you are dead.");
    YoureDead(); // ##### RIP #####
    return 0;
  }
  else
    return blow;
}



int VillainBlow(int i, int youre_out)
{
  int attack, defense, original_defense, blow, defense_weapon, next_weapon;

  YouAreStaggered = 0;

  if (VillainStaggered[i])
  {
    VillainStaggered[i] = 0;
    PrintText("The ");
    PrintText(VillainName[i]);
    PrintLine(" slowly regains his feet.");
    return 1;
  }

  attack = VillainFightStrength(i, 0); // don't specify player weapon here

  defense = PlayerFightStrength(1);
  if (defense <= 0) return 1;

  original_defense = PlayerFightStrength(0);

  defense_weapon = FindWeapon(OBJ_YOU);

  blow = GetBlow(attack, defense);

  if (youre_out)
  {
    if (blow == BLOW_STAGGER)
      blow = BLOW_HESITATE;
    else
      blow = BLOW_SITTING_DUCK;
  }

  if (blow == BLOW_STAGGER && defense_weapon && PercentChance(25, HERO ? 10 : 50))
    blow = BLOW_LOSE_WEAPON;

  PrintBlowRemark(0, i, blow, defense_weapon); // 0: villain blow


  if (blow == BLOW_MISSED || blow == BLOW_HESITATE)
    {}
  else if (blow == BLOW_UNCONSCIOUS)
    {}
  else if (blow == BLOW_KILLED || blow == BLOW_SITTING_DUCK)
    defense = 0;
  else if (blow == BLOW_LIGHT_WOUND)
  {
    defense--; if (defense < 0) defense = 0;
    if (LoadAllowed > 50) LoadAllowed -= 10;
  }
  else if (blow == BLOW_SERIOUS_WOUND)
  {
    defense -= 2; if (defense < 0) defense = 0;
    if (LoadAllowed > 50) LoadAllowed -= 20;
  }
  else if (blow == BLOW_STAGGER)
    YouAreStaggered = 1;
  else
  {
    Obj[defense_weapon].loc = Obj[OBJ_YOU].loc;

    next_weapon = FindWeapon(OBJ_YOU);
    if (next_weapon)
    {
      PrintText("Fortunately, you still have a ");
      PrintWeaponName(next_weapon);
      PrintLine(".");
    }
  }


  return PlayerResult(defense, blow, original_defense);
}

//-----------------------------------------------------------------------------

int VillainBusy(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
    {
    }
    else if (Obj[OBJ_AXE].loc == Obj[OBJ_YOU].loc && PercentChance(75, 90))
    {
      Obj[OBJ_AXE].loc = INSIDE + OBJ_TROLL;
      Obj[OBJ_AXE].prop |= PROP_NODESC;
      Obj[OBJ_AXE].prop |= PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop &= ~PROP_WEAPON;

      TrollDescType = 0; // default

      if (Obj[OBJ_TROLL].loc == Obj[OBJ_YOU].loc)
        PrintLine("The troll, angered and humiliated, recovers his weapon. He appears to have an axe to grind with you.");
      return 1;
    }
    else if (Obj[OBJ_TROLL].loc == Obj[OBJ_YOU].loc)
    {
      TrollDescType = 2; // unarmed
      PrintLine("The troll, disarmed, cowers in terror, pleading for his life in the guttural tongue of the trolls.");
      return 1;
    }
  }
  else if (i == VILLAIN_THIEF)
  {
    if (Obj[OBJ_STILETTO].loc == INSIDE + OBJ_THIEF)
    {
    }
    else if (Obj[OBJ_STILETTO].loc == Obj[OBJ_THIEF].loc)
    {
      Obj[OBJ_STILETTO].loc = INSIDE + OBJ_THIEF;
      Obj[OBJ_STILETTO].prop |= PROP_NODESC;
      Obj[OBJ_STILETTO].prop |= PROP_NOTTAKEABLE;
      if (Obj[OBJ_THIEF].loc == Obj[OBJ_YOU].loc)
        PrintLine("The robber, somewhat surprised at this turn of events, nimbly retrieves his stiletto.");
      return 1;
    }
  }

  return 0;
}



void VillainDead(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
    {
      Obj[OBJ_AXE].loc = Obj[OBJ_YOU].loc;
      Obj[OBJ_AXE].prop &= ~PROP_NODESC;
      Obj[OBJ_AXE].prop &= ~PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop |= PROP_WEAPON;
    }
    TrollAllowsPassage = 1;
  }
  else if (i == VILLAIN_THIEF)
  {
    int flag;

    Obj[OBJ_STILETTO].loc = Obj[OBJ_YOU].loc;
    Obj[OBJ_STILETTO].prop &= ~PROP_NODESC;
    Obj[OBJ_STILETTO].prop &= ~PROP_NOTTAKEABLE;

    flag = ThiefDepositBooty(Obj[OBJ_YOU].loc);

    if (Obj[OBJ_YOU].loc == ROOM_TREASURE_ROOM)
    {
      int obj;

      for (obj=2; obj<NUM_OBJECTS; obj++)
        if (Obj[obj].loc == ROOM_TREASURE_ROOM &&
            obj != OBJ_CHALICE &&
            obj != OBJ_THIEF)
      {
        Obj[obj].prop &= ~PROP_NODESC;
        Obj[obj].prop &= ~PROP_NOTTAKEABLE;
      }

      Obj[OBJ_CHALICE].prop |= PROP_NODESC;
      PrintPresentObjects(ROOM_TREASURE_ROOM, "As the thief dies, the power of his magic decreases, and his treasures reappear:", 1); // 1: list, no desc
      Obj[OBJ_CHALICE].prop &= ~PROP_NODESC;

      PrintLine("The chalice is now safe to take.");
    }
    else if (flag)
      PrintLine("His booty remains.");
  }
}



int VillainStrikeFirst(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (PercentChance(33, -1))
    {
      VillainAttacking[i] = 1;
      return 1;
    }
  }
  else if (i == VILLAIN_THIEF)
  {
    if (ThiefHere && (Obj[OBJ_THIEF].prop & PROP_NODESC) == 0 && PercentChance(20, -1))
    {
      VillainAttacking[i] = 1;
      return 1;
    }
  }

  return 0;
}



void VillainUnconcious(int i)
{
  if (i == VILLAIN_TROLL)
  {
    VillainAttacking[i] = 0;

    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
    {
      Obj[OBJ_AXE].loc = Obj[OBJ_YOU].loc;
      Obj[OBJ_AXE].prop &= ~PROP_NODESC;
      Obj[OBJ_AXE].prop &= ~PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop |= PROP_WEAPON;
    }

    TrollDescType = 1; // unconcious
    TrollAllowsPassage = 1;
  }
  else if (i == VILLAIN_THIEF)
  {
    VillainAttacking[i] = 0;

    Obj[OBJ_STILETTO].loc = Obj[OBJ_YOU].loc;
    Obj[OBJ_STILETTO].prop &= ~PROP_NODESC;
    Obj[OBJ_STILETTO].prop &= ~PROP_NOTTAKEABLE;

    ThiefDescType = 1; // unconcious
  }
}



void VillainConscious(int i)
{
  if (i == VILLAIN_TROLL)
  {
    if (Obj[OBJ_TROLL].loc == Obj[OBJ_YOU].loc)
    {
      VillainAttacking[i] = 1;
      PrintLine("The troll stirs, quickly resuming a fighting stance.");
    }

    if (Obj[OBJ_AXE].loc == INSIDE + OBJ_TROLL)
      TrollDescType = 0; // default
    else if (Obj[OBJ_AXE].loc == ROOM_TROLL_ROOM)
    {
      Obj[OBJ_AXE].loc = INSIDE + OBJ_TROLL;
      Obj[OBJ_AXE].prop |= PROP_NODESC;
      Obj[OBJ_AXE].prop |= PROP_NOTTAKEABLE;
      Obj[OBJ_AXE].prop &= ~PROP_WEAPON;
      TrollDescType = 0; // default
    }
    else
      TrollDescType = 3; // simple description

    TrollAllowsPassage = 0;
  }
  else if (i == VILLAIN_THIEF)
  {
    if (Obj[OBJ_THIEF].loc == Obj[OBJ_YOU].loc)
    {
      VillainAttacking[i] = 1;
      PrintLine("The robber revives, briefly feigning continued unconsciousness, and, when he sees his moment, scrambles away from you.");
    }

    ThiefDescType = 0; // default
    ThiefRecoverStiletto();
  }
}



void FightRoutine(void)
{
  int i, obj, youre_attacked = 0, youre_out = 0;

  if (YouAreDead)
    return;

  for (i=0; i<NUM_VILLAINS; i++)
  {
    obj = VillainObj[i];

    if (Obj[obj].loc == Obj[OBJ_YOU].loc &&
        (Obj[obj].prop & PROP_NODESC) == 0)
    {
      if (obj == OBJ_THIEF && ThiefEngrossed)
        ThiefEngrossed = 0;
      else if (VillainStrength[i] < 0)
      {
        if (VillainWakingChance[i] != 0 &&
            PercentChance(VillainWakingChance[i], -1))
        {
          VillainWakingChance[i] = 0;
          if (VillainStrength[i] < 0)
          {
            VillainStrength[i] = -VillainStrength[i];
            VillainConscious(i);
          }
        }
        else
          VillainWakingChance[i] += 25;
      }
      else if (VillainAttacking[i] || VillainStrikeFirst(i))
        youre_attacked = 1;
    }
    else
    {
      if (VillainAttacking[i])
        VillainBusy(i);
      if (obj == OBJ_THIEF)
        ThiefEngrossed = 0;
      YouAreStaggered = 0;
      VillainStaggered[i] = 0;
      VillainAttacking[i] = 0;
      if (VillainStrength[i] < 0)
      {
        VillainStrength[i] = -VillainStrength[i];
        VillainConscious(i);
      }
    }
  }

  if (youre_attacked)
    for (;;)
  {
    for (i=0; i<NUM_VILLAINS; i++)
    {
      if (VillainAttacking[i] == 0) {}
      else if (VillainBusy(i)) {}
      else
      {
        int blow = VillainBlow(i, youre_out);

        if (blow == 0) return;
        else if (blow == BLOW_UNCONSCIOUS)
          youre_out = 1 + 1+GetRandom(3);
      }
    }

    if (youre_out) youre_out--;
    if (youre_out == 0) break;
  }
}

//*****************************************************************************



//*****************************************************************************

void CureRoutine(void)
{
  if (EnableCureRoutine == 0) return;
  EnableCureRoutine--;
  if (EnableCureRoutine != 0) return;

       if (PlayerStrength > 0) PlayerStrength = 0;
  else if (PlayerStrength < 0) PlayerStrength++;

  if (PlayerStrength < 0)
  {
    if (LoadAllowed < LOAD_MAX)
      LoadAllowed += 10;
    EnableCureRoutine = CURE_WAIT;
  }
  else
  {
    LoadAllowed = LOAD_MAX;
    EnableCureRoutine = 0;
  }
}

//*****************************************************************************



//*****************************************************************************

void VillainsRoutine(void)
{
  ThiefRoutine();
  FightRoutine();
  CureRoutine();
}

//*****************************************************************************



//*****************************************************************************

void VillainResult(int i, int defense, int blow)
{
  VillainStrength[i] = defense;

  if (defense == 0)
  {
    PrintText("Almost as soon as the ");
    PrintText(VillainName[i]);
    PrintLine(" breathes his last breath, a cloud of sinister black fog envelops him, and when the fog lifts, the carcass has disappeared.");

    VillainAttacking[i] = 0;
    Obj[VillainObj[i]].loc = 0;

    VillainDead(i);
  }
  else if (blow == BLOW_UNCONSCIOUS)
    VillainUnconcious(i);
}



// obj is thing being attacked by player

void PlayerBlow(int obj, int player_weapon)
{
  int i, attack, defense, defense_weapon, blow;

  for (i=0; i<NUM_VILLAINS; i++)
    if (VillainObj[i] == obj) break;

  if (i < NUM_VILLAINS)
    VillainAttacking[i] = 1;

  if (YouAreStaggered)
  {
    YouAreStaggered = 0;
    PrintLine("You are still recovering from that last blow, so your attack is ineffective.");
    return;
  }

  if (obj == OBJ_YOU)
  {
    PrintLine("Well, you really did it that time. Is suicide painless?");
    YoureDead(); // ##### RIP #####
    return;
  }

  attack = PlayerFightStrength(1);
  if (attack < 1) attack = 1;

  if (i < NUM_VILLAINS)
    defense = VillainFightStrength(i, player_weapon);
  else
    defense = 0;

  if (defense == 0) // catches case of i == NUM_VILLAINS
  {
    PrintLine("Attacking that is pointless.");
    return;
  }

  defense_weapon = FindWeapon(obj);

  if ((defense_weapon == 0 && obj != OBJ_CYCLOPS) || defense < 0)
  {
    PrintText("The ");
    if (defense < 0) PrintText("unconscious ");
    else             PrintText("unarmed ");
    PrintText(VillainName[i]);
    PrintLine(" cannot defend himself: He dies.");
    blow = BLOW_KILLED;
  }
  else
  {
    blow = GetBlow(attack, defense);

    if (blow == BLOW_STAGGER && defense_weapon && PercentChance(25, -1))
      blow = BLOW_LOSE_WEAPON;

    PrintBlowRemark(1, i, blow, player_weapon); // 1: player blow
  }


  if (blow == BLOW_MISSED || blow == BLOW_HESITATE)
  {
  }
  else if (blow == BLOW_UNCONSCIOUS)
    defense = -defense;
  else if (blow == BLOW_KILLED || blow == BLOW_SITTING_DUCK)
    defense = 0;
  else if (blow == BLOW_LIGHT_WOUND)
  {
    defense--;
    if (defense < 0) defense = 0;
  }
  else if (blow == BLOW_SERIOUS_WOUND)
  {
    defense -= 2;
    if (defense < 0) defense = 0;
  }
  else if (blow == BLOW_STAGGER)
    VillainStaggered[i] = 1;
  else
  {
    Obj[defense_weapon].loc = Obj[OBJ_YOU].loc;
    Obj[defense_weapon].prop &= ~PROP_NODESC;
    Obj[defense_weapon].prop &= ~PROP_NOTTAKEABLE;
    Obj[defense_weapon].prop |= PROP_WEAPON;
  }


  VillainResult(i, defense, blow);
}

//*****************************************************************************



//*****************************************************************************

// call just before player enters treasure room

void ThiefProtectsTreasure(void)
{
  int obj, flag = 0;

  // if thief is dead or unconcious
  if (Obj[OBJ_THIEF].loc == 0 ||
      ThiefDescType == 1) // unconcious
    return;

  if (Obj[OBJ_THIEF].loc != ROOM_TREASURE_ROOM)
  {
    PrintLine("You hear a scream of anguish as you violate the robber's hideaway. Using passages unknown to you, he rushes to its defense.");
  
    Obj[OBJ_THIEF].loc = ROOM_TREASURE_ROOM;
    Obj[OBJ_THIEF].prop &= ~PROP_NODESC;
  
    VillainAttacking[VILLAIN_THIEF] = 1;
  
    for (obj=2; obj<NUM_OBJECTS; obj++)
      if (Obj[obj].loc == ROOM_TREASURE_ROOM &&
          obj != OBJ_CHALICE &&
          obj != OBJ_THIEF)
    {
      if (flag == 0)
      {
        flag = 1;
        PrintLine("The thief gestures mysteriously, and the treasures in the room suddenly vanish.");
      }
  
      Obj[obj].prop |= PROP_NODESC;
      Obj[obj].prop |= PROP_NOTTAKEABLE;
    }

    PrintText("\n");
  }
}

//*****************************************************************************
