# DEPA, Disassembler + Emulated Processor + Assembler, V2.1

## Description & Features
DEPA is an emulation of a CISC-like processor, working with fixed-point *int\*1000* numbers with an assembler and a disassembler, compatible with it. While working, the cpu emulation creates an emulation of RAM (with video segment integrated), a number of common-use registers (4 by default) and two stacks - the main one (storing fixed-point numbers the *.asm* program is working with) and the address one (storing the integer addresses in binary code and used by the function callers). Both stacks are automatically resized. Most common parameters of emulation (like number of regs or video segment location) can be easily adjusted by entering your own numbers in some exact places at the beginning of *common/common.hpp* header file. The assembler is able to work with labels by default (see the instruction below) two-pass assembling is used. The command list of the processor can be easily configured too (see the instruction below). Every command used is technically able to use as many arguments strored in a binary file as you want. Size and type of each argument is not limited as well. Every part of assembling, disassembling and execution processes is verified and secured from critical flaws (custom commands' safety must be controlled by the author of the commands). Listing file is created on every succesful (or interupted on second pass) compilation automatically.

## Latest version
The latest DEPA version can be found on its author's GitHub page: [https://github.com/quaiion/DEPA]().

## Documentation
The essential documentation on editing the processor's command list can be found in *COMMANDS.md* file.
Common documentation lines (suitable for Doxygen usage) will be soon added to the header files.

## How-to-use
### Compilation
To compile the program on your computer, you will need a **GCC g++** compiler. You can either use the **GNU make**, or launch the compilation manually. If you prefer using **make**, you can use the built-in *Makefile* with 4 standart targets to use: `make all` - compile all the programs at once, `make asm` - compile the assembler program, `make cpu` - compile the cpu emulation program, `make disasm` - compile the disassembler program, `make clean_obj` - delete all the objective files, `make clean_output` - delete all logs, listing files and binary files (be careful with that one). To switch to debug compilation mode, manage the flag variables in the *Makefile*. If you prefer manual compilation, you should use `g++ asm/asm.cpp asm/asm_func.cpp common/common_func.cpp -w -o run_asm` to complile th assembler program, `g++ cpu/cpu.cpp cpu/cpu_func.cpp cpu/cpu_stack_func.cpp common/common_func.cpp -w -o run_cpu` to compile the cpu emulation program and `g++ disasm/disasm.cpp disasm/disasm_func.cpp common/common_func.cpp -w -o run_disasm` to compile the disassembler program, respectively. After compilation 3 executable files will be added: *run_asm*, *run_cpu*, *run_disasm* in the project's root folder.

### Launching
To launch every program, you should just run the executable file with 1 parameter mentioned in the console - *.asm* target file for assembler or *.bin* target file for cpu emulation and disassembler (be aware of the fact that all *.bin* files are automatically stored in *bin* folder, *.asm* files in *prog* folder). While launching assembler and disassembler, you can additionally specify the output file's name by entering it as the second launch parameter. Default binary file's name is *QO_code_file.bin*, default *.asm* file's name is *QO_prog_file.asm*.

### *.asm* programs
There are several simple syntax rules you should follow while writing down your *.asm* program:
1. Every command should be written on a separate line.
2. No empty lines allowed. To leave a space line write a comment symbol "/" at the beginning of the empty line.
3. Arguments of each command (if there are any) should be separated from the command itself by **one** space symbol.
4. Every label should be written on a separate line as well (when not used as an argument). Max label's name size can be adjusted at the beginning of *common/common.hpp* header file. If the label's name is not used as an argument, the ":" symbol should follow after its name.
5. To comment out lines, use the comment symbol "/" at the beginning of the line. To write a simple comment, type this symbol after the code line, separated by **one** space symbol from it, and enter the comment of any size and format possible.
6. You don't have to use **HLT** command, if the execution finish matches the real program's finish - execution will be stopped automatically.

The basic included commands list and the accurate instruction of how to edit it and create your own commands can be found in *COMMANDS.md* file. Some code examples can be found in *code_exaples* folder (*fibb.asm* - prints the specified Fibbonacci number, *kv.asm* - calculates the roots of a square polynom, based on polynom's coefficients entered, *circle.asm* - prints a 7x7 circle with center coordinates inserted in the 35x162 console, using the video segment of emulated memory), *.bin* relative files and disassembled files are stored in *bin* and *prog* folders respectively.

***IMPORTANT FACT ABOUT RAM EMULATION***: after emulation initializing all the memory (including video segment) is set to zero.

## License
DEPA is an open-source educational project. You can freely use, edit and distribute any versions of this code in personal and commercial purposes.

## Contacts
#### Korney Ivanishin, author of the project
e-mail: korney059@gmail.com,
GitHub: quaiion
