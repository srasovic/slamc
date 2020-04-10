//
//  socket_senders.c
//  SLAM
//
//  Created by Sasa Rasovic on 12/07/2019.
//  Copyright © 2019 Sasa Rasovic. All rights reserved.
//


#include "../headers/slam.h"


void *udp_queue_aggregator_process(void *udptr){
    
    
    tcount++;
    fprintf(stderr, "entering UDP aggregator thread: %d\n", tcount);
    int  i, cc = 0, sqr;
    struct socket_construct *udp = (struct socket_construct *)udptr;

    int max_offset = init_offset+REGION_SIZE;
    int copy_needed, copy_needed_beg, copy_needed_end;
    
 
    
    while(1) {
        
        if (squeue_index==RING_SIZE-1) {
            squeue = shead;
            squeue_index = 0;
        }
        
        
        if (dqueue[udp->index].wrap_counter) {
            
            
            copy_needed_end = max_offset - dqueue[udp->index].qtail;
            copy_needed_beg = dqueue[udp->index].qtail - dqueue[udp->index].qhead;

            memcpy(&squeue[squeue_index], &iqueue[dqueue[udp->index].qtail], copy_needed_end*sizeof(struct incoming_queue_entry));
            memcpy(&squeue[squeue_index+copy_needed_end], &iqueue[dqueue[udp->index].qhead], copy_needed_beg*sizeof(struct incoming_queue_entry));
            
            squeue_index = squeue_index+copy_needed_end+copy_needed_beg;
            copy_needed_end = copy_needed_beg = 0;

            iqueue[udp->index].head = 0;
            iqueue[udp->index].tail = 0;
            dqueue[udp->index].qtail = 0;
            dqueue[udp->index].qhead = 0;
            dqueue[udp->index].wrap_counter = 0;
            udp->index_ctr = 0;
            udp->mem_reg_offset = init_offset;

            pthread_mutex_unlock(&lock);

        }
        
        else {
            
            if (dqueue[udp->index].qtail - dqueue[udp->index].qhead){
                
                pthread_mutex_lock(&lock);

                sqr = squeue_readable;
                
                copy_needed = dqueue[udp->index].qtail - dqueue[udp->index].qhead;

                memcpy(&squeue[squeue_index], &iqueue[dqueue[udp->index].qhead], copy_needed*sizeof(struct incoming_queue_entry));
                
                for (i=0; i<copy_needed;i++) {
                    squeue[squeue_index+i].protocol = 17;
                }

             //   fprintf(stderr, "%d. Aggregated buffer is: %s\n", squeue_index+1, squeue[squeue_index].buffer);

                squeue_index = squeue_index+copy_needed;

                sqr = sqr + copy_needed;
                copy_needed = 0;
/*
                iqueue[udp->index].head = init_offset;
                iqueue[udp->index].tail = init_offset;
                dqueue[udp->index].qtail = init_offset;
                dqueue[udp->index].qhead = init_offset;
 */
                dqueue[udp->index].qhead = dqueue[udp->index].qtail;
                dqueue[udp->index].wrap_counter = 0;
                udp->index_ctr = 0;
//                udp->mem_reg_offset = init_offset;
                squeue_readable = sqr;

                pthread_mutex_unlock(&lock);

            }
            
        }

    }
    
}



void *tcp_queue_aggregator_process(void *tcptr){
    
    tcount++;
    fprintf(stderr, "entering TCP aggregator thread: %d\n", tcount);
    int i, cc = 0, sqr;
    struct socket_construct *tcp = (struct socket_construct *)tcptr;
    
    int init_offset = tcp->mem_reg_offset;
    int max_offset = init_offset+REGION_SIZE;
    int copy_needed, copy_needed_beg, copy_needed_end;
    
    
    while(1) {
        
        
        if (squeue_index==RING_SIZE-1) {
            squeue = shead;
            squeue_index = 0;
        }
        
        if (dqueue[tcp->index].wrap_counter) {
            
            pthread_mutex_lock(&lock);
            
            copy_needed_end = max_offset - dqueue[tcp->index].qtail;
            copy_needed_beg = dqueue[tcp->index].qtail - dqueue[tcp->index].qhead;
            
            memcpy(&squeue[squeue_index], &iqueue[dqueue[tcp->index].qtail], copy_needed_end*sizeof(struct incoming_queue_entry));
            memcpy(&squeue[squeue_index+copy_needed_end], &iqueue[dqueue[tcp->index].qhead], copy_needed_beg*sizeof(struct incoming_queue_entry));
            
            squeue_index = squeue_index+copy_needed_end+copy_needed_beg;
            copy_needed_end = copy_needed_beg = 0;
            
            iqueue[tcp->index].head = 0;
            iqueue[tcp->index].tail = 0;
            dqueue[tcp->index].qtail = 0;
            dqueue[tcp->index].qhead = 0;
            dqueue[tcp->index].wrap_counter = 0;
            tcp->index_ctr = 0;
            tcp->mem_reg_offset = init_offset;
            
            pthread_mutex_unlock(&lock);
            
        }

        else {

            if (dqueue[tcp->index].qtail - dqueue[tcp->index].qhead){

                pthread_mutex_lock(&lock);

                sqr = squeue_readable;
                copy_needed = dqueue[tcp->index].qtail - dqueue[tcp->index].qhead;
                
                memcpy(&squeue[squeue_index], &iqueue[dqueue[tcp->index].qhead], copy_needed*sizeof(struct incoming_queue_entry));
                for (i=0; i<copy_needed;i++)
                    squeue[squeue_index+i].protocol = 6;
                
                //fprintf(stderr, "%d. Aggregated buffer is: %s\n", squeue_index+1, squeue[squeue_index].buffer);
                
                squeue_index = squeue_index+copy_needed;
                
                sqr = sqr + copy_needed;
                copy_needed = 0;
                
                dqueue[tcp->index].qhead = dqueue[tcp->index].qtail;
                dqueue[tcp->index].wrap_counter = 0;
                tcp->index_ctr = 0;
                //                udp->mem_reg_offset = init_offset;
                squeue_readable = sqr;

                pthread_mutex_unlock(&lock);

                
            }
        }
        
    }
}

