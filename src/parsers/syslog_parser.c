//
//  syslog_parser.c
//  SLAM
//
//  Created by Sasa Rasovic on 29/07/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//

#include "../../headers/slam.h"

struct kafka_syslog_header *kheader;

void *syslog_parser(void *varg) {
    

    const char delim1[2] = "<";
    const char delim2[2] = ">";

    int lcount = 0;
    
    kheader = calloc(1, sizeof(struct kafka_syslog_header));
    u_char * syslog = (u_char *)calloc(1, 1024);
    kheader->raw_message = calloc(1, 1024);
    
    fprintf(stderr, "entering syslog parser thread\n");

    char *token;
    u_char sender[15];
    int i;
    u_int8_t pri, fac;
    u_int8_t map = 0xF;
    unsigned long crc = crc32(0L, Z_NULL, 0);

    u_int8_t raw[3];
    
    kheader->uuid_title = (u_char *)"UUID: ";
    kheader->facility_title = (u_char *)"FACILITY: ";
    kheader->priority_title = (u_char *)"PRIORITY: ";
    kheader->host_title = (u_char *)"HOSTNAME: ";
    kheader->app_title = (u_char *)"PROGRAM: ";
    kheader->date_title = (u_char *)"DATE: ";
    kheader->message_title = (u_char *)"MESSAGE: ";
    kheader->raw_message_title = (u_char *)"ORIGINAL MESSAGE: ";
    kheader->source_title = (u_char *)"SOURCE: ";
    kheader->protocol_title = (u_char *)"PROTOCOL: ";
    
 
    
//    if (squeue_counter == squeue_index)
  //      squeue_counter = 0;

    
    while (1) {
        
        while (squeue_readable) {
            
            memset(syslog, 0, 1024);
            memset(kheader->raw_message, 0, 1024);
            
            pthread_mutex_lock(&lock);
            strncpy((char *)syslog, (char *)squeue[squeue_counter].buffer, 1024);

            squeue_readable--;

            strncpy(kheader->raw_message, syslog, strlen(syslog));
            
            kheader->uuid = crc32(crc, (char *)syslog, strlen(syslog)); //must add timestamp to calculation
            
            kheader->protocol = squeue[squeue_counter].protocol;
            memset(sender, 0, 15);
            inet_ntop(AF_INET, &squeue[squeue_counter].ip_source.sin_addr, (char *)sender, INET_ADDRSTRLEN);
            kheader->source = sender;
            
            token = strtok((char *)syslog, delim1);
            //token = strtok(NULL, delim1);
            token = strtok((char *)token, delim2);
            
            for (i=0;i<strlen(token); i++)
                raw[i] = token[i]&map;
            char v[strlen(token)];
            for (i=0;i<strlen(token); i++)
                v[i] = '0'+raw[i];
            
            pri = atoi(v) %8;
            fac = (atoi(v) - pri) /8;
            
            switch (pri) {
                case 0:
                    kheader->priority = (u_char *)"emergency";
                    break;
                case 1:
                    kheader->priority = (u_char *)"alert";
                    break;
                case 2:
                    kheader->priority = (u_char *)"critical";
                    break;
                case 3:
                    kheader->priority = (u_char *)"error";
                    break;
                case 4:
                    kheader->priority = (u_char *)"warning";
                    break;
                case 5:
                    kheader->priority = (u_char *)"notice";
                    break;
                case 6:
                    kheader->priority = (u_char *)"info";
                    break;
                case 7:
                    kheader->priority = (u_char *)"debug";
                    break;
                    
                default:
                    break;
            }
            
            switch (fac) {
                case 0:
                    kheader->facility = (u_char *)"kernel";
                    break;
                case 1:
                    kheader->facility = (u_char *)"user";
                    break;
                case 2:
                    kheader->facility = (u_char *)"mail";
                    break;
                case 3:
                    kheader->facility = (u_char *)"system";
                    break;
                case 4:
                    kheader->facility = (u_char *)"auth";
                    break;
                case 5:
                    kheader->facility = (u_char *)"syslogd";
                    break;
                case 6:
                    kheader->facility = (u_char *)"printer";
                    break;
                case 7:
                    kheader->facility = (u_char *)"news";
                    break;
                case 8:
                    kheader->facility = (u_char *)"uucp";
                    break;
                case 9:
                    kheader->facility = (u_char *)"clock";
                    break;
                case 10:
                    kheader->facility = (u_char *)"auth";
                    break;
                case 11:
                    kheader->facility = (u_char *)"ftp";
                    break;
                case 12:
                    kheader->facility = (u_char *)"ntp";
                    break;
                case 13:
                    kheader->facility = (u_char *)"audit";
                    break;
                case 14:
                    kheader->facility = (u_char *)"alert";
                    break;
                case 15:
                    kheader->facility = (u_char *)"clock 2";
                    break;
                case 16:
                    kheader->facility = (u_char *)"local0";
                    break;
                case 17:
                    kheader->facility = (u_char *)"local1";
                    break;
                case 18:
                    kheader->facility = (u_char *)"local2";
                    break;
                case 19:
                    kheader->facility = (u_char *)"local3";
                    break;
                case 20:
                    kheader->facility = (u_char *)"local4";
                    break;
                case 21:
                    kheader->facility = (u_char *)"local5";
                    break;
                case 22:
                    kheader->facility = (u_char *)"local6";
                    break;
                case 23:
                    kheader->facility = (u_char *)"local7";
                    break;
                    
                default:
                    break;
            }

            strncpy((char *)syslog, (char *)squeue[squeue_counter].buffer, 1024);
            
            
            token = strtok((char *)syslog, delim2);
            token = strtok(NULL, delim2);
    //        strncpy((char *)syslog, (char *)squeue[squeue_counter].buffer, 1024);
            
            if (token[0]=='1')
                ietf_parser(&token[2]);
            else
                bsd_parser(token);
            
            if (kheader->message[strlen(kheader->message)-1] == ' ')
                kheader->message[strlen(kheader->message)-1] = '\0';

            squeue_counter++;
            lcount++;
            
            fprintf(stderr, "%d. Transformed packet: %s, %s, %s\n", lcount, kheader->facility, kheader->priority, kheader->message);

            //call_kafka_producer(kheader);

            pthread_mutex_unlock(&lock);
            
            
            
        }

    }
    
}


