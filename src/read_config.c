//
//  read_config.c
//  SLAM
//
//  Created by Sasa Rasovic on 26/06/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//

#include "../headers/slam.h"

#define LINE_MIN_LEN    10


void read_config_file(struct slam_param_listener_list *head) {

    int bytes_read, line_num = 1;
    size_t len = 0;
    char *line_string = NULL;
    int proper=0;
    
    FILE *stream;
    stream = fopen("slam.conf", "r");
    if (stream == NULL) {
        fprintf(stderr, "Unable to open slam.conf file\n");
        exit(EXIT_FAILURE);
    }
    
    line_string = (char *) calloc (1, len + 1);
    
    while ((bytes_read = getline (&line_string, &len, stream))!=EOF) {
        
        if (bytes_read ==0 || bytes_read ==1) {
            line_num++;
            continue;
        }
        
        if (bytes_read <LINE_MIN_LEN) {
            line_num++;
            fprintf(stderr, "line number %d not properly formatted...skipping.\n", line_num);
            continue;
        }
        else {
            proper = proper + read_sanitize(line_string, line_num);
            if (proper >= 4) {
                fprintf(stderr, "Number of configured listeners is more than allowed 4. Ignoring the rest.\n");
                //here, there should be a routine that calls another container to be spawned and run additional workload, instead of just quiting
                break;
            }
            line_num++;
        }
    }
    
    if (proper ==0) {
        fprintf(stderr, "Empty or improperly formatted file - nothing to do. Exiting.\n");
        exit(EXIT_FAILURE);
    }
//    listener_count = 0;
    
}


int read_sanitize(char *line_string, int line_num) {

    
    u_char* kafka_t = "KAFKA";
    u_char* listener_t = "LISTENERS:";
    
    const char delim1[2] = "=";
    const char delim2[2] = " ";
    const char delim3[2] = "\n";

    char *token, *port, *address;

    
    u_char ip_address[15];
    int status, count, port_n;
    
    memset(ip_address, 0, 15);
    
    if (listener_count == 16) {
        count = listener_count;
        listener_count = 0;
        return count;
    }
    

    if (strncmp(kafka_t, line_string, strlen(kafka_t))==0) {
        
        token = strtok(line_string, delim1);
        token = strtok(NULL, delim1);
        address = strtok(token, delim2);
        port = strtok(NULL, delim3);
        
        if (strlen(port) > 5) {
            fprintf(stderr, "Incorrect port specification of KAFKA server on line %d\n", line_num);
            exit(EXIT_FAILURE);
        }
        port_n = atoi(port);
        kafka.dport = port_n;
        
        status = inet_pton(AF_INET, address, &(kafka.kafka_address.sin_addr));
        if (status!=1) {
            fprintf(stderr, "Incorrect IP address of KAFKA server on line %d\n", line_num);
            exit(EXIT_FAILURE);
        }
        return 0;
    }
       
    if (strncmp(listener_t, line_string, strlen(listener_t))==0) {
        return 0;
    }
        
    else {
        
        token=strtok(line_string, delim2);
        strncpy(ip_address, token, strlen(token));
        status = inet_pton(AF_INET, ip_address, &(head->source.sin_addr));
        if (status!=1) {
            fprintf(stderr, "Incorrect IP address of listener on line %d\n", line_num);
            exit(EXIT_FAILURE);
        }
        
        token=strtok(NULL, delim2);
        if (strncmp(token, "udp", 3)!=0) {
            if (strncmp(token, "tcp", 3) !=0) {
                fprintf(stderr, "Incorrect listener protocol on line %d\n", line_num);
                exit(EXIT_FAILURE);
            }
            else {
                head->protocol = 6;
                pp.tcp++;
            }
        }
        else {
            head->protocol = 17;
            pp.udp++;
        }
        
        token=strtok(NULL, delim2);
        if (atoi(token))
            head->source.sin_port = htons(atoi(token));
        else {
            fprintf(stderr, "Incorrectly configured listener port on line %d\n", line_num);
            exit(EXIT_FAILURE);
        }
        
        token=strtok(NULL, delim2);
        if (token){
            if (token[strlen(token)-1] == '\n')
                strncpy(head->interface, token, strlen(token)-1);
            else
                strncpy(head->interface, token, strlen(token));
        }
        else {
            fprintf(stderr, "Incorrectly configured interface on line %d\n", line_num);
            exit(EXIT_FAILURE);
        }

    }

    listener_count++;
    head->next = calloc(1, sizeof (struct slam_param_listener_list));
    head = head->next;
    return 1;
    
}

