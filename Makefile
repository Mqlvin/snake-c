CC := gcc
C_FLAGS := -Wall
L_FLAGS := -lm -lSDL2 -lSDL2_image

HDRS :=
SRCS := snake.c

OUT := snake

all: $(SRCS)
	$(CC) $(SRCS) $(HEADERS) $(C_FLAGS) $(L_FLAGS) -o $(OUT)

clean:
	rm -f $(OBJS)
