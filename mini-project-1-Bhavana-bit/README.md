[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/fkrRRp25)

# Custom Shell Project

## Overview
This is a custom Unix shell implemented in C. It supports basic shell functionalities, intrinsic commands, input/output redirection, piping, background execution, and advanced job control.

---

## Features

### A. Basic Shell Functionality
- Display prompt: `<username@hostname:current_path>`
- Read and parse user input with whitespace handling.
- Validate commands against a context-free grammar.

### B. Shell Intrinsics
- **hop**: Change current working directory (`~`, `.`, `..`, `-`, or relative/absolute paths)
- **reveal**: List files/directories with flags `-a` (all) and `-l` (line by line)
- **log**: Store and manage command history (up to 15 commands, supports `execute <index>` and `purge`)

### C. File Redirection and Pipes
- Input redirection (`<`) and output redirection (`>`, `>>`)
- Command piping (`|`)
- Handles multiple redirections correctly (last redirection takes effect)
- Supports combined redirection and piping

### D. Sequential and Background Execution
- Sequential execution using `;` operator
- Background execution using `&` operator
- Tracks background jobs, prints job numbers and PIDs
- Reports normal/abnormal termination of background processes

### E. Exotic Shell Intrinsics
- **activities**: Lists all active processes spawned by the shell, sorted lexicographically
- **ping <pid> <signal_number>**: Sends a signal to a process
- Job control via `Ctrl-C`, `Ctrl-Z`, `fg`, and `bg`

---

## Compilation

```bash
gcc -Wall -O2 -o shell src/*.c
make all



# Networking Mini-Project: TCP-like Reliable UDP

## Project Overview

This project implements a reliable UDP communication protocol with features similar to TCP, including:

- **Three-way handshake** (SYN, SYN-ACK, ACK) for connection setup.
- **File transfer mode** (sending/receiving binary files reliably).
- **Chat mode** (real-time message exchange between client and server).
- **Graceful connection termination** using FIN/ACK.
- **Logging** of key events (`RUDP_LOG`).

The project uses **C language**, POSIX sockets, and `select()` for handling multiple events in chat mode.

---

## Directory Structure

1)For shell prompt:
-I have 3 folders inside shell that is src,include,makefile.
-In src i have files:
1.prompt.c
2.parser.c
3.hop.c
4.log.c
5.reveal.c
6.background.c
7.exec.c
8.job.c
9.exotic.c
10.main.c
-In include folder i have files:
1.prompt.h
2.parser.h
3.hop.h
4.log.h
5.reveal.h
6.background.h
7.exec.h
8.job.h
9.exotic.h
-I run this using make all and ./shell.out


2)In networking I have 3 files:
1.sham.h
2.server.c
3.client.c
I run this by taking 2 terminals and running ./server and ./client with some port numbers.

Scheduler Modifications

### Supported Modes
- `FCFS`: First Come First Serve — non-preemptive, run-to-completion
- `CFS`: Completely Fair Scheduler — uses `vruntime` and `weight`
- `Round Robin`: Default xv6 scheduler

### Activation
Set the scheduler mode via Makefile:
```bash
make SCHEDULER=FCFS V=1 or make qemu SCHEDULER=FCFS
make SCHEDULER=CFS V=1 OR  make qemu SCHEDULER=FCFS
make V=1  / make qemu # for Round Robin

