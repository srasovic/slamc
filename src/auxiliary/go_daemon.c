//
//  go_daemon.c
//  SLAM
//
//  Created by Sasa Rasovic on 26/06/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//


#include "../../headers/slam.h"


int fd0, fd1, fd2;


int go_daemon(void) {
    
    //struct rlimit rl;
    struct sigaction lsig, vsig;
    pid_t pid_receiver, pid_conf;
    int i;
    
    umask(0);
    
    /*
     if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
     perror("RLIMIT error");
     
     /*
     /* Ignore SIG handler :
     */
    
    lsig.sa_handler = lsig_handler;
    lsig.sa_flags=0;
    sigemptyset(&lsig.sa_mask);
    
    vsig.sa_flags=SA_SIGINFO;
    vsig.sa_handler = (void*)vsig_handler;
    sigemptyset(&vsig.sa_mask);
    
    sigaction(SIGINT,&lsig, NULL);
    sigaction(SIGHUP,&lsig, NULL);
    sigaction(SIGSEGV,&vsig, NULL);
    
    
    
    pid_receiver = fork();                    //fork for client
    
    if (pid_receiver == -1) {
        perror("fork");
        return -1;
    }
    
    else if (pid_receiver==0){
        
        printf("Starting socket daemon with PID[%d].\n", getpid());
        printf("Please use 'kill -s kill pid' to terminate when needed.\n");
        
        if (setsid() ==-1)                  //set owner session.
            perror("setsid");
        
        if (chdir("/home/sasa/code/slam/") <0)
            perror("CHDIR");
        
        
        /*
         Close all file descriptors:
         */
        
        /*if (rl.rlim_max == RLIM_INFINITY)
         rl.rlim_max = 10;              //shouldn't have more than 3 though...just in case. */
        
        for (i=0; i<10; i++)
            close(i);
        
        
        /*
         Ditch all the fds to dev/null:
         */
        
        fd0 = open("/dev/null", O_RDWR);
        fd1 = dup(0);
        fd2 = dup(0);
        
        main_entry(listener);
        
    }
    
    else
        exit (EXIT_SUCCESS);
    
  /*
    else
        pid_conf = fork();
    
    if (pid_conf == -1) {
        perror("fork");
        return -1;
    }
    
    else if (pid_conf ==0){
        printf("Starting configuration scanner process with PID[%d].\n", getpid());
        printf("Please use 'kill -s kill pid' to terminate when needed.\n");
        
        if (setsid() ==-1)                  //set owner session.
            perror("setsid");
        
        if (chdir("/home/sasa/code/slam/") <0)
            perror("CHDIR");
        
        for (i=0; i<10; i++)
            close(i);
        
    
        
        fd0 = open("/dev/null", O_RDWR);
        fd1 = dup(0);
        fd2 = dup(0);
        
        //config_scanner();
        
    }
    else
        exit (EXIT_SUCCESS);
  */
    
}


void lsig_handler(int sig) {
    
    if (sig==SIGINT)
        logging("Received SIGINT...Ignoring");
    else if (sig==SIGHUP)
        logging("Received SIHUP...Ignoring");
}

void vsig_handler(int sig, siginfo_t *info, void * context){
    
    char *sig_err = (char *)calloc(1, 150);
    snprintf(sig_err, 150, "Error number %d. slam daemon with PID %d received a SIGSEGV signal with signal code %d.", info->si_errno, (int)info->si_pid, info->si_code);
    
    if (sig==SIGSEGV) {
        logging("Segmentation fault happened due to ");
        logging(sig_err);
        free(sig_err);
        exit(-1);
    }
}
