CC = gcc
CFLAGS = -Wall -g -O
LDFLAGS = -lm
SRC = tinyraytracer

$(SRC): $(SRC).c
	$(CC) $(CFLAGS) -o $(SRC) $(SRC).c $(LDFLAGS)

run: $(SRC)
	./$(SRC)

clean:
	rm -f $(SRC)