// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif



#include "_def.h"
#include "_tables.h"



FILE *InputStream;

#ifndef NO_STATUS_LINE
int VTMode;
#endif

int CursorColumn;

int NumStrWords;
int CurWord;

char *StrWord[80]; //pointers to words in str; writing to data pointed-to here is prohibited

enum
{
  V_BRIEF,
  V_VERBOSE,
  V_SUPERBRIEF
};

int Verbosity = V_BRIEF;

int ItObj, PrevItObj; //the obj "it" refers to

char *StrAnd = "and";   //point to this string when comma is encountered in input
char *StrThen = "then"; //point to this string when period is encountered in input

#ifndef NO_STATUS_LINE
char StatusLineText[80];
#endif

unsigned char TimePassed; //flag: time passed during action
unsigned char GameOver; //flag, but with special value 2: restart was requested



//from game.c
extern int NumMoves;
extern int Score;
extern int LoadAllowed;
extern struct GOFROM_STRUCT GoFrom[];
extern struct DOMISCWITH_STRUCT DoMiscWithTo[];
extern struct DOMISCTO_STRUCT DoMiscGiveTo[];
extern struct DOMISC_STRUCT DoMisc[];
extern struct OVERRIDEROOMDESC_STRUCT OverrideRoomDesc[];
extern struct OVERRIDEOBJECTDESC_STRUCT OverrideObjectDesc[];

//from compdata.c
extern unsigned char *RoomDesc[];
extern unsigned char *BlockMsg[];
extern unsigned char *ObjectDesc[];
extern unsigned char *Message[];



//*****************************************************************************
int LineWidth = 79;



#ifndef NO_STATUS_LINE

int EnableVTMode(void)
{

#ifdef _WIN32
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;

  if (hOut == INVALID_HANDLE_VALUE) return 0;
  if (!GetConsoleMode(hOut, &dwMode)) return 0;
  dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
  if (!SetConsoleMode(hOut, dwMode)) return 0;

  fputs("\x1b" "[2r", stdout); // set scrolling region to line 2; limits scrollback buffer

  return 1;
#elif __DJGPP__
  return 1;
#else
  return 0;
#endif  

}



void PrintStatusLine(void)
{
  char buf[128];
  int len, i;

  if (VTMode == 0) return;

  fputs("\x1b" "[s" "\x1b" "[1;1H", stdout); // save yx; set yx to (1,1)
  fputs("\x1b" "[7m", stdout); // swap foreground/background colors

  sprintf(buf, "Score: %i   Moves: %i", Score, NumMoves);
  len = strlen(buf);
  if (len > 39) len = 39;
  memcpy(StatusLineText + (79 - len), buf, len);

  for (i=0; i<80; i++)
    fputc(StatusLineText[i], stdout);

  fputs("\x1b" "[0m", stdout); // restore default colors
  fputs("\x1b" "[u", stdout); // restore xy
}

#endif



//does word wrapping; recognizes newline char
//print terminated by '^' or nullchar
void PrintText(char *p)
{
  int width;
  char *q;

  for (;;) //outer loop
  {
    for (;;) //inner loop
    {
      q = p;
      width = LineWidth - CursorColumn; //try to print rest of current line
      while (width > 0 && *q != 0 && *q != '^' && *q != '\n') //find actual width we can print
      {
        width--;
        q++;
      }
      if (width == 0) break; //we can print all so break into outer loop

      while (*p != 0 && *p != '^' && *p != '\n') //print up until end of string or newline
      {
        putchar(*p++);
        CursorColumn++;
      }
      if (*p == 0 || *p == '^') return; //end of string; done

      putchar('\n'); //it was a newline
      CursorColumn = 0;
      p++; //skip newline char and repeat inner loop
    }

    while (q > p && *q != ' ') q--; //try to find a space char to break line
    width = q - p;
    if (width < 1) width = LineWidth - CursorColumn; //didn't find a space so break it ourselves
    while (width > 0) //print number of characters we calculated
    {
      width--;
      putchar(*p++);
    }
    putchar('\n'); //go to next line
    CursorColumn = 0;
    while (*p == ' ') p++; //skip any spaces and repeat outer loop
  }
}



//prints a newline automatically after text
void PrintLine(char *p)
{
  PrintText(p);

  putchar('\n');
  CursorColumn = 0;
}



void PrintCompText(char *comp_text)
{
  char *uncomp_text;
  int comp_size;

  comp_size = strlen(comp_text) + 1;
  uncomp_text = malloc(GetDecompressTextSize(comp_text, comp_size));
  DecompressText(comp_text, comp_size, uncomp_text);

  if (*uncomp_text != 0)
    PrintText(uncomp_text);

  free(uncomp_text);
}



//prints a newline automatically after text
void PrintCompLine(char *p)
{
  PrintCompText(p);

  putchar('\n');
  CursorColumn = 0;
}



void PrintBlankLine(void)
{
  putchar('\n');
  CursorColumn = 0;
}



void PrintBlockMsg(int newroom)
{
  if (newroom >= BL0 && newroom <= 255)
    PrintCompLine(BlockMsg[newroom - BL0]);
}



void PrintObjectDesc(int obj, int desc_flag)
{
  int i;

  //look for object in override obj desc list
  for (i=0; OverrideObjectDesc[i].f != 0; i++)
    if (OverrideObjectDesc[i].obj == obj) break;
  if (OverrideObjectDesc[i].f != 0)
    OverrideObjectDesc[i].f(desc_flag);
  else
  {
    char *compressed_text, *decompressed_text, *p, *q;
    int compressed_size;

    compressed_text = ObjectDesc[obj];
    compressed_size = strlen(compressed_text)+1;
    decompressed_text = p = malloc(GetDecompressTextSize(compressed_text, compressed_size));
    DecompressText(compressed_text, compressed_size, decompressed_text);
  
    if (desc_flag)
    {
      p = strchr(p, '^');
      q = NULL;
      if (p != NULL)
      {
        p++;
        q = strchr(p, '^');
        if (q != NULL)
          q++;
      }
  
      if (Obj[obj].prop & PROP_MOVEDDESC)
        p = q;
    }
  
    if (p != NULL && *p != '^' && *p != 0)
      PrintText(p);
  
    free(decompressed_text);
  }
}



void PrintInteger(int num)
{
  int neg = 0, size = 0, i;
  char buf[80];

  if (num == 0)
  {
    putchar('0');
    CursorColumn++;
    return;
  }

  if (num < 0)
  {
    num = -num;
    neg = 1;
  }

  while (num)
  {
    buf[size++] = '0' + (num % 10);
    num /= 10;
  }

  if (neg)
  {
    putchar('-');
    CursorColumn++;
  }

  for (i=size-1; i>=0; i--)
  {
    putchar(buf[i]);
    CursorColumn++;
  }
}



void PrintScore(void)
{
  PrintText(GameOver ? "\nYou scored " : "Your score is ");
  PrintInteger(GetScore());
  PrintText(" points out of a maximum of ");
  PrintInteger(GetMaxScore());
  PrintText(", using ");
  PrintInteger(NumMoves);
  PrintText((NumMoves == 1) ? " move.\n" : " moves.\n");

  if (GameOver)
  {
    PrintText("\nThat gives you a rank of ");
    PrintText(GetRankName());
    PrintText(".\n");
  }
}



//get input from player

void GetWords(char *prompt)
{
  char *p;

  static char str[80];

  NumStrWords = 0;
  CurWord = 0;

  memset(str, 0, 80);

  PrintText(prompt);
  CursorColumn = 0; // this is due to fgets below

#ifndef NO_STATUS_LINE
  PrintStatusLine();
#endif

  //get input line from stdin or file
  fgets(str, 80, InputStream);
  if (InputStream != stdin && (feof(InputStream) || ferror(InputStream)))
  {
    fclose(InputStream);
    InputStream = stdin;
    fgets(str, 80, InputStream);
  }
  if (InputStream != stdin) PrintText(str);


  //convert upper case chars to lower case, replace whitespace chars with null char, replace ! and ? with .
  p = str;
  while (p < str+80)
  {
    if (isupper(*p)) *p = tolower(*p);
    else if (isspace(*p)) *p = 0;
    else if (*p == '!' || *p == '?') *p = '.';
    p++;
  }

  //fill array of pointers to words in str, or to string literals if replacing punctuation
  p = str;
  for (;;)
  {
    while (*p == 0 && p < str+80) p++;
    if (p == str+80) break;

    if (*p != ',' && *p != '.')
      StrWord[NumStrWords++] = p;

    while (*p != 0 && p < str+80)
    {
      if (*p == ',' || *p == '.')
      {
        StrWord[NumStrWords++] = (*p == ',') ? StrAnd : StrThen;
        *p++ = 0;
        break;
      }
      p++;
    }
    if (p == str+80) break;
  }
}
//*****************************************************************************



