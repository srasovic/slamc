//
//  socket_readers.c
//  SLAM
//
//  Created by Sasa Rasovic on 11/07/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//

#include "../headers/slam.h"


void *udp_socket_reader_process(void *udptr) {
 
    
    tcount++;
    int lcount = tcount;
    int cc = 0;
    fprintf(stderr, "entering UDP reader thread: %d\n", tcount);
    int udp_rcv = 0;
    socklen_t len = sizeof(struct sockaddr_in);
    struct socket_construct *udp = (struct socket_construct *)udptr;
    u_char str[15], buffer[MAX_PACK_SIZE];
    init_offset = udp->mem_reg_offset;
    

    int init_queue_head = dqueue[udp->index].qhead = dqueue[udp->index].qtail = udp->mem_reg_offset;
    
    int total = 0;
    
    
    while (1) {
        
       
        udp_rcv = recvfrom(udp->fd, &iqueue[udp->mem_reg_offset].buffer, MAX_PACK_SIZE, 0, (struct sockaddr *)&iqueue[udp->mem_reg_offset].ip_source, &len);

        pthread_mutex_lock(&lock);

        if (udp->mem_reg_offset == init_offset+REGION_SIZE) {
            udp->mem_reg_offset = init_offset;
            dqueue[udp->index].qtail = init_queue_head;
            
            if (dqueue[udp->index].qtail == init_queue_head){
                dqueue[udp->index].wrap_counter = 1;
            }
        }
        
        if (dqueue[udp->index].wrap_counter)
            dqueue[udp->index].qhead = dqueue[udp->index].qtail;
        

        udp->index_ctr = udp->index_ctr + 1;
        total = udp->index_ctr;
        
        inet_ntop(AF_INET, &iqueue[udp->mem_reg_offset].ip_source.sin_addr, str, INET_ADDRSTRLEN);
//        memcpy(&buffer, &iqueue[udp->mem_reg_offset].buffer, udp_rcv);
        
        udp->mem_reg_offset ++;
        dqueue[udp->index].qtail = udp->mem_reg_offset;
        if (!dqueue[udp->index].wrap_counter)
            total = dqueue[udp->index].qtail - dqueue[udp->index].qhead;
        else (total = 256);


        pthread_mutex_unlock(&lock);
//        fprintf(stderr, "Hello from thread %d. I've just received packet %s, from address %s. Queue is currently filled with %d packets. Current offset number is %d. Head is %d and Tail is %d. Wrapup counter is %d\n", lcount, buffer, str, total, udp->mem_reg_offset, dqueue[udp->index].qhead, dqueue[udp->index].qtail, dqueue[udp->index].wrap_counter);
        cc++;
        fprintf(stderr, "%d. Packet received: %s\n", cc, buffer);

        
    }
    
    pthread_exit(NULL);
}


void *tcp_socket_reader_process(void *tcptr) {
    
    tcount++;
    int lcount = tcount;
    int cc = 0;
    int acc;
    fprintf(stderr, "entering TCP reader thread: %d\n", tcount);
    
    u_char str[15], buffer[MAX_PACK_SIZE];
    
    int tcp_rcv = 0;
    socklen_t len = sizeof(struct sockaddr_in);
    struct socket_construct *tcp = (struct socket_construct *)tcptr;
    
    int init_socket = tcp->fd;
    int init_offset = tcp->mem_reg_offset;
    int init_queue_head;
    
    init_queue_head = dqueue[tcp->index].qhead = dqueue[tcp->index].qtail = tcp->mem_reg_offset;
    
    int total = tcp->index_ctr;
//
    fd_set readfds;
    struct timeval tv;
    int max_sock, rv;
    acc=-1;

    while(1) {
    
        
        while (acc==-1)
            acc = accept(tcp->fd, NULL, NULL);
            //sleep(2);
        

        tcp->fd = acc;
    
        while (1) {
            
            FD_ZERO(&readfds);
            FD_SET(tcp->fd, &readfds);
            max_sock = (tcp->fd)+1;      //need to change
            tv.tv_sec = 1;
            rv = select(max_sock, &readfds, NULL, NULL, &tv);
            
            if (rv==-1) {
                perror("select()");
            }
            else if (rv) {

                
                tcp_rcv = recv(tcp->fd, &iqueue[tcp->mem_reg_offset].buffer, MAX_PACK_SIZE, 0);

                if (tcp_rcv>0) {

                    pthread_mutex_lock(&lock);

                    if (tcp->mem_reg_offset == init_offset+REGION_SIZE) {
                        tcp->mem_reg_offset = init_offset;
                        dqueue[tcp->index].qtail = init_queue_head;
                        
                        if (dqueue[tcp->index].qtail == init_queue_head){
                            dqueue[tcp->index].wrap_counter = 1;
                            //fprintf(stderr, "setting tail to %d\n", init_queue_head);
                        }
                        //fprintf(stderr, "Rolling over queue.\n");
                    }
                    
                    
                    if (dqueue[tcp->index].wrap_counter)
                        dqueue[tcp->index].qhead = dqueue[tcp->index].qtail;
                    
                    //needs to be changed to select + read + getpeer address:
                    //tcp_rcv = recv(tcp->fd, r_packet, MAX_PACK_SIZE, 0);
                    
                    getpeername(tcp->fd, (struct sockaddr *)&iqueue[tcp->mem_reg_offset].ip_source, &len);
                    
                    tcp->index_ctr = tcp->index_ctr + 1;
                    total = tcp->index_ctr;

    //                iqueue[tcp->mem_reg_offset].protocol = 6;
                    inet_ntop(AF_INET, &iqueue[tcp->mem_reg_offset].ip_source.sin_addr, str, INET_ADDRSTRLEN);
                    memcpy(&buffer, &iqueue[tcp->mem_reg_offset].buffer, tcp_rcv);
                    
                    tcp->mem_reg_offset++;

                    dqueue[tcp->index].qtail = tcp->mem_reg_offset;
                    if (!dqueue[tcp->index].wrap_counter)
                        total = dqueue[tcp->index].qtail - dqueue[tcp->index].qhead;
                    else (total = 256);
                    fprintf(stderr, "%d. I've just received a packet %s from address %s. Queue is currently filled with %d packets\n\n", cc, buffer, str, total);

                    pthread_mutex_unlock(&lock);
                    cc++;
//                    fprintf(stderr, "%d. I've just received a packet %s from address %s. Queue is currently filled with %d packets\nCurrent offset number is %d\nHead is %d and Tail is %d\nWrapup counter is %d\n\n", cc, buffer, str, total, tcp->mem_reg_offset, dqueue[tcp->index].qhead, dqueue[tcp->index].qtail, dqueue[tcp->index].wrap_counter);

                    
                }
                
                else {
                    close(tcp->fd);
                    tcp->fd = init_socket;
                    acc=-1;
                    break;
                }
                
            }
            
            else {
                continue;
            }
            
        }
        
    }
    
    //tcp->texit = 1;
    pthread_exit(NULL);
    
}
