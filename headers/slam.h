//
//  slam_start.h
//  SLAM
//
//  Created by Sasa Rasovic on 26/06/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <syslog.h>
#include <signal.h>
//#include <sys/inotify.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <zlib.h>

#define MAX_PACK_SIZE 9126
#define SYSLOG  1


#define REGION_SIZE 256
#define RING_SIZE   1024


struct incoming_queue_entry {
    u_char buffer[MAX_PACK_SIZE];
    struct sockaddr_in ip_source;
    u_int32_t protocol;
    u_int32_t port;
    u_int32_t head;
    u_int32_t tail;
    struct incoming_queue_entry *next;
};

struct incoming_queue_descr {
    u_int32_t qhead;
    u_int32_t qtail;
    u_int32_t wrap_counter;
    struct incoming_queue_descr *next;
};

struct outgoing_queue_entry {
    u_char buffer[MAX_PACK_SIZE];
    u_int32_t index;
};

struct slam_param_listener_list {
    
    struct sockaddr_in source;
    u_char interface[32];
    u_int32_t protocol;
    u_int32_t socket;
    struct slam_param_listener_list *next;
    
};

struct slam_kafka_params {
    struct sockaddr_in kafka_address;
    u_int32_t dport;
    struct sockaddr_in my_address;
    u_int32_t sport;
    u_int32_t socket;
};

struct slam_pid_list {
    pid_t pid_receiver;
    pid_t pid_conf;
};

struct socket_construct {
    u_int32_t protocol;
    u_int32_t fd;
    u_int32_t mem_reg_offset;
    u_int32_t index;
    u_int32_t index_ctr;
    //u_int32_t texit;    
};

struct proto_present {
    int udp;
    int tcp;
};

struct kafka_syslog_header {
    
    u_char *uuid_title;
    u_int64_t uuid;                 //hash value
    u_char *facility_title;
    u_char *facility;
    u_char *priority_title;
    u_char *priority;
    u_char *host_title;
    u_char *host;
    u_char *app_title;
    u_char *app;
    u_char *date_title;
    long date;
    u_char *message_title;
    u_char *message;
    u_char *raw_message_title;
    u_char *raw_message;
    u_char *source_title;
    u_char *source;
    u_char *protocol_title;
    u_int32_t protocol;

};


struct slam_pid_list *plist;

struct slam_param_listener_list *root, *head, *temp;

struct slam_kafka_params kafka;

struct proto_present pp;

int *file_changed;

int listener_count, listener;

struct incoming_queue_entry *iqueue;
struct incoming_queue_descr *dqueue;

struct incoming_queue_entry *squeue, *shead, *stemp;


pthread_mutex_t lock;

int tcount;
int squeue_index, squeue_counter, squeue_readable;

int init_offset;



void logging(char *cmd);
void read_config_file(struct slam_param_listener_list *head);
int read_sanitize(char *line_string, int line_num);
int go_daemon(void);
void lsig_handler(int sig);
void vsig_handler(int sig, siginfo_t *info, void * context);

void main_entry(int listener);
void get_my_ip(u_char *interface_name, struct sockaddr_in * my_ip);
void get_udp_socket(struct slam_param_listener_list *head);
void get_tcp_socket(struct slam_param_listener_list *head);
void establish_tcp_session(struct slam_kafka_params kafka);
void close_tcp_session(int sockfd);

void *udp_socket_reader_process(void *udp);
void *tcp_socket_reader_process(void *tcptr);

void *udp_queue_aggregator_process(void *udp);
void *tcp_queue_aggregator_process(void *tcptr);

void *syslog_parser(void * varg);
void bsd_parser(u_char *syslog);
void ietf_parser(u_char *syslog);

void call_kafka_producer(struct kafka_syslog_header *kheader);
