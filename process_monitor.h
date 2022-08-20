//#define _POSIX_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>//richiesto da man 2 kill
#include <signal.h> //idem di sopra

int kill_PID(int process_pid);
int sleep_PID(int process_pid);
int terminate_PID(int process_pid);


//man 2 sigaction:
/*The following is a list of all signals with names as in the include file
⟨_s_i_g_n_a_l_._h⟩:

NNAAMMEE            Default Action          Description
SIGHUP          terminate process       terminal line hangup
SIGINT          terminate process       interrupt program
SIGQUIT         create core image       quit program
SIGILL          create core image       illegal instruction
SIGTRAP         create core image       trace trap
SIGABRT         create core image       abort(3) call (formerly SIGIOT)
SIGEMT          create core image       emulate instruction executed
SIGFPE          create core image       floating-point exception
SIGKILL         terminate process       kill program
SIGBUS          create core image       bus error
SIGSEGV         create core image       segmentation violation
SIGSYS          create core image       non-existent system call invoked
SIGPIPE         terminate process       write on a pipe with no reader
SIGALRM         terminate process       real-time timer expired
SIGTERM         terminate process       software termination signal
SIGURG          discard signal          urgent condition present on socket
SIGSTOP         stop process            stop (cannot be caught or ignored)
SIGTSTP         stop process            stop signal generated from keyboard
SIGCONT         discard signal          continue after stop
SIGCHLD         discard signal          child status has changed
SIGTTIN         stop process            background read attempted from
                                        control terminal
SIGTTOU         stop process            background write attempted to
                                        control terminal
SIGIO           discard signal          I/O is possible on a descriptor
                                        (see fcntl(2))
SIGXCPU         terminate process       cpu time limit exceeded (see
                                        setrlimit(2))
SIGXFSZ         terminate process       file size limit exceeded (see
                                        setrlimit(2))
SIGVTALRM       terminate process       virtual time alarm (see
                                        setitimer(2))
SIGPROF         terminate process       profiling timer alarm (see
                                        setitimer(2))
SIGWINCH        discard signal          Window size change
SIGINFO         discard signal          status request from keyboard
SIGUSR1         terminate process       User defined signal 1
SIGUSR2         terminate process       User defined signal 2*/
