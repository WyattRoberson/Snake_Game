#include <ncurses.h>
#include <stdlib.h>
#include "key.h"

int get_char(){
  int ch = getch();
  switch(ch){
    case KEY_LEFT:
      return LEFT;
      break;
    case KEY_RIGHT:
      return RIGHT;
      break;
    case KEY_UP:
      return UP;
      break;
    case KEY_DOWN:
      return DOWN;
      break;
    default:
      return(ch);
      break;
  }
}