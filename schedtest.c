#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"


// schedtest spawns two children processes, each running the loop application.
// One child A is given initial timeslice length of sliceA and runs loop
// sleepA; the other B is given initial timeslice length of sliceB and runs
// loop sleepB.
//int main(int argc, char *argv[]) {
//  if (argc != 6){
//    printf(0,"Invalid input\n");
//    exit();
//  }
//  int sliceA = atoi(argv[1]);
//  int sleepA = atoi(argv[2]);
//  int sliceB = atoi(argv[3]);
//  int sleepB = atoi(argv[4]);
//  int sleepParent = atoi(argv[5]);
//  printf(0,"input: %d, %d, %d, %d, %d\n",sliceA,sleepA,sliceB,sleepB,sleepParent);
//  struct pstat *pstat_data = malloc(sizeof (struct pstat));
////  struct pstat *childB = malloc(sizeof (struct pstat));
//  int fork_status = fork2(sliceA);
//  if(fork_status != -1) {
//      if (fork_status == 0) {
//          char *arg[] = {"loop", argv[2], 0};
////          printf(0, "Before A loop\n");
//          printf(0, "ChildA PID: %d\n", getpid());
//          exec("loop", arg);
//          printf(0, "After B loop\n");
//
//      } else {
//          int fork_status2 = fork2(sliceB);
//          if(fork_status2 != -1) {
//              if (fork_status2 == 0) {
//                  char *arg2[] = {"loop", argv[4], 0};
////                  printf(0, "Before B loop\n");
//                  printf(0, "ChildB PID: %d\n", getpid());
//                  exec("loop", arg2);
//                  printf(0, "After B loop\n");
//
//              } else {
//                  sleep(sleepParent);
//                  getpinfo(pstat_data);
//                  int i = 0;
//                  while(pstat_data->pid[i] != fork_status){
//                      i++;
//                  }
//                  int childAcompTicks = pstat_data->compticks[i];
//                  int j = 0;
//                  while(pstat_data->pid[j] != fork_status2){
//                      j++;
//                  }
//                  int childBcompTicks = pstat_data->compticks[j];
//
//                  printf(1, "%d %d\n", childAcompTicks, childBcompTicks);
//                  wait();
//                  wait();
//                  printf(0, "After waits\n");
//              }
//          }
//      }
//  }
//  exit();
//}
int main(int argc, char *argv[]) {
    if (argc != 6){
        printf(0,"Invalid input\n");
        exit();
    }
    int sliceA = atoi(argv[1]);
//    int sleepA = atoi(argv[2]);
    int sliceB = atoi(argv[3]);
//    int sleepB = atoi(argv[4]);
    int sleepParent = atoi(argv[5]);
//    printf(0,"input: %d, %d, %d, %d, %d\n",sliceA,sleepA,sliceB,sleepB,sleepParent);
    struct pstat *pstat_data = malloc(sizeof (struct pstat));
//  struct pstat *childB = malloc(sizeof (struct pstat));
    int fork_status = fork2(sliceA);
    if(fork_status != -1) {
        if (fork_status == 0) {
            char *arg[] = {"loop", argv[2], 0};
//          printf(0, "Before A loop\n");
//            printf(0, "ChildA PID: %d\n", getpid());
            exec("loop", arg);
//            printf(0, "After B loop\n");

        } else {
            int fork_status2 = fork2(sliceB);
            if(fork_status2 != -1) {
                if (fork_status2 == 0) {
                    char *arg2[] = {"loop", argv[4], 0};
//                  printf(0, "Before B loop\n");
//                    printf(0, "ChildB PID: %d\n", getpid());
                    exec("loop", arg2);
//                    printf(0, "After B loop\n");

                } else {
                    sleep(sleepParent);
                    getpinfo(pstat_data);
                    int i = 0;
                    while(pstat_data->pid[i] != fork_status){
                        i++;
                    }
                    int childAcompTicks = pstat_data->compticks[i];
                    int j = 0;
                    while(pstat_data->pid[j] != fork_status2){
                        j++;
                    }
                    int childBcompTicks = pstat_data->compticks[j];

                    printf(1, "%d %d\n", childAcompTicks, childBcompTicks);
                    wait();
                    wait();
//                    printf(0, "After waits\n");
                }
            }
        }
    }
    exit();
}