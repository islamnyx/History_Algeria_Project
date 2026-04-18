CC      = gcc
CFLAGS  = $(shell pkg-config --cflags gtk+-3.0) -Wall -Wno-unused-function -O2
LIBS    = $(shell pkg-config --libs gtk+-3.0) -lm
TARGET  = algeria_history
SRC     = main_gui.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS)
	@echo "Build successful! Run: ./algeria_history"

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

install-deps:
	sudo apt-get install -y libgtk-3-dev gcc make

.PHONY: all clean run install-deps