//*****************************************************************************

//call only for:  OBJ_YOU < obj < NUM_OBJECTS

//objects inside containers are only visible one level deep

int IsObjVisible(int obj)
{
  if (Obj[obj].prop & PROP_EVERYWHERE) return 1; //presence must be checked by calling function

  if (Obj[obj].loc == 2048 + OBJ_YOU) return 1;   //obj is in your inventory
  if (Obj[obj].loc == Obj[OBJ_YOU].loc) return 1; //obj is in same room as you
  if (Obj[obj].loc >= 2048) //obj is inside something
  {
    int container;

    container = Obj[obj].loc - 2048; //obj is inside container
    if (Obj[container].prop & PROP_OPEN) //container is open
    {
      if (Obj[container].loc == 2048 + OBJ_YOU) return 1;   //container is in your inventory
      if (Obj[container].loc == Obj[OBJ_YOU].loc) return 1; //container is in same room as you
    }
  }
  return 0;
}



int IsPlayerInDarkness(void)
{
  int i, obj;

  if (Room[Obj[OBJ_YOU].loc].prop & R_LIT) return 0; // room is lit

  if (Obj[OBJ_YOU].prop & PROP_LIT) return 0; // you are lit

  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;
    if (IsObjVisible(obj) && (Obj[obj].prop & PROP_LIT)) return 0; // a lit obj is visible
  }

  return 1;
}



//move order of "obj" to last in printing order
void MoveObjOrderToLast(int obj)
{
  int i, j;

  for (i=2; i<NUM_OBJECTS; i++)
    if (obj == Obj[i].order)
  {
    for (j=i; j<NUM_OBJECTS-1; j++)
      Obj[j].order = Obj[j+1].order;
    Obj[j].order = obj;
    break;
  }
}



//returns number of objects in location
//location can be a room, player's inventory, or inside object
int GetNumObjectsInLocation(int loc)
{
  int count, i, obj;

  count = 0;
  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;
    if (Obj[obj].loc == loc) count++;
  }
  return count;
}



//returns total size of objects in location
//location can be a room, player's inventory, or inside object
int GetTotalSizeInLocation(int loc)
{
  int size, i, obj;

  size = 0;
  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;
    if (Obj[obj].loc == loc) size += Obj[obj].size;
  }
  return size;
}



// only display objects inside something else one level deep

void PrintContents(int obj, char *heading, int print_empty)
{
  int flag = 0, i;

  for (i=2; i<NUM_OBJECTS; i++)
  {
    int obj_inside = Obj[i].order;

    if ( Obj[obj_inside].loc == 2048 + obj &&
         (Obj[obj_inside].prop & PROP_NODESC) == 0 &&
         !( (Obj[obj_inside].prop & PROP_INSIDEDESC) &&
            (Obj[obj_inside].prop & PROP_MOVEDDESC) == 0 ))
    {
      if (flag == 0) {PrintLine(heading); flag = 1;}
      PrintText("  ");
      PrintObjectDesc(obj_inside, 0);
      PrintText("\n");
    }
  }

  for (i=2; i<NUM_OBJECTS; i++)
  {
    int obj_inside = Obj[i].order;

    if ( Obj[obj_inside].loc == 2048 + obj &&
         (Obj[obj_inside].prop & PROP_NODESC) == 0 &&
         ( (Obj[obj_inside].prop & PROP_INSIDEDESC) &&
           (Obj[obj_inside].prop & PROP_MOVEDDESC) == 0 ))
    {
      flag = 1;
      PrintObjectDesc(obj_inside, 1);
      PrintText("\n");
    }
  }

  if (print_empty && flag == 0)
    PrintLine("It's empty.");
}



// print objects at "location"; location can be player's inventory

void PrintPresentObjects(int location, char *heading, int list_flag)
{
  int flag, i, obj, obj_inside;

  flag = 0;
  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;

    if (Obj[obj].loc == location &&
        (Obj[obj].prop & PROP_NODESC) == 0)
    {
      if (flag == 0)
      {
        flag = 1;

        if (location == 2048 + OBJ_YOU)
          PrintLine("You're carrying:");
        else if (heading != 0)
          PrintLine(heading);
      }

      if (location == 2048 + OBJ_YOU || list_flag)
        PrintObjectDesc(obj, 0);
      else
        PrintObjectDesc(obj, 1);

      PrintText("\n");

      if ((Obj[obj].prop & PROP_OPEN) &&
          (Obj[obj].prop & PROP_ACTOR) == 0)
        PrintContents(obj, "  (which contains)", 0); // 0: don't print "It's empty."
    }
  }

  if (location == 2048 + OBJ_YOU && flag == 0) PrintLine("You're not carrying anything.");
}
//*****************************************************************************



//*****************************************************************************
void PrintRoomDesc(int room, int force_description)
{
  char *compressed_text, *decompressed_text, *p;
  int compressed_size, i, ch;

  compressed_text = RoomDesc[room];
  compressed_size = strlen(compressed_text)+1;
  decompressed_text = p = malloc(GetDecompressTextSize(compressed_text, compressed_size));
  DecompressText(compressed_text, compressed_size, decompressed_text);

  if (*p != '^' && *p != 0)
  {

#ifndef NO_STATUS_LINE
    memset(StatusLineText, ' ', 80);
    for (i=0; i<39; i++)
    {
      ch = p[i];
      if (ch == 0 || ch == '^') break;
      StatusLineText[1+i] = ch;
    }
#endif

    PrintLine(p); //print room name
  }

  if (force_description || Verbosity != V_SUPERBRIEF)
  {
    if ((Room[room].prop & R_ALWAYSDESC) || force_description || Verbosity == V_VERBOSE)
      Room[room].prop &= ~R_DESCRIBED;

    //look for room in override room desc list
    for (i=0; OverrideRoomDesc[i].f != 0; i++)
      if (OverrideRoomDesc[i].room == room) break;
    if (OverrideRoomDesc[i].f != 0)
      OverrideRoomDesc[i].f();
    else
      if ((Room[room].prop & R_DESCRIBED) == 0)
    {
      p = strchr(p, '^');
      if (p != NULL)
      {
        p++;
        if (*p != '^' && *p != 0) PrintLine(p);
      }
    }
  }

  // game logic depends on this being set even if player never actually sees full description
  Room[room].prop |= R_DESCRIBED;

  free(decompressed_text);
}



void PrintPlayerRoomDesc(int force_description)
{
  if (IsPlayerInDarkness())
    PrintLine("It is pitch black. You are likely to be eaten by a grue.");
  else
  {
    PrintRoomDesc(Obj[OBJ_YOU].loc, force_description);
    if (force_description || Verbosity != V_SUPERBRIEF) PrintPresentObjects(Obj[OBJ_YOU].loc, 0, 0);
  }
}
//*****************************************************************************



//*****************************************************************************

#define SAVE_FILENAME                "zork1_#.sav"
#define SAVE_FILENAME_NUMCHAR_INDEX  6



