//
//  main_entry.c
//  SLAM
//
//  Created by Sasa Rasovic on 26/06/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//


#include "../headers/slam.h"


void main_entry(int listener) {

    
    int count = 1;
    int mc, i, j, k=1, n=0, m=0;
    static int udpcounter = 0, x;

    pthread_t tid[listener_count];          //reader threads
    pthread_t aid[listener_count];          //aggregator threads
    pthread_t sid[listener_count];          //syslog threads
    
    int terr;

    head=root;
    
    //establish_tcp_session(kafka);
    
    listener_count = pp.tcp+pp.udp;
    
    u_int32_t memory_regions[listener_count];
    u_int32_t index[listener_count];
    
    memory_regions[0] = 0;
    
    for (i=0; i<listener_count; i++) {
        index[i] = 0;
        memory_regions[i+1] = REGION_SIZE*(i+1);
    }
    
    //different logic for UDP and TCP sockets:
    
    for (temp=root;temp->protocol;temp=temp->next) {
        if (temp->protocol==17) {
            //if (!udpcounter) {
            get_udp_socket(temp);
              //  x = temp->socket;
               // udpcounter=1;
            //}
            //else
              //  temp->socket = x;
        }
    }
        
    for (temp=root;temp->protocol;temp=temp->next) {
        if (temp->protocol==6)
            get_tcp_socket(temp);
    }
    
    
    struct socket_construct sc[listener_count];
    struct socket_construct udp[pp.udp];
    struct socket_construct tcp[pp.tcp];

    iqueue = mmap(NULL, RING_SIZE*sizeof(struct incoming_queue_entry), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

    squeue = mmap(NULL, RING_SIZE*sizeof(struct incoming_queue_entry), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    
    dqueue = mmap(NULL, listener_count*sizeof(struct incoming_queue_descr), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

    shead = squeue;
        
    for (temp=root, i=0; temp->protocol; temp = temp->next, i++) {
        
        if (temp->protocol == 17) {
            sc[i].protocol = 17;
            sc[i].fd = temp->socket;      //change
            sc[i].index = i;
            sc[i].index_ctr = index[i];
            sc[i].mem_reg_offset = memory_regions[i];
        }
        
        else {
            sc[i].protocol = 6;
            sc[i].fd = temp->socket;  //change
            sc[i].index = i;
            sc[i].index_ctr = index[i];
            sc[i].mem_reg_offset = memory_regions[i];            
        }
    }
    
    for (i=0;i<listener_count;i++) {
        
        if (sc[i].protocol == 17) {
            udp[n] = sc[i];
            n++;
        }
        else if (sc[i].protocol == 6) {
            tcp[m] = sc[i];
            m++;
        }
    }
    
    
    head=root;
    squeue = shead;
    
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
        exit(EXIT_FAILURE);
    }

    //create UDP listener/aggregator threads:
    
    for (i=0; i< pp.udp; i++) {
        //reader threads;
        terr = pthread_create(&tid[i], NULL, udp_socket_reader_process, (void *)&udp[i]);
        if (terr != 0) {
            printf("\ncan't create thread :[%s]", strerror(terr));
            exit(-1);
        }
        //sender threads:
        terr = pthread_create(&aid[i], NULL, udp_queue_aggregator_process, (void *)&udp[i]);
        if (terr != 0) {
            printf("\ncan't create thread :[%s]", strerror(terr));
            exit(-1);
        }
    }

    //create TCP listener/aggregator threads:

    for (i=0; i< pp.tcp; i++) {
        //readers threads:
        terr = pthread_create(&tid[i], NULL, tcp_socket_reader_process, (void *)&tcp[i]);
        if (terr != 0) {
            printf("\ncan't create thread :[%s]", strerror(terr));
            exit(-1);
        }
        //sender threads:
        terr = pthread_create(&aid[i], NULL, tcp_queue_aggregator_process, (void *)&tcp[i]);
        if (terr != 0) {
            printf("\ncan't create thread :[%s]", strerror(terr));
            exit(-1);
        }
    }
    

    for (i=0;i<listener_count;i++) {
        terr = pthread_create(&sid[i], NULL, syslog_parser, NULL);
    }

    for (i=0; i< listener_count; i++)
        pthread_join(tid[i], NULL);
    for (i=0; i< listener_count; i++)
        pthread_join(aid[i], NULL);
    for (i=0; i< listener_count; i++)
        pthread_join(sid[i], NULL);


    pthread_mutex_destroy(&lock);

    
}

