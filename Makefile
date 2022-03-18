.PHONY: all clean_output clean_obj

# compliler cmd name
COMPILER = g++

# standard flag set for Memory Sanitizer
SAN_FLAGS = -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

# set of flags for debug compilation mode
GCC_DB_FLAGS = -D _DEBUG -ggdb3 -std=c++2a -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations							\
-Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported						\
-Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness			\
-Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith					\
-Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2				\
-Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default					\
-Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix						\
-Wno-missing-field-initializers -Wno-narrowing -Wunused -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new			\
-fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer							\
-fPIE -pie -Wstack-usage=8192

# no-debug compilation mode flag
GCC_GO = -w

all: cpu asm disasm

cpu: cpu.o cpu_func.o cpu_stack_func.o common_func.o
	$(COMPILER) -o run_cpu cpu.o cpu_func.o cpu_stack_func.o common_func.o

cpu.o: cpu/cpu.cpp
	$(COMPILER) $(GCC_GO) -c cpu/cpu.cpp

cpu_func.o: cpu/cpu_func.cpp
	$(COMPILER) $(GCC_GO) -c cpu/cpu_func.cpp

cpu_stack_func.o: cpu/cpu_stack_func.cpp
	$(COMPILER) $(GCC_GO) -c cpu/cpu_stack_func.cpp


asm: asm.o asm_func.o common_func.o
	$(COMPILER) -o run_asm asm.o asm_func.o common_func.o

asm.o: asm/asm.cpp
	$(COMPILER) $(GCC_GO) -c asm/asm.cpp

asm_func.o: asm/asm_func.cpp
	$(COMPILER) $(GCC_GO) -c asm/asm_func.cpp


disasm: disasm.o disasm_func.o common_func.o
	$(COMPILER) -o run_disasm disasm.o disasm_func.o common_func.o

disasm.o: disasm/disasm.cpp
	$(COMPILER) $(GCC_GO) -c disasm/disasm.cpp

disasm_func.o: disasm/disasm_func.cpp
	$(COMPILER) $(GCC_GO) -c disasm/disasm_func.cpp


common_func.o: common/common_func.cpp
	$(COMPILER) $(GCC_GO) -c common/common_func.cpp


clean_obj:
	rm -rf *.o

clean_output:
	rm -rf log/* bin/* *.bin
