// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



//fragments generated at https://www.dcode.fr/frequency-analysis

//64 fragments of four letters each
char *FourFragments =
  " thethe  Theing . ThThe  you You is  to You  of . Yon't and  and"
  " can thaour you thatyourn thhere in f thof the t aree thou cto t"
  "are o thu caed. hat he rwith wit nothis ng tith can'an'the bs to"
  " be he cs yo thiis anot dinghe s lea it ou at the ro rooroom her"
;

//64 fragments of three letters each
char *ThreeFragments =
  "he  ththeing Thou Theng . Tis you yoYou Yo toto  is a re  of anof andnd e t. Y cas a ines hatat "
  "'t n't nohere soured n tcans tthaered.  beur er e ot.  hein thi on itly  wiare arth e. t ost e b"
;

//128 fragments of two letters each
char *TwoFragments =
  "e  theths . ou at in ire o saneratngisd  b cn  Tr  w,  hg Throu "
  "areastesndo tole ly  yithayoonYo Y d fhisef a ened rteorurloofve"
  "ash  n  noomcaalnellooly gowl den''t pbe mtiriotcheeutpeusunwiho"
  "m ntliradid.opla! blt. eoltrelice.geecwametadosiettsosadcks.ilbo"
;



//must add up to 128
#define NUM_4_FRAGMENTS  30  //max 64
#define NUM_3_FRAGMENTS   0  //max 64
#define NUM_2_FRAGMENTS  98  //max 128



//text_out size will always be less than or equal to text_in size
//text must not contain characters 128-255: they will represent fragments
int CompressText(char *text_in, int size_in, char *text_out)
{
  int size_out = 0, i;
  char *p = text_in, *q;

  while (p - text_in <= size_in - 4)
  {
    q = FourFragments;
    for (i=0; i<NUM_4_FRAGMENTS; i++, q += 4)
      if (p[0] == q[0] && p[1] == q[1] && p[2] == q[2] && p[3] == q[3]) break;
    if (i < NUM_4_FRAGMENTS) {p += 4; text_out[size_out++] = 128+i; continue;}

    q = ThreeFragments;
    for (i=0; i<NUM_3_FRAGMENTS; i++, q += 3)
      if (p[0] == q[0] && p[1] == q[1] && p[2] == q[2]) break;
    if (i < NUM_3_FRAGMENTS) {p += 3; text_out[size_out++] = 128+NUM_4_FRAGMENTS+i; continue;}

    q = TwoFragments;
    for (i=0; i<NUM_2_FRAGMENTS; i++, q += 2)
      if (p[0] == q[0] && p[1] == q[1]) break;
    if (i < NUM_2_FRAGMENTS) {p += 2; text_out[size_out++] = 128+NUM_4_FRAGMENTS+NUM_3_FRAGMENTS+i; continue;}

    text_out[size_out++] = *p++;
  }

  while (p - text_in < size_in)
  {
    text_out[size_out++] = *p++;
  }

  return size_out;
}



int GetDecompressTextSize(char *text_in, int size_in)
{
  char *p = text_in;
  int size_out = 0;

  while (p - text_in < size_in)
  {
    unsigned char ch = *p++;

    if (ch < 128) size_out++;
    else if (ch < 128 + NUM_4_FRAGMENTS) size_out += 4;
    else if (ch < 128 + NUM_4_FRAGMENTS+NUM_3_FRAGMENTS) size_out += 3;
    else size_out += 2;
  }
  return size_out;
}



int DecompressText(char *text_in, int size_in, char *text_out)
{
  char *p = text_in, *t = text_out, *q;

  while (p - text_in < size_in)
  {
    unsigned char ch = *p++;

    if (ch < 128) *t++ = ch;
    else if (ch < 128 + NUM_4_FRAGMENTS)
    {
      q = FourFragments + 4*(ch - 128);
      *t++ = *q++; *t++ = *q++; *t++ = *q++; *t++ = *q++;
    }
    else if (ch < 128 + NUM_4_FRAGMENTS+NUM_3_FRAGMENTS)
    {
      q = ThreeFragments + 3*(ch - (128 + NUM_4_FRAGMENTS));
      *t++ = *q++; *t++ = *q++; *t++ = *q++;
    }
    else
    {
      q = TwoFragments + 2*(ch - (128 + NUM_4_FRAGMENTS+NUM_3_FRAGMENTS));
      *t++ = *q++; *t++ = *q++;
    }
  }
  return (t - text_out);
}
