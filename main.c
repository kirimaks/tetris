#include"tetris.h"

void fill_array(char (*remains)[gen_window_wide], Remains *rem_sizes, char ch)
{ /* Clean array. Copy character ch to all strings of the array. */
  uint16_t local_line = 0, local_col = rem_sizes-> cols;

  for(local_line = 0; local_line < rem_sizes-> lines; local_line++) {
      memset(remains[local_line], ch, local_col);
      remains[local_line][local_col-1] = '\0';
  }
}

int8_t next_figure_buff = -1;

uint8_t figure_num_gen(void)
{
  int8_t tmp = (next_figure_buff > 0) ? next_figure_buff : RANDOM_GEN;

  while((next_figure_buff = RANDOM_GEN) == tmp);

  return tmp;
}

uint8_t color_gen(void)
{ /* Generate color index (number). Returns number between 1 and 7. */
  uint8_t tmp = random() % 8;	/* TODO: define number of colors. */

  while(!tmp) {	/* If it was zero, generate again. */
      tmp = random() % 8;
  }
  return tmp;
}

void copy_to_remains(Tetris_data *data)
{ /* Copy current figure to remains array. */
  char (*remains)[gen_window_wide] = data-> remains_p;
  Local_data lv = {
      .f_index = 	data-> cur_figure,
      .f_area = 	data-> figure_p[lv.f_index].area,
      .f_width = 	data-> figure_p[lv.f_index].width,
      .cur_line = 	data-> cur_line -1,
      .cur_col = 	data-> column -1,
      .f_dots = 	data-> figure_p[lv.f_index].dots,
      .ch = SYMBOL,
  };
  register uint8_t area_index = 0, line_index = 0, col_index = 0;

  for(; area_index < lv.f_area; area_index++) {
      if(lv.f_dots[area_index]) {
          memcpy(&remains[lv.cur_line + line_index][lv.cur_col + col_index], &lv.ch, 1);
      }
      col_index++;
      if(col_index == lv.f_width) {
          line_index++;
	  col_index = 0;
      }
  }
}

bool check_side(Tetris_data *data, uint8_t side)
{ /* Check side around the figure (LEFT,RIGHT,DOWN). */
  char (*remains)[gen_window_wide] = data-> remains_p;
  Local_data lv = {
      .f_index = data-> cur_figure,
      .f_area = data-> figure_p[lv.f_index].area,
      .f_width = data-> figure_p[lv.f_index].width,
      .f_height = data-> figure_p[lv.f_index].height,
      .f_dots = data-> figure_p[lv.f_index].dots,
      .cur_line = data-> cur_line,
  };

  /* Set cur_col and check edges of the screen. */
  if(LEFT == side) {
      lv.cur_col = data-> column -2;
      if(1 == data-> column) {
          return TRUE;
      } 
  } else if(RIGHT == side) {
      lv.cur_col = data-> column;
      if((lv.cur_col + lv.f_width) >= data-> gen_win.wt -1) {
          return TRUE;
      }
  } else if(DOWN == side) {
      lv.cur_col = data-> column -1;
      if(lv.cur_line + lv.f_height >= data-> gen_win.ht) {
          return TRUE;
      }
  }

  register uint8_t area_index = 0, line_index = 0, col_index = 0;
  
  for(; area_index < lv.f_area; area_index++) {
      if(lv.f_dots[area_index] && remains[lv.cur_line + line_index][lv.cur_col + col_index] == SYMBOL) {
          return TRUE;
      }
      col_index++;
      if(col_index == lv.f_width) {
          col_index = 0;
	  line_index++;
      }
  }

  return FALSE;
}

void increase_line(Tetris_data *data)
{ /* Increase the figure position. */
  if(pthread_mutex_trylock(&flow_mutex)) { 
      goto exit; 	/* Try to take the mutex, and if it's taken already, do nothing. */
  }
  data-> cur_line++;
  pthread_mutex_unlock(&flow_mutex);
  exit:
  return;
}

