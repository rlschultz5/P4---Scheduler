#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(void)
{
//  printf(0,"%d\n", setslice(1,2));
  printf(0,"%d\n", getslice(0));
    printf(0,"%d\n", getslice(1));
    printf(0,"%d\n", getslice(2));
  printf(0,"pid = %d\n", fork2(7));
//    printf(0,"DONE FORKING\n");

  struct pstat *trial = malloc (sizeof (struct pstat));
//   printf(0,"DID it WORK?? %d  OR NAHHH\n",getpinfo(trial));
    getpinfo(trial);
    for(int i = 0; i < 1; i++) {
     printf(0,"inuse: %d  pid: %d  timeslice: %d  compticks: %d"
     "  schedticks: %d  sleepticks: %d  switches: %d\n",
     trial->inuse[i],trial->pid[i],trial->timeslice[i],
     trial->compticks[i],trial->schedticks[i],trial->sleepticks[i],
     trial->switches[i]);
    }
    free(trial);
  exit();
}
