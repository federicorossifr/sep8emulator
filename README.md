# sep8emulator
sep8 emulator is a C++ emulator for the didactic processor called sep8, as described in [this](http://www.edizioniets.com/scheda.asp?n=9788846743114) book.

Implemented features:
- CPU loop supporting decoding of the entire instruction set described in the book
- IO system supporting following operations:
    - Blocking mode
    - Interrupt mode
    - DMA mode

- IO Devices implemented:
    - APIC with programmable interrupt handlers for each IR entry
    - HDD (block device) with SATA-like interface (supports DMA mode) emulated on file
    - Dummy device (random number generator)

- Extras:
    - Assembler written in python: enable high-level (assembly-like syntax) language to machine-level language assembly
    - Example programs:
        - BIOS: load super-simple OS from HDD using DMA operation
        - Some interrupt gates
        - User space program that does some IN/OUT operations using INT service requests.




# Instructions

In order to execute the emulator with example programs:

```sh
make all 
```
```sh
./run -l
```


# Contributors
[Federico Rossi](https://github.com/federicorossifr)<br>

