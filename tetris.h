#if !defined(_tetris_h)
#define _tetris_h

#include<ncurses.h>	/* Includes stdio.h */
#include<assert.h>
#include<unistd.h>	/* usleep() */
#include<pthread.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#define INFO_WINDOW_WIDTH 	11
#define GEARS 			    15
#define MAX_SPEED 		    20000U
#define MIN_SPEED 		    1000000U
#define SYMBOL			    '#'		/* The symbol to make figure from. 	*/
#define MAX_AREA		    9
#define NUMBER_OF_FIGURES	17	
#define SYMBOL_TO_FILL		'.'		/* The symbol to make empty area (background).	*/
#define SPEED_VALUES  		MIN_SPEED,700000,600000,400000,300000,200000,100000,90000,80000,70000,60000,50000,40000,30000,MAX_SPEED
#define KEY_SPACE		    ' '
#define BURN_TIME		    50000
#define BURN_SYMBOL		    '*'
#define LEVEL_SCORE		    40		/* How many scope need go get in a level.	*/
#define ONE_BURN_SCOPE		5		/* How many scope gives one burn.		*/
#define NUMBER_OF_COLORS	7
#define START_ALIGN		    10		/* The speed, figures start to fall the same place where it was before. */

/* General window size. */
#define GEN_WINDOW_WIDE  	26
#define GEN_WINDOW_HEIGHT 	25

/* Color pairs. */
#define COLOR_PAIRS	init_pair(1, COLOR_RED, 	COLOR_BLACK);	\
  			init_pair(2, COLOR_GREEN, 	    COLOR_BLACK);	\
  			init_pair(3, COLOR_YELLOW, 	    COLOR_BLACK);	\
  			init_pair(4, COLOR_BLUE, 	    COLOR_BLACK);	\
  			init_pair(5, COLOR_MAGENTA, 	COLOR_BLACK);	\
  			init_pair(6, COLOR_CYAN, 	    COLOR_BLACK);	\
  			init_pair(7, COLOR_WHITE, 	    COLOR_BLACK)

/* Figure dots. */
/* Define figures: area, width, height, dots{..} to print, ## figure_number, max_figure_of_this type (start from zero). */
#define FIGURES_DOTS   	{ 6, 3, 2, .dots = {1,1,0,0,1,1}, 0, 1 },	\
      			{ 6, 2, 3, .dots = {0,1,1,1,1,0}, 1, 1 },	\
									\
      			{ 4, 1, 4, .dots = {1,1,1,1}, 0, 1},		\
      			{ 4, 4, 1, .dots = {1,1,1,1}, 1, 1},		\
									\
      			{ 6, 2, 3, .dots = {1,0,1,0,1,1}, 0, 3 },	\
      			{ 6, 3, 2, .dots = {1,1,1,1,0,0}, 1, 3 },	\
      			{ 6, 2, 3, .dots = {1,1,0,1,0,1}, 2, 3 },	\
      			{ 6, 3, 2, .dots = {0,0,1,1,1,1}, 3, 3 },	\
									\
      			{ 6, 3, 2, .dots = {1,1,1,0,1,0}, 0, 3 },	\
      			{ 6, 2, 3, .dots = {0,1,1,1,0,1}, 1, 3 },	\
      			{ 6, 3, 2, .dots = {0,1,0,1,1,1}, 2, 3 },	\
      			{ 6, 2, 3, .dots = {1,0,1,1,1,0}, 3, 3 },	\
									\
      			{ 4, 2, 2, .dots = {1,1,1,1}, 0, 0 },		\
									\
      			{ 6, 3, 2, .dots = {1,1,1,1,0,1}, 0, 3 },	\
      			{ 6, 2, 3, .dots = {1,1,0,1,1,1}, 1, 3 },	\
      			{ 6, 3, 2, .dots = {1,0,1,1,1,1}, 2, 3 },	\
      			{ 6, 2, 3, .dots = {1,1,1,0,1,1}, 3, 3 },	


enum sides { LEFT, RIGHT, DOWN };

#define FIGURE_NUM_RANGE random() % NUMBER_OF_FIGURES
#define COLOR_NUM_RANGE	 random() % (NUMBER_OF_COLORS -1) +1
#define color_gen() COLOR_NUM_RANGE

uint16_t score;

/* Type for local remains access (just convinient). */
typedef char (*Remains_p)[GEN_WINDOW_WIDE];

/* Mutexes. */
pthread_mutex_t flow_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tetris_mutex = PTHREAD_MUTEX_INITIALIZER;

/* TYPES. */
typedef struct {		/* Local variables for functions. */
  const uint8_t f_area;
  const uint8_t f_width;
  const uint8_t f_height;
  const uint16_t cur_line;
  uint16_t cur_col;
  const bool *f_dots;
  char ch;
} Local_data;	

typedef struct {
  uint16_t lines;
  uint16_t cols;
} Remains_xy;

typedef struct {
  WINDOW *winp;
  uint16_t ht;	/* Height. */
  uint16_t wt;	/* Width.  */
} Tetris_window;

typedef struct {
  uint8_t area;
  uint8_t width;
  uint8_t height;
  bool dots[MAX_AREA];
  uint8_t local_figure_number;
  uint8_t max_local_figure_number;
} Tetris_figure;

typedef struct {
  Tetris_window gen_win; 	/* Windows. 			        */
  Tetris_window info_win;
  size_t timeout;		    /* (Speed).			            */
  uint16_t cur_line;		/* Current line inside loop.	*/
  uint16_t column;		    /* Current colum inside loop.	*/
  size_t *speed;
  uint8_t cur_speed;		/* Speed index. 		        */
  Tetris_figure *figure_p;	/* Pointer to figures.		    */
  uint8_t cur_figure;		/* Figure index. 		        */
  void *remains_p;		    /* Pointer to remains.			*/
  Remains_xy *rem_sizes;	/* Pointer to size of remains array. 	*/
  bool tetris_exit;
  uint8_t figure_color;		/* Figure color number.			*/
} Tetris_data;

void write_screen(Tetris_data*);
void write_info(Tetris_data*);
void _nc_free_and_exit(int);
void show_remains(Tetris_data*);

#endif
