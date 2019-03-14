CC := gcc
CFLAGS := -Wall -g -Os

SHDIR := ../base

OBJS = to_server.o $(SHDIR)/to_base.o

all: tos

tos: $(OBJS)
	@$(CC) -o tos $(CFLAGS) $(OBJS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f *.o tos
	@rm -f ../base/*.o
	@echo Done cleaning
