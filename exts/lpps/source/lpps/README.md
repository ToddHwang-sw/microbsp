
## Getting Started 

LPPS is the implementation of PPS(Persistent Publish and Subscription System) which is originally supported by QNX real-time operating system on top of Linux operating system. 
LPPS is based on LIBFUSE library and interacts with kernel libfuse file system framework. 
It can be activated with super-user priviledges. 
Simply just by now, the following things are tested and released. 

* touch 
* cp 
* mv 
* ln 
* cd
* rm 
* mkdir 
* rmdir
* cat 
* more 
* cat <filename>?wait / ?wait,delta 
* cat .all?deltadir,wait
* cat .memory
* vi (editor) 

## References 

* [Linux LIBFUSE](https://github.com/libfuse/libfuse)
* [QNX PPS](http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.qnxcar2.ppsref%2Ftopic%2Fhowppsworks.html)

## Build and Running

1. Build

   ```sh
   ./build.sh 
   ./configure --prefix= 
   make 
   ```

2. Running (if your user name is "todd") 
 
   ```sh
   # mkdir /var/tmp/lpps     <mount point> 
   # sudo ./lpps --user=todd --conf=./lpps.conf /var/tmp/lpps 
   ```

   "sudo" user level hosting is required since LPPS is based on Linux libfuse. 

3. Daemonizing
 
   ```sh
   # sudo ./lpps --user=todd --conf=./lpps.conf --daemon=1 /var/tmp/lpps 
   ```

   "--dameon=0/1" option determines to spawn up the lpps as a system daemon or not. 
   When it is forked as a daemon, the logging will be done through "syslog" and we can see the log by using the following command. 
   All lines of text log begins with "[LPPS]". 

   ```sh
   # sudo tail -f /var/log/syslog | grep LPPS 
   ```

4. Syslog support 

   ```sh
   # sudo ./lpps --user=todd --conf=./lpps.conf --syslog=1 --daemon=1 /var/tmp/lpps 
   ```
   "--syslog=1" enforces log messages to be directed to /var/log/syslog\* files. 
   User can trace the log with the following command. 

   ```sh
   # sudo tail -f /var/log/syslog 
   ```

## Supporting memory debug \& Directory tracking 

1. Memory debug

   ```sh
   # cd /var/tmp/lpps
   # cat .memory
   ```

   Filename ".memory" is used as an internal file to print out memory utilization. 
   ".memory" file cannot be created and we can change the name of this internal file 
   in lpps.conf. User can see "memtrace =" option in lpps.conf. 

1. Directory tracking 

   ```sh
   # cd /var/tmp/lpps
   # cat .all?deltadir,wait 
   ```

   Filename ".all" is used as an internal file to monitor activities; directory creation/deletion, 
   file creation/deletion.  ".all" file cannot be created and we can change the name of this internal 
   file in lpps.conf. User can see "acctrace =" option in lpps.conf. 

## Testing

   ```sh
    # cd /var/tmp/lpps
    # touch hello
    # cat hello
    @hello
    # cat hello?wait &
    # echo "num1:n:100" >> hello
    @hello
    num1:n:100
    # 
    # echo "num2:n:200" >> hello
    @hello
    num1:n:100
    num2:n:200
    # 
    # echo "num1:n:400" >> hello
    @hello
    num1:n:400
    num2:n:200
    #

    ```

   ```sh
    # cd /var/tmp/lpps
    # cat .all?deltadir,wait &
    # mkdir LL
    +@LL\ 
    # touch myfile
    +@myfile 
    # rm myfile
    -@myfile
    # rmdir LL
    -@LL\

    ```

Currently, Linux 6.6.22 for X86 64 Qemu mode does not accept "default" direct IO mode. User should has to use "cat -n " instead of "cat", i.e, "cat -n hello?wait &". 


      
