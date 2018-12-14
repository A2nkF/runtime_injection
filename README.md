# runtime_injection

## Description

This code starts a new thread in a running process and executes shellcode.
i'm using `task_for_pid` to get the task port and mach_vm_read/write to read/write to 
the memory.

## Note

You need to be root in order to call `task_for_pid()`

## Build

`gcc main.c -o main`
