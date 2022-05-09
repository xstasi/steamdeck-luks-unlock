#include <ncurses.h>
#include <string.h>

// deck size = 50x160

#define ACCEPT "ACCEPT"

void draw_accept(int y, int col){

  int button_size = strlen(ACCEPT);
  int pos_x, pos_y;
  
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_BLUE);
  attron(COLOR_PAIR(1));
  button_size = strlen(ACCEPT) ;

  // Start after bottom row and try to center the button
  pos_y = y;
  pos_x = (col - (button_size+4)) / 2;

  // Draw top line
  mvaddch(pos_y, pos_x, ACS_ULCORNER);
  hline(ACS_HLINE,button_size+2);
  mvaddch(pos_y, pos_x + button_size + 3, ACS_URCORNER);
  pos_y++;

  // Draw | ACCEPT |
  mvaddch(pos_y, pos_x, ACS_VLINE);
  mvprintw(pos_y, pos_x+1, " %s ", ACCEPT);
  mvaddch(pos_y, pos_x+button_size+3, ACS_VLINE);
  pos_y++;

  // Draw bottom line
  mvaddch(pos_y, pos_x, ACS_LLCORNER);
  hline(ACS_HLINE,button_size+2);
  mvaddch(pos_y, pos_x+button_size+3, ACS_LRCORNER);
  attroff(COLOR_PAIR(1));

  refresh();
}

int main(){

  char letters[36] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  int out[36];

  int ch;
  int row, col,
      corner_x, corner_y,
      base_y,
      pos_x, pos_y;

  int cur_y;
  int cur_x;

  bool take_input = TRUE;

  ch = 0;
  row = col = corner_x = corner_y = pos_x = pos_y = cur_x = cur_y = 0;

  bzero(out,36);

  initscr();
  raw();

  getmaxyx(stdscr, row, col);
  keypad(stdscr, TRUE);

  // Identify where to start drawing the square, each box being 3*5 characters
  corner_x = (col - 5*6) / 2;
  base_y = (row - 3*6) / 2;
  corner_y = base_y;

  // Draw the letter box
  for(int j=0;j<6;j++){
    for(int i=0;i<6;i++){
      // Draw top of the boxes
      mvaddch(corner_y, corner_x + i*5, ACS_ULCORNER);
      hline(ACS_HLINE,3);
      mvaddch(corner_y, corner_x + i*5 + 4, ACS_URCORNER);

      // Draw left border --> "|"
      mvaddch(corner_y+1, corner_x + i*5, ACS_VLINE);

      // Draw each letter with the separator --> " X |"
      mvprintw(corner_y+1, corner_x + i*5 + 1, " %c ",letters[j*6+i]);
      mvaddch(corner_y+1, corner_x + i*5+4, ACS_VLINE);

      // Draw bottom of boxes
      mvaddch(corner_y+2, corner_x + i*5, ACS_LLCORNER);
      hline(ACS_HLINE,3);
      mvaddch(corner_y+2, corner_x + i*5 + 4, ACS_LRCORNER);

    }
    corner_y = corner_y + 3;
  }

  draw_accept(corner_y, col);

  // Move cursor on the first letter
  cur_y = (row - 3*6) / 2 + 1;
  cur_x = corner_x+2;
  move(cur_y, cur_x);
  noecho();

  // Start taking input
  while(take_input) {
    ch = getch();
    switch(ch){
      case KEY_UP:
        if(cur_y > base_y+1){
          cur_y -= 3;
          move(cur_y, cur_x);
        }
        break;
      case KEY_DOWN:
        if(cur_y < base_y+16){
          cur_y += 3;
          move(cur_y, cur_x);
        }
        if(cur_y == base_y+16){
          cur_y += 3;
          move(cur_y,col/2);
        }
        break;
      case KEY_LEFT:
        // Don't allow horizontal movement on the accept button
        if(cur_y > base_y+16) break;
        if(cur_x > corner_x+2){
          cur_x -= 5;
          move(cur_y, cur_x);
        }
        break;
      case KEY_RIGHT:
        if(cur_y > base_y+16) break;
        if(cur_x < corner_x+24){
          cur_x += 5;
          move(cur_y, cur_x);
        }
        break;
      case 10:
        take_input = FALSE;
    }
    refresh();
  }
  endwin();

  return 0;
}
