## dvbtee & libdvbtee

#### Introduction

libdvbtee is a stream parser and service information aggregator library for
MPEG2 transport streams. The library includes a program service information
(PSI) parser and support for various network streaming methods and is aware
of the linux-dvb kernel API as well as HDHomeRun network streaming APIs.

The library contains enough functionality to power a full featured television
middleware application, including the ability to acquire and stream data
through UDP, TCP, HTTP, DMA and various other mechanisms.

`libdvbtee_server` allows for these features to be controlled from an http
session or command line interface, and can be used to control multiple tuners.

The dvbtee executable application provides a variety of features, including:
scanning, tuning, EPG browsing, receive incoming network streams,
stream out network streams, manage multiple tuners, and more...

The server can be accessed either by telnet or http. The default port number
is `64080`, but this is configurable. The `server_example` uses port number `62080`.

When accessing the server via http, precede commands by / and arguments by =.
When accessing the server via telnet, enter in the form 'command=argument'
and press enter.

Michael Ira Krufky  
mkrufky at linuxtv dot org

#### Command line arguments:
```
-a      adapter id
-A      (1 for ATSC, 2 for ClearQAM)
-b      display bitrates & statistics
-c      channel to tune /
        comma (,) separated list of channels to scan /
        scan minimum channel
-C      channel to tune /
        comma (,) separated list of channels to scan /
        scan maximum channel
-f      frontend id
-F      filename to use as input
-t      timeout
-T      number of tuners (dvb adapters) allowed to use, 0 for all
-s      scan, optional arg when using multiple tuners:
        1 for speed, 2 for redundancy,
        3 for speed AND redundancy,
        4 for optimized speed / partial redundancy
-S      server mode, optional arg 1 for command server,
        2 for http stream server, 3 for both
-i      pull local/remote tcp/udp port for data
-I      request a service and its associated PES streams by its service id
-E      enable EPG scan, optional arg to limit the number of EITs to parse
-o      output filtered data, optional arg is a filename / URI, ie udp://127.0.0.1:1234
-O      output options: (or-able) 1 = PAT/PMT, 2 = PES, 4 = PSIP
-H      use a HdHomeRun device, optional arg to specify the device string
-j      enable json output of decoded tables & descriptors
-d      debug level
-h      display additional help
```
#### Example Usage

To tune to service id 1 of physical channel 33 and stream it to a udp port:
```
  dvbtee -c33 -I1 -oudp://192.168.1.100:1234
```

To tune the second frontend of adapter 1 and stream the full TS of physical channel 44 to a tcp listener:
```
  dvbtee -c44 -otcp://192.168.1.200:5555
```

To listen to a TCP port and stream to a UDP port:
```
  dvbtee -itcp://5555 -oudp://192.168.1.100:1234
```

To parse a captured file and filter out the PSIP data, saving the PAT/PMT and PES streams to a file:
```
  dvbtee -Finput.ts -O3 -ofile://output.ts
```

To parse a UDP stream for ten seconds:
```
  dvbtee -iudp://127.0.0.1:1234 -t10
```

To scan for ClearQAM services using 5 tuners optimized for speed and partial redundancy:
```
  dvbtee -A2 -T5 -s4
```

To scan for ATSC services using 2 HdHomeRun tuners optimized for speed and redundancy:
```
  dvbtee -A1 -H -T2 -s3
```

To start a server using adapter 0:
```
  dvbtee -a0 -S
```

To start a server using tuner1 of a specific HdHomeRun device (ex: ABCDABCD):
```
  dvbtee -H ABCDABCD-1 -S
```

#### Server commands

```
tuner           specify tuner id
feeder          specify feeder id
scan            scan for services, optional arg comma (,) separated list of channels to scan
scanepg         same as 'scan' with EPG data collection enabled during scan
startscan       same as 'scan' but asynchronous
startscanepg    combo of 'scanepg' and 'startscan'
tune            tune to a channel or service, ie:
                tune=33 (full mux) / tune=44~3 (svc id 3)
channels        list channels
channel         tune physical channel (unsafe - use tune instead)
service         select service (unsafe - use tune instead)
stream          add output file / tcp / udp / http stream, ie:
                stream=udp:192.168.1.100:1234
                stream=tcp:192.168.1.200:5555
                stream=file:output.ts
                stream (over http with no arg will begin streaming to current socket)
epg             dump collected EPG data
xmltv           dump collected EPG data in XMLTV format
stop            stop the tuner
stopoutput      stop the tuner and output
check           display debug info if debug is enabled
debug           enable debug
parser          enable / disable the parser for data shovel
listen          listen for TS on a TCP or UDP port
save            save scanned channels
quit            stop the server and exit
```
