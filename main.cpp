#include <iostream>
#include <ncurses.h>
#include <deque>
#include <ratio>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

// -- Game Settings -- 
float delay = 100.0f; // defacto snake speed

// -- Global vars -- 
int boardY, boardX;
int score;
bool RUN = true;

void UpdateScore()
{
    mvprintw(0, 1, "[ Score: %d ]", score);
}

struct Point
{
    int y,x;
    Point(int y, int x) : y(y), x(x) {}
};

std::deque<Point> grid;

std::vector<Point> PointSetDifference(std::deque<Point> a, std::deque<Point> b)
{
    std::vector<Point> ret;
    for (Point p : a) {
        bool found = false;
        for (Point p2 : b) {
            if (p.x == p2.x && p.y == p2.y) {
                found = true;
                break;
            }
        }
        if(found) continue;
        ret.push_back(p);
    }
    return ret;
}

struct Apple
{
    int y,x;
    Apple() : y(-1), x(-1) {}

    void DrawNew()
    {
        mvaddch(y, x, ' ');
        srand(time(0));
        y = (rand()%(boardY-4)) + 2;
        x = (rand()%(boardX-4)) + 2;
        mvaddch(y, x, '*');
    }

    void DrawNew(std::deque<Point> snake_body)
    {
        mvaddch(y, x, ' '); // clear prev
        // Diff all available points against snake_body points to get free positions to spawn the apples.
        std::vector<Point> available = PointSetDifference(grid, snake_body);

        Point randpoint = available[rand()%(available.size())];
        y = randpoint.y;
        x = randpoint.x;
        mvaddch(y, x, '*');
    }
};

Apple apple;

struct Snake
{
    int y,x;
    std::deque<Point> body;

    void Draw()
    {
        Point &head = body[body.size() - 1];
        bool remove_back = true;

        if (head.x == apple.x && head.y == apple.y) {
            remove_back = false;
            apple.DrawNew(body);
            score++;
            // Check if won
            if (score == ((boardX-2) * (boardY-2)))
                RUN = false;
            UpdateScore();
        }

        // Check if collides with self
        for (int i = 0; i < body.size() - 1; i++) {   
                Point &p = body[i];
                if (body.size() > 1 && head.x == p.x && head.y == p.y) {
                    RUN = false;
            }
        }
        // remove the tail element
        if (body.size() > 1 && remove_back) {
            mvaddch(body[0].y, body[0].x, ' ');
            body.pop_front();
        }
        // add new head
        mvaddch(body[body.size() - 1].y, body[body.size() - 1].x, '#');
        refresh();
    }
};

int main (int argc, char *argv[])
{
    Snake snake;

    initscr();
    curs_set(0);
    noecho();
    getmaxyx(stdscr, boardY, boardX);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    box(stdscr, 0,0);
    
    // Generate grid
    for(int y = 1; y < boardY-1; y++) {
        for(int x = 1; x < boardX-1; x++) {
            grid.push_back(Point(y,x));
        }                    
    }

    apple.DrawNew();
    UpdateScore();

    int y,x, ymod,xmod;
    y = boardY/2;
    x = boardX/2;
    ymod = -1;
    xmod = 0;
    // north, east, south, west
    char direction = 's';
    int c;
    while(RUN) {
        c=getch();
        switch(c) {
            case KEY_UP:
                if (direction != 's') {
                    ymod = -1;
                    xmod = 0;
                    direction = 'n';
                }         
                break;
            case KEY_DOWN:
                if (direction != 'n') {
                    ymod = 1;
                    xmod = 0;
                    direction = 's';
                }         
                break;
            case KEY_LEFT:
                if (direction != 'e') {
                    ymod = 0;
                    xmod = -1;
                    direction = 'w';
                }         
                break;
            case KEY_RIGHT:
                if (direction != 'w') {
                    ymod = 0;
                    xmod = 1;
                    direction = 'e';
                }         
                break;
        }
        
        y += ymod;
        x += xmod;
        // check if collides with wall
        if (x >= boardX-1 || x < 1 || y >= boardY-1 || y < 1)
            RUN = false;

        snake.body.push_back(Point(y,x));
        snake.Draw();
        int msdelay = direction == 'n' || direction == 's' ? int(delay) : int(delay/2);
        std::this_thread::sleep_for(std::chrono::milliseconds(msdelay));
    }

    nodelay(stdscr, FALSE);

    const char* message;
    if (score == (boardX-2) * (boardY-2))
        message = "Congratulations!";
    else 
        message = "Game Over!";

    mvprintw(boardY/2, boardX/2 - sizeof(message), message);
    getch();

    echo();
    curs_set(2);
    endwin();
}
