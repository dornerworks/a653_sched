# Xen ARINC653 Scheduling Utility

This project provides a way to set an arinc653 schedule from the command line.

## Build Instructions

These build instructions assume that you are using the Xen Xilinx Distribution
and following the XZD User Manual.

1.	`cd $RELEASE_DIR`
2.  `git clone https://github.com/dornerworks/a653_sched`
3.  `cd a653_sched`
4.	`make`

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

`./a653_sched -p 1 dom1:10 :20 dom2:30`

### Domain Configuration

This tool assumes that each domain in the schedule (except for Dom0) has
been assigned a UUID that is representable as an ASCII string.

To set a domain's UUID accordingly:

- Convert the domain's name to hex
```
echo -n "dom1" | xxd -p
646f6d31
```
- Format the hex name as a UUID and assign it to the domain in the xen config
  file for the domain.
`uuid = "646f6d31-0000-0000-0000-000000000000"`

### Pool Id

The ID of a scheduling pool can be found by running the command
`xenstore-ls /local/pool` after the pool has been created.  The initial pool
has an id of 0.

### Help
```
Usage: ./a653_sched <domname:runtime> ...

	All times are in milliseconds
	Major Frame is the sum of all runtimes
Options:
	--help|-h		display this usage information
	--pool|-p		pool id
```
