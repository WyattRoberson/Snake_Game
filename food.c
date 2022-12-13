#include <stdlib.h>
#include <ncurses.h>
#include <stdbool.h>
#include "food.h"


//Create new food
Food* create_food(int x, int y, enum Type type){
    Food* new_food = malloc(sizeof(Food));
     
    new_food->x = x;
    new_food->y = y;
    int option = rand() % 2;
    if (type == Increase){
      if(option == 1){
	new_food->type = 'O';
      }
      else {
	new_food->type = '+';
      }
    }
    else if(type == Decrease){
      if(option == 1){
        new_food->type = 'X';
      }
      else {
	new_food->type = '-';
      }
    }
    else if(type == obstacle){
      new_food->type = 'W';
    }
    new_food->next = NULL;
    
    return new_food;
}

//Check if food exists at coordinates
bool food_exists(Food* foods, int x, int y){
    Food* temp = foods;
    while(temp){
        if(temp->x == x && temp->y == y)
            return true;
        temp = temp->next;
    }
    return false;
}

//Add new food to end of food list
void add_new_food(Food* foods, Food* new_food){
  Food* temp = foods;
  while(temp->next) {
    temp = temp->next;
  }
  temp->next = new_food;
}


enum Type food_type(Food* foods, int x, int y){
  //Implement the code to return the type of the food 
  //present at position (x, y)
  Food* temp = foods;
  while(temp){
    if(temp->x == x && temp->y == y)
      return temp->type;
    temp = temp->next;
  } 	
}

Food* remove_eaten_food(Food* foods, int x, int y){
  //Implement the code to remove food at position (x,y).
  //Create a new linked list of type Food containing only the
  //needed food and return this new list

  Food* temp = foods;

  if(temp->x == x && temp->y == y){
    foods = temp->next;
    free(temp);
    return foods;
  } else{
    while(temp->next){
      if (temp->next->x == x && temp->next->y == y){
        break;
      } else{
          temp = temp->next;
      }
    }
  }

  Food* clear_food = temp->next;
  temp->next = temp->next->next;
  free(clear_food);
  return foods;
}

// Display all the food
void draw_food (Food *foods){
  Food* temp = foods;
  while(temp){
    if(temp->type != 'W'){
      mvprintw(temp->y, temp->x, "%c", temp->type);
    }
    temp = temp->next;
  }
}

void draw_obstacles(Food *foods){
  Food* temp = foods;
  while(temp){
    if(temp->type == 'W'){
      mvprintw(temp->y, temp->x, "%c", temp->type);
    }
    temp = temp->next;
  }
}