void show_burn(WINDOW *winp, const uint16_t line, const uint16_t cols)
{ /* When a line is full, show burn line with different color. */
  register uint16_t i;
  char ch = BURN_SYMBOL;
  uint8_t color = color_gen();

  wattron(winp, A_BOLD | COLOR_PAIR(color));

  for(i = 1; i <= cols; i++) {
      mvwaddch(winp, line, i, ch);
      wrefresh(winp);
      usleep(BURN_TIME);
  }

  wattroff(winp, A_BOLD | COLOR_PAIR(color));
}

void fall_remains(Tetris_data *data, uint16_t line_index, const uint16_t cols)
{ /* When a line is burned, copy all the screen to one line down. */
  char (*remains)[gen_window_wide] = data-> remains_p;

  for(; line_index > 0; line_index--) {
      memcpy(remains[line_index], remains[line_index-1], cols);
  }
}

void fill_screen(Tetris_data *data, const char *str, const char ch)
{ /* Fill screen to character ch and show string str */
  WINDOW *winp = data-> gen_win.winp;
  const uint16_t cols = gen_window_wide -2;
  register uint16_t line_index = gen_window_height -2;
  register uint16_t col_index = 1;
  uint8_t color = color_gen();

  wattron(winp, A_BOLD);
  mvwprintw(winp, 3, cols/2 - (strlen(str)/2), str);

  wattron(winp, COLOR_PAIR(color));
  while(line_index > 0) {
      mvwaddch(winp, line_index, col_index, ch);
      wrefresh(winp);
      usleep(BURN_TIME/4);

      if(col_index == cols) {
          col_index = 1;
	  line_index--;
      } else {
          col_index++;
      }
  }
  wattroff(winp, A_BOLD|COLOR_PAIR(color));
}

void level_up(Tetris_data *data)
{ /* Call functions needed for next level. And increase the speed. */
  Remains *rem_sizes = data-> rem_sizes;
  char (*remains)[gen_window_wide] = data-> remains_p;

  fill_array(remains, rem_sizes, SYMBOL_TO_FILL);
  fill_screen(data, "Level up!!!", BURN_SYMBOL);

  werase(data-> gen_win.winp);
  box(data-> gen_win.winp, 0, 0);	/* Show the box. */

  data-> timeout = data-> speed[++data-> cur_speed];
}

void check_full_lines(Tetris_data *data)
{ /* Check full lines on to the screen and burn it. Also do checks for next level. */
  char (*remains)[gen_window_wide] = data-> remains_p;
  const uint16_t cols = data-> rem_sizes-> cols -1;
  register uint16_t line_index = data-> rem_sizes-> lines -1;

  while(line_index) {
      if(!memchr(remains[line_index], SYMBOL_TO_FILL, cols)) {	/* If there NO any SYMBOL_TO_FILL (full line of remain symbols) */
	  show_burn(data-> gen_win.winp, line_index, cols);
	  fall_remains(data, line_index, cols);
	  werase(data-> gen_win.winp);
          box(data-> gen_win.winp, 0, 0);	/* Show the box. */
	  show_remains(data);

          score += ONE_BURN_SCOPE;
          write_info(data);

	  if(!(score % LEVEL_SCORE)) {
	      level_up(data);
	      return;
	  }
	  continue;
      }
      line_index--;
  }
}

