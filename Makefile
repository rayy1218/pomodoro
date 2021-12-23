C_SOURCE = ${wildcard src/*.c}
HEADER = ${wildcard src/*.h}

OBJ = ${C_SOURCE:.c=.o}
LINKER_FLAGS = -lncurses
PROGRAM_NAME = pomodoro

build: ${OBJ}
	make clean
	mkdir build
	gcc ${LINKER_FLAGS} -o ./build/${PROGRAM_NAME} $^

run:
	make build
	./build/${PROGRAM_NAME} ${arg}

clean:
	rm -rf build

%o: %c ${HEADER}
	gcc -c -o $@ $^
