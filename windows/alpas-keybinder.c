#include <stdio.h>
#include <conio.h>  

int mygetch() {
    if (_kbhit()) {
        return _getch(); 
    } else {
        return -1; 
    }
}
