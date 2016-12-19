# ntptimestamp -- program to get ntptimestamps.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Sun Dec 11 01:29:10 EET 2016

RM = rm -f
targets = ntpts
TOCLEAN += $(targets)

all: $(targets)
clean:
	$(RM) $(TOCLEAN)

ntpts_objs = ntpts.o
ntpts_libs = 
TOCLEAN += $(ntpts_objs)

ntpts: $(ntpts_objs)
	$(CC) $(LDFLAGS) -o $@ $(ntpts_objs)

.depend: $(ntpts_objs:.o=.c)
	$(CC) -MM $? >$@

-include .depend
