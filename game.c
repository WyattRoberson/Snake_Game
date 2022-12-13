#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "snake.h"
#include "food.h"
#include "game_window.h"
#include "key.h"
#include "game.h"
void generate_points(int *food_x, int *food_y, int width, int height, int x_offset, int y_offset){
    *food_x = rand() % (width-1) + x_offset+1;
    *food_y = rand() % (height-1) + y_offset+1;
}

void game(){
    enum State state = INIT; // Set the initial state
    static int x_max, y_max; //Max screen size variables
    int x_offset, y_offset; // distance between the top left corner of your screen and the start of the board
    gamewindow_t *window; // Name of the board
    Snake *snake; // The snake
    Snake *enemy_snake; //Enemy snake
    int snake_length = 3; //Length of snake body
    int enemy_snake_length = 3; //Length of enemy snake
    Food *foods,*new_food; // List of foods (Not an array)
    int size = 0; //Size of increase/decreased board
    int foodsAte = 0; //Number of foods ate by snake
    float speed = 4; //Denominator of timeret.tv_nsec
    int score = 0; //Score of game
    int lives = 3; //Snakes lives
    int difficulty = 1; //Game difficulty (Number of obstacles and food)
    int randTurn = 0; //Helps determine next time enemy snake should make a random turn
    int height = 30; //Height of game border
    int width = 70; //Width of game board
    char ch; //Key pressed
    char lastCh; //Last key pressed
    char enemy_snake_dir; //Enemy snake direction
    char last_enemy_snake_dir; //Enemy snake's last direction

    time_t t;
    srand((unsigned) time(&t));
    struct timespec timeret;
    timeret.tv_sec = 0;
    timeret.tv_nsec = 999999999/4;
    
    while(state != EXIT){
        switch(state){
        case INIT:
            initscr();
            start_color();
	    init_pair(1, COLOR_GREEN, COLOR_BLACK);
	    init_pair(2, COLOR_CYAN, COLOR_BLACK);
	    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
	    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	    init_pair(5, COLOR_RED, COLOR_BLACK);
            nodelay(stdscr, TRUE); //Dont wait for char
            noecho(); // Don't echo input chars
            getmaxyx(stdscr, y_max, x_max);
            keypad(stdscr, TRUE); // making keys work
            curs_set(0); // hide cursor
            timeout(100);

            // Setting height and width of the board
            x_offset = (x_max / 2) - (width / 2);
            y_offset = (y_max / 2) - (height / 2);
            
	    //On boot menu and instructions
	    int load = 0;
	    while(ch != 's' && ch != 'S' && ch != 'l' && ch != 'L'){
	      attron(COLOR_PAIR(1));
	      mvprintw(y_offset, x_offset, "Snake Game!");
	      mvprintw(y_offset + 2, x_offset, "Use the arrow keys to control your snake and stay alive");
	      mvprintw(y_offset + 4, x_offset, "The objective is to eat the food and gain as many points as possible:");
	      attron(COLOR_PAIR(3));
	      mvprintw(y_offset + 6, x_offset + 4, "O's and +'s make you grow and are worth 20 points!");
	      mvprintw(y_offset + 8, x_offset + 4, "X's and -'s make you shrink and subtract 10 points!");
	      attron(COLOR_PAIR(1));
	      mvprintw(y_offset + 10, x_offset, "Be sure not to run into yourself or the wall (W's)!");
	      mvprintw(y_offset + 12, x_offset, "Press P to pause and Q to quit at any time.");
	      attron(COLOR_PAIR(2));
	      mvprintw(y_offset + 14, x_offset, "Please select a difficulty below by pressing the corresponding letter.");
	      attron(COLOR_PAIR(5));
	      mvprintw(y_offset + 16, x_offset + 4, "E - Easy");
	      attron(COLOR_PAIR(1));
	      mvprintw(y_offset + 18, x_offset + 4, "H - Hard");
	      mvprintw(y_offset + 20, x_offset + 4, "A - Advanced");
	      attron(COLOR_PAIR(2));
	      mvprintw(y_offset + 22, x_offset, "PRESS S TO START GAME, L TO LOAD A GAME, OR T FOR TOP 10 SCORES!");
	      attroff(COLOR_PAIR(2));
	      int back = 0;
	      while((ch != 's' && ch != 'S') && back != 1){
	    	if(ch == 'e' || ch == 'E'){
	    	  attron(COLOR_PAIR(5));
		  mvprintw(y_offset + 16, x_offset + 4, "E - Easy");
	    	  attron(COLOR_PAIR(1));
		  mvprintw(y_offset + 18, x_offset + 4, "H - Hard");
		  mvprintw(y_offset + 20, x_offset + 4, "A - Advanced");
		  attroff(COLOR_PAIR(1));
		  difficulty = 1;
		  speed = 4;
		  timeret.tv_nsec = 999999999/speed;
		}
		else if(ch == 'h' || ch == 'H'){
		  attron(COLOR_PAIR(1));
		  mvprintw(y_offset + 16, x_offset + 4, "E - Easy");
		  attron(COLOR_PAIR(5));
		  mvprintw(y_offset + 18, x_offset + 4, "H - Hard");
		  attron(COLOR_PAIR(1));
		  mvprintw(y_offset + 20, x_offset + 4, "A - Advanced");
		  attroff(COLOR_PAIR(1));
		  difficulty = 2;
		  speed = 8;
		  timeret.tv_nsec = 999999999/speed;
		}
		else if(ch == 'a' || ch == 'A'){
		  attron(COLOR_PAIR(1));
		  mvprintw(y_offset + 16, x_offset + 4, "E - Easy");
		  mvprintw(y_offset + 18, x_offset + 4, "H - Hard");
		  attron(COLOR_PAIR(5));
		  mvprintw(y_offset + 20, x_offset + 4, "A - Advanced");
		  attroff(COLOR_PAIR(5));
		  difficulty = 3;
		  speed = 16;
		  timeret.tv_nsec = 999999999/speed;
		}
		else if(ch == 'l' || ch == 'L'){
		  load = 1;
		  clear();
		  break;
		}
		else if(ch == 't' || ch == 'T'){
		  clear();
		  int scores[10] = {0};
		  FILE* file = fopen("./saves/save_best_10.game", "r");
		  int i, j, num;
		  int index = 0;
		  while(fscanf(file, "%d", &num) != EOF){
		    scores[index] = num;
		    index++;
		  }
		  fclose(file);
		  int max = -100000;
		  for(i = 0; i < 10; i++){
		    for(j = 0; j < 10; j++){
		      if(scores[j] > max){
			max = scores[j];
			index = j;
		      }
		    }
		    scores[index] = -100000;
		    mvprintw(y_offset+i, x_offset, "%d. %d", i, max);
		    max = -100000;
		  }
		  mvprintw(y_offset+11, x_offset, "Press B to go back to main menu");
		  refresh();
		  while(ch != 'b' && ch != 'B'){
		    ch = get_char();
		  }
		  clear();
		  back = 1;
		}
		ch = get_char();
		refresh();
	      }
	      refresh();
            }
	    clear();

	    if(load == 1){
	      char saveNum[4];
	      int count = 0;
	      mvprintw(y_offset + (height / 4), x_offset + (width / 2), "Type a Save Number (3 max): ");
	      while(ch != 10 && count < 3){
		ch = getch();
		if(ch >= 48 && ch <= 57){
		  saveNum[count] = ch;
		  mvprintw(y_offset + (height / 4), x_offset + (width / 2) + 28 + count, "%c", ch);
		  count++;
		  refresh();
		}
	      }
	      saveNum[3] = '\0';
	      char buf[0x100];
	      snprintf(buf, sizeof(buf), "./saves/save_%s.game", saveNum);
	      FILE* file = fopen(buf, "r");
	      char nums[25];
	      int stop = 0;
	      int snakeX, snakeY;
	      int food_x, food_y;
	      char food_type;
	      
	      //Loads snake
	      fgets(nums, sizeof nums, file);
	      sscanf(nums, "%d,%d", &snakeX, &snakeY);
	      snake = create_tail(snakeX, snakeY);
	      Snake* temp = snake;
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
		sscanf(nums, "%d,%d", &snakeX, &snakeY);
		temp->next = create_tail(snakeX, snakeY);
		temp = temp->next;
	      }
	      
	      //Load last direction of snake
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
		sscanf(nums, "%hhd", &lastCh);
	      }
	      
	      //Load Foods
	      fgets(nums, sizeof nums, file);
              sscanf(nums, "%d,%d,%c", &food_x, &food_y, &food_type);
	      if(food_type == 'O' || food_type == '+')
		foods = create_food(food_x, food_y, Increase);
	      else if(food_type == 'X' || food_type == '-')
		foods = create_food(food_x, food_y, Decrease);
	      else if(food_type == 'W')
                foods = create_food(food_x, food_y, obstacle);
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
	        sscanf(nums, "%d,%d,%c", &food_x, &food_y, &food_type);
		if(food_type == 'O' || food_type == '+')
		  new_food = create_food(food_x, food_y, Increase);
		else if(food_type == 'X' || food_type == '-')
		  new_food = create_food(food_x, food_y, Decrease);
		else if(food_type == 'W')
		  new_food = create_food(food_x, food_y, obstacle);
		add_new_food(foods, new_food);
              }
	      
	      //Loading Board
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d,%d,%d,%d", &x_offset, &y_offset, &width, &height);
              }
	      window = init_GameWindow(x_offset, y_offset, width, height);
	      
	      //Loading Score
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d", &score);
              }
	      
	      //Loading Difficulty
              while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d", &difficulty);
              }

	      //Loading foodsAte
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d", &foodsAte);
              }

	      //Loading Snake Speed
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%f", &speed);
		timeret.tv_nsec = 999999999/speed;
              }
	      
	      //Loading Board Size
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d", &size);
              }
	      
	      //Loading Lives
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d", &lives);
              }

	      //Loading Snake Length
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d", &snake_length);
              }
	      
	      //Loading Enemy Snake
	      int enemy_snakeX, enemy_snakeY;
	      fgets(nums, sizeof nums, file);
              sscanf(nums, "%d,%d", &enemy_snakeX, &enemy_snakeY);
              enemy_snake = create_tail(enemy_snakeX, enemy_snakeY);
              Snake* temp2 = enemy_snake;
              while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d,%d", &enemy_snakeX, &enemy_snakeY);
                temp2->next = create_tail(enemy_snakeX, enemy_snakeY);
                temp2 = temp2->next;
              }
	      
	      //Loading direction of enemy snake
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%c", &enemy_snake_dir);
              }
	      
	      //Loading length of enemy snake
	      while(fgets(nums, sizeof nums, file) != NULL && nums[0] != '#'){
                sscanf(nums, "%d", &enemy_snake_length);
              }

	      fclose(file);
	      state = ALIVE;
	      break;
	    }
	    
            //Init board
            window = init_GameWindow(x_offset, y_offset, width, height);

            // Init snake
            snake = init_snake(x_offset + (width / 2), y_offset + (height / 2));
	    
            // Init foods
            int food_x, food_y, i;
            enum Type type;

	    
	    //Creates obstacles based on difficulty
		generate_points(&food_x, &food_y, width, height, x_offset, y_offset);
        type = obstacle;
        foods = create_food(food_x, food_y, type);
	    int numObstacles = 7;
	    for(i = 1; i < numObstacles*difficulty; i++){
			int obstacleLength = (rand() % 3) + 1;
			int obstacleWidth = (rand() % 3) + 1;
			generate_points(&food_x, &food_y, width, height, x_offset, y_offset);
			while (food_exists(foods,food_x, food_y)){
				generate_points(&food_x, &food_y, width, height, x_offset, y_offset);
			}
			//type = obstacle;
			int j, k;
			for(j = 0; j < obstacleLength; j++){
				for(k = 0; k < obstacleWidth; k++){
					if(food_x+j < x_offset+width && food_y+k < y_offset+height){
						new_food = create_food(food_x+j, food_y+k, type);
						add_new_food(foods, new_food);
					}
				}
			}
	    }
	    
		//Generate foods based on difficulty
            //generate_points(&food_x, &food_y, width, height, x_offset, y_offset);
            //type = (rand() > RAND_MAX/2) ? Increase : Decrease; // Randomly deciding type of food
            //foods = create_food(food_x, food_y, type);
            for(i = 1; i < 7*difficulty; i++){
                generate_points(&food_x, &food_y, width, height, x_offset, y_offset);
                while (food_exists(foods,food_x, food_y))
                    generate_points(&food_x, &food_y, width, height, x_offset, y_offset);
                type = (rand() > RAND_MAX/2) ? Increase : Decrease;
                new_food = create_food(food_x, food_y, type);
                add_new_food(foods, new_food);
            }

	    // Init enemy snake
	    Snake* head = create_tail(x_offset + (width / 2) - 5, y_offset + (height / 2));
	    Snake* tail1 = create_tail(x_offset + (width / 2) - 4, y_offset + (height / 2));
	    Snake* tail2 = create_tail(x_offset + (width / 2) - 3, y_offset + (height / 2));
	    tail1->next = tail2;
	    head->next = tail1;
	    enemy_snake = head;

	    lastCh = 2;
	    enemy_snake_dir = 1;
            state = ALIVE;
            break;

        case ALIVE:
	    ch = get_char();
	    clear();

	    //If no new direction is chosen and the quit/pause option is not chosen
	    //Move the snake in the same direction
	    if(ch != 1 && ch != 2 && ch != 3 && ch != 4 && ch != 'q' && ch != 'Q' & ch != 'p' && ch != 'P' && ch != '=' && ch != '-' && ch != 'f' && ch != 'F'){
	      snake = move_snake(snake, lastCh);
	    }
	    //If new direction is chosen
	    else if(ch == 1 || ch == 2 || ch == 3 || ch == 4){
	      if((ch == 1 && lastCh == 2) || (ch == 2 && lastCh == 1) || (ch == 3 && lastCh == 4) || (ch == 4 && lastCh == 3)){
		snake = move_snake(snake, lastCh);
	      }
	      else{
	        snake = move_snake(snake, ch);
		lastCh = ch;
	      }
	    }
	    
	    //Enemy snake
	    randTurn++;
	    if(randTurn % 5 == 0){
	      last_enemy_snake_dir = enemy_snake_dir;
	      enemy_snake_dir = (rand() % 4)+1;
	      if((enemy_snake_dir == 1 && last_enemy_snake_dir == 2) || (enemy_snake_dir == 2 && last_enemy_snake_dir == 1) || (enemy_snake_dir == 3 && last_enemy_snake_dir == 4) || (enemy_snake_dir == 4 && last_enemy_snake_dir == 3)){
		enemy_snake_dir = last_enemy_snake_dir;
	      }
	    }
	    if(enemy_snake_dir == 2 && enemy_snake->x + 2 >= x_offset + width){
	      enemy_snake_dir = 4;
	    }
	    else if(enemy_snake_dir == 1 && enemy_snake->x - 2 <= x_offset){
	      enemy_snake_dir = 3;
            }
	    else if(enemy_snake_dir == 3 && enemy_snake->y - 2 <= y_offset){
	      enemy_snake_dir = 2;
            }
	    else if(enemy_snake_dir == 4 && enemy_snake->y + 2 >= y_offset + height){
	      enemy_snake_dir = 1;
            }
	    if(enemy_snake_dir == 2 && food_type(foods, enemy_snake->x + 1, enemy_snake->y) == 'W'){
	      enemy_snake_dir = 1;
	    }
	    else if(enemy_snake_dir == 1 && food_type(foods, enemy_snake->x - 1, enemy_snake->y) == 'W'){
              enemy_snake_dir = 2;
            }
	    else if(enemy_snake_dir == 3 && food_type(foods, enemy_snake->x, enemy_snake->y - 1) == 'W'){
              enemy_snake_dir = 4;
            }
	    else if(enemy_snake_dir == 4 && food_type(foods, enemy_snake->x, enemy_snake->y + 1) == 'W'){
              enemy_snake_dir = 3;
            }
	    enemy_snake = move_snake(enemy_snake, enemy_snake_dir);
	    
	    //If the size hasn't increased by 15, increase it by 5
	    if((ch == '=' || ch == '+') && size < 3){
	      x_offset -= 5;
	      y_offset -= 5;
	      width += 10;
	      height += 10;
	      size++;
	      window = changeGameWindow(x_offset, y_offset, width, height, window);
	    }
	    //If the size hasn't decreased by 15, decrease it by 5
	    else if(ch == '-' && size > -3){
	      x_offset += 5;
              y_offset += 5;
              width -= 10;
              height -= 10;
	      size--;
	      window = changeGameWindow(x_offset, y_offset, width, height, window);
	    }
	    //Pauses game
	    else if(ch == 'p' || ch == 'P'){
	      int pause = 1;
	      int select = 0;
	      while(pause == 1 && ch != 10){
	     	//Display menu options
	     	clear();
		attron(COLOR_PAIR(1));
	     	mvprintw(y_offset + (height / 4) + select, x_offset + (width / 2)-3, "->");
		if(select == 0){
		  attron(COLOR_PAIR(1));
	    	  mvprintw(y_offset + (height / 4) + 0, x_offset + (width / 2), "RESUME");
		  attron(COLOR_PAIR(4));
		  mvprintw(y_offset + (height / 4) + 1, x_offset + (width / 2), "SAVE");
		  mvprintw(y_offset + (height / 4) + 2, x_offset + (width / 2), "QUIT");
		}
		else if(select == 1){
		  attron(COLOR_PAIR(1));
	    	  mvprintw(y_offset + (height / 4) + 1, x_offset + (width / 2), "SAVE");
		  attron(COLOR_PAIR(4));
		  mvprintw(y_offset + (height / 4) + 0, x_offset + (width / 2), "RESUME");
		  mvprintw(y_offset + (height / 4) + 2, x_offset + (width / 2), "QUIT");
		}
		else if(select == 2){
		  attron(COLOR_PAIR(1));
		  mvprintw(y_offset + (height / 4) + 2, x_offset + (width / 2), "QUIT");
		  attron(COLOR_PAIR(4));
		  mvprintw(y_offset + (height / 4) + 0, x_offset + (width / 2), "RESUME");
		  mvprintw(y_offset + (height / 4) + 1, x_offset + (width / 2), "SAVE");
		}
		attroff(COLOR_PAIR(4));
	    	ch = get_char();
		if(ch == DOWN){
		  if(select < 2)
		    select++;
		  else
		    select = 0;
		}
		else if(ch == UP){
		  if(select > 0)
		    select--;
		  else
		    select = 2;
		}
		refresh();
		if(ch == 'p' || ch == 'P'){
		  clear();
                  pause = 0;
		  select = 0;
	        }
	      }
	      //Save option
	      if(select == 1){
		clear();
		mvprintw(y_offset + (height / 4), x_offset + (width / 2), "Type a Save Number (3 max): ");
                char saveNum[4];
		int count = 0;
		ch = 'w'; //ch was still equal to 10 from selecting save option
		while(ch != 10 && count < 3){
		  ch = getch();
		  if(ch >= 48 && ch <= 57){
		    saveNum[count] = ch;
		    mvprintw(y_offset + (height / 4), x_offset + (width / 2) + 28 + count, "%c", ch);
		    count++;
		    refresh();
		  }
		}
		saveNum[3] = '\0';
		if(count == 3) //Allows user to see the number they type before exiting program
		  sleep(2);

		//Save all necessary data
		char buf[0x100];
		snprintf(buf, sizeof(buf), "./saves/save_%s.game", saveNum);
		FILE* saveFile = fopen(buf, "w");
		Snake* copy = snake;
		while(copy){
		  fprintf(saveFile, "%d,%d\n", copy->x, copy->y);
		  copy = copy->next;
		}
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", lastCh);
		fprintf(saveFile, "#\n");
		Food* fcopy = foods;
		while(fcopy){
		  fprintf(saveFile, "%d,%d,%c\n", fcopy->x, fcopy->y, fcopy->type);
		  fcopy = fcopy->next;
		}
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d,%d,%d,%d\n", window->upper_left_x, window->upper_left_y, window->width, window->height);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", score);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", difficulty);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", foodsAte);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%.2f\n", speed);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", size);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", lives);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", snake_length);
		fprintf(saveFile, "#\n");
		copy = enemy_snake;
                while(copy){
                  fprintf(saveFile, "%d,%d\n", copy->x, copy->y);
                  copy = copy->next;
                }
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", enemy_snake_dir);
		fprintf(saveFile, "#\n");
		fprintf(saveFile, "%d\n", enemy_snake_length);
		fprintf(saveFile, "#\n");
		fclose(saveFile);
		clear();
		endwin();
		exit(0);
	      }
	      //QUIT selected
	      else if(select == 2){
		endwin();
		state = EXIT;
	      }
	      clear();
	    }
	    
	    //Quits game
	    else if(ch == 'q' || ch == 'Q'){
	      clear();
	      endwin();
	      state = EXIT;
	      break;
	    }

	    else if(ch == 'f' || ch == 'F'){
	      clear();
	      mvprintw(y_offset + (height / 4), x_offset + (width / 2), "Type a Save Number (3 max): ");
	      char saveNum[4];
	      int count = 0;
	      ch = 'w'; //ch was still equal to 10 from selecting save option
	      while(ch != 10 && count < 3){
		ch = getch();
		if(ch >= 48 && ch <= 57){
		  saveNum[count] = ch;
		  mvprintw(y_offset + (height / 4), x_offset + (width / 2) + 28 + count, "%c", ch);
		  count++;
		  refresh();
		}
	      }
	      saveNum[3] = '\0';
	      if(count == 3) //Allows user to see the number they type before exiting program
		sleep(2);

	      //Save all necessary data
	      char buf[0x100];
	      snprintf(buf, sizeof(buf), "./saves/save_%s.game", saveNum);
	      FILE* saveFile = fopen(buf, "w");
	      Snake* copy = snake;
	      while(copy){
		fprintf(saveFile, "%d,%d\n", copy->x, copy->y);
		copy = copy->next;
	      }
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", lastCh);
	      fprintf(saveFile, "#\n");
	      Food* fcopy = foods;
	      while(fcopy){
		fprintf(saveFile, "%d,%d,%c\n", fcopy->x, fcopy->y, fcopy->type);
		fcopy = fcopy->next;
	      }
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d,%d,%d,%d\n", window->upper_left_x, window->upper_left_y, window->width, window->height);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", score);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", difficulty);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", foodsAte);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%.2f\n", speed);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", size);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", lives);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", snake_length);
	      fprintf(saveFile, "#\n");
	      copy = enemy_snake;
	      while(copy){
		fprintf(saveFile, "%d,%d\n", copy->x, copy->y);
		copy = copy->next;
	      }
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", enemy_snake_dir);
	      fprintf(saveFile, "#\n");
	      fprintf(saveFile, "%d\n", enemy_snake_length);
	      fprintf(saveFile, "#\n");
	      fclose(saveFile);
	      clear();
	      endwin();
	      exit(0);
	    }
	    
	    Snake* temp = enemy_snake;
	    while(temp){
	      if(snake->x == temp->x && snake->y == temp->y){
		state = DEAD;
		attron(COLOR_PAIR(1));
		mvprintw(20, 20, "Key entered: %c", ch);
		mvprintw(21, 20, "SCORE: %d", score);
		mvprintw(22, 20, "LIVES: %d", lives);
		mvprintw(23, 20, "Gameboard Size: %d", size);
		attron(COLOR_PAIR(3));
		draw_Gamewindow(window);
		draw_obstacles(foods);
		attron(COLOR_PAIR(1));
		draw_snake(snake);
		attron(COLOR_PAIR(5));
                draw_snake(enemy_snake);
		attron(COLOR_PAIR(2));
		draw_food(foods);
		attroff(COLOR_PAIR(2));
		break;
	      }
	      temp = temp->next;
	    }

	    //If it hits a wall, kill snake
	    if(snake->y <= y_offset || snake->y >= y_offset + height || snake->x <= x_offset || snake->x >= x_offset + width){
	      state = DEAD;
	      attron(COLOR_PAIR(1));
	      mvprintw(20, 20, "Key entered: %c", ch);
	      mvprintw(21, 20, "SCORE: %d", score);
	      mvprintw(22, 20, "LIVES: %d", lives);
	      mvprintw(23, 20, "Gameboard Size: %d", size);
	      attron(COLOR_PAIR(3));
	      draw_Gamewindow(window);
	      draw_obstacles(foods);
	      attron(COLOR_PAIR(1));
	      draw_snake(snake);
	      attron(COLOR_PAIR(5));
	      draw_snake(enemy_snake);
	      attron(COLOR_PAIR(2));
	      draw_food(foods);
	      attroff(COLOR_PAIR(2));
	      break;
	    }
	    //Checks if it eats itself, kill snake
	    if(eat_itself(snake)){
	      state = DEAD;
	      attron(COLOR_PAIR(1));
	      mvprintw(20, 20, "Key entered: %c", ch);
	      mvprintw(21, 20, "SCORE: %d", score);
	      mvprintw(22, 20, "LIVES: %d", lives);
	      mvprintw(23, 20, "Gameboard Size: %d", size);
	      attron(COLOR_PAIR(3));
	      draw_Gamewindow(window);
	      draw_obstacles(foods);
	      attron(COLOR_PAIR(1));
	      draw_snake(snake);
	      attron(COLOR_PAIR(5));
	      draw_snake(enemy_snake);
	      attron(COLOR_PAIR(2));
	      draw_food(foods);
	      attroff(COLOR_PAIR(2));
	      break;
	    }

	    //Removes food if ate for enemy snake
	    if(food_exists(foods, enemy_snake->x, enemy_snake->y)){
	      type = food_type(foods, enemy_snake->x, enemy_snake->y);
              if(type != 'W')
                foods = remove_eaten_food(foods, enemy_snake->x, enemy_snake->y);
	      Snake* temp = enemy_snake;
	      Snake* prevSnake;
	      while(temp){
		prevSnake = temp;
		temp = temp->next;
	      }
	      if((type == 'O' || type == '+') && enemy_snake_length < 10){
		prevSnake->next = create_tail(prevSnake->x, prevSnake->y);
		enemy_snake_length++;
	      }
	      else if((type == 'X' || type == '-') && enemy_snake_length > 3){
	        enemy_snake = remove_tail(enemy_snake);
	        enemy_snake_length--;
	      }
	    }

	    //Removes food if ate
	    if(food_exists(foods, snake->x, snake->y)){
	      type = food_type(foods, snake->x, snake->y);
	      if(type != 'W')
	        foods = remove_eaten_food(foods, snake->x, snake->y);
	      Snake* temp = snake;
	      Snake* prevSnake;
	      while(temp){
	      	prevSnake = temp;
	        temp = temp->next;
	      }
	      
	      //Grow snake
	      if(type == 'O' || type == '+'){
		score += 20;
	        prevSnake->next = create_tail(prevSnake->x, prevSnake->y);
		foodsAte++;
		snake_length++;
	      }
	      //Shrink snake
	      else if(type == 'X' || type == '-'){
		if(snake_length == 1){
		  state = DEAD;
		  attron(COLOR_PAIR(1));
		  mvprintw(20, 20, "Key entered: %c", ch);
		  mvprintw(21, 20, "SCORE: %d", score);
		  mvprintw(22, 20, "LIVES: %d", lives);
		  mvprintw(23, 20, "Gameboard Size: %d", size);
		  attron(COLOR_PAIR(3));
		  draw_Gamewindow(window);
		  draw_obstacles(foods);
		  attron(COLOR_PAIR(1));
		  draw_snake(snake);
		  attron(COLOR_PAIR(5));
		  draw_snake(enemy_snake);
		  attron(COLOR_PAIR(2));
		  draw_food(foods);
		  attroff(COLOR_PAIR(2));
		  break;
		}
		score -= 10;
		snake = remove_tail(snake);
		foodsAte++;
		snake_length--;
	      }
	      //Obstacle type, kill snake
	      else if(type == 'W'){
		state = DEAD;
		attron(COLOR_PAIR(1));
		mvprintw(20, 20, "Key entered: %c", ch);
		mvprintw(21, 20, "SCORE: %d", score);
		mvprintw(22, 20, "LIVES: %d", lives);
		mvprintw(23, 20, "Gameboard Size: %d", size);
		attron(COLOR_PAIR(3));
		draw_Gamewindow(window);
		draw_obstacles(foods);
		attron(COLOR_PAIR(1));
		draw_snake(snake);
		attron(COLOR_PAIR(5));
		draw_snake(enemy_snake);
		attron(COLOR_PAIR(2));
		draw_food(foods);
		attroff(COLOR_PAIR(2));
		break;
	      }
	      
	      //Increase snake speed every 10 foods
	      if(foodsAte % 10 == 0){
		speed+=1.2;
		timeret.tv_nsec = 999999999/speed;
	      }
	      if(type == 'O' || type == '+' || type == 'X' || type == '-'){
		generate_points(&food_x, &food_y, width, height, x_offset, y_offset);
		type = (rand() > RAND_MAX/2) ? Increase : Decrease;
		new_food = create_food(food_x, food_y, type);
		add_new_food(foods, new_food);
	      }
	    }
	    
	    //Tells user what key they pressed
	    attron(COLOR_PAIR(1));
            mvprintw(20, 20, "Key entered: %c", ch);
	    mvprintw(21, 20, "SCORE: %d", score);
	    mvprintw(22, 20, "LIVES: %d", lives);
	    mvprintw(23, 20, "Gameboard Size: %d", size);
	    attron(COLOR_PAIR(3));
            draw_Gamewindow(window);
	    draw_obstacles(foods);
	    attron(COLOR_PAIR(1));
            draw_snake(snake);
	    attron(COLOR_PAIR(5));
	    draw_snake(enemy_snake);
	    attron(COLOR_PAIR(2));
            draw_food(foods);
	    attroff(COLOR_PAIR(2));
            break;

        case DEAD:
	    lives--;
	    if(lives != 0){
	      state = ALIVE;
	      snake = init_snake(x_offset + (width / 2), y_offset + (height / 2));
	      lastCh = 2;
	      snake_length = 3;
	      if(difficulty == 1){
		speed = 4;
		timeret.tv_nsec = 999999999/speed;
	      }
	      else if(difficulty == 2){
		speed = 8;
		timeret.tv_nsec = 999999999/speed;
	      }
	      else if(difficulty == 3){
		speed = 16;
		timeret.tv_nsec = 999999999/speed;
	      }
	      break;
	    }
	    else{
	      clear();
	      //Gave over screen
	      while(ch != 'q' && ch != 'Q'){
		attron(COLOR_PAIR(5));
		mvprintw(y_offset + (height / 4), x_offset + (width / 2), "GAME OVER!");
		attroff(COLOR_PAIR(5));
		mvprintw(y_offset + (height / 4) + 3, x_offset + (width / 2), "SCORE: %d", score);
		mvprintw(y_offset + (height / 4) + 6, x_offset + (width / 2), "PRESS Q TO QUIT");
		refresh();
		ch = get_char();
	      }
	      state = EXIT;
	      break;
	    }
        }
        refresh();
        nanosleep(&timeret, NULL);
    }
    int num;
    char strNum[10];
    int scores[10] = {0};
    int index = 0;
    FILE* file;
    if((file = fopen("./saves/save_best_10.game", "r+")) == NULL)
      file = fopen("./saves/save_best_10.game", "w+");
    //Populate array with current top 10 scores
    while(fscanf(file, "%s", strNum) != EOF){
      num = atoi(strNum);
      scores[index] = num;
      index++;
    }
    
    //Finds smallest score
    int i;
    int min = scores[0];
    index = 0;
    for(i = 1; i < 10; i++){
      if(scores[i] < min){
	min = scores[i];
	index = i;
      }
    }
    
    //If new score is greater than smallest score, replace it
    if(score > min)
      scores[index] = score;
    
    //Repopulate the file with correct top 10 scores
    rewind(file);
    for(i = 0; i < 10; i++){
      fprintf(file, "%d\n", scores[i]);
    }
    fclose(file);
    
    endwin();
}