//returns 1 if successful
int DoSave(void)
{
  FILE *f;
  int error = 1, slot, size;
  unsigned char *p, string[80];

  GetWords("Save to which slot (0-9; default=0; q to cancel)>");
  if (NumStrWords == 0) slot = 0;
  else
  {
    for (slot=0; slot<10; slot++)
    {
      string[0] = '0' + slot;
      string[1] = 0;
      if (MatchCurWord(string)) break;
    }
    if (slot == 10) {PrintLine("*** Save cancelled. ***"); return 0;}
  }

  strcpy(string, SAVE_FILENAME);
  string[SAVE_FILENAME_NUMCHAR_INDEX] = '0' + slot;

  f = fopen(string, "rb");
  if (f != NULL)
  {
    fclose(f);
    for (;;)
    {
      GetWords("Saved game exists. Overwrite? ");
      if (MatchCurWord("y") || MatchCurWord("yes")) break;
      if (MatchCurWord("n") || MatchCurWord("no")) {PrintLine("*** Save cancelled. ***"); return 0;}
      PrintLine("Please answer yes or no.");
    }
  }

  size = GetSaveStateSize();
  p = malloc(size);
  if (p == NULL) goto done;
  ReadSaveState(p);
  f = fopen(string, "wb");
  if (f == NULL) {free(p); goto done;}
  if (fwrite(p, size, 1, f) != 1) {fclose(f); free(p); goto done;}
  fclose(f);
  free(p);

  error = 0;
done:
  if (error)
  {
    PrintLine("*** Save failed. ***");
    return 0;
  }
  else
  {
    PrintLine("*** Save successful. ***");
    return 1;
  }
}



//returns 1 if successful
int DoRestore(void)
{
  FILE *f;
  int error = 1, slot, size;
  unsigned char *p, string[80];

  GetWords("Restore from which slot (0-9; default=0; q to cancel)>");
  if (NumStrWords == 0) slot = 0;
  else
  {
    for (slot=0; slot<10; slot++)
    {
      string[0] = '0' + slot;
      string[1] = 0;
      if (MatchCurWord(string)) break;
    }
    if (slot == 10) {PrintLine("*** Restore cancelled. ***"); return 0;}
  }

  strcpy(string, SAVE_FILENAME);
  string[SAVE_FILENAME_NUMCHAR_INDEX] = '0' + slot;

  size = GetSaveStateSize();
  p = malloc(size);
  if (p == NULL) goto done;
  f = fopen(string, "rb");
  if (f == NULL) {free(p); PrintLine("File not found."); goto done;}
  if (fread(p, size, 1, f) != 1) {fclose(f); free(p); goto done;}
  fclose(f);
  WriteSaveState(p);
  free(p);

  error = 0;
done:
  if (error)
  {
    PrintLine("*** Restore failed. ***");
    return 0;
  }
  else
  {
    NumStrWords = 0;
    ItObj = 0;
    GameOver = 0;

    PrintLine("*** Restore successful. ***\n");
    PrintPlayerRoomDesc(1); //force description

    return 1;
  }
}
//*****************************************************************************



//*****************************************************************************
//Parse Routines



//if "match" matches current word in input, move to next word and return 1
int MatchCurWord(const char *match)
{
  if (NumStrWords >= CurWord+1 && strcmp(StrWord[CurWord], match) == 0)
  {
    CurWord++;
    return 1;
  }
  return 0;
}



//string contains words separated by commas with no whitespace
int IsWordInCommaString(const char *p, const char *word)
{
  char *q, buffer[80];
  int len;

  if (p != 0)
    for (;;)
  {
    q = strchr(p, ',');
    if (q == 0) q = strchr(p, 0);
    len = q-p;
    if (len > 0 && len < 80)
    {
      memcpy(buffer, p, len);
      buffer[len] = 0;
      if (strcmp(word, buffer) == 0) return 1;
    }

    if (*q == 0) break;
    else p = q+1; //skip past comma
  }

  return 0;
}



int GetActionFromInput(void)
{
  char buffer[80*2];
  const char *p;
  const char *q;
  int i, j, len, action, temp;

  i = 0;
  for (;;)
  {
    action = VerbToAction[i].action;
    if (action == 0) break; //reached end of list without finding anything


    //fill buffer with up to two words of verb phrase
    memset(buffer, 0, 80*2);
    p = VerbToAction[i].phrase;
    for (j=0; j<2; j++)
    {
      q = strchr(p, ' ');
      if (q == 0) q = strchr(p, 0);
      len = q-p; if (len > 0 && len < 80) memcpy(buffer+80*j, p, len);
      if (*q == 0) break;
      else p = q+1; //skip past space
    }


    temp = CurWord;
    if (NumStrWords >= CurWord+1 && strcmp(StrWord[CurWord], buffer+80*0) == 0)
    {
      CurWord++;
      if (buffer[80*1] != 0)
      {
        if (NumStrWords >= CurWord+1 && strcmp(StrWord[CurWord], buffer+80*1) == 0)
          CurWord++;
        else CurWord = temp;
      }
    }
    if (CurWord > temp) break; //found action

    i++;
  }

  return action;
}



//matches a maximum of three words

//if there are multiple match objects that are visible,
//this function will ask player to be more specific by returning -1

int GetObjFromInput(void)
{
  char buffer[80*3];
  int num_matches, i, j;
  unsigned short match_obj[80];
  unsigned char match_size[80];

  num_matches = 0;

  i = 0;
  for (;;) //look through noun phrase table
  {
    int obj, size;
    const char *p;

    obj = NounPhraseToObj[i].obj;
    if (obj == 0) break;

    size = 0;
    p = NounPhraseToObj[i].phrase;
    for (j=0; j<3; j++)
    {
      int len;
      const char *q;

      q = strchr(p, ' ');
      if (q == 0) q = strchr(p, 0);
      len = q-p;
      if (len > 0 && len < 80)
      {
        memcpy(buffer+80*j, p, len);
        buffer[80*j+len] = 0;
        size++;
      }
      if (*q == 0) break;
      else p = q+1; //skip past space
    }

    if (size) //size of noun phrase to match with input
    {
      j = 0;
      if (NumStrWords >= CurWord + size) //is input long enough to match
        for (; j<size; j++)
          if (strcmp(StrWord[CurWord+j], buffer+80*j)) break; //break early if words are different

      if (j == size) //all words of phrase matched
      {
        match_obj[num_matches] = obj;
        match_size[num_matches] = size;
        num_matches++;
        if (num_matches == 80) break;
      }
    }

    i++; //try another noun phrase from table
  }

  if (num_matches == 0) return 0;

  //if more than one match, check if objects are both visible at same time
  //if so, return -1 so that player can know to be more specific
  for (i=0; i<num_matches; i++)
    if (IsObjVisible(match_obj[i]))
      for (j=i+1; j<num_matches; j++)
        if (match_obj[j] != match_obj[i] && IsObjVisible(match_obj[j]))
          return -1;

  //watch out: need the longest matched size here
  //first match should always have longest size because
  //phrases must be listed in order of decreasing size in phrase table
  for (i=0; i<num_matches; i++)
    if (IsObjVisible(match_obj[i])) break; //use first visible match, if possible
  if (i == num_matches) i = 0; //if no visible matches, use first one

  CurWord += match_size[i];
  return match_obj[i];
}



int GetFixedObjFromInput(int room)
{
  char buffer[80*2];
  const char *p;
  const char *q;
  int i, j, len, fobj, fobj_inroom, cw_inroom, fobj_notinroom, cw_notinroom, temp;

  fobj_inroom = 0;    //matched fixed obj that was found in room
  fobj_notinroom = 0; //                           not found in room

  temp = CurWord; //keep track of word position because we will be matching multiple noun phrases

  i = 0;
  for (;;)
  {
    fobj = NounPhraseToFixedObj[i].fobj;
    if (fobj == 0) break; //reached end of list without finding anything


    //fill buffer with up to two words of noun phrase
    memset(buffer, 0, 80*2);
    p = NounPhraseToFixedObj[i].phrase;
    for (j=0; j<2; j++)
    {
      q = strchr(p, ' ');
      if (q == 0) q = strchr(p, 0);
      len = q-p; if (len > 0 && len < 80) memcpy(buffer+80*j, p, len);
      if (*q == 0) break;
      else p = q+1; //skip past space
    }


    CurWord = temp;
    if (NumStrWords >= CurWord+1 && strcmp(StrWord[CurWord], buffer+80*0) == 0)
    {
      CurWord++;
      if (buffer[80*1] != 0)
      {
        if (NumStrWords >= CurWord+1 && strcmp(StrWord[CurWord], buffer+80*1) == 0)
          CurWord++;
        else CurWord = temp;
      }
    }

    if (CurWord > temp) //found noun phrase
    {
      if (NounPhraseToFixedObj[i].room == room) //found a fixed obj in room
      {
        if (fobj_inroom == 0) //only use first one found, though there should be only one anyway
        {
          fobj_inroom = fobj;
          cw_inroom = CurWord; //keep track of word position past this match
        }
      }
      else //found a fixed obj not in room
      {
        if (fobj_notinroom == 0) //only use first one found, though there should be only one anyway
        {
          fobj_notinroom = fobj;
          cw_notinroom = CurWord; //keep track of word position past this match
        }
      }
    }

    i++;
  }

  //return fixed or scenery objects in room first

  //fixed object in room
  if (fobj_inroom)
  {
    CurWord = cw_inroom;
    return fobj_inroom;
  }

  //scenery object in room
  if (NumStrWords >= CurWord+1 && room < NUM_ROOMS && IsWordInCommaString(Room[room].scenery, StrWord[CurWord]))
  {
    CurWord++;
    return FOBJ_SCENERY_VIS;
  }

  //fixed object not in room
  if (fobj_notinroom)
  {
    CurWord = cw_notinroom;
    return FOBJ_NOTVIS;
  }

  //scenery object not in room
  if (NumStrWords >= CurWord+1 && IsWordInCommaString(SceneryNouns, StrWord[CurWord]))
  {
    CurWord++;
    return FOBJ_SCENERY_NOTVIS;
  }

  return 0;
}



//gets obj, fixed obj, or scenery obj, whichever comes first

//returns -1 if player needs to be more specific
//        -2 if player used "it" but it wasn't clear what itobj is

//itobj starts at 0
//if itobj already refers to an object, any additional object invalidates itobj (sets to -1)

int GetAllObjFromInput(int room)
{
  int obj;

  if (MatchCurWord("it") || MatchCurWord("them"))
  {
    //itobj from previous sentence
    if (PrevItObj <= 0)
    {
      PrintLine("I'm not sure what you're referring to with one or more of those nouns.");
      return -2;
    }
    ItObj = PrevItObj;
    return ItObj;
  }

  //skip article (if any) immediately before object
  if (MatchCurWord("the") || MatchCurWord("a") || MatchCurWord("an")) {}

  //convert noun phrase to obj
  obj = GetObjFromInput(); //can return -1 if player needs to be more specific
  if (obj == 0) obj = GetFixedObjFromInput(room);

  if (obj == 0)
  {
    ItObj = -1;
    PrintLine("I didn't recognize one or more of those nouns, or you didn't specify one.");
  }
  else if (obj == -1)
  {
    ItObj = -1;
    PrintLine("You need to be more specific with one or more of those nouns.");
  }
  else
  {
    if (ItObj == 0) ItObj = obj; //first object encountered this sentence; set itobj
    else ItObj = -1; //another obj encountered; invalidate itobj
  }

  return obj;
}



//same as GetAllObjFromInput() above but just skips noun phrase without doing or printing anything
void SkipObjFromInput(int room)
{
  int obj;

  if (MatchCurWord("it") || MatchCurWord("them")) return;

  if (MatchCurWord("the") || MatchCurWord("a") || MatchCurWord("an")) {}

  obj = GetObjFromInput();
  if (obj == 0) obj = GetFixedObjFromInput(room);
}
//*****************************************************************************



//*****************************************************************************
//set TimePassed to 1 when an action completes successfully



void ParseActionDirection(int action)
{
  int newroom, prev_darkness;

  newroom = RoomPassages[Obj[OBJ_YOU].loc].passage[action - A_NORTH];
  if (newroom >= BL0)
  {
    PrintBlockMsg(newroom);
    return;
  }

  // handle things like water and boats in game-specific code
  if (ActionDirectionRoutine(newroom)) return; 

  prev_darkness = IsPlayerInDarkness();

  Obj[OBJ_YOU].loc = newroom;
  TimePassed = 1;

  if (IsPlayerInDarkness())
  {
    if (prev_darkness)
    {
      //kill player that tried to walk from dark to dark
      PrintLine("\n\n\n\n\nOh, no! You have walked into the slavering fangs of a lurking grue!");
      YoureDead(); // ##### RIP #####
      return;
    }
    else PrintLine("You have moved into a dark place.");
  }

  PrintPlayerRoomDesc(0);
}



//returns 1 to signal to calling routine that command was processed

int TakeOutOfRoutine(int *container)
{
  MatchCurWord("of");

  *container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if ((*container) <= 0) return 1;

  if ((*container) == FOBJ_SCENERY_NOTVIS || (*container) == FOBJ_NOTVIS)
  {
    PrintLine("At least one of those objects isn't visible here!");
    return 1;
  }

  if ((*container) == OBJ_YOU || (*container) >= NUM_OBJECTS)
  {
    PrintLine("You can't take anything out of that.");
    return 1;
  }

  if (IsObjVisible(*container) == 0)
  {
    PrintObjectDesc(*container, 0);
    PrintText(": ");

    PrintLine("You can't see that here.");
    return 1;
  }

  if ((Obj[*container].prop & PROP_OPEN) == 0 ||
      (Obj[*container].prop & PROP_ACTOR))
  {
    PrintObjectDesc(*container, 0);
    PrintText(": ");

    if (Obj[*container].prop & PROP_OPENABLE)
    {
      PrintLine("You need to open it first.");
      ItObj = *container;
    }
    else
      PrintLine("You can't take anything out of that!");

    return 1;
  }

  return 0;
}



//returns 1 to signal to calling routine that command was processed
//container may be 0 (none specified)

int VerifyTakeableObj(int obj, int container, int num_takes)
{
  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
  {
    if (num_takes > 1)
      PrintLine("At least one of those objects isn't visible here!");
    else
      PrintLine("You don't see that here!");

    return 1;
  }

  if (obj == FOBJ_AMB)
  {
    if (num_takes > 1)
      PrintLine("You need to be more specific about at least one of those objects.");
    else
      PrintLine("You need to be more specific.");

    return 1;
  }

  if (InterceptTakeFixedObj(obj)) return 1;

  if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
  {
    if (num_takes > 1)
      PrintLine("At least one of those objects can't be taken!");
    else
      PrintLine("You can't take that!");

    return 1;
  }

  if (container != 0 && Obj[obj].loc != 2048 + container)
  {
    PrintObjectDesc(obj, 0);
    PrintText(": ");

    PrintLine("That's not inside of it.");

    return 1;
  }

  return 0;
}



// returns 1 if take intercepted or failed

int TakeRoutine(int obj, char *msg)
{
  int num, chance;

  if (InterceptTakeObj(obj)) return 1;

  if (Obj[obj].prop & PROP_NOTTAKEABLE)
    {PrintLine("You can't take that."); return 1;}


  if (GetTotalSizeInLocation(2048 + OBJ_YOU) + Obj[obj].size > LoadAllowed)
  {
    PrintText("Your load is too heavy");
    if (LoadAllowed < LOAD_MAX)
      PrintLine(", especially in light of your condition.");
    else
      PrintLine(".");
    return 1;
  }

  num = GetNumObjectsInLocation(2048 + OBJ_YOU);
  chance = INV_LIMIT_CHANCE * num; if (chance > 100) chance = 100;
  if (num > MAX_INVENTORY_ITEMS && PercentChance(chance, -1))
  {
    PrintLine("You're holding too many things already!");
    return 1;
  }


  TimePassed = 1;
  PrintLine(msg);

  Obj[obj].loc = 2048 + OBJ_YOU;
  Obj[obj].prop |= PROP_MOVEDDESC;

  MoveObjOrderToLast(obj);

  return 0;
}



