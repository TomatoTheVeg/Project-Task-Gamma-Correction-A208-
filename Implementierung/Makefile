CC      := gcc
CFLAGS  := -Wall -Wextra -O2
TARGET  := gamma_correct

SRC_C   := main.c pnm_image.c benchmark.c
SRC_S   := gamma_correct.S gamma_correct_V1.S gamma_correct_V2.S power.S powerSIMD.S power_V1.S powerSIMD_V1.S

OBJS_C  := $(SRC_C:.c=.o)
OBJS_S  := $(SRC_S:.S=.o)

OBJS    := $(OBJS_C) $(OBJS_S)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
