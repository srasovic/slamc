#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <time.h>



#define PCKT_LEN 8192
#define MAX_SRCS  50
#define SYSLOG_PORT 514
#define NETFLOW_PORT  9995

struct tuple {
    u_char *source_ip;
    u_char *destination_ip;
    int protocol;
    int number;
    double interval;
} tuple;

struct option longopts[] = {
    {"source_ip",   optional_argument, 0, 's'},
    {"destination_ip",  required_argument, 0, 'd'},
    {"protocol",   optional_argument, 0, 'p'},
    {"interval",   optional_argument, 0, 't'},
    {"number",   optional_argument, 0, 'n'},
    {NULL,        0,                   0, 0}
};

struct sa {
    u_char sa[16];
    struct sa *next;
};

//char *dummy_v4_address = "1.12.3.5";

void print_help() {
    
    fprintf(stderr, "slamc 1.0 logging simulator\n");
    fprintf(stderr, "Usage: slamc --destination-address <address> [Options]\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "\t-m/--source_ip <source ip address>: if source address is not specified, client will send packets from randomized source addresses.\n");
    fprintf(stderr, "\t-p/ --protocol <syslog|netflow>: if not specified, client will send a combination of syslog and netflow. \n");
    fprintf(stderr, "\t-n/ --number <number of packets>: Default is continuous. \n");
    fprintf(stderr, "\t-t/ --interval <time between packets in miliseconds>: If not specified, default is 1 second. \n");
    
}

unsigned short csum(unsigned short *buf, int nwords){
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

/*
 void rand_ipv4_gen(u_char *address_string, u_char *address) {
 
 struct sockaddr_in sa;
 
 inet_pton(AF_INET, address_string, &(sa.sin_addr));
 sa.sin_addr.s_addr = ntohl(sa.sin_addr.s_addr);
 sa.sin_addr.s_addr += rand() % 255;
 sa.sin_addr.s_addr = htonl(sa.sin_addr.s_addr);
 
 inet_ntop(AF_INET, &(sa.sin_addr), address, INET_ADDRSTRLEN);
 
 if (!strncmp(address+strlen(address)-3, "255", 3) || !strncmp(address+strlen(address)-2, ".0", 2))
 rand_ipv4_gen(address_string, address);
 }
 */

void rand_ipv4_gen_list(struct sa *saddress, int num) {
    
    u_char buffer[3];
    u_char *temp = calloc(1, 16);
    memset(buffer, 0, 3);
    memset(temp, 0, 16);
    
    int i, j, cnt;
    
    struct sa *init;
    struct sa *head;
    init = saddress;
    head = saddress;
    
    srand(time(NULL));
    
    for(i=0;i<num;i++) {
        for(j=0;j<4;j++) {
            cnt=rand()%255;
            sprintf(buffer, "%d", cnt);
            strncpy(&temp[strlen(temp)], buffer, strlen(buffer));
            if(j<3)
                temp[strlen(temp)]='.';
            memset(buffer, 0, 3);
        }
        strncpy(init->sa, temp, strlen(temp));
        init->next = calloc(1, sizeof(struct sa));
        init = init->next;
        memset(buffer, 0, 3);
        memset(temp, 0, 16);
    }
    free(temp);
}

int main(int argc, char *argv[]) {
    
    char *syslog_data[3];
    
    syslog_data[0] =
    "\x3c\x31\x33\x34\x3e\x4d\x61\x79\x20\x32\x32\x20\x30\x38\x3a\x35" \
    "\x37\x3a\x33\x30\x20\x70\x66\x3a\x20\x30\x30\x3a\x30\x30\x3a\x35" \
    "\x36\x2e\x33\x32\x36\x34\x30\x38\x20\x72\x75\x6c\x65\x20\x33\x2f" \
    "\x30\x28\x6d\x61\x74\x63\x68\x29\x3a\x20\x62\x6c\x6f\x63\x6b\x20" \
    "\x69\x6e\x20\x6f\x6e\x20\x65\x6d\x32\x3a\x20\x28\x74\x6f\x73\x20" \
    "\x30\x78\x30\x2c\x20\x74\x74\x6c\x20\x31\x32\x38\x2c\x20\x69\x64" \
    "\x20\x36\x32\x30\x33\x2c\x20\x6f\x66\x66\x73\x65\x74\x20\x30\x2c" \
    "\x20\x66\x6c\x61\x67\x73\x20\x5b\x6e\x6f\x6e\x65\x5d\x2c\x20\x70" \
    "\x72\x6f\x74\x6f\x20\x55\x44\x50\x20\x28\x31\x37\x29\x2c\x20\x6c" \
    "\x65\x6e\x67\x74\x68\x20\x32\x32\x39\x29";
    
    
    syslog_data[1] =
    "\x3c\x33\x34\x3e\x31\x20\x32\x30\x30\x33\x2d\x31\x30\x2d\x31\x31" \
    "\x54\x32\x32\x3a\x31\x34\x3a\x31\x35\x2e\x30\x30\x33\x5a\x20\x6c" \
    "\x6f\x63\x61\x6c\x68\x6f\x73\x74\x2e\x65\x6c\x6f\x64\x69\x6e\x61" \
    "\x2e\x6e\x65\x74\x20\x73\x75\x20\x2d\x20\x49\x44\x32\x66\x66\x66" \
    "\x66\x33\x20\x2d\x20\x61\x20\x73\x69\x6d\x70\x6c\x65\x20\x6d\x65" \
    "\x73\x73\x61\x67\x65\x20\x53\x61\x73\x61\x0a";
    
    
    syslog_data[2] =
    "\x3c\x33\x32\x3e\x4d\x61\x79\x20\x32\x32\x20\x30\x38\x3a\x35\x39" \
    "\x3a\x34\x33\x20\x70\x68\x70\x3a\x20\x2f\x69\x6e\x64\x65\x78\x2e" \
    "\x70\x68\x70\x3a\x20\x53\x75\x63\x63\x65\x73\x73\x66\x75\x6c\x20" \
    "\x6c\x6f\x67\x69\x6e\x20\x66\x6f\x72\x20\x75\x73\x65\x72\x20\x27" \
    "\x61\x64\x6d\x69\x6e\x27\x20\x66\x72\x6f\x6d\x3a\x20\x31\x39\x32" \
    "\x2e\x31\x36\x38\x2e\x39\x39\x2e\x37";
    
    char *flow_data =  "    Ãû*Sà½¥º·ïÈ    áêæCiklæáãêæCáâãäêæCd@¹BÜBÜÕ@{È»»G¥äåd@¡¹¡[Úz[ÚzÏkß`PPG¥äåd@1¹1zzÎä¶tááG¥äåd@1¹1_C_CÎâ¶uââG¥äåd@Ùh¹Ù¹ñ¹ñx°PPG¥äåd@1¹1²/z ²/z Îã¶vááG¥äæd@ÅÉ¹ÅWð¢OWð¢O ]àPPG¥äæd@¹´y´yCZc,y,yG¥äæd@g¹WðÒWðÒü9:ÂPPG¥äæd@ô;¹ôÂ Â ²ZÉ»»G¥äæd@¹&¹¹V;í'V;í'Q¨iÈÕÈÕG¥äæd@Æ,¹ÆÙv_AÙv_AÓóÒÎS±S±G¥äæd@R²¹RWõÄjWõÄj PPG¥äæd@P¹P¼+o ¼+o ûû\\X|X|G¥äæ";
    
    
    char ch;
    int i, x, y, z, w, indexptr = 0, count=0, timer;
    struct sa *saddress, *ptr;
    
    while ((ch = getopt_long(argc, argv, ":s:d:p:t:n:", longopts, &indexptr)) != -1) {
        
        switch(ch) {
                
            case 's':
                tuple.source_ip= optarg;
                break;
            case 'd':
                tuple.destination_ip = optarg;
                break;
            case 'p':
                if (strncmp(optarg, "syslog", 6)==0)
                    tuple.protocol = 1;
                else if (strncmp(optarg, "flow", 4)==0)
                    tuple.protocol = 2;
                else
                    tuple.protocol = 3;
                break;
            case 'n':
                tuple.number = atoi(optarg);
                break;
            case 't':
                tuple.interval = (double)atoi(optarg);
                break;
            case 'h':
            case '?':
            default:
                print_help();
                exit(-1);
        }
    }
    
    
    for (i = optind; i < argc; i++)
        printf("Redundant argument - %s\n", argv[i]);
    /*
     if (argc != 5) {
     printf("Error: Invalid parameters!\n");
     printf("Usage: %s <source hostname/IP> <source port> <target hostname/IP> <target port>\n", argv[0]);
     exit(1);
     }
     */
    
    if(!tuple.destination_ip) {
        fprintf(stderr, "Cmon, you have to give me at least a destination address. Try again.\n");
        exit(-1);
    }
    
    if (!tuple.source_ip) {
        x = rand()%MAX_SRCS;
        saddress = calloc(1, sizeof(struct sa));
        rand_ipv4_gen_list(saddress, x);
    }
    
    if (!tuple.number) {
        tuple.number = 0;
    }
    
    if (!tuple.protocol) {
        tuple.protocol = 3;
    }
    
    u_int16_t src_port, dst_port;
    u_int32_t src_addr, dst_addr;
    
    dst_addr = inet_addr(tuple.destination_ip);
    
    int sd;
    char buffer[PCKT_LEN];
    struct iphdr *ip = (struct iphdr *) buffer;
    struct udphdr *udp = (struct udphdr *) (buffer + sizeof(struct iphdr));
    
    struct sockaddr_in sin;
    int one = 1;
    const int *val = &one;
    
    memset(buffer, 0, PCKT_LEN);
    
    // create a raw socket with UDP protocol
    sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sd < 0) {
        perror("socket() error");
        exit(2);
    }
    
    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("setsockopt() error");
        exit(2);
    }
    
    ip->ihl      = 5;
    ip->version  = 4;
    ip->tos      = 16; // low delay
    ip->id       = htons(rand()%54321);
    ip->ttl      = 64; // hops
    ip->protocol = 17; // UDP
    ip->daddr = dst_addr;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = dst_addr;
    time_t t;
    srand((unsigned)time(&t));

    if (tuple.protocol==1) {
        sin.sin_port = htons(SYSLOG_PORT);
        
    }
    else if (tuple.protocol == 2) {
        sin.sin_port = htons(NETFLOW_PORT);
    }
    else {
        sin.sin_port = htons(0);
    }
    
    udp->dest = sin.sin_port;
    
    while(count<tuple.number) {
        
        if (tuple.protocol==1) {
            
            w=rand()%3;
            z=rand()%30;
            fprintf(stderr, "And now, z = %d\n", z);
            
            udp->source = htons(SYSLOG_PORT);
            ip->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(syslog_data[w]);
            udp->len = htons(sizeof(struct udphdr)+strlen(syslog_data[w]));
            strncpy(&buffer[sizeof(struct iphdr) + sizeof(struct udphdr)], syslog_data[w], strlen(syslog_data[w]));
            ip->check = csum((unsigned short *)buffer,
                             sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(syslog_data[w]));
            
            if (!tuple.source_ip){
                srand(time(NULL));
                y=rand()%x;
                ptr=saddress;
                for (z=0;z<y;z++) {
                    ptr=ptr->next;
                }
                src_addr=inet_addr(ptr->sa);
                ip->saddr = src_addr;
            }
            else {
                src_addr=inet_addr(tuple.source_ip);
                ip->saddr = src_addr;
            }
            if (sendto(sd, buffer, ip->tot_len, 0,
                       (struct sockaddr *)&sin, sizeof(sin)) < 0)
            {
                perror("sendto()");
                exit(3);
            }
            fprintf(stderr, "!");
            count++;
            if (tuple.interval)
                usleep(tuple.interval*1000);
            else
                sleep(1);
            
            srand(rand()%1024);
        }
        
        else if (tuple.protocol==2) {
            
            udp->source = htons(rand()%(65000-1024));
            ip->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr) + 700;
            udp->len = htons(sizeof(struct udphdr)+700);
            strcpy(&buffer[sizeof(struct iphdr) + sizeof(struct udphdr)], flow_data);
            ip->check = csum((unsigned short *)buffer,
                             sizeof(struct iphdr) + sizeof(struct udphdr) + 700);
            
            if (!tuple.source_ip){
                srand(time(NULL));
                y=rand()%x;
                ptr=saddress;
                for (z=0;z<y;z++) {
                    ptr=ptr->next;
                }
                src_addr=inet_addr(ptr->sa);
                ip->saddr = src_addr;
            }
            else {
                src_addr=inet_addr(tuple.source_ip);
                ip->saddr = src_addr;
            }
            if (sendto(sd, buffer, ip->tot_len, 0,
                       (struct sockaddr *)&sin, sizeof(sin)) < 0)
            {
                perror("sendto()");
                exit(3);
            }
            fprintf(stderr, "!");
            count++;
            if (tuple.interval)
                usleep(tuple.interval*1000);
            else
                sleep(1);
        }
        
        else {
            
            srand(time(NULL));
            w=rand()%30;
            w=w/10;
            fprintf(stderr, "w = %d\n", w);

            srand(time(NULL));
            if (rand()%10 <=5) {
                sin.sin_port = htons(SYSLOG_PORT);
                udp->dest = sin.sin_port;
                udp->source = htons(SYSLOG_PORT);
                ip->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(syslog_data[w]);
                udp->len = htons(sizeof(struct udphdr)+strlen(syslog_data[w]));
                strncpy(&buffer[sizeof(struct iphdr) + sizeof(struct udphdr)], syslog_data[w], strlen(syslog_data[w]));
                ip->check = csum((unsigned short *)buffer,
                                 sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(syslog_data[w]));
            }
            else {
                sin.sin_port = htons(NETFLOW_PORT);
                udp->dest = sin.sin_port;
                udp->source = htons(rand()%(65000-1024));
                ip->tot_len  = sizeof(struct iphdr) + sizeof(struct udphdr) + 700;
                udp->len = htons(sizeof(struct udphdr)+700);
                strcpy(&buffer[sizeof(struct iphdr) + sizeof(struct udphdr)], flow_data);
                ip->check = csum((unsigned short *)buffer,
                                 sizeof(struct iphdr) + sizeof(struct udphdr) + 700);
            }
            
            if (!tuple.source_ip){
                srand(time(NULL));
                y=rand()%x;
                ptr=saddress;
                for (z=0;z<y;z++) {
                    ptr=ptr->next;
                }
                src_addr=inet_addr(ptr->sa);
                ip->saddr = src_addr;
            }
            else {
                src_addr=inet_addr(tuple.source_ip);
                ip->saddr = src_addr;
            }
            if (sendto(sd, buffer, ip->tot_len, 0,
                       (struct sockaddr *)&sin, sizeof(sin)) < 0)
            {
                perror("sendto()");
                exit(3);
            }
            fprintf(stderr, "!");
            count++;
            if (tuple.interval)
                usleep(tuple.interval*1000);
            else
                sleep(1);
        }
        
    }
    fprintf(stderr, "\n");
    close(sd);
    return 0;
}
