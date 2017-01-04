#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define PLANE_X 100
#define PLANE_Y 30
#define STARTING_LEN 4
#define GROW_LEN 4
#define STARTING_X 50
#define STARTING_Y 15
#define MAX_WORM_LEN 200

typedef struct __coord {
    int x;
    int y;
} coord;

char plane[PLANE_Y][PLANE_X+1];     // Add extra byte for string termination NULL
coord worm[MAX_WORM_LEN];           // Worm coordinates
int len = STARTING_LEN;             // Length which we are approching to
int curlen = 1;                     // Current worm length
int dead = 0;                       // Are we dead?
int score = 0;                      // Percentage of zeel

void initPlane()
{
    // Game field initialization
    int i,j;

    // Fill plane with spaces
    for (i=0; i<PLANE_Y; i++) for (j=0; j<PLANE_X; j++) plane[i][j] = ' ';

    // Set plane borders (obstacles)
    for (i=0; i<PLANE_Y; i++) {
        for (j=0; j<PLANE_X; j++) {
            if (i==0 || i==PLANE_Y-1) plane[i][j] = '-';
            if (j==0 || j==PLANE_X-1) plane[i][j] = '|';
            if ((i==0 && j==0) || (i==PLANE_Y-1 && j==PLANE_X-1)) plane[i][j] = '/';
            if ((i==0 && j==PLANE_X-1) || (i==PLANE_Y-1 && j==0)) plane[i][j] = '\\';
        }
        plane[i][PLANE_X] = 0; // Add some 0's to terminate strings
    }
}

void drawPlane(WINDOW *field_w, WINDOW *score_w)
{
    // Field draw routine
    int i;
    char c;

    // Clear windows first
    wclear(field_w);
    wclear(score_w);

    // Draw plane itself
    for (i=0; i<PLANE_Y; i++) {
        mvwprintw(field_w, i, 0, "%s", plane[i]);
    }

    // Draw worm
    for (i=0; i<curlen; i++) {
        if (dead==1) c = 'x'; else if (i==0) c = 'z'; else if (i==curlen-1) c = 'l'; else c = 'e';
        mvwprintw(field_w, worm[i].y, worm[i].x, "%c", c);
    }

    // Draw score
    if (!dead) mvwprintw(score_w, 0, 0, "Score: %d%% zeel", score*100);
        else mvwprintw(score_w, 0, 0, "Game over. Your score: %d%% zeel. Still not zeel enough.", score*100);

    // Do actual screen output
    wrefresh(field_w);
    wrefresh(score_w);
}

void placeMeal()
{
    // Place '*' on field
    int x,y;
    do {
        x = (rand() % (PLANE_X-2)) + 1;
        y = (rand() % (PLANE_Y-2)) + 1;
    } while (plane[y][x]!=' ');
    plane[y][x] = '*';
}

void placeObstacle()
{
    // Make zeel's life harder
    int x,y,o,i;
    char *obstacles[] = {"DDoS", "KVM_Bug", "I/O_Load"};
    o = rand() % 3;

    do {
        x = (rand() % (PLANE_X-2-strlen(obstacles[o]))) + 1;
        y = (rand() % PLANE_Y-2) + 1;
    } while (plane[y][x]!=' ');

    for (i=0;i<strlen(obstacles[o]);i++) {
        if (plane[y][x+i] == ' ')
        {
            plane[y][x+i] = obstacles[o][i];
        }
    }
}

int isHarakiri(int x, int y)
{
    // Check if zeel is eating himself
    int i;
    for (i=0;i<curlen;i++)
        if (worm[i].x==x && worm[i].y==y) return 1;
    return 0;
}

int main(int argc, char *argv[])
{
    WINDOW *field_w, *score_w;      // ncurses stuff
    int d_x, d_y, key, i;           // Directions, pressed key, and counter
    int x = STARTING_X;             // Position of zeel's head
    int y = STARTING_Y;             //
    int quit = 0;                   // If we want to quit...
    time_t t;                       // For random number initialization

    if(argc>1) {
      printf("ZeeLax Simulator, version 0.1b\n");
      return 0;
    }

    // Make some good random numbers
    srand((unsigned) time(&t));

    // ncurses stuff
    initscr();                  // Init ncurses
    cbreak();                   // Do not cache any input
    noecho();                   // Do not echo characters
    curs_set(FALSE);            // Disable cursor
    nodelay(stdscr, TRUE);      // Do not wait on getch() calls
    scrollok(stdscr, FALSE);    // Thou shall not scroll!
    keypad(stdscr, TRUE);       // Enable arrow keys to send single int

    // Init game field
    initPlane();

    // Create game field ncurses windows
    field_w = newwin(PLANE_Y, PLANE_X, 0, 0);
    score_w = newwin(1, PLANE_X, PLANE_Y, 2);

    // Set initial worm direction
    d_x = 1;
    d_y = 0;

    // Place some initial meal
    placeMeal();
    placeMeal();
    placeMeal();

    // Main loop
    while (!quit) {
        if (!(rand()%(20+curlen*2))) placeObstacle();   // Chtob zhizn' malinoy ne kazalas'
        drawPlane(field_w, score_w);                    // Redraw field

        if (dead)
        {
            // AHHAHAHAHAHAHAHHAHAHAHAHAH e.g. meow
            sleep(5);
            break;
        }

        usleep(200000-curlen*2000); // Speed up a little bit!
        key = getch();
        if (key != ERR) {
            // Key is pressed!
            switch(key) {
                case KEY_UP:
                    if (d_y != 0) break;
                    d_y = -1;
                    d_x = 0;
                    break;
                case KEY_DOWN:
                    if (d_y != 0) break;
                    d_y = 1;
                    d_x = 0;
                    break;
                case KEY_LEFT:
                    if (d_x != 0) break;
                    d_x = -1;
                    d_y = 0;
                    break;
                case KEY_RIGHT:
                    if (d_x != 0) break;
                    d_x = 1;
                    d_y = 0;
                    break;
                case 'x':
                    placeObstacle();
                    break;
                case 'c':
                    placeMeal();
                    break;
                case 'z':
                    len += GROW_LEN;
                    break;
                case 'q':
                case 'Q':
                    quit = 1;
                default:
                    continue;
            }
        }

        // Change worm direction according to pressed key
        x += d_x;
        y += d_y;

        // Check if we are ok to move
        if ((plane[y][x] == ' ' || plane[y][x] == '*') && !isHarakiri(x,y))
        {
            // Ok to move here
            if (plane[y][x] == '*') {
                // Some tamak consumed
                len += GROW_LEN;
                score++;
                plane[y][x] = ' ';
                placeMeal();
            }

            // Move worm itself
            for (i=curlen;i>0;i--)
            {
                worm[i].x = worm[i-1].x;
                worm[i].y = worm[i-1].y;
            }
            worm[0].x = x;
            worm[0].y = y;

            // Grow if needed
            if (curlen<len && curlen < MAX_WORM_LEN) curlen++;

        } else {
            // Some shit is here!
            dead = 1;
        }
    }

    // Clear ncurses stuff
    delwin(field_w);
    delwin(score_w);
    endwin();

    return 0;
}
