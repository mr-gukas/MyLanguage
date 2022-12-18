CPPFLAGS := -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -pie -fPIE -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr


front:
	g++ $(CPPFLAGS)	frontend/frontend.cpp tree/tree.cpp  filework/filework.cpp standart/standart.cpp log/LOG.cpp -o run/front

middle:
	g++ $(CPPFLAGS)	middleend/diff.cpp tree/tree.cpp filework/filework.cpp standart/standart.cpp log/LOG.cpp -o run/middle

back:
	g++ $(CPPFLAGS)	backend/backend.cpp backend/stack/stack.cpp tree/tree.cpp filework/filework.cpp standart/standart.cpp log/LOG.cpp -o run/back

cp:
	g++ $(CPPFLAGS) cpu/cpu.cpp cpu/stack/stack.cpp log/LOG.cpp -o run/cpu
	
asm: 
	g++ $(CPPFLAGS) cpu/asm.cpp filework/filework.cpp -o run/asm

trans:
	g++ $(CPPFLAGS) translator/translator.cpp filework/filework.cpp tree/tree.cpp standart/standart.cpp log/LOG.cpp -o run/trans

	
MK:
	g++ $(CPPFLAGS) MK.cpp -o lang 