void TakeAllRoutine(void)
{
  int num_exceptions, num_takes, obj, container, i, j;
  unsigned short exception[80]; //stores objects not to be taken by "take all"
  unsigned short take[80];      //stores objects to be taken by "take all"

  num_exceptions = 0;

  if (MatchCurWord("but") || MatchCurWord("except"))
  {
    MatchCurWord("for"); // skip "for" if it exists

    for (;;)
    {
      obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  
      if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
        {PrintLine("At least one of those objects isn't visible here!"); return;}
  
      if (obj == FOBJ_AMB)
        {PrintLine("You need to be more specific about at least one of those objects."); return;}
  
      if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
        {PrintLine("At least one of those objects can't be taken!"); return;}
  
      if (Obj[obj].loc == 2048 + OBJ_YOU)
        {PrintLine("You're already holding at least one of those objects!"); return;}
  
      if (IsObjVisible(obj) == 0)
        {PrintLine("At least one of those objects isn't visible here!"); return;}
  
      exception[num_exceptions++] = obj;
      if (num_exceptions == 80 || CurWord == NumStrWords) break;
  
      if (MatchCurWord("from") || MatchCurWord("out"))
      {
        if (MatchCurWord("of")) CurWord--;
        CurWord--;                         //back up so from/out (of) can be processed below
        break;
      }
  
      if (MatchCurWord("then"))
      {
        CurWord--; //end of this turn's command; back up so "then" can be matched later
        break;
      }
  
      if (MatchCurWord("and") == 0)
        {PrintLine("Please use a comma or the word \"and\" between nouns."); return;}

      for (;;) if (MatchCurWord("and") == 0) break; // handle repeated "and"s like "take one, two, and three"
    }
  }

  container = 0;

  if (MatchCurWord("from") || MatchCurWord("out"))
    if (TakeOutOfRoutine(&container)) return;

  num_takes = 0;

  for (i=2; i<NUM_OBJECTS; i++)
  {
    obj = Obj[i].order;

    if (Obj[obj].loc == (container ? 2048 + container : Obj[OBJ_YOU].loc))
    {
      for (j=0; j<num_exceptions; j++)
        if (obj == exception[j]) break;
      if (j == num_exceptions)
      {
        take[num_takes++] = obj;
        if (num_takes == 80) break;
      }
    }
  }

  for (i=0; i<num_takes; i++)
  {
    obj = take[i];

    PrintObjectDesc(obj, 0);
    PrintText(": ");

    TakeRoutine(obj, "Okay.");
  }

  if (num_takes == 0)
    PrintLine("There was nothing to take.");
}



void ParseActionTake(void)
{
  int num_takes, obj, container, i;
  unsigned short take[80]; //stores objects to be taken

  if (MatchCurWord("all") || MatchCurWord("everything"))
    {TakeAllRoutine(); return;}

  num_takes = 0;
  container = 0;

  for (;;)
  {
    obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;

    take[num_takes++] = obj;
    if (num_takes == 80 || CurWord == NumStrWords) break;

    if (MatchCurWord("from") || MatchCurWord("out"))
    {
      if (TakeOutOfRoutine(&container)) return;
      break;
    }

    if (MatchCurWord("then"))
    {
      CurWord--; //end of this turn's command; back up so "then" can be matched later
      break;
    }

    if (MatchCurWord("and") == 0)
      {PrintLine("Please use a comma or the word \"and\" between nouns."); return;}

    for (;;) if (MatchCurWord("and") == 0) break; // handle repeated "and"s like "take one, two, and three"
  }

  for (i=0; i<num_takes; i++)
    if (VerifyTakeableObj(take[i], container, num_takes)) return;

  for (i=0; i<num_takes; i++)
  {
    obj = take[i];

    if (num_takes > 1)
    {
      PrintObjectDesc(obj, 0);
      PrintText(": ");
    }

    if (Obj[obj].loc == 2048 + OBJ_YOU)
      PrintLine("You're already holding that!");
    else if (IsObjVisible(obj) == 0)
      PrintLine("You can't see that here.");
    else
      TakeRoutine(obj, "Okay.");
  }
}



void ParseActionDropPut(int put_flag)
{
  int obj;

  if (MatchCurWord("all") || MatchCurWord("everything")) //beginning of "drop/put all"
  {
    int num_exceptions, nothing_dropped, into_flag, container;
    unsigned short exception[80]; //stores objects not to be dropped/put by "drop/put all"

    container = 0;
    num_exceptions = 0;

    if (MatchCurWord("but") || MatchCurWord("except"))
    {
      MatchCurWord("for"); // skip "for" if it exists

      for (;;)
      {
        obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  
        if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
        {
          PrintLine("You're not holding at least one of those objects!");
          return;
        }
  
        if (Obj[obj].loc != 2048 + OBJ_YOU)
        {
          PrintLine("You're not holding at least one of those objects!");
          return;
        }
  
        exception[num_exceptions++] = obj;
        if (num_exceptions == 80 || CurWord == NumStrWords) break;
  
        if (MatchCurWord("in") || MatchCurWord("into") || MatchCurWord("inside"))
        {
          CurWord--; //back up so in/into/inside can be processed below
          break;
        }
  
        if (MatchCurWord("then"))
        {
          CurWord--; //end of this turn's command; back up so "then" can be matched later
          break;
        }
  
        if (MatchCurWord("and") == 0)
        {
          PrintLine("Please use a comma or the word \"and\" between nouns.");
          return;
        }

        for (;;) if (MatchCurWord("and") == 0) break; // handle repeated "and"s like "take one, two, and three"
      }
    }

    //change "drop" to "put" (if not already) if container will be specified
    if (MatchCurWord("in") || MatchCurWord("into") || MatchCurWord("inside"))
    {
      into_flag = 1;
      put_flag = 1;
    }
    else into_flag = 0;

    if (put_flag)
    {
      if (into_flag == 0)
      {
        PrintLine("You need to specify a container.");
        return;
      }

      container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (container <= 0) return;

      if (container == FOBJ_SCENERY_NOTVIS || container == FOBJ_NOTVIS)
      {
        PrintLine("At least one of those objects isn't visible here!");
        return;
      }

      if (container == OBJ_YOU || container >= NUM_OBJECTS)
      {
        PrintLine("You can't put anything into that.");
        return;
      }

      if (IsObjVisible(container) == 0)
      {
        PrintObjectDesc(container, 0);
        PrintText(": ");

        PrintLine("You can't see that here.");

        return;
      }

      if ((Obj[container].prop & PROP_OPEN) == 0 ||
          (Obj[container].prop & PROP_ACTOR))
      {
        PrintObjectDesc(container, 0);
        PrintText(": ");

        if (Obj[container].prop & PROP_OPENABLE)
        {
          PrintLine("You need to open it first.");
          ItObj = container;
        }
        else
          PrintLine("You can't put anything into that!");

        return;
      }
    }

    nothing_dropped = 1;
    for (;;)
    {
      int i;

      for (i=2; i<NUM_OBJECTS; i++)
      {
        obj = Obj[i].order;
        if (Obj[obj].loc == 2048 + OBJ_YOU && obj != container)
        {
          int j;

          for (j=0; j<num_exceptions; j++)
            if (obj == exception[j]) break;
          if (j == num_exceptions)
          {
            PrintObjectDesc(obj, 0);
            PrintText(": ");

            if (put_flag && GetTotalSizeInLocation(2048 + container) + Obj[obj].size > Obj[container].capacity)
            {
              PrintLine("It won't hold any more!");
              return;
            }

            Obj[obj].loc = put_flag ? 2048 + container : Obj[OBJ_YOU].loc;

            MoveObjOrderToLast(obj);

            PrintLine("Okay.");

            TimePassed = 1;

            nothing_dropped = 0;
            break;
          }
        }
      }
      if (i == NUM_OBJECTS) break;
    }

    if (nothing_dropped)
    {
      if (put_flag)
        PrintLine("There was nothing to put into it.");
      else
        PrintLine("There was nothing to drop.");
    }
    return;
  } //end of "drop/put all"


  {
    int i, num_drops, container;
    unsigned short drop[80]; //stores objects to be dropped

    container = 0;
    num_drops = 0;

    for (;;)
    {
      obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;

      drop[num_drops++] = obj;
      if (num_drops == 80 || CurWord == NumStrWords) break;

      if (MatchCurWord("in") || MatchCurWord("into") || MatchCurWord("inside"))
      {
        put_flag = 1; //turn drop into put if not already

        container = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (container <= 0) return;

        if (container == FOBJ_SCENERY_NOTVIS || container == FOBJ_NOTVIS)
        {
          PrintLine("At least one of those objects isn't visible here!");
          return;
        }

        if (container == OBJ_YOU || container >= NUM_OBJECTS)
        {
          PrintLine("You can't put anything into that.");
          return;
        }

        if (IsObjVisible(container) == 0)
        {
          PrintObjectDesc(container, 0);
          PrintText(": ");

          PrintLine("You can't see that here.");

          return;
        }

        if ((Obj[container].prop & PROP_OPEN) == 0 ||
            (Obj[container].prop & PROP_ACTOR))
        {
          PrintObjectDesc(container, 0);
          PrintText(": ");

          if (Obj[container].prop & PROP_OPENABLE)
          {
            PrintLine("You need to open it first.");
            ItObj = container;
          }
          else
            PrintLine("You can't put anything into that!");

          return;
        }

        break;
      }

      if (MatchCurWord("then"))
      {
        CurWord--; //end of this turn's command; back up so "then" can be matched later
        break;
      }

      if (MatchCurWord("and") == 0)
      {
        PrintLine("Please use a comma or the word \"and\" between nouns.");
        return;
      }

      for (;;) if (MatchCurWord("and") == 0) break; // handle repeated "and"s like "take one, two, and three"
    }

    if (put_flag && container == 0)
    {
      PrintLine("You need to specify a container.");
      return;
    }

    for (i=0; i<num_drops; i++)
    {
      obj = drop[i];

      if (obj == OBJ_YOU || obj >= NUM_OBJECTS)
      {
        if (num_drops > 1)
          PrintLine("You're not holding at least one of those objects!");
        else
          PrintLine("You're not holding that!");
        return;
      }
    }

    for (i=0; i<num_drops; i++)
    {
      obj = drop[i];

      if (num_drops > 1)
      {
        PrintObjectDesc(obj, 0);
        PrintText(": ");
      }

      if (Obj[obj].loc != 2048 + OBJ_YOU)
        PrintLine("You're not holding that!");
      else if (obj == container)
        PrintLine("But it would disappear from reality!");
      else
      {
        if (put_flag && GetTotalSizeInLocation(2048 + container) + Obj[obj].size > Obj[container].capacity)
          PrintLine("It won't hold any more!");
        else
        {
          Obj[obj].loc = put_flag ? 2048 + container : Obj[OBJ_YOU].loc;

          MoveObjOrderToLast(obj);

          PrintLine("Okay.");

          TimePassed = 1;
        }
      }
    }
  }
}



