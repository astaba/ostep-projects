
# Projects for an Operating Systems Class

This repository holds a number of projects that can be used in an operating
systems class aimed at upper-level undergraduates and (in some cases)
beginning graduate students. They are based on years of teaching such a course
at the University of Wisconsin-Madison.

Also (increasingly) available are some tests to see if your code works; eventually
every project will have a set of tests available. The testing framework that is
currently available is found [here](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/tester).
A specific testing script, found in each project directory, can be used to run
the tests against your code. 

For example, in the initial utilities project, the relatively simple `wcat`
program that you create can be tested by running the `test-wcat.sh` script.
This could be accomplished by the following commands:
```sh
prompt> git clone https://github.com/remzi-arpacidusseau/ostep-projects
prompt> cd ostep-projects/initial-utilities/wcat
prompt> emacs -nw wcat.c 
prompt> gcc -o wcat wcat.c -Wall 
prompt> ./test-wcat.sh
test 1: passed
test 2: passed
test 3: passed
test 4: passed
test 5: passed
test 6: passed
test 7: passed
prompt> 
```
Of course, this sequence assumes (a) you use `emacs` (you should!), (b) your
code is written in one shot (impressive!), and (c) that it works perfectly
(well done!). Even for simple assignments, it is likely that the
compile/run/debug cycle might take a few iterations.

## C/Linux Projects

### Initial Projects

These projects are meant to get you warmed up with programming in the C/UNIX
environment. None are meant to be particularly hard, but should be enough so
that you can get more comfortable programming. 

Realize the best thing you can do to learn to program in any environment is to
program **a lot**. These small projects are only the beginning of that
journey; you'll have to do more on your own to truly become proficient.

* [Unix Utilities](initial-utilities) (cat, grep, zip/unzip)
* Sort (text-based)
* Sort (binary)
* [Reverse](initial-reverse) (very simple reverse program)

### Processes and Scheduling

* [Shell](processes-shell)

### Virtual Memory

* Memory Allocator

### Concurrency

* [Web Server](concurrency-webserver)
* [Parallel Zip](concurrency-pzip)
* [MapReduce](concurrency-mapreduce)
* Web Crawler

### File Systems

* [File System Checker](filesystems-checker)

### Distributed Systems


## Kernel Hacking Projects (xv6)

These projects all are to be done inside the
[xv6](https://pdos.csail.mit.edu/6.828/2017/xv6.html) kernel based on an early
version of Unix and developed at MIT. Unlike the C/Linux projects, these give
you direct experience inside a real, working operating system (albeit a simple
one).

Read the [install notes](INSTALL-xv6.md) to see how to download the latest xv6 
and install the tools you'll need.

### Initial Projects

* [Intro To xv6](initial-xv6)

### Processes and Scheduling

* [Scheduling (Lottery)](scheduling-xv6-lottery)

### Virtual Memory

* [Virtual Memory (Null Pointer and Read-Only Regions)](vm-xv6-intro)

### Concurrency

* [Kernel Threads (Basic Implementation)](concurrency-xv6-threads)

### File Systems

## Practice order

```bash
tree -d -L 1
.
├── 1.  initial-utilities            (done)
├── 2.  initial-reverse              (done)
├── 3.  initial-kv                   (done)
├── 4.  initial-memcached
├── 5.  processes-shell
├── 6.  concurrency-pzip
├── 7.  concurrency-sort
├── 8.  concurrency-webserver
├── 9.  concurrency-mapreduce
├── 10. initial-xv6
├── 11. initial-xv6-tracer
├── 12. vm-xv6-intro
├── 13. concurrency-xv6-threads
├── 14. scheduling-xv6-lottery
├── 15. filesystems-checker
├── 16. filesystems-distributed
├── 17. filesystems-distributed-ufs
└── tester

19 directories
```

Here is the combined, recommended order that aligns with the book's structure
and a natural learning curve, incorporating all the projects from this `README`
file.

---

### 1. Foundational C & Utilities

These projects are excellent for getting comfortable with the C/UNIX environment,
command-line arguments, and basic I/O.

* `initial-utilities` (cat, grep, etc.)
* `initial-reverse`
* `initial-kv`
* `initial-memcached`

### 2. Processes

This is the heart of the first major section of the book.

* `processes-shell` (Build your own shell)

### 3. Virtual Memory & Threads (User Space)

This section introduces memory allocation and concurrency outside of the kernel.

* `Memory Allocator` (This project is not in a specific directory in your `tree`
but is a common assignment in the book's VM chapter)

* `concurrency-pzip`
* `concurrency-sort`
* `concurrency-webserver`
* `concurrency-mapreduce`

### 4. Kernel Hacking (xv6)

These are the most advanced projects, where you'll modify the kernel itself.
It's best to tackle them after you've mastered the concepts in user space.

* `initial-xv6`
* `vm-xv6-intro` (Virtual Memory project inside xv6)
* `concurrency-xv6-threads` (Kernel Threads)
* `scheduling-xv6-lottery`
* `initial-xv6-tracer`

### 5. File Systems

This is the final major section, dealing with on-disk data structures.

* `filesystems-checker`
* `filesystems-distributed`
* `filesystems-distributed-ufs`

This unified roadmap provides a clear path forward, building from simpler
concepts to more complex ones. It should make the project sequence much easier to follow.

