target := libuthread.a
objs := queue.o uthread.o context.o preempt.o 

CC := gcc
STAT_CC	:= ar
CFLAGS := -Wall -Werror
STAT_FLAGS := rcs
CFLAGS += -g

ifneq ($(V),1)
Q = @
endif

all: $(target)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)


$(target): $(objs)
	@echo "LD $@"
	$(Q)$(STAT_CC) $(STAT_FLAGS) -o $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

clean:
	@echo "clean"
	$(Q)rm -f $(target) $(objs) $(deps)