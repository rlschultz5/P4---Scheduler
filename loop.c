#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf(0,"Invalid parameters. Pass in number of sleeps only");
        return -1;
    }
    int sleepT = atoi(argv[1]);
    if(sleepT < 1) {
        printf(0,"Number of sleeps must be greater than 0.");
        return -1;
    }
    sleep(sleepT);

    int i = 0, j = 0;
    while (i < 800000000) {
        j += i * j + 1;
        i++;
    }
    exit();
}
