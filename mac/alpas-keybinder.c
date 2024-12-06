#include<stdio.h>
#include<string.h>
#include<termios.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>  
#include <sys/select.h>  

struct termios Otty;


void restore_terminal_settings(int signo) {
    tcsetattr(0, TCSANOW, &Otty); 
    _exit(0);
}

int mygetch() {
    char ch;
    int error;
    struct termios Ntty;

    if (tcgetattr(0, &Otty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    Ntty = Otty; 

    signal(SIGINT, restore_terminal_settings);

    Ntty.c_iflag  =  0;      
    Ntty.c_oflag  =  OPOST;       
    Ntty.c_lflag &= ~(ICANON | ECHO);
    Ntty.c_cc[VMIN]  = 1;    
    Ntty.c_cc[VTIME] = 0;  

    // Apply the modified terminal settings
    if ((error = tcsetattr(0, TCSAFLUSH, &Ntty)) != 0) {
        perror("tcsetattr");
        return -1;
    }

  
    fd_set set;
    struct timeval timeout;

    FD_ZERO(&set);         
    FD_SET(0, &set);   

    timeout.tv_sec = 0; 
    timeout.tv_usec = 100000;  

    int rv = select(1, &set, NULL, NULL, &timeout);

    if (rv == 1) {
        error = read(0, &ch, 1);
    } else {
        ch = -1;
    }

    if (tcsetattr(0, TCSANOW, &Otty) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return (error == 1 ? (int) ch : -1);
}
