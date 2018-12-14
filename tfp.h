//
//  tfp.h
//  Mach_Messages
//
//  Created by A2nkF on 8/29/18.
//  Copyright © 2018 A2nkF. All rights reserved.

#ifndef tfp_h
#define tfp_h
// Make logs colorful
#define printc(c,f,s) printf ("\033[%dm" f "\033[0m", 30 + c, s);
#define fail() printc(1, "%s","[-] ")
#define good() printc(2, "%s","[+] ")
#define goodn() printc(2, "%s","[+]\n")
#define warn() printc(3, "%s","[!] ")
#define que() printc(4, "%s","[?] ")
#define pend() printc(5, "%s","[~] ")
#define input() printc(6,  "%s","[>] ")
#define info() printc(7, "%s","[i] ")


typedef struct {
    mach_msg_header_t header;
    char content[1024];
} our_msg;

typedef struct {
    mach_msg_header_t header;
    char content[1024];
    mach_msg_trailer_t trailer;
} rcv_msg;


mach_msg_return_t sendMessage(char *content, mach_port_t from, mach_port_t to);
mach_msg_return_t rcvMessage(char *outputContent, mach_port_t rcvPort);
mach_port_t connect_to_service(const char* service_name);
mach_port_t task_for_pid_workaround(int Pid);
#endif