// returns 1 if obj is not visible or not specific enough

int ValidateObject(int obj)
{
  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
  {
    PrintLine("At least one of those objects isn't visible here!");
    return 1;
  }

  if (obj == FOBJ_AMB)
  {
    PrintLine("You need to be more specific about at least one of those objects.");
    return 1;
  }

  if (obj > 0 && obj < NUM_OBJECTS && IsObjVisible(obj) == 0)
  {
    PrintLine("At least one of those objects isn't visible here!");
    return 1;
  }

  return 0;
}



// CAUTION: called function will have to check if with_to >= NUM_OBJECTS

void ParseActionWithTo(int action, char *match_hack, char *cant)
{
  int obj, with_to, i;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  if (ValidateObject(obj)) return;

  if (match_hack != 0) MatchCurWord(match_hack); //skip specified word

  with_to = 0;
  if (MatchCurWord("with") || MatchCurWord("using") || MatchCurWord("to") || MatchCurWord("from"))
  {
    with_to = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (with_to <= 0) return;
  }
  if (ValidateObject(with_to)) return;

  for (i=0; DoMiscWithTo[i].f != 0; i++)
    if (DoMiscWithTo[i].action == action && DoMiscWithTo[i].obj == obj)
  {
    DoMiscWithTo[i].f(with_to);
    return;
  }

  if (obj == OBJ_YOU) {PrintLine("Seriously?!"); return;}

  PrintText("You can't ");
  PrintLine(cant);
}



// for use outside parser
// returns -1 if failure message was printed

int GetWith(void)
{
  int with = 0;

  if (MatchCurWord("with") || MatchCurWord("using"))
  {
    with = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (with <= 0) return -1;
  }

  if (ValidateObject(with)) return -1;

  if (with >= NUM_OBJECTS)
    {PrintLine("You aren't holding that!"); return -1;}

  return with;
}



// not as in wearing

void ParseActionPutOn(void)
{
  int obj, surface;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc);
  if (obj <= 0) return;
  if (ValidateObject(obj)) return;

  surface = 0;
  if (MatchCurWord("on"))
  {
    surface = GetAllObjFromInput(Obj[OBJ_YOU].loc);
    if (surface <= 0) return;
  }
  if (ValidateObject(surface)) return;

  PrintLine("You don't need to place objects in that way.");
}



// not as in wearing

void ParseActionTakeOff(void)
{
  int obj, surface;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc);
  if (obj <= 0) return;
  if (ValidateObject(obj)) return;

  surface = 0;
  if (MatchCurWord("off"))
  {
    surface = GetAllObjFromInput(Obj[OBJ_YOU].loc);
    if (surface <= 0) return;
  }
  if (ValidateObject(surface)) return;

  PrintLine("You don't need to specify what an item is sitting on.");
}



void ParseActionExamine(void)
{
  int obj, i, flag;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc);
  if (obj <= 0) return;

  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLine("That isn't visible here!"); return;}

  if (obj == FOBJ_AMB)
    {PrintLine("You need to be more specific."); return;}

  //fixed objects only
  for (i=0; DoMisc[i].f != 0; i++)
    if (DoMisc[i].action == A_EXAMINE && DoMisc[i].obj >= NUM_OBJECTS && DoMisc[i].obj == obj)
  {
    DoMisc[i].f();
    return;
  }

  if (obj == OBJ_YOU)
    {PrintLine("You look fairly ordinary."); return;}

  if (obj >= NUM_OBJECTS)
    {PrintLine("You don't see anything unusual."); return;}

  if (IsObjVisible(obj) == 0)
  {
    TimePassed = 0;
    PrintLine("You can't see that here.");
    return;
  }

  //non-fixed objects only
  for (i=0; DoMisc[i].f != 0; i++)
    if (DoMisc[i].action == A_EXAMINE && DoMisc[i].obj < NUM_OBJECTS && DoMisc[i].obj == obj)
  {
    DoMisc[i].f();
    return;
  }


  flag = 0;

  if (Obj[obj].prop & PROP_LIT)
  {
    flag = 1;
    PrintLine("It's lit.");
  }

  if (Obj[obj].prop & PROP_OPENABLE)
  {
    flag = 1;
    if (Obj[obj].prop & PROP_OPEN)
      PrintLine("It's open.");
    else
      PrintLine("It's closed.");
  }

  if ((Obj[obj].prop & PROP_OPEN) &&
      (Obj[obj].prop & PROP_ACTOR) == 0)
  {
    flag = 1;
    PrintContents(obj, "It contains:", 1); // 1: allow printing "It's empty."
  }

  if (flag == 0)
    PrintLine("You don't see anything unusual.");
}



// called function must check if player is holding obj

void ParseActionGive(void)
{
  int obj, to, flag, swap, i;

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (obj <= 0) return;
  flag = MatchCurWord("to");
  to = GetAllObjFromInput(Obj[OBJ_YOU].loc); if (to <= 0) return;

  if (flag == 0)
  {
    //if "to" omitted, swap obj and to, as in "give plant water"
    swap = obj;
    obj = to;
    to = swap;
  }

  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLine("At least one of those objects isn't visible here!"); return;}

  if (obj == FOBJ_AMB)
    {PrintLine("You need to be more specific about at least one of those objects."); return;}

  if (obj == OBJ_YOU)
    {PrintLine("Seriously?!"); return;}

  if (obj >= NUM_OBJECTS)
    {PrintLine("You aren't holding that!"); return;}


  if (to == FOBJ_SCENERY_NOTVIS || to == FOBJ_NOTVIS)
    {PrintLine("At least one of those objects isn't visible here!"); return;}

  if (to == FOBJ_AMB)
    {PrintLine("You need to be more specific about at least one of those objects."); return;}

  if (to == OBJ_YOU)
    {PrintLine("Seriously?!"); return;}

  if (to > 0 && to < NUM_OBJECTS && IsObjVisible(to) == 0)
    {PrintLine("At least one of those objects isn't visible here!"); return;}


  for (i=0; DoMiscGiveTo[i].f != 0; i++)
    if (DoMiscGiveTo[i].to == to)
  {
    DoMiscGiveTo[i].f(obj);
    return;
  }

  PrintLine("You can't give something to that!");
}



