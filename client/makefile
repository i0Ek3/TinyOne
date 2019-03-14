CC := gcc
CFLAGS := -Wall -g -Os

SHDIR := ../base

OBJS = to_client.o $(SHDIR)/to_base.o

all: toc

toc: $(OBJS)
	@$(CC) -o toc $(CFLAGS) $(OBJS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f *.o toc
	@rm -f ../base/*.o
	@echo Done cleaning
