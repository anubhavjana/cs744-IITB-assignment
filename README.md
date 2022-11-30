# cs744-IITB-assignment
This repo includes all the assignments that I solved in the course "Design and Engineering of Computng Systems" (CS 744) in Autumn 2022-23 taught by Prof. Mythili Vukuturu

Course 

**Valgrind assingment** - checks runnng linux commands as given in the pdf file corresponding to it including gdb and valgrind. Check memory leaks using Valgrind and run debugger using GDB

**Shell assignment** - build a shell from scratch user basic functionalities - like ls,cat, signal handling to catch CTRL+C signal, grouping of foreground and backgroud processes etc

**Socket Programming** - build a client-server model from scratch. Uses the concept of master-worker threads, threadpool,queues (where server will take accept a conect request and place the socket fd in queue, worker threads will dequeue it and process it)

**Load Testing** - uses a closed-loop testing by sending request from concurrent users (along with test duration and think time) and calculate throughput, average response time, check for bottlenecks and plot graphs for throuhput and RTT against number of concurrent users
