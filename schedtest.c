#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"


// schedtest spawns two children processes, each running the loop application.
// One child A is given initial timeslice length of sliceA and runs loop
// sleepA; the other B is given initial timeslice length of sliceB and runs
// loop sleepB.
    int main(int argc, char *argv[]) {
      if (argc != 6){
        printf(0,"Invalid input\n");
        exit();
      }
      int sliceA = atoi(argv[1]);
      int sleepA = atoi(argv[2]);
      int sliceB = atoi(argv[3]);
      int sleepB = atoi(argv[4]);
      int sleepParent = atoi(argv[5]);
      printf(0,"input: %d, %d, %d, %d, %d\n",sliceA,sleepA,sliceB,sleepB,sleepParent);
      struct pstat *childA = malloc(sizeof (struct pstat));
      struct pstat *childB = malloc(sizeof (struct pstat));
      int fork_status = fork2(sliceA);
      if(fork_status == 0){
        char *arg[] = {"loop",argv[2],0};
        printf(0,"Before A loop\n");
        exec("loop",arg);
        printf(0,"ChildA PID: %i\n",getpid());
        printf(0,"After B loop\n");
        
      }
      else{
        int fork_status2 = fork2(sliceB);
        if(fork_status2 == 0){
          char *arg2[] = {"loop",argv[4],0};
          printf(0,"Before B loop\n");
          exec("loop",arg2);
          printf(0,"After B loop\n");
          printf(0,"ChildB PID: %i\n",getpid());
          
        }
        else{
          sleep(sleepParent);
          getpinfo(childA);
          getpinfo(childB);
          printf(1, "Compticks: %d %d\n", childA->compticks, childB->compticks);
          wait();
          wait();
          printf(0,"After waits\n");
        }
      }
    // (int sliceA, int sleepA, int sliceB, int sleepB, int sleepParent) {
    // Specifically, the parent process calls fork2() and exec() for the two
    // children loop processes,A before B,with the specified initial timeslice;
//    pid_t pid = fork2(sliceA);
    // TODO: NOT CORRECT
//    if (pid != -1) {
//        if (pid == 0) {
//            // child
//            setslice(pid,sliceA)
//            loop(sleepA);
//            // if child reached here, exec failed
//            printf("child: exec failed\n");
//            _exit(1);
//
//        } else {
//            // parent
//            int status;
//            waitpid(pid, &status, 0);
//            printf("parent: child process exits\n");
//        }
//
//    } else {
//        // fork failed
//        printf("parent: fork failed\n");
    exit();
    }


    // The parent schedtest process then sleeps for sleepParent ticks by
    // calling sleep(sleepParent) (you may assume that sleepParent is much
    // larger than sliceA+2*sleepA+sliceB+2*sleepB);

    // After sleeping, the parent calls getpinfo(), and prints one line of
    // two numbers separated by a space:
//    printf(1, "%d %d\n", compticksA, compticksB);
    //  where compticksA is the compticks of process A in the pstat structure
    //  and similarly for B. The parent then waits for the two loop processes
    //  by calling wait() twice, and exits.


    // (Note that a time tick of 10ms is a lot of time for a fast CPU, hence
    // it is likely that the calls to fork and exec will finish and the parent
    // will call wait before any child process is scheduled; therefore, you
    // don't need to worry about the parent process interfering with the
    // scheduling queue.)

    // Once you have these two user applications, you will be able to run
    // schedtest to test the basic compensation behavior of your scheduler.
    // For example:

    // prompt> schedtest 2 3 5 5 100 3 5   # Expected output -- you should be
    // able to figure out why if you have understood our RR compensation policy
