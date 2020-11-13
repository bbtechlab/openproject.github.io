# BBLOGGER - Embedded Logger Framework

## Features ###
![alt text](https://github.com/bbtechlab/bblogger/blob/master/docs/BBLOGGER_v1.0.jpg)

## Document ##

Contact to bamboo@bbtechlab.com to get full documents.

## Using BBLOGGER ##

### Clone repository ###
```sh
bamboo@BBTECHLAB:~$ git clone https://github.com/bbtechlab/bblogger
bamboo@BBTECHLAB:~/bblogger$ tree -L 3 ./
./
├── apps
│   └── liblogger
│       ├── expliblogger_debug.c
│       ├── expliblogger_error.c
│       ├── expliblogger_main.c
│       ├── expliblogger_message.c
│       ├── expliblogger_trace.c
│       ├── expliblogger_warn.c
│       └── Makefile
├── docs
│   └── BBLOGGER_v1.0.jpg
├── liblogger
│   ├── bbliblogger.c
│   ├── bbliblogger_cmd.c
│   ├── bbliblogger_layout.c
│   ├── include
│   │   ├── bbliblogger_cmd.h
│   │   ├── bbliblogger.h
│   │   └── bbliblogger_layout.h
│   └── Makefile
├── libos
│   ├── bblibos_event.c
│   ├── bblibos_mutex.c
│   ├── bblibos_sem.c
│   ├── bblibos_thr.c
│   ├── bblibos_tsk.c
│   ├── include
│   │   ├── bbliberr.h
│   │   ├── bblibos.h
│   │   └── bblibtype.h
│   └── Makefile
├── libutils
│   ├── bblibutils_list.c
│   ├── bblibutils_que.c
│   ├── bblibutils_ringBuffer.c
│   ├── bblibutils_shareMemory.c
│   ├── bblibutils_stack.c
│   ├── include
│   │   ├── bbliblist.h
│   │   └── bblibutils.h
│   └── Makefile
├── make
│   ├── Dependency.mak
│   ├── Makefile
│   ├── Makefile.app
│   ├── Makefile.lib
│   ├── output
│   │   ├── bin
│   │   └── lib
│   ├── Rule.mak
│   ├── so.lst
│   └── Toolset.mak
└── README.md

13 directories, 40 files

``` 
### Compile ###
```sh
bamboo@BBTECHLAB:~$ cd ~/bblogger/make
bamboo@BBTECHLAB:~/bblogger/make$ make world
```
The output locate as below
```sh 
bamboo@BBTECHLAB:~/bblogger/make$ tree -L 3 ./
output/
├── bin
│   ├── bblog_testapp
│   └── expliblogger
└── lib
    ├── libbblogger.a
    └── libbblogger.so

2 directories, 4 files
```
### Demo ###
#### Show information ####
```sh 
$ echo show > /tmp/.bbliblogger.cfg
example)
bamboo@BBTECHLAB:~/bblogger/make/output/bin$ ./bblog_testapp &
bamboo@BBTECHLAB:~/bblogger/make/output/bin$ echo show > /tmp/.bbliblogger.cfg
```
Then you shall see the log print out as below
![alt text](https://github.com/bbtechlab/bblogger/blob/master/docs/BBLOGGER_DEMO_v1.0.jpg)
#### Control log level ####
```sh 
$ echo level:[level]:[module name] > path of configuration file
example)
$ echo level:trace:BBLOG_EXAMPLE_ERROR > /tmp/.bbliblogger.cfg
$ echo level:dbg:BBLOG_EXAMPLE_ERROR > /tmp/.bbliblogger.cfg
$ echo level:msg:BBLOG_EXAMPLE_ERROR > /tmp/.bbliblogger.cfg
$ echo level:wrn:BBLOG_EXAMPLE_ERROR > /tmp/.bbliblogger.cfg
```
#### Control log output level ####
```sh 
echo print:[level]:[module name] > path of configuration file
example)
$ echo print:all:BBLOG_EXAMPLE_ERROR > /tmp/.bbliblogger.cfg
$ echo print:console:BBLOG_EXAMPLE_ERROR > /tmp/.bbliblogger.cfg
$ echo print:file:BBLOG_EXAMPLE_ERROR > /tmp/.bbliblogger.cfg
```
#### Control log format (optional)  ####
```sh 
echo fmt:[format] > path of configuration file
```
