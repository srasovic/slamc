//
//  kafka_producer.c
//  SLAM
//
//  Created by Sasa Rasovic on 29/07/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//

#include "../headers/slam.h"

void call_kafka_producer(struct kafka_syslog_header *kheader) {
    
    
    while (kafka.socket) {
        
//        if (get RST)
            //close_tcp_session(kafka.socket);
    }
    
    establish_tcp_session(kafka);
    
}

