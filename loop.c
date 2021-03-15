#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(void)
{
//  printf(0,"%d\n", setslice(1,7));
  printf(0,"%d\n", getslice(1));
//  printf(0,"%d\n", getslice(1));
//  printf(0,"%d\n", getslice(2));
  int pid = fork2(3);

  if (pid != -1) {
        if (pid == 0) {
            // child
            printf(0,"child: execing \n");

            struct pstat *trial = malloc (sizeof (struct pstat));
            //   printf(0,"DID it WORK?? %d  OR NAHHH\n",getpinfo(trial));
            getpinfo(trial);
            for(int i = 0; i < 1; i++) {
                printf(0,"inuse: %d  pid: %d  timeslice: %d  compticks: %d"
                         "  schedticks: %d  sleepticks: %d  switches: %d"
                         "  name: %s\n",
                       trial->inuse[i],trial->pid[i],trial->timeslice[i],
                       trial->compticks[i],trial->schedticks[i],trial->sleepticks[i],
                       trial->switches[i],trial->names[i]);
            }
            int j = 0;
            printf(0,"Head: ");
            while (trial->headtotail[j] != 0){
                printf(0,"%s ->", trial->headtotail[j]);
                ++j;
            }
            free(trial);

            char *args[2] = {"ls", 0};
//            printf(0,"%d\n", setslice(4,90));
            printf(0,"%d\n", getslice(4));
            exec(args[0], args);

            // if child reached here, exec failed
            printf(0,"child: exec failed\n");
            exit();

        } else {
            // parent
            int i = wait();
            printf(0,"wait: %d\n", i);
            printf(0,"get slice of proc %d: %d\n", i, getslice(3));
            printf(0,"parent: child process exits\n");
        }

    } else {
        // fork failed
        printf(0,"parent: fork failed\n");
    }


//  printf(0,"DONE FORKING\n");

//  struct pstat *trial = malloc (sizeof (struct pstat));
////   printf(0,"DID it WORK?? %d  OR NAHHH\n",getpinfo(trial));
//    getpinfo(trial);
//    for(int i = 0; i < 1; i++) {
//     printf(0,"inuse: %d  pid: %d  timeslice: %d  compticks: %d"
//     "  schedticks: %d  sleepticks: %d  switches: %d\n",
//     trial->inuse[i],trial->pid[i],trial->timeslice[i],
//     trial->compticks[i],trial->schedticks[i],trial->sleepticks[i],
//     trial->switches[i]);
//    }
//    free(trial);





    // Actual loop.c code
//    loop(int sleepT) {
        // First, sleep for sleepT ticks;

        // Then, call a loop like this which loops on a huge workload (don't
        // try to code and run any real programs like this! It is just for
        // testing purpose of this project):
//        int i = 0, j = 0;
//        while (i < 800000000) {
//            j += i * j + 1;
//            i++;
//        }
//    }



  exit();
}
