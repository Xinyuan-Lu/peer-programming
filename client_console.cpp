#include "client.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <chrono>
#include <thread>
#include <ncurses.h>
using namespace std;

int main(int argc, char** argv){

    client* myClient = new client("34.221.54.244", "31248");
    srand(time(NULL));
    myClient->myID = rand();
    std::thread th = std::thread([=]{myClient->run();});


    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, true);
    //raw();
    //timeout(1);
    //int height, width, start_y, start_x;
    //height = 20;
    //width = 100;
    //start_y = start_x = 5;
    int x, y;
    WINDOW* win = stdscr;

    int width = getmaxx(win);
    wrefresh(win);
    keypad(win, true);
    //std::string context = "Hello world";
    x = myClient->context.size();
    
    int c;
    char tab = '\t';
    y = 1;
    wmove(win, y, x);
    while(1){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        getyx(win, y, x);
        clear();
        mvwprintw(win, 0, 0, "x: %d, y: %d", x, y);
        std::string cont = "";
        myClient->outqLock.lock();
        cont = myClient->context;
        myClient->outqLock.unlock();
        mvwprintw(win, 1, 0, "%s", cont.c_str());
        wmove(win, y, x);
        refresh();
        wrefresh(win);
        //wmove(win, y, x);
        c = getch();
        
        if (c == KEY_LEFT){
            if (x > 0){
                x = x - 1;
            }else{
                if (y > 1){
                    x = width-2;
                    y = y-1;
                }else{
                    x = 1;
                }               
            }
            
            wmove(win, y, x);
            continue;   
        }else if (c == KEY_RIGHT){
            if (x + (y-1)*(width) >= myClient->context.size()){
                continue;
            }
            if (x < width-1){
                x = x + 1;           
            }else{
                x = 0;
                y = y + 1;
            }
            
            wmove(win, y, x);
            continue;
        }else if (c == 127){
            myClient->outqLock.lock();
            size_t cur_pos = x + (y-1)*(width);
            if(cur_pos <= 0){
                continue;
            }
            myClient->Delete(cur_pos-1, 1);
            
            //context = context.substr(0,cur_pos-1) + context.substr(cur_pos, context.size()-cur_pos);
            if (x > 0){
                x = x - 1;
            }else{
                if (y > 1){
                    x = width-2;
                    y = y-1;
                }else{
                    x = 1;
                }               
            }

            wrefresh(win);
            refresh();
            wmove(win, y, x);
            //wdelch(win);
            myClient->outqLock.unlock();
            continue;
        }else if (c == KEY_UP){
            continue;
        }else if (c == KEY_DOWN){
            continue;
        }else if (c == KEY_ENTER){
            continue;
        }else if(c == (int)tab){
            continue;
        }else if(isprint(c)){
            myClient->outqLock.lock();
            size_t cur_pos = x + (y-1)*(width);
            myClient->Insert(cur_pos, std::string(1, (char )c));
            //context = context.substr(0, cur_pos) + std::string(1, (char )c) + context.substr(cur_pos, context.size()-cur_pos);
            if (x < width-1){
                x = x + 1;           
            }else{
                x = 0;
                y = y + 1;
            }
            wmove(win, y, x);
            myClient->outqLock.unlock();
            
        }else{
            continue;
        }
        //clear();
        wrefresh(win);
        refresh();
        
    }
    endwin();
    th.join();
}