void next_step(Tetris_data *data)
{ /* Generate next step of iteration (new figure from the top). */
  if(data-> cur_line <= 2) {	/* Check for GAME OVER. */
      data-> tetris_exit = TRUE;
      data-> timer_exit = TRUE;
      fill_screen(data, "GAME_OVER", SYMBOL);
      return;
  }

  if(pthread_mutex_trylock(&flow_mutex)) {
      goto exit;
  }

  copy_to_remains(data);		/* Copy current figure to remains array. 	*/

  pthread_mutex_unlock(&tetris_mutex);
  pthread_mutex_lock(&tetris_mutex);	/* Unlock if it was locked before, and lock it here. */
  check_full_lines(data);		/* Maybe level_up().		*/
  pthread_mutex_unlock(&tetris_mutex);

  data-> cur_line = 1;			/* Start from new line.				*/
  data-> cur_figure = figure_num_gen();	/* Generate new figure number. 			*/
  data-> figure_color = color_gen();	/* Generate color number for new figure.	*/

  write_info(data);

  /* Current column plus widht of the new figure (preven to to edge). */
  if(data-> column + data-> figure_p[data-> cur_figure].width >= data-> gen_win.wt) {
      data-> column -= data-> figure_p[data-> cur_figure].width;
  }

  #ifdef CENTER_POSITION
  if(data-> cur_speed < 12) {		/* Align next figure position if speed is low.	*/
      data-> column = data-> gen_win.wt / 2;
  }
  #endif
  #ifdef ONE_STEP	/* Debug. Just do only one iteration and exit. */
  data-> timer_exit = TRUE;
  data-> tetris_exit = TRUE;
  #endif

  pthread_mutex_unlock(&flow_mutex);
  exit:
  return;
}

void *timer_flow(void *tmp_ptr)
{ /* Thread with the timer. */
  Tetris_data *data = (Tetris_data*)tmp_ptr;

  for(;;) {
      if(data-> timer_exit) { 
          return NULL;				/* Close the thread. 	*/
      } 

      write_screen(data);			/* Update screen.	*/
      increase_line(data);

      if(check_side(data,DOWN)) {
          next_step(data);
      }
      usleep(data-> timeout);
  }
}

uint16_t line_buff;

void rotate(Tetris_data *data)	/* FIXME: has a bug (if hold space). */
{ /* Rotate the figure. */
  const uint8_t f_index = data-> cur_figure;
  const uint8_t local_figure_num = data-> figure_p[f_index].local_figure_number;
  const uint8_t local_figure_max = data-> figure_p[f_index].max_local_figure_number;

  const uint8_t last_height = data-> figure_p[data-> cur_figure].height;

  if(local_figure_num < local_figure_max) {
      ++data-> cur_figure;
  } else {
      data-> cur_figure -= local_figure_max;
  }

  /* Make choise about the line from witch new figure will be shown. */
  if(data-> cur_line > 3 && line_buff != data-> cur_line) {
      if(data-> figure_p[data-> cur_figure].height > last_height) {
          data-> cur_line -= data-> figure_p[data-> cur_figure].height - last_height;
      }
      line_buff = data-> cur_line;
  }
}

