//
//  socket_ops.c
//  SLAM
//
//  Created by Sasa Rasovic on 08/07/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//

#include "../headers/slam.h"

#define LISTENQ 10


void get_udp_socket(struct slam_param_listener_list *head) {
    
    int sockfd;
    struct sockaddr_in myaddr, anyaddr;
    
    bzero(&myaddr, sizeof(myaddr));
    bzero(&anyaddr, sizeof(anyaddr));

    //anyaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    get_my_ip(head->interface, &myaddr);
    myaddr.sin_family = AF_INET;                //change later according to interface configured
    myaddr.sin_port = htons(head->source.sin_port);
    
    int sa_len = sizeof(myaddr);
    int ret;
    int broadcastEnable=1;
    
    char str[INET_ADDRSTRLEN];
    
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


    ret=setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    if (ret == -1) {
        fprintf(stderr, "Unable to set SO_BROADCAST option on socket. Exiting.\n");
        exit(-1);
    }

    
    ret=setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &broadcastEnable, sizeof(broadcastEnable));
    if (ret == -1) {
        fprintf(stderr, "Unable to set SO_REUSEADDR option on socket. Exiting.\n");
        exit(-1);
    }

    
    ret = bind(sockfd, (const struct sockaddr *)&myaddr, sa_len);
    if (ret == -1) {
        fprintf(stderr, "Unable to bind a socket. Exiting.\n");
        exit(-1);
    }
    
    //call iptables here.
    
    head->socket = sockfd;

}


void get_tcp_socket(struct slam_param_listener_list *head) {
    
    
    int sockfd, sc, b, l, init_status, update_ack_status, update_status, enable=1;
    int slt = 0;
    char str[INET_ADDRSTRLEN];
    
    struct sockaddr_in myaddr;
    
    bzero(&myaddr, sizeof(myaddr));
    
    get_my_ip(head->interface, &myaddr);
    myaddr.sin_family = AF_INET;                //change later according to interface configured
    myaddr.sin_port = htons(head->source.sin_port);
    
    int sa_len = sizeof(myaddr);
    socklen_t addrlen;
    
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        fprintf(stderr, "Unable to get socket. Exiting.\n");
        exit(-1);
    }
    
    b = bind(sockfd, (const struct sockaddr *)&myaddr, sa_len);
    if (b == -1) {
        fprintf(stderr, "Unable to bind a tcp socket. Exiting.\n");
        exit(-1);
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    

    l = listen(sockfd, LISTENQ);
    if (l == -1) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }
    
    head->socket = sockfd;
}


void establish_tcp_session(struct slam_kafka_params kafka) {
    
    int sockfd = 0;
    struct sockaddr_in readaddr;
    
    int sa_len = sizeof(readaddr);
    int n, er;
    int flags = 1;
    
    bzero(&readaddr, sizeof(readaddr));
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Unable to get socket. Exiting.\n");
        exit(-1);
    }
    
    kafka.socket = sockfd;
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags))) {
        perror("ERROR: setsocketopt(), SO_KEEPALIVE");
        exit(-1);
    }

    for (n = 0, er = 1; n<3; n++) {
        
        sleep(1);
        
        if (er>=3 && er<=5) {
            fprintf(stderr, "Kafka server is not responding. Waiting and trying again.\n");
            sleep(3);
        }
        
        else if (er>=6 && er<9) {
            fprintf(stderr, "Kafka server is not responding. Waiting and trying again.\n");
            sleep(6);
        }
        
        else if (er == 9) {
            fprintf(stderr, "Unable to establish connection to Kafka server. Exiting.\n");
            exit(-1);
        }
        
        if (connect(sockfd, (struct sockaddr *) &kafka.kafka_address, sizeof(kafka.kafka_address)) !=0) {
            
            if (errno == ETIMEDOUT) {
                fprintf(stderr, "Connection timed out. Attempt number %d\n", er);
                er++;
            }
            else if (errno == ECONNREFUSED){
                fprintf(stderr, "RST received. Attempt number %d\n", er);
                er++;
            }
            else if (errno == EHOSTUNREACH || errno == ENETUNREACH) {
                fprintf(stderr, "Destination unreachable. Exiting.\n");
                exit(-1);
            }
            else {
                fprintf(stderr, "Unspecified error occured during socket establishment.\n");
                exit(-1);
            }
            
        }
        
        else {
            //return sockfd;
            
            if (er>2)
                fprintf(stderr, "Connection to Kafka server established successfully.\n");
            
            if (getsockname(sockfd,(struct sockaddr *)&readaddr,(socklen_t *)&sa_len) == 0) {
                kafka.sport = ntohs(readaddr.sin_port);
                kafka.my_address = readaddr;
                break;
            }
            else {
                fprintf(stderr, "Unable to determine source port for the socket.\n");
                exit(-1);
            }
        }
        
    }
    
}


void close_tcp_session(int sockfd) {
        
    if (close(sockfd) !=0) {
        fprintf(stderr, "Error occured while closing the socket. Exiting.\n");
        exit(-1);
    }
    
    sockfd = 0;
    
}


void get_my_ip(u_char *interface_name, struct sockaddr_in * my_ip) {
    
    struct ifaddrs *id, *ida;
    int val;
    char *addr;
    val = getifaddrs(&id);
    
    for (ida=id; ida; ida=ida->ifa_next) {
        if (strncmp(ida->ifa_name, interface_name, strlen(interface_name))==0) {
            if (ida->ifa_addr->sa_family == AF_INET) {
                memcpy(my_ip,(struct sockaddr_in *) ida->ifa_addr, sizeof(struct sockaddr_in));
                addr = inet_ntoa(my_ip->sin_addr);
                break;
            }
        }
    }
    
    freeifaddrs(id);
    
}


