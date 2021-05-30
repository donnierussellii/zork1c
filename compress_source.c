// Zork I: The Great Underground Empire
// (c) 1980 by INFOCOM, Inc.
// C port and parser (c) 2021 by Donnie Russell II

// This source code is provided for personal, educational use only.
// You are welcome to use this source code to develop your own works,
// but the story-related content belongs to the original authors of Zork.



#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int TotalUncompressed;
int TotalCompressed;



//from compress.c
int CompressText(char *text_in, int size_in, char *text_out);



void CompressPrintString(FILE *f, char *uncomp, char *comp, int uncomp_size)
{
  int comp_size;
  char *hex = "0123456789abcdef";

  comp_size = CompressText(uncomp, uncomp_size, comp);

  TotalUncompressed += uncomp_size;
  TotalCompressed += comp_size;

  while (comp_size--)
  {
    unsigned char ch = *comp++;

    fputs("\\x", f);
    fputc(hex[ch >> 4], f);
    fputc(hex[ch & 15], f);
  }
}



void CompressFile(char *filename_in, char *filename_out)
{
  FILE *f;
  int size, i;
  char *buffer, *p, *uncomp, *u, *comp;

  char *match1 = "PrintLine(\"", *match1c = "PrintCompLine(\"";
  int match1_len = 11;

  char *match2 = "PrintText(\"", *match2c = "PrintCompText(\"";
  int match2_len = 11;

  f = fopen(filename_in, "rb");
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fclose(f);

  buffer = malloc(size);
  uncomp = malloc(size);
  comp = malloc(size);

  f = fopen(filename_in, "rb");
  fread(buffer, 1, size, f);
  fclose(f);

  f = fopen(filename_out, "wb");

  p = buffer;
  while (p < buffer+size)
  {
    if ((p-buffer <= size-match1_len && memcmp(p, match1, match1_len) == 0) ||
        (p-buffer <= size-match2_len && memcmp(p, match2, match2_len) == 0))
    {
      if (memcmp(p, match1, match1_len) == 0) {fputs(match1c, f); p += match1_len;}
      else                                    {fputs(match2c, f); p += match2_len;}

      u = uncomp;
      while (p < buffer+size)
      {
        if (*p == '\\')  // escape sequence
        {
          p++;
          if (p < buffer+size)
          {
                 if (*p ==  'n') *u++ = '\n';
            else if (*p == '\"') *u++ = '\"';
            else if (*p == '\\') *u++ = '\\';
            p++;
          }
        }
        else if (*p == '\"')  // end of string
        {
          CompressPrintString(f, uncomp, comp, u - uncomp);
          putc(*p++, f);
          break;
        }
        else
          *u++ = *p++;
      }
    }
    else putc(*p++, f);
  }

  fclose(f);

  free(buffer);
  free(uncomp);
  free(comp);
}



void ExtractTextFromFile(char *filename_in, char *filename_out, int append)
{
  FILE *f;
  char *buffer, *p, *match1 = "PrintLine(\"", *match2 = "PrintText(\"";
  int size, i, match1_len = 11, match2_len = 11;

  f = fopen(filename_in, "rb"); fseek(f, 0, SEEK_END); size = ftell(f); fclose(f);
  buffer = malloc(size);
  f = fopen(filename_in, "rb"); fread(buffer, 1, size, f); fclose(f);
  f = fopen(filename_out, append ? "ab" : "wb");

  p = buffer;
  while (p < buffer+size)
  {
    if ((p-buffer <= size-match1_len && memcmp(p, match1, match1_len) == 0) ||
        (p-buffer <= size-match2_len && memcmp(p, match2, match2_len) == 0))
    {
      if (memcmp(p, match1, match1_len) == 0) p += match1_len; else p += match2_len;
      while (p < buffer+size)
      {
        if (*p == '\\') p += 2;
        else if (*p == '\"') {putc('\n', f); break;}
        else putc(*p++, f);
      }
    }
    else p++;
  }

  fclose(f);
  free(buffer);
}



void main(void)
{
  TotalUncompressed = 0;
  TotalCompressed = 0;

  CompressFile("game.c",     "_game.c");
  CompressFile("parser.c",   "_parser.c");
  CompressFile("villains.c", "_villain.c");

  printf("%i%%\n", 100 * TotalCompressed / TotalUncompressed);

  ExtractTextFromFile("game.c",     "_extract.txt", 0);
  ExtractTextFromFile("parser.c",   "_extract.txt", 1);
  ExtractTextFromFile("villains.c", "_extract.txt", 1);
}
