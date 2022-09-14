#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include <dirent.h>
#include <termios.h>
#include <unistd.h>

#include "cbase/base.h"
#include "cbase/base.c"

struct string {
  S32  l;
  char *str;
};

internal struct string
strinit(char *str, U64 l)
{
  struct string result;
  result.l   = l;
  result.str = (char *)malloc((l+1) * sizeof(str));
  strcpy(result.str, str);
  return(result);
}
#define strlit(s) strinit(s, arraycount(s)-1)

internal inline void
strdie(struct string str)
{
  free(str.str);
}

internal void 
recurse_dir(char *path, struct string paths[], S32 *i_path, S32 depth)
{
  S32             l;
  char            newpath[256];
  DIR             *d;
  struct dirent   *dir;

  if ((d = opendir(path))) 
  {
    l = strlen(path);
    paths[*i_path] = strinit(path, l);
    ++(*i_path);
    while ((dir = readdir(d))) 
    {
      if (dir->d_name[0] != '.')
      {
        strcpy(newpath, path);
        newpath[l] = '/';
        strcpy(newpath + l + 1, dir->d_name);
        recurse_dir(newpath, paths, i_path, depth+1);
      }
    }
    closedir(d);
  }
}


internal S32 
findletter(char lett, struct string word, S32 i_start)
{
  S32 result = -1, i = i_start;
  while (i < word.l && result < 0)
  {
    if (word.str[i] == lett)
      result = i;
    ++i;
  }
  return(result);
}

internal S32
search(struct string searchterm, struct string paths[], S32 pathcnt, B32 activepaths[])
{
  struct string   pathcurr;
  S32             activecnt, i_last, i_word, i_path;
  char            lett;
  
  activecnt = 0;
  for (i_path = 0; i_path < pathcnt; ++i_path)
  {
    pathcurr = paths[i_path];
    i_last = 0, i_word = 0;
    while (i_word < searchterm.l && i_last >= 0)
    {
      lett = searchterm.str[i_word];
      i_last = findletter(lett, pathcurr, i_last);
      ++i_word;
    }
    if (i_last != -1)
    {
      activepaths[i_path] = 1;
      ++activecnt;
    }
  }

  return(activecnt);
}

internal char
keypress()
{
  char c, echo;
  struct termios state_saved, state_new;

  if (tcgetattr(STDIN_FILENO, &state_saved) == -1)
    return EOF;

  state_new = state_saved;
  echo = ECHO;

  state_new.c_lflag &= ~(echo | ICANON);
  state_new.c_cc[VMIN] = 1;
  
  if (tcsetattr(STDIN_FILENO, TCSANOW, &state_new) == -1)
    return EOF;
  
  c = getchar();
  
  if (tcsetattr(STDIN_FILENO, TCSANOW, &state_saved) == -1)
    return EOF;
  
  return(c);
}

#define MAX_PATHS 512
#define MAX_WORD_LEN 64

#define KEY_RETURN 10
#define KEY_BACK   127
#define KEY_UP     65
#define KEY_DOWN   66
#define KEY_ESC    27

#define SCREENCLS  "\033[2J\n"
#define CURSORCLS  "\033[0;0f\n"

S32
main()
{
  S32             pathcnt, activecnt, i_path, i_arr, i_selected, esclv, i_active;
  struct string   pathcurr, *pathselected, searchterm, paths[MAX_PATHS];
  B32             enter, activepaths[MAX_PATHS];
  char            c, chararr[MAX_WORD_LEN];

  pathcnt = 0;
  memset(paths, 0, sizeof(paths));
  recurse_dir(".", paths, &pathcnt, 0);
    
  memset(chararr, 0, sizeof(chararr));
  enter = false, i_arr = 0, i_selected = 0, activecnt = 0, esclv = 0;
  while (!enter) 
  {
    // TODO(Elias): possibly a better way instead of constanly mallocing and freeing
    searchterm = strinit(chararr, i_arr);
    memset(activepaths, 0, sizeof(activepaths));
    activecnt = search(searchterm, paths, pathcnt, activepaths);
    strdie(searchterm);
    
    printf(SCREENCLS CURSORCLS);
    printf("searchterm: %s\n", (chararr[0])? chararr : "(null)");
    printf("possible directories:\n");
    
    i_active = 0;
    for (i_path = 0; i_path < arraycount(paths); ++i_path)
    {
      if (activepaths[i_path])
      {
        if (i_active == i_selected)
        {
          pathselected = &paths[i_path];
          printf(" >>> ");
        }
        else
        {
          printf("     ");
        }
        printf("%s\n", paths[i_path].str);
        ++i_active;
      }
    }
    
    switch ((c = keypress()))
    {
      case (KEY_RETURN):
        enter = true;
        break;
      case (KEY_ESC):
        esclv = 1;
        break;
      case ('['):
        if (esclv > 0)
        {
          ++esclv;
          break; 
        }
      case (KEY_UP):
        if (esclv > 1)
        {
          i_selected = clampbot(0, i_selected - 1);
          esclv = 0;
          break;
        }
      case (KEY_DOWN):
        if (esclv > 1)
        {
          i_selected = clamptop(i_selected + 1, activecnt - 1);
          esclv = 0;
          break;
        }
      case (KEY_BACK):
        i_selected = 0;
        i_arr = clampbot(0, i_arr - 1);
        chararr[i_arr] = 0;
        break;
      default:
        if (inbetween(c, ' ', '~') && esclv < 2)
        {
          i_selected = 0;
          chararr[i_arr] = c;
          i_arr = clamptop(i_arr + 1, MAX_WORD_LEN - 2);
        }
        else 
        {
          esclv = 0;
        }
        break;
    }
    
    // TODO(Elias): do we need to cap loops per second?
  } 

  printf(SCREENCLS CURSORCLS);
  printf("\ngoing to the following directory: %s\n\n", pathselected->str);

  // NOTE(Elias): change direcorty not working, this only changes the directory of the current process
  S32 err = chdir("..");
  printf("%d\n", err);

  for (i_path = 0; i_path < pathcnt; ++i_path)
    strdie(paths[i_path]);
}
