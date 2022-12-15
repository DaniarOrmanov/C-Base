object = Final_task.o temp_functions.o

all: prog

prog: $(object)
	gcc -o prog $(object)
	
Final_task.o: Final_task.c temp_functions.h
	gcc -c -o Final_task.o Final_task.c
	
temp_functions.o: temp_functions.c
	gcc -c -o temp_functions.o temp_functions.c

clean:
	rm -rf *.o prog
