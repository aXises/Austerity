#include "player.h"
#define PLAYER_NAME "shenzi"

int main(int argc, char **argv) {
    check_args(argc, argv, PLAYER_NAME);
    // fprintf(stderr, "argc: %i, argv0: %s, argv1: %s\n", argc, argv[0], argv[1]);
    char c[80];
    char *e = c;
    size_t s = 80;
    if (getline(&e,&s,stdin) == -1) {
        fprintf(stderr, "player err\n");
    }
    fprintf(stderr, "%s: recieved from hub: %s\n", argv[1], c);
    fprintf(stderr, "sending to hub...\n");
    printf("hello this is alex\n");
    fflush(stdout);
}