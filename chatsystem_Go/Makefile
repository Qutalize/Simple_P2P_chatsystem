TARGET  = go_chat
SRC     = go_chat.go

.PHONY: all build clean run

all: build

build:
	go build -o $(TARGET) $(SRC)

run:
	go run $(SRC)

clean:
	rm -f $(TARGET)