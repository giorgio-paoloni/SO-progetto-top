#include "stdin_parser.h"

void keypressed(){
    fd_set fds;
    int maxfd;
    int sd = 0;
    // sd is a UDP socket

    maxfd = (sd > STDIN)?sd:STDIN;



        FD_ZERO(&fds);
        FD_SET(sd, &fds);
        FD_SET(STDIN, &fds);

        select(maxfd+1, &fds, NULL, NULL, NULL);

        if(FD_ISSET(STDIN, &fds)){
              printf("\nUser input - stdin");
        }
        if(FD_ISSET(sd, &fds)){
              // socket code
        }

}
