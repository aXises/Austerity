#include "player.h"

int main(int argc, char **argv) {
    printf("argc: %i, argv0: %s, argv1: %s\n", argc, argv[0], argv[1]);
    fprintf(stderr, "argc: %i, argv0: %s, argv1: %s\n", argc, argv[0], argv[1]);
    check_args(argc, argv);
}