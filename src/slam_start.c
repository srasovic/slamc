//
//  slam_start.c
//  SLAM
//
//  Created by Sasa Rasovic on 26/06/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//

#include "../headers/slam.h"

int main (int argc, char ** argv) {
    
    int err;
    
    //this needs to change to be cli argument:
    listener = SYSLOG;
    
    head=calloc(1, sizeof (struct slam_param_listener_list));
    root=head;
    
    //map global variable in the shared memory space - needed in order to signal between forked processes:

    //file_changed = mmap(NULL, sizeof *file_changed, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    read_config_file(head);
        
    //err = go_daemon();
    
    main_entry(listener);   //change later

    if (err ==-1) {
        logging("Unable to fork processes. Exiting\n");
        exit(EXIT_FAILURE);
    }
    
    
}