void ParseActionRestartOrQuit(int action)
{
  for (;;)
  {
    GetWords((action == A_RESTART) ? "Do you want to restart the game? " : "Do you want to quit now? ");

    if (MatchCurWord("y") || MatchCurWord("yes"))
    {
      GameOver = (action == A_RESTART) ? 2 : 1;
      return;
    }

    if (MatchCurWord("n") || MatchCurWord("no")) return;

    PrintLine("Please answer yes or no.");
  }
}



void OpenObj(int obj)
{
  int prev_darkness;

  if ((Obj[obj].prop & PROP_OPENABLE) == 0 ||
      (Obj[obj].prop & PROP_ACTOR))
    {PrintLine("You can't open that!"); return;}

  if (Obj[obj].prop & PROP_OPEN)
    {PrintLine("It's already open."); return;}

  prev_darkness = IsPlayerInDarkness();

  Obj[obj].prop |= PROP_OPEN; //open object
  PrintLine("Okay.");
  TimePassed = 1;

  if (IsPlayerInDarkness() == 0)
    PrintContents(obj, "It contains:", 0); // 0: don't print "It's empty."

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintBlankLine();
    PrintPlayerRoomDesc(1);
  }
}



void CloseObj(int obj)
{
  int prev_darkness;

  if ((Obj[obj].prop & PROP_OPENABLE) == 0 ||
      (Obj[obj].prop & PROP_ACTOR))
    {PrintLine("You can't close that!"); return;}

  if ((Obj[obj].prop & PROP_OPEN) == 0)
    {PrintLine("It's already closed."); return;}

  prev_darkness = IsPlayerInDarkness();

  Obj[obj].prop &= ~PROP_OPEN; //close object
  PrintLine("Okay.");
  TimePassed = 1;

  if (IsPlayerInDarkness() != prev_darkness)
  {
    PrintBlankLine();
    PrintPlayerRoomDesc(1);
  }
}



void LookInObj(int obj)
{
  if ((Obj[obj].prop & PROP_OPEN) &&
      (Obj[obj].prop & PROP_ACTOR) == 0)
    PrintContents(obj, "It contains:", 1); // 1: allow printing "It's empty."
  else
  {
    if ((Obj[obj].prop & PROP_OPENABLE) == 0)
      PrintLine("You can't look inside that!");
    else
      PrintLine("It's closed.");
  }
}



void EmptyObj(int obj)
{
  int flag = 0, i;

  if ((Obj[obj].prop & PROP_OPENABLE) == 0)
    {PrintLine("You can't empty that!"); return;}

  if ((Obj[obj].prop & PROP_OPEN) == 0)
    {PrintLine("It's closed."); return;}

  for (i=2; i<NUM_OBJECTS; i++)
  {
    int obj_inside = Obj[i].order;

    if (Obj[obj_inside].loc == 2048 + obj)
    {
      flag = 1;
      TimePassed = 1;

      Obj[obj_inside].loc = Obj[OBJ_YOU].loc;
      Obj[obj_inside].prop |= PROP_MOVEDDESC;

      PrintObjectDesc(obj_inside, 0);
      PrintLine(": Dropped.");
    }
  }

  if (flag == 0)
    PrintLine("It's already empty.");
}



void PrintCantAction(int action)
{
  switch (action)
  {
    case A_OPEN:         PrintLine("You can't open that!");                 break;
    case A_CLOSE:        PrintLine("You can't close that!");                break;
    case A_LOOKIN:       PrintLine("You can't look inside that.");          break;
    case A_LOOKTHROUGH:  PrintLine("You can't look through that.");         break;
    case A_MOUNT:        PrintLine("You can't get on that!");               break;
    case A_DISMOUNT:     PrintLine("You're not on that!");                  break;
    case A_EAT:          PrintLine("That does not sound very appetizing!"); break;
    case A_DRINK:        PrintLine("You can't drink that!");                break;
    case A_WEAR:         PrintLine("You can't wear that!");                 break;
    case A_REMOVE:       PrintLine("You aren't wearing that!");             break;
    case A_PLAY:         PrintLine("You can't play that!");                 break;
    case A_SLEEPON:      PrintLine("You can't sleep on that!");             break;
    case A_JUMPINTO:     PrintLine("You can't jump into that!");            break;
    case A_RAISE:        PrintLine("You can't raise that.");                break;
    case A_LOWER:        PrintLine("You can't lower that.");                break;
    case A_ENTER:        PrintLine("You can't go inside that!");            break;
    case A_EXIT:         PrintLine("You aren't in that!");                  break;
    case A_READ:         PrintLine("There's nothing to read.");             break;
    case A_RING:         PrintLine("You can't ring that!");                 break;
    case A_WIND:         PrintLine("You can't wind that!");                 break;
    case A_CLIMB:        PrintLine("You can't climb that!");                break;
    case A_CLIMBUP:      PrintLine("You can't climb up that!");             break;
    case A_CLIMBDOWN:    PrintLine("You can't climb down that!");           break;
    case A_SLIDEUP:      PrintLine("You can't slide up that!");             break;
    case A_SLIDEDOWN:    PrintLine("You can't slide down that!");           break;
    case A_CLIMBTHROUGH: PrintLine("You can't climb through that!");        break;

    default:             PrintLine("That would be futile.");                break;
  }
}



void DoActionOnObject(int action)
{
  int must_hold = 0, obj, i;

  // eat and drink must-hold checks must be handled by called function
  switch (action)
  {
    case A_EMPTY:
    case A_WEAR:
    case A_REMOVE:
    case A_PLAY:
    case A_RING:
    case A_WIND:
    case A_WAVE:
      must_hold = 1;
    break;
  }

  switch (action)
  {
    case A_ENTER:
    case A_EXIT:
      MatchCurWord("of"); // "go inside of"  "get out of"
    break;
  }

  obj = GetAllObjFromInput(Obj[OBJ_YOU].loc);
  if (obj <= 0) return;

  switch (action)
  {
    case A_WEAR:   MatchCurWord("on");  break; // "put obj on"
    case A_REMOVE: MatchCurWord("off"); break; // "take obj off"
  }

  if (obj == FOBJ_SCENERY_NOTVIS || obj == FOBJ_NOTVIS)
    {PrintLine("That isn't visible here!"); return;}

  if (obj == FOBJ_AMB)
    {PrintLine("You need to be more specific."); return;}

  if (obj > 0 && obj < NUM_OBJECTS)
  {
    if (must_hold && obj == OBJ_YOU)
      {PrintLine("Seriously?!"); return;}

    if (IsObjVisible(obj) == 0)
      {PrintLine("You can't see that here."); return;}

    if (must_hold && Obj[obj].loc != 2048 + OBJ_YOU)
      {PrintLine("You aren't holding it."); return;}
  }

  for (i=0; DoMisc[i].f != 0; i++)
    if (DoMisc[i].action == action && DoMisc[i].obj == obj)
  {
    DoMisc[i].f();
    return;
  }

  if (obj == OBJ_YOU)
    {PrintLine("Seriously?!"); return;}

  if (obj > 0 && obj < NUM_OBJECTS)
    switch (action)
  {
    case A_OPEN:   OpenObj(obj);   return;
    case A_CLOSE:  CloseObj(obj);  return;
    case A_LOOKIN: LookInObj(obj); return;
    case A_EMPTY:  EmptyObj(obj);  return;
  }

  PrintCantAction(action);
}
//*****************************************************************************



