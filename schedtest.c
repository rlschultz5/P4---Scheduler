#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

// schedtest spawns two children processes, each running the loop application.
// One child A is given initial timeslice length of sliceA and runs loop
// sleepA; the other B is given initial timeslice length of sliceB and runs
// loop sleepB.
schedtest(int sliceA, int sleepA, int sliceB, int sleepB, int sleepParent) {
    // Specifically, the parent process calls fork2() and exec() for the two
    // children loop processes,A before B,with the specified initial timeslice;
    pid_t pid = fork2(sliceA);
    // TODO: NOT CORRECT
    if (pid != -1) {
        if (pid == 0) {
            // child
            setslice(pid,sliceA)
            loop(sleepA);
            // if child reached here, exec failed
            printf("child: exec failed\n");
            _exit(1);

        } else {
            // parent
            int status;
            waitpid(pid, &status, 0);
            printf("parent: child process exits\n");
        }

    } else {
        // fork failed
        printf("parent: fork failed\n");
    }


    // The parent schedtest process then sleeps for sleepParent ticks by
    // calling sleep(sleepParent) (you may assume that sleepParent is much
    // larger than sliceA+2*sleepA+sliceB+2*sleepB);

    // After sleeping, the parent calls getpinfo(), and prints one line of
    // two numbers separated by a space:
    printf(1, "%d %d\n", compticksA, compticksB)
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
