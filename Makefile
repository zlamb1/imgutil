OUTDIR ?= out

INCDIR := include
SRCDIR := src

EXT4CP      := $(OUTDIR)/ext4cp
EXT4CP_SRCS := $(SRCDIR)/main.c
EXT4CP_DEP  := $(EXT4CP).d

.PHONY: all clean

all: $(EXT4CP)

clean:
	rm -rf $(OUTDIR)

$(OUTDIR):
	mkdir -p $@

$(EXT4CP): $(EXT4CP_SRCS) | $(OUTDIR)
	$(CC) -I$(INCDIR) -MMD $(EXT4CP_SRCS) -o $@

-include $(EXT4CP_DEP)