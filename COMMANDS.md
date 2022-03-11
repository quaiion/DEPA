*Here you can find the description of processor's system of commands and instructions of how to insert your own command into its command list.*

The processor emulation's architecture specificities allow to work with only 31 or less different commands. Basic version includes 26 most common ones (leaving 5 free spaces), their brief description is provided below. Please feel free to change, delete and restore any of them.

### Operation byte structure
Each command has it's own bit representation - a 5-bit number from 0 to 30. These bit codes are generated and assigned automatically, you don't have to care about them yourself. In a *binary* executionable file each operation is represented by 1 byte - 8 bits. 5 junior ones equal the bit code of a command, 3 senior represent its specifications.

A simple exaple from the basic command set - **PUSH** command. This command can describe 6 different operations: "push *val*" (push value to the stack), "push *reg*" (push registers contents to the stack), "push *reg*+*val*" (push the sum of a vlaue and register's contents to the stack), and "push \[*reg*]", "push \[*val*]", "push \[*reg*+*val*], pushing contents from the memory addressed by the numbers calculated the same way. Each of these operations has the same junior bits: **00001**. But first senior bit is set **1** if the operation is dealing with the registers, second - if immediate number is involved, third - if memory is addressed by the operation. As the result, operation codes look like this:

  **PUSH** type         |  reg  |  imm  |  ram  |  resulting opcode
------------------------|:-----:|:-----:|:-----:|:------------------:
push *val*              | nope  | yup   | nope  | **010**00001
push *reg*              | yup   | nope  | nope  | **100**00001
push *reg*+*val*        | yup   | yup   | nope  | **110**00001
push \[*val*]           | nope  | yup   | yup   | **011**00001
push \[*reg*]           | yup   | nope  | yup   | **101**00001
push \[*reg*+*val*]     | yup   | yup   | yup   | **111**00001

Of course, simpler system of operation specification could be used (e.g. considering different types of one command as completely different commands). But this way of storing the binary information was many times more interesting to implement (which is a good argument for an educational project). Beyond this, such code's execution runs a bit faster and (looking forward to using this CPU emulation in other projects) binary files generated that way may be more efficient to automatically analyse/translate/modify etc.

### Deleting/Inserting/Changing commands
All of this kind of actions can be done through the *common/commands.hpp* header file. Each command is described by 9 parameters/algorithms, including:

  Parameter name    |  Description  |  Special info
:------------------:|:--------------|:--------------
token | The name of a command (the one used in the *.asm* code itself) |
arg_extraction_alg | Represents actions, needed to transform some bytes in a binary file to an argument, that can be printed by the disassembler or used by the processor, provides *arg_byte_size* variable establishment if necessary. | Variables, declared in this segment, can be used in *execution_alg* and *arg_disas_print* code segments. If *arg_byte_size* parameter represents a name of a variable, this variable **must** be declared and it's value **must** be calculated in this code segment. **PREDECLARED VARIABLES, THAT CAN BE USED**: *unsigned char \*code_ptr* - pointer to a command byte in a binary buffer followed by argument bytes (the pointer itself **must not** be changed, its shift is provided automatically).
arg_byte_size | Size of an argument, stored in a binary file, counted in bytes. | Can be either a constant value or a name of a *size_t* variable, that must necessarily be declared and value of which must be calculated in *arg_extraction_alg*, *preasm_format_alg* and *arg_assem_alg* code segments.
execution_alg | Represents actions, performed by the processor while processing the operation described. | Variables, declared in *arg_extraction_alg* code segment can be used. **PREDECLARED VARIABLES, THAT CAN BE USED**: *size_t arg_size* - equals *arg_byte_size* value, *struct cpu_t \*cpu* with useful members: *size_t ip* - instruction pointer (an index, actually), *int \*ram* - pointer to ram emulation's start, *int reg \[NUM_OF_REGS]* - registers array (NUM_OF REGS can be adjusted in *common/common.hpp*), *stack_t stack* - main stack (the *stack_p = &(cpu->stack)* is predeclared as well), *addr_stack_t addr_stack* - address stack (the *addr_stack_p = &(cpu->addr_stack)* is predeclared as well).
preasm_format_alg | Represents special actions on modifying the operation's bit code according to the format of the *.asm* code line, provides *arg_byte_size* variable establishment if necessary. | If *arg_byte_size* parameter represents a name of a variable, this variable **must** be declared and it's value **must** be calculated in this code segment. **PREDECLARED VARIABLES, THAT CAN BE USED**: *unsigned char bit_code* - initially equals the basic command code and can be modified, *char \*line* - pointer to a single string, representing the *.asm* code line, *char \*space* - pointer to the separating space symbol in this string or *NULL* if no spaces found.
extern_arg | Indicates if the command has an argument that should be somehow parsed in the *.asm* code or not. | Can be a *true* constant, a *false* constant or a name of a *bool* variable, that must necessarily be declared and set to the right position in *arg_assem_alg* code segment.
arg_assem_alg | Records the argument parsed in *.asm* code line to the binary buffer, provides *extern_arg* variable establishment if necessary. | If *extern_arg* or *arg_byte_size* parameter represents a name of a variable, this variable **must** be declared and its value **must** be calculated in this code segment. **PREDECLARED VARIABLES, THAT CAN BE USED**: *unsigned char \*arg_start* - pointer to the first byte related to the argument in the binary buffer (the pointer itself **must not** be changed, its shift is provided automatically), *unsigned long line_num* - number of *.asm* code line parsed, *char \*space* - pointer to the separating space in *.asm* code line string, *unsigned char bit_code* - modified ready-to-use command bit code (either the code line corresponds this code or its format is wrong), *mark_t \*label_tbl* - an array of *mark_t* structures, including such members as *char \*name [MAX_LABEL_NAME_BYTE_SIZE + sizeof (char)]* (label's name) and *size_t idx* (label's jump address), *unsigned num_of_labels* - number of labels in *label_tbl*. Also, predeclared *char end_symb_1* and *char end_symb_2* (2 symbols, following after the last argument symbol) **must** be scanned (left untouched if the line ended) in this code segment.
arg_disas_print | Represents actions, needed to print the argument according to its binary encoding. | Variables, declared in *arg_extraction_alg* code segment can be used. **PREDECLARED VARIABLES, THAT CAN BE USED**: *char \*arg_str* - pointer to a string where the disassembled argument should be placed.
max_disasm_arg_len | The upper estimate of the number of symbols needed to print the disassembled argument. |

To create a new command, insert a line, discribing this command, formated as `CMD_PATTERN (token, arg_extraction_alg, arg_byte_size, execution_alg, preasm_format_alg, extern_arg, arg_assem_alg, arg_disas_print, max_disasm_arg_len)` to the last part of the *common/commands.hpp* header file and complete the file with `#define ...` macros mentioned in your description line. Don't forget to use `#undef ...` in the end of the file. Deletion and changing are done in the same way.

All the code segments inserted should follow a set of special rules, including:
1. To stop the process, where the segment is used (execution, assembling or disassembling), set the member *stop* of a special predeclared structure variable *signal* to *true*: `signal.stop = true;`. To stop the process emergency (returning error and leaving memory-cleaning function the last one performed), you can additionally set the *err* member of the same variable to *true*: `signal.err = true;`. ***SETTING ONLY THE "ERR" MEMBER TO TRUE WILL BE IGNORED BY THE PROGRAM!*** To interrupt the current iteration as soon as possible, you can add a `break;` line after this.
2. All the lines should respond to all basic standards of C++ syntax, including closing `;` in the end of every line.
3. If no special actions needed (e.g. often in *preasm_format_alg*), the code-segment-requirung parameter can be left empty.
4. Custom functions or constants can be used. To use your constant, define it in the *common/common.hpp* header file. To include a function, insert its prototype into the *common/common.hpp* header file and its code into the *common/common_func.cpp* file. Then your function will be able to use in any program of the project.
5. Be careful with using predeclared variables and declaring your own. ***DON'T USE THE PREDECLARED VARIABLES THAT ARE NOT MENTIONED IN THE "AVAILABLE-TO-USE" LIST IN THE TABLE ABOVE!***. If you accidentally redecalre some internal variable, you will see a special compiler's message after recompiling the project. 
6. Don't forget to fit your code segments with special verificators or dump-functions to keep the emulation secure from critical flaws.

### Basic command set
Here is a brief description of every command, included into a basic command set:

- **HLT** - stop the program execution. Has no argument.
- **PUSH** - push a value to the stack. Has some specific argument formats, can work with immediate numbers, registers and RAM (see the accurate formats' description in **Operation byte structure** section).
- **POP** - pop a value from the stack. Has a set of argument formats, similar to the **PUSH**'s one (in this case the argument describes the destination, where the popped value will be moved instead of describing the source of a value), except that it can be used without any argument (throw the value away) and cannot be used with just *val* or *reg*+*val*.
- **ADD** - pop two top values, stored in the stack, add them and push the result back. Has no argument.
- **SUB** - pop two top values, stored in the stack, subtract the one popped last from the one popped first and push the result back. Has no argument.
- **MUL** - pop two top values, stored in the stack, multiply them and push the result back. Has no argument.
- **DIV** - pop two top values, stored in the stack, divide the one popped first by the one popped last and push the result back. Has no argument.
- **IN** - push a number, inserted from the keyboard. Has no argument.
- **OUT** - pop the the top value, stored in the stack, and print it in the console.
- **STDMP**, **ASTDMP**, **RGDMP** - dump all the information about the main stack, the address stack of the program, or the CPU registers to the *.log* file.
- **STVRF**, **ASTVRF** - verify the main program stack or its address stack, dump the information to the *.log* file in case of problems detection.
- **JMP** - jump on the label (mentioned as an argument) described wherever in the code (see the label-using instructions in the *README.md* file).
- **JA**, **JAE**, **JB**, **JBE**, **JE**, **JNE**, **JF** - conditional jump on the label: if second from the top value in the stack is greater *OR* greater or equal *OR* less *OR* less or equal *OR* equal *OR* not equal to the top one *OR* if today's weekday is Friday, respectively.
- **CALL** - call the procedure, starting on the label (mentioned as an argument) described wherever in the code and ending with a **RET** command, push the **CALL**'s program address to the address stack.
- **RET** - pop the call address from the address stack and jump to that address in the program.
- **PRNT** - print the contents of the RAM's video segment in the console (empty space if **zero** is stored in the related memory *int* cell, "#" symbol if **non-zero**), according to the VIDEOSEG_START, VIDEOSEG_SIZE, CONSOLE_SIZE_VERT and CONSOLE_SIZE_HOR constants, that can be adjusted in the *common/common.hpp* header file. Has no argument.
- **SHW** - same as **PRNT**, but the console is cleaned after printing.