void *tetris_flow(void *tmp_ptr)
{ /* Thread with button handler. */
  Tetris_data *data = tmp_ptr;

  #if DEBUG					/* For Clean array. */
  Remains *rem_sizes = data-> rem_sizes;
  char (*remains)[gen_window_wide] = data-> remains_p;
  #endif
  bool tmp = FALSE, prev = FALSE;
  int16_t ch;

  for(;;) {
      if(data-> tetris_exit) {
          return NULL;				/* Close thread. */
      }

      pthread_mutex_lock(&tetris_mutex);	/* Will be just wait here. */
      pthread_mutex_unlock(&tetris_mutex);
      flushinp();

      switch(ch = getch()) {
          case 'q':
	      data-> timer_exit = TRUE;		/* Tell to the timer to close. 	*/
	      return NULL;			/* Close the thread. 		*/
          
	  case KEY_LEFT:
	  case 'a':
	      if(!(tmp = check_side(data,LEFT))) {
	          data-> column--;
		  if(prev) {
		      usleep(data-> timeout/2);
		  } 
                  write_screen(data);	
	      } 
	      prev = tmp;
	      break;

	  case KEY_RIGHT:
	  case 'd':
	      if(!(tmp = check_side(data,RIGHT))) { 
	          data-> column++;
		  if(prev) {
		      usleep(data-> timeout/2);
		  } 
                  write_screen(data);	
	      }
	      prev = tmp;
	      break;

	  case KEY_DOWN:
	  case 's':
              write_screen(data);
              increase_line(data);
              if(check_side(data,DOWN)) {
	          next_step(data);
	      } else {
                  write_screen(data);
	      }
	      break;

	  case KEY_SPACE:	/* Make choise about rotate figure or not. */
	      if(!check_side(data,LEFT) && !check_side(data,RIGHT) && !check_side(data,DOWN)) {
	          rotate(data);
              } else if(data-> column == 1) {
	          rotate(data);
	      } else if(data-> column + data-> figure_p[data-> cur_figure].width >= data-> gen_win.wt -1) {
	          rotate(data);
		  data-> column = data-> gen_win.wt - data-> figure_p[data-> cur_figure].width -1;
	      }
              write_screen(data);	/* Do it anyway. */
	      usleep(data-> timeout/4);
	      break;

          #if DEBUG	/* For speed control and clear screen. */
	  case '+':
	      if(data-> cur_speed < GEARS -1) {
	          data-> timeout = data-> speed[++data-> cur_speed];
                  write_info(data);
	      }
	      break;

	  case '-':
	      if(data-> cur_speed > 0) {
	          data-> timeout = data-> speed[--data-> cur_speed];
                  write_info(data);
	      }
	      break;

	  case 'c':
              fill_array(remains, rem_sizes, SYMBOL_TO_FILL);
	      break;
	  #endif
      }
  }

  return NULL;
}

WINDOW *create_win(uint16_t h, uint16_t w, uint16_t y,uint16_t x)
{ /* Create and show window. */
  WINDOW *tmp_win = newwin(h, w, y, x);
  assert(tmp_win);
  box(tmp_win, 0, 0);	/* Show box. */
  wrefresh(stdscr);
  wrefresh(tmp_win);

  return tmp_win;
}

void show_remains(Tetris_data *data)
{ /* Show remains array. */
  WINDOW *win_p = data-> gen_win.winp;
  register uint16_t line;
  char (*remains)[gen_window_wide] = data-> remains_p;

  for(line = 1; line < gen_window_height -1; line++) {
      mvwprintw(win_p, line, 1, "%s", remains[line]); 
  }
}

void show_figure(WINDOW *win_p, Tetris_figure *fig_p, const uint8_t line, const uint8_t col, const uint8_t figure_color)
{
  register uint8_t f_area, f_line, f_dot;
  wattron(win_p, A_BOLD | COLOR_PAIR(figure_color));	/* Enable color. */

  for(f_dot = f_line = f_area = 0; f_area < fig_p-> area; f_area++, f_dot++) {
      if(f_dot == fig_p-> width) {
          f_dot = 0;
	  f_line++;
      }
      if(fig_p-> dots[f_area]) {
          mvwaddch(win_p, line + f_line, col + f_dot, SYMBOL);
      }
  }
  wattroff(win_p, A_BOLD | COLOR_PAIR(figure_color));	/* Disable color. */
}

void write_screen(Tetris_data *data)
{ /* Call functions for update screen. */
  if(pthread_mutex_trylock(&flow_mutex)) {
      goto exit;	/* Try to take mutex and if it's taken already, do nothing. */
  }

  WINDOW *win_p = data-> gen_win.winp;

  werase(win_p);
  box(win_p, 0, 0);	/* Show the box. */

  show_remains(data);
  show_figure(win_p, &data-> figure_p[data-> cur_figure], data-> cur_line, data-> column, data-> figure_color);
  
  wrefresh(win_p);

  pthread_mutex_unlock(&flow_mutex);
  exit:
  return;
}

