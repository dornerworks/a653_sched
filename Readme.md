# Xen ARINC653 Scheduling Utility

This project provides a way to set an arinc653 schedule from the command line.

## Build Instructions

These build instructions assume that you have a properly installed and sourced
Xen Xilinx Distribution Yocto toolchain.

1.  `git clone https://github.com/dornerworks/a653_sched`
2.  `cd a653_sched`
3.	`make`

## Run Instructions

Using a653\_sched we can assign sequential minor frames by specificying the
domain to run and its time slice in ms with a
"\<domain name\>:\<time slice in ms\>" pair.  The major frame is set to
the sum of all minor frames and if Dom0 is in the arinc653 pool it uses
idle time for its' processing.

For example, the following command will create a schedule with three minor
frames and a 60 ms major frame.  The two guest VMs have 10 ms and 30 ms
minor frames respectively, while a 20 ms minor frame is given to idle
processing.

`./a653_sched -p arinc_pool dom1:10 :20 dom2:30`

### Domain UUID

This tool assumes that each domain in the schedule (except for Dom0) has
been assigned a UUID in it's xen config file.  For example:

```
name = "dhyi"
uuid = "64687969-0000-0000-0000-000000000000"
```

This UUID will be retreived from the xenstore as long as the domain has
been created for this platform at least once in the past.

If a UUID is not assigned to the domain, the uuid may change each time the
domain is created; a situation which this tool is not able to handle.

If you have already created the domain previously without setting a UUID
you will need to clear the domain information cached in the xenstore
to avoid UUID conflicts.  To do this run the `xenstore-rm /vm`
command in dom0.

### Help
```
Usage: ./a653_sched <domname:runtime> ...

	All times are in milliseconds
	Major Frame is the sum of all runtimes
Options:
	--help|-h		display this usage information
	--pool|-p		pool name
```
