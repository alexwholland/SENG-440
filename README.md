# SENG-440: Embedded Systems

## Project: Colour Space Conversion
This repository contains the code to the Colour Space Conversion (CSC) project completed on August 1. The repository contains:

- `unoptimized.c`: the un-optimized CSC project
- `optimized.c`: the optimized CSC project
- `input_test.raw`: a RAW 100x100 pixel image of marbles
- `cache.sh`: bash script for measuring cache misses with `valgrind`
- `perf.sh`: bash script for measuring program performance with `perf`


### Enviroment
Installed from the SENG 440 directory: https://sw.seng.uvic.ca/repo/seng440/

The program was designed to be runned on an 32-bit ARM-based virutal machine running Fedora 29 configured using QEMU. Specifically, the machine has the following architecture:

- Architecture:    	armv7l
- Byte Order:      	Little Endian
- CPU(s):          	1
- On-line CPU(s) list: 0
- Thread(s) per core:  1
- Core(s) per socket:  1
- Socket(s):       	1
- Vendor ID:       	ARM
- Model:           	0
- Model name:     Cortex-A15
- Stepping:        	r4p0
- BogoMIPS:        125.00

### Running the program
To run `unoptimized.c` run:
```
gcc -o unoptimized -g unoptimized.c
./unoptimized
```

To run `optimized.c` run:
```
gcc -o optimized -g optimized.c
./optimized
```

To measure performance ensure that you install valgrind and perf with:

```
dnf install valgrind
dnf install perf
```

To measure the cache misses of the specified program you can run:
```
./cache.sh name_of_file.c
```

To measure the execution speed of hte specified program you can run:
```
./perf.sh name_of_file.c
```