//*****************************************************************************
void Parse(void)
{
  int action, i, temp, temp2;

  TimePassed = 0;

  if (CurWord == NumStrWords || MatchCurWord("then")) return;

  if (CurWord > 0 && Verbosity != V_SUPERBRIEF) PrintBlankLine(); //print blank line between turns

  action = GetActionFromInput();

  if (InterceptAction(action)) return;

  if (action == 0)
  {
    PrintLine("I didn't recognize a verb in that sentence.");
    return;
  }


  //hacks to allow "turn obj on/off" "take obj off" "put obj on" to be
  //caught by "activate/deactivate" "remove" and "wear"
  //NOTE: this will allow strange commands like "spin obj on" "remove obj off" "wear obj on"

  temp = CurWord; SkipObjFromInput(Obj[OBJ_YOU].loc);

  if (action == A_TURN && MatchCurWord("on" ))
    action = A_ACTIVATE;   // "turn obj on"

  if (action == A_TURN && MatchCurWord("off"))
    action = A_DEACTIVATE; // "turn obj off""

  if (action == A_TAKE && MatchCurWord("off"))
  {
    temp2 = CurWord; SkipObjFromInput(Obj[OBJ_YOU].loc);

    if (temp2 == CurWord) action = A_REMOVE;  // "take obj off"
    else                  action = A_TAKEOFF; // "take obj off obj2"
  }

  if (action == A_PUT && MatchCurWord("on"))
  {
    temp2 = CurWord; SkipObjFromInput(Obj[OBJ_YOU].loc);

    if (temp2 == CurWord) action = A_WEAR;  // "put obj on"
    else                  action = A_PUTON; // "put obj on obj2"
  }

  CurWord = temp;


  //replace "go dir" with "dir"
  if (action == A_GO)
  {
    for (i=0; ; i++)
    {
      if (VerbToAction[i].phrase == 0)
      {
        PrintLine("I couldn't find a direction to go in that sentence.");
        return;
      }

      action = VerbToAction[i].action;
      if (action >= A_NORTH && action <= A_OUT && MatchCurWord(VerbToAction[i].phrase)) break;
    }
  }

  //special movements; executed function can fall through if it returns 0
  //A_IN and A_OUT must be handled here
  for (i=0; ; i++)
  {
    if (GoFrom[i].f == 0) break;

    if (GoFrom[i].room == Obj[OBJ_YOU].loc && GoFrom[i].action == action)
    {
      SkipObjFromInput(Obj[OBJ_YOU].loc); // hack; in case player types something like "go in house"

      if (GoFrom[i].f()) return;
    }
  }

  if (action == A_IN || action == A_OUT) //if these actions are not handled above, print blocked message
  {
    PrintBlockMsg(BL0);
    return;
  }

  if (action >= A_NORTH && action <= A_DOWN)
  {
    SkipObjFromInput(Obj[OBJ_YOU].loc); // hack; in case player types something like "go down chute"

    ParseActionDirection(action);
    return;
  }


  switch (action)
  {
    case A_SAVE:     DoSave();     return;
    case A_RESTORE:  DoRestore();  return;
    case A_SCORE:    PrintScore(); return;
    case A_VERSION:  DoVersion();  return;
    case A_DIAGNOSE: DoDiagnose(); return;

    case A_RESTART:
    case A_QUIT:
      ParseActionRestartOrQuit(action);
      return;

    case A_BRIEF:      Verbosity = V_BRIEF;      PrintLine("Now using brief descriptions."); return;
    case A_VERBOSE:    Verbosity = V_VERBOSE;    PrintLine("Now using verbose descriptions.\n"); PrintPlayerRoomDesc(1); return;
    case A_SUPERBRIEF: Verbosity = V_SUPERBRIEF; PrintLine("Now using superbrief descriptions."); return;

    case A_ECHO:     DoEcho();     return;
    case A_PRAY:     DoPray();     return;
    case A_ODYSSEUS: DoOdysseus(); return;

    case A_LOOK: PrintPlayerRoomDesc(1);            return;
    case A_WAIT: PrintLine("Zzz."); TimePassed = 1; return;

    case A_ACTIVATE:   ParseActionWithTo(action, "on", "light that!");   return;
    case A_DEACTIVATE: ParseActionWithTo(action, "off", "extinguish that!"); return;

    // NOTE: when in darkness, player is still allowed to open/close because light might be inside container
    case A_OPEN:
    case A_CLOSE:
      DoActionOnObject(action);
      return;
  }

  // actions above this line work when in darkness
  if (IsPlayerInDarkness()) {PrintLine("It is too dark to see anything."); return;}
  // actions below this line are not possible when in darkness

  switch (action)
  {
    case A_INVENTORY: PrintPresentObjects(2048 + OBJ_YOU, 0, 0); return;

    case A_TAKE:    ParseActionTake();     return;
    case A_DROP:    ParseActionDropPut(0); return;
    case A_PUT:     ParseActionDropPut(1); return;
    case A_PUTON:   ParseActionPutOn();    return; // not as in wearing
    case A_TAKEOFF: ParseActionTakeOff();  return; // not as in wearing
    case A_EXAMINE: ParseActionExamine();  return;
    case A_GIVE:    ParseActionGive();     return;

    case A_DIG:     ParseActionWithTo(action, 0, "dig that!");     return;
    case A_LOCK:    ParseActionWithTo(action, 0, "lock that!");    return;
    case A_UNLOCK:  ParseActionWithTo(action, 0, "unlock that!");  return;
    case A_TURN:    ParseActionWithTo(action, 0, "turn that!");    return;
    case A_OIL:     ParseActionWithTo(action, 0, "oil that!");     return;
    case A_TIE:     ParseActionWithTo(action, 0, "tie that!");     return;
    case A_UNTIE:   ParseActionWithTo(action, 0, "untie that!");   return;
    case A_FIX:     ParseActionWithTo(action, 0, "fix that!");     return;
    case A_INFLATE: ParseActionWithTo(action, 0, "inflate that!"); return;
    case A_DEFLATE: ParseActionWithTo(action, 0, "deflate that!"); return;
    case A_FILL:    ParseActionWithTo(action, 0, "fill that!");    return;
    case A_ATTACK:  ParseActionWithTo(action, 0, "attack that!");  return;

    case A_JUMP:      DoJump();      return;
    case A_SLEEP:     DoSleep();     return;
    case A_DISEMBARK: DoDisembark(); return;
    case A_LAUNCH:    DoLaunch();    return;
    case A_LAND:      DoLand();      return;
    case A_SWIM:      DoSwim();      return;

    default:
      DoActionOnObject(action);
      return;
  }

  //not reached
}
//*****************************************************************************



//#############################################################################
void Shutdown(void)
{
  if (InputStream != NULL && InputStream != stdin) fclose(InputStream);
}



void RestartGame(void)
{
  InitGameState(); // sets ItObj

  NumStrWords = 0;
  GameOver = 0;

  PrintLine("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nWelcome to Zork I: The Great Underground Empire!\n(c) 1980 by INFOCOM, Inc.\n  C port and parser (c) 2021 by Donnie Russell II\n\n");

  PrintPlayerRoomDesc(0);
}



void main(int argc, char *argv[])
{
  //process command line arguments
  atexit(Shutdown);
  if (argc < 2) InputStream = stdin;
  else
  {
    InputStream = fopen(argv[1], "r");
    if (InputStream == NULL) InputStream = stdin;
  }

#ifndef NO_STATUS_LINE
  VTMode = EnableVTMode();
#endif

  InitRandom(time(NULL));

  RestartGame();

  for (;;)
  {
    if (NumStrWords == 0)
    {
      for (;;)
      {
        if (Verbosity != V_SUPERBRIEF) PrintBlankLine();
        GetWords(">");
        if (NumStrWords == 0) PrintLine("Please type your command.");
        else break;
      }
    }

    PrevItObj = ItObj;
    ItObj = 0;

    Parse();

    if (TimePassed) NumMoves++;

    if (TimePassed && GameOver == 0) RunEventRoutines();

    if (GameOver)
    {
      if (GameOver == 2) {RestartGame(); continue;}

      PrintScore();
      for (;;)
      {
        GetWords("\nDo you want to restart, restore or exit? ");
        if (MatchCurWord("restart")) {RestartGame(); break;}
        if (MatchCurWord("restore")) {if (DoRestore()) break;}
        if (MatchCurWord("exit")) exit(0);
      }
      continue;
    }

    // if no time passed or
    //   parsed command contained extra words, such as "open box okay"
    //     then abort parsing any remaining commands from input
    if (TimePassed == 0 || MatchCurWord("then") == 0) NumStrWords = 0;

    //"continue"s above take us here
  }
}
//#############################################################################