void write_info(Tetris_data *data)
{ /* Update info screen. */
  WINDOW *win_p = data-> info_win.winp;

  werase(win_p);			/* werase() is faster. */
  box(win_p, 0, 0);

  wattron(win_p, A_BOLD);

  wattron(win_p, COLOR_PAIR(3));
  mvwprintw(win_p, 1, 1, "[%07u]", score);	/* Score. */
  wattroff(win_p, COLOR_PAIR(3));

  mvwprintw(win_p, 3, 1, "Speed:%u", data-> cur_speed);
  mvwprintw(win_p, 5, 1, "H:%u/W:%u", data-> gen_win.ht, data-> gen_win.wt);

  mvwprintw(win_p, 7, 1, "Next:");
  show_figure(win_p, &data-> figure_p[next_figure_buff], 9, 4, data-> figure_color);

  mvwprintw(win_p, gen_window_height -4, 2, "<a s d>");
  mvwprintw(win_p, gen_window_height -3, 2, "   V   ");
  mvwprintw(win_p, gen_window_height -2, 2, "[space]");
  wrefresh(win_p);

  wattroff(win_p, A_BOLD);
}

void tetris_exit(void *tmp_ptr)
{
  Tetris_data *data = tmp_ptr;
  delwin(data->gen_win.winp);
  delwin(data->info_win.winp);
  endwin();
  _nc_free_and_exit(EXIT_SUCCESS);
}

int main(void)
{
  initscr();
  start_color();
  cbreak();
  noecho(); 
  keypad(stdscr, TRUE);
  curs_set(0);			/* Invisibly cursor. */

  /* Colors definition. */
  COLOR_PAIRS;

  /* The array of remains. */
  Remains rem_sizes = { gen_window_height -1, gen_window_wide - 1 };
  char remains[rem_sizes.lines][rem_sizes.cols];

  /* Clean and fill the array. */
  fill_array(remains, &rem_sizes, SYMBOL_TO_FILL);

  /* Define figures: area, width, height, dots{..} to print, ## figure_number, max_figure_of_this type (start from zero). */
  Tetris_figure figures[NUMBER_OF_FIGURES] = { FIGURES_DOTS };

  /* Data. */
  Tetris_data window_data = { 
      { NULL, gen_window_height, gen_window_wide }, 		/* General window. 	*/
      { NULL, gen_window_height, INFO_WINDOW_WIDTH },		/* Info window. 	*/
      .timeout = MIN_SPEED,					/* Timeout.			*/
      .cur_line = 1,						/* Line inside the cicle. 	*/
      .column = gen_window_wide/2,				/* Initial figure position. 	*/
      .cur_speed = 0,						/* Initial speed (gear). */
      .figure_p = figures,
      .speed = { SPEED_VALUES },
      .remains_p = remains,
      .rem_sizes = &rem_sizes,
      .timer_exit = FALSE,
      .tetris_exit = FALSE,
  };

  /* Create windows. */
  window_data.gen_win.winp = create_win(window_data.gen_win.ht, window_data.gen_win.wt, 0, 0);
  window_data.info_win.winp = create_win(window_data.info_win.ht, window_data.info_win.wt, 0, gen_window_wide);

  /* Generate the first figure. */
  window_data.cur_figure = figure_num_gen();

  /* Generate color for first figure. */
  window_data.figure_color = color_gen();

  /* Write info first time. */
  write_info(&window_data);

  /* Threads. */
  pthread_t tetris_thread;
  pthread_t timer_thread;

  if(pthread_create(&tetris_thread, NULL, tetris_flow, (void*)&window_data)) {
      return 1;	/* pthread_create() returns 0 if success. */
  }

  if(pthread_create(&timer_thread, NULL, timer_flow, (void*)&window_data)) {
      return 1;
  }

  pthread_join(tetris_thread, NULL);
  pthread_join(timer_thread, NULL);

  tetris_exit(&window_data);

  return 0;
}