void bsd_parser(u_char *syslog){

    const char delim1[2] = ">";
    const char delim2[2] = " ";

    struct tm t;
    time_t time;
    u_char msg[1024], date[16], month[3], day[2],hour[2], min[2], sec[2];
    memset(&msg, 0, 1024);
    memset(&date, 0, 16);
    char *token;

//    token = strtok((char *)syslog, delim1);
//    token = strtok(NULL, delim1);
    strncpy((char *)date, (char *)syslog, 16);
    strncpy((char *)month, (char *)date, 3);
    strncpy((char *)day, (char *)&date[4], 2);
    strncpy((char *)hour, (char *)&date[7], 2);
    strncpy((char *)min, (char *)&date[10], 2);
    strncpy((char *)sec, (char *)&date[13], 2);

    t.tm_year = 0;
    t.tm_mon = atoi((char *)month);
    t.tm_mday = atoi((char *)day);
    t.tm_hour = atoi((char *)hour);
    t.tm_min = atoi((char *)min);
    t.tm_sec = atoi((char *)sec);
    t.tm_isdst = -1;
    time = mktime(&t);
    kheader->date = time;

    strncpy((char *)msg, &syslog[16], 1024);
    token = strtok((char *)msg, delim2);
    kheader->host = (u_char *)token;
    kheader->app = (u_char *)"\0";
    token = strtok(NULL, delim1);
    kheader->message = (u_char *)token;
    
}


void ietf_parser(u_char * syslog) {
    
    const char delim1[2] = " ";
    struct tm t;
    time_t time;
    char *token;
    u_char msg[1024], date[32], year[4], month[2], day[2],hour[2], min[2], sec[2];
    
    memset(&date, 0, 32);

    strncpy(msg, syslog, 1024);
    
    //token = strtok((char *)syslog, delim1);
    //token = strtok(NULL, delim1);
    
    strcat((char *)date, (char *)syslog);
    strncpy((char *)year, (char *)date, 4);
    strncpy((char *)month, (char *)&date[5], 2);
    strncpy((char *)day, (char *)&date[8], 2);
    strncpy((char *)hour, (char *)&date[11], 2);
    strncpy((char *)min, (char *)&date[14], 2);
    strncpy((char *)sec, (char *)&date[17], 2);

    t.tm_year = atoi((char *)year);
    t.tm_mon = atoi((char *)month);
    t.tm_mday = atoi((char *)day);
    t.tm_hour = atoi((char *)hour);
    t.tm_min = atoi((char *)min);
    t.tm_sec = atoi((char *)sec);
    t.tm_isdst = -1;
    time = mktime(&t);
    kheader->date = time;
    
    token = strtok((char *)syslog, delim1);
    token = strtok(NULL, delim1);
    kheader->host = (u_char*)token;
    
    token = strtok(NULL, delim1);
    kheader->app = (u_char *)token;

    memset(&msg, 0, 1024);
    token = strtok(NULL, delim1);

    while (token !=NULL) {
        strcat((char *)msg, token);
        strcat((char *)msg, " ");
        token = strtok(NULL, delim1);
    }
    kheader->message = msg;

}
