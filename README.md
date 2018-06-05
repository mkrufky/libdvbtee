## dvbtee & libdvbtee

[![Build Status](https://travis-ci.org/mkrufky/libdvbtee.svg?branch=master)](https://travis-ci.org/mkrufky/libdvbtee)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/mkrufky-libdvbtee/badge.svg)](https://scan.coverity.com/projects/mkrufky-libdvbtee)

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

`libdvbtee_parser` is a library built from [libdvbtee](https://github.com/mkrufky/libdvbtee) sources for use by [node-dvbtee](https://www.npmjs.com/package/dvbtee)
`libdvbtee_parser` is hosted within the same git development repository as `libdvbtee` and as such,
they share this same single README.md file.

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

#### How to build / run
The recommended way to build libdvbtee is by using the `build-auto.sh` script:
```
./build-auto.sh
dvbtee/dvbtee < mpegfile.ts
```

#### Other build methods
libdvbtee was originally developed using the qmake build system for easy
multi-platform build support. libdvbtee still supports the qmake system,
but now uses autotools as the preferred build system for various reasons.
```
autoreconf --install
./configure
make
dvbtee/dvbtee < mpegfile.ts
```

If, for some odd reason, you want to build libdvbtee using the old version of
the table / descriptor decoder, specify --enable-olddecoder to configure
```
autoreconf --install
./configure --enable-olddecoder
make
dvbtee/dvbtee < mpegfile.ts
```

If you prefer to use qmake rather than autotools, use the following commands:
```
qmake -r
make
LD_LIBRARY_PATH=libdvbtee:libdvbtee_server dvbtee/dvbtee < mpegfile.ts
```

If, for some odd reason, you want to build libdvbtee using the old version of
the table / descriptor decoder, specify CONFIG+=olddecoder to qmake
```
qmake -r CONFIG+=olddecoder
make
LD_LIBRARY_PATH=libdvbtee:libdvbtee_server dvbtee/dvbtee < mpegfile.ts
```

libdvbtee depends on libdvbpsi for PSIP parsing.
Although libdvbtee will build against older libdvbpsi releases,
a more recent build is required in order to provide
all of libdvbtee's latest features, such as ATSC support.

libdvbtee's build system is capable of building and linking
against the latest version of libdvbpsi by placing a copy of the
libdvbpsi repository in the top level of libdvbtee's source tree.

Rather than giving step-by-step instructions in this README,
shell scripts are provided that will fetch the latest libdvbpsi sources,
configure the build system, and build both libraries together.

For the integrated build of libdvbtee along with the latest version of libdvbpsi using autotools (recommended):
```
./build-auto.sh
dvbtee/dvbtee < mpegfile.ts
```

For the integrated build of libdvbtee along with the latest version of libdvbpsi using qmake:
```
./build-qmake.sh
LD_LIBRARY_PATH=libdvbtee:libdvbtee_server dvbtee/dvbtee < mpegfile.ts
```

#### Command line arguments
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
-n      bind to a specific network interface
-I      request a service and its associated PES streams by its service id
-E      enable EPG scan, optional arg to limit the number of EITs to parse
-e      enable ETT extended text tables (EPG descriptions, ATSC only)
-o      output filtered data, optional arg is a filename / URI, ie udp://127.0.0.1:1234
-O      output options: (or-able) 1 = PAT/PMT, 2 = PES, 4 = PSIP
-H      use a HdHomeRun device, optional arg to specify the device string
-j      enable json output of decoded tables & descriptors
-d      debug level
-q      quiet most logging
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

To scan for services using the first connected tuner:
```
  dvbtee -s -a0
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

#### Example output:

To scan a channel with JSON output enabled, the following command:
```
  dvbtee -s -c13 -j
```
...should generate output similar to the following:
```
scan channel 13...
store PAT: v0, ts_id: 2011
           3 | 30
           4 | 40
           5 | 50
store PMT: v12, service_id 3, pcr_pid 49
  es_pid | type
      31 | 0x02 (Video MPEG-2) |
      34 | 0x81 (Audio AC3 (ATSC)) | eng
      35 | 0x81 (Audio AC3 (ATSC)) | spa
      36 | 0x81 (Audio AC3 (ATSC)) | fre
store VCT: v6, ts_id 2011, b_cable_vct 0
  channel | service_id | source_id | service_name
     13.1 |          3 |         1 | WNET-HD | eng, spa, fre
     13.2 |          4 |         2 | KIDS | eng, spa, fre
     13.3 |          5 |         3 | V-Me | eng
store MGT: v6
  table type |   pid  | ver | bytes
    0x0000   | 0x1ffb |   6 | 187
    0x0100   | 0x1d00 |   6 | 1103
    0x0101   | 0x1d01 |   6 | 1105
    0x0102   | 0x1d02 |   6 | 1132
    0x0103   | 0x1d03 |   6 | 1305
    0x0200   | 0x1e00 |   6 | 1197
    0x0201   | 0x1e01 |   6 | 1220
    0x0202   | 0x1e02 |   6 | 1157
    0x0203   | 0x1e03 |   6 | 1213
store PMT: v12, service_id 5, pcr_pid 81
  es_pid | type
      51 | 0x02 (Video MPEG-2) |
      54 | 0x81 (Audio AC3 (ATSC)) | eng
store PMT: v12, service_id 4, pcr_pid 65
  es_pid | type
      41 | 0x02 (Video MPEG-2) |
      44 | 0x81 (Audio AC3 (ATSC)) | eng
      45 | 0x81 (Audio AC3 (ATSC)) | spa
      46 | 0x81 (Audio AC3 (ATSC)) | fre

# channel 13, 213000000,
13.1-WNET-HD:213000000:8VSB:49:52:3
13.2-KIDS:213000000:8VSB:65:68:4
13.3-V-Me:213000000:8VSB:81:84:5
found 3 services

TSID#07db: [ { "programs": [ { "number": 3, "pid": 48 }, { "number": 4, "pid": 64 }, { "number": 5, "pid": 80 } ], "tableId": 0, "tableName": "PAT", "tsId": 2011, "version": 0 }, { "pcrPid": 49, "program": 3, "streams": [ { "pid": 49, "streamType": 2, "streamTypeString": "Video MPEG-2" }, { "descriptors": [ { "bitRateCode": 14, "bsid": 8, "bsmod": 0, "description": "", "descriptorTag": 129, "fullSvc": true, "language": "eng", "numChannels": "2/0", "sampleRate": "48", "surroundMode": "Not indicated" }, { "ISO639Lang": [ { "audioType": 0, "language": "eng" } ], "descriptorTag": 10 } ], "pid": 52, "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "descriptors": [ { "bitRateCode": 8, "bsid": 8, "bsmod": 0, "description": "", "descriptorTag": 129, "fullSvc": true, "language": "spa", "numChannels": "2/0", "sampleRate": "48", "surroundMode": "Not indicated" }, { "ISO639Lang": [ { "audioType": 0, "language": "spa" } ], "descriptorTag": 10 } ], "pid": 53, "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "descriptors": [ { "bitRateCode": 8, "bsid": 8, "bsmod": 2, "description": "", "descriptorTag": 129, "fullSvc": true, "language": "fre", "numChannels": "2/0", "sampleRate": "48", "surroundMode": "Not indicated" }, { "ISO639Lang": [ { "audioType": 0, "language": "fre" } ], "descriptorTag": 10 } ], "pid": 54, "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" } ], "tableId": 2, "tableName": "PMT", "version": 12 }, { "channels": [ { "accessControlled": false, "carrierFreq": 0, "descriptors": [ { "descriptorTag": 161, "serviceLocation": [ { "esPid": 49, "streamType": 2, "streamTypeString": "Video MPEG-2" }, { "esPid": 52, "lang": "eng", "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "esPid": 53, "lang": "spa", "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "esPid": 54, "lang": "fre", "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" } ] } ], "etmLocation": 0, "hidden": false, "hideGuide": false, "major": 13, "minor": 1, "modulation": 4, "outOfBand": true, "pathSelect": true, "program": 3, "serviceName": "WNET-HD", "serviceType": 2, "sourceId": 1, "tsId": 2011 }, { "accessControlled": false, "carrierFreq": 0, "descriptors": [ { "descriptorTag": 161, "serviceLocation": [ { "esPid": 65, "streamType": 2, "streamTypeString": "Video MPEG-2" }, { "esPid": 68, "lang": "eng", "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "esPid": 69, "lang": "spa", "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "esPid": 70, "lang": "fre", "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" } ] } ], "etmLocation": 0, "hidden": false, "hideGuide": false, "major": 13, "minor": 2, "modulation": 4, "outOfBand": true, "pathSelect": true, "program": 4, "serviceName": "KIDS", "serviceType": 2, "sourceId": 2, "tsId": 2011 }, { "accessControlled": false, "carrierFreq": 0, "descriptors": [ { "descriptorTag": 161, "serviceLocation": [ { "esPid": 81, "streamType": 2, "streamTypeString": "Video MPEG-2" }, { "esPid": 84, "lang": "eng", "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" } ] } ], "etmLocation": 0, "hidden": false, "hideGuide": false, "major": 13, "minor": 3, "modulation": 4, "outOfBand": true, "pathSelect": true, "program": 5, "serviceName": "V-Me", "serviceType": 2, "sourceId": 3, "tsId": 2011 } ], "isCableVCT": false, "tableId": 200, "tableName": "VCT", "tsId": 2011, "version": 6 }, { "tableId": 199, "tableName": "MGT", "tables": [ { "bytes": 187, "pid": 8187, "type": 0, "version": 6 }, { "bytes": 1103, "pid": 7424, "type": 256, "version": 6 }, { "bytes": 1105, "pid": 7425, "type": 257, "version": 6 }, { "bytes": 1132, "pid": 7426, "type": 258, "version": 6 }, { "bytes": 1305, "pid": 7427, "type": 259, "version": 6 }, { "bytes": 1197, "pid": 7680, "type": 512, "version": 6 }, { "bytes": 1220, "pid": 7681, "type": 513, "version": 6 }, { "bytes": 1157, "pid": 7682, "type": 514, "version": 6 }, { "bytes": 1213, "pid": 7683, "type": 515, "version": 6 } ], "version": 6 }, { "pcrPid": 81, "program": 5, "streams": [ { "descriptors": [ { "CaptionService": [ { "digital_cc": "708", "easyReader": false, "iso639lang": "eng", "line21field": true, "serviceNumber": 1, "wideAspectRatio": false } ], "descriptorTag": 134 } ], "pid": 81, "streamType": 2, "streamTypeString": "Video MPEG-2" }, { "descriptors": [ { "bitRateCode": 10, "bsid": 8, "bsmod": 0, "description": "", "descriptorTag": 129, "fullSvc": true, "language": "eng", "numChannels": "2/0", "sampleRate": "48", "surroundMode": "Not indicated" }, { "ISO639Lang": [ { "audioType": 0, "language": "eng" } ], "descriptorTag": 10 } ], "pid": 84, "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" } ], "tableId": 2, "tableName": "PMT", "version": 12 }, { "pcrPid": 65, "program": 4, "streams": [ { "descriptors": [ { "CaptionService": [ { "digital_cc": "708", "easyReader": false, "iso639lang": "eng", "line21field": true, "serviceNumber": 1, "wideAspectRatio": false } ], "descriptorTag": 134 } ], "pid": 65, "streamType": 2, "streamTypeString": "Video MPEG-2" }, { "descriptors": [ { "bitRateCode": 14, "bsid": 8, "bsmod": 0, "description": "", "descriptorTag": 129, "fullSvc": true, "language": "eng", "numChannels": "2/0", "sampleRate": "48", "surroundMode": "Not indicated" }, { "ISO639Lang": [ { "audioType": 0, "language": "eng" } ], "descriptorTag": 10 } ], "pid": 68, "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "descriptors": [ { "bitRateCode": 8, "bsid": 8, "bsmod": 0, "description": "", "descriptorTag": 129, "fullSvc": true, "language": "spa", "numChannels": "2/0", "sampleRate": "48", "surroundMode": "Not indicated" }, { "ISO639Lang": [ { "audioType": 0, "language": "spa" } ], "descriptorTag": 10 } ], "pid": 69, "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" }, { "descriptors": [ { "bitRateCode": 8, "bsid": 8, "bsmod": 2, "description": "", "descriptorTag": 129, "fullSvc": true, "language": "fre", "numChannels": "2/0", "sampleRate": "48", "surroundMode": "Not indicated" }, { "ISO639Lang": [ { "audioType": 0, "language": "fre" } ], "descriptorTag": 10 } ], "pid": 70, "streamType": 129, "streamTypeString": "Audio AC3 (ATSC)" } ], "tableId": 2, "tableName": "PMT", "version": 12 } ]
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


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/mkrufky/libdvbtee/trend.png)](https://bitdeli.com/free "Bitdeli Badge")
[![Analytics](https://ga-beacon.appspot.com/UA-71301363-2/mkrufky/libdvbtee/README.md)](https://github.com/igrigorik/ga-beacon)
