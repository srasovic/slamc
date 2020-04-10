//
//  logger.c
//  SLAM
//
//  Created by Sasa Rasovic on 26/06/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//

#include "../../headers/slam.h"

char *cad = "SLAM Daemon";
char *rlmit = "----- Last message repeated 10 times -----";
static int count = 0;

void logging(char *cmd) {
    
    
    if (count >4 && count <10)
        count++;
    
    else if (count == 10) {
        openlog(cad, 0, LOG_LOCAL5);
        syslog(LOG_ERR, "%s",(const char *) rlmit);
        closelog();
        count = 0;
    }
    
    else {
        openlog(cad, LOG_CONS, LOG_LOCAL5);
        syslog(LOG_ERR,"%s",(const char *) cmd);
        closelog();
        count++;
    }
    
}

