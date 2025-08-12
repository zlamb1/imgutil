OUTDIR ?= out

INCDIR := include
SRCDIR := src

EXT2CP      := $(OUTDIR)/ext2cp
EXT2CP_SRCS := $(SRCDIR)/linux/file.c $(SRCDIR)/fs/ext2.c $(SRCDIR)/file.c $(SRCDIR)/cp.c
EXT2CP_DEP  := $(EXT2CP).d

.PHONY: all clean

all: $(EXT2CP)

clean:
	rm -rf $(OUTDIR)

$(OUTDIR):
	mkdir -p $@

$(EXT2CP): $(EXT2CP_SRCS) | $(OUTDIR)
	$(CC) -I$(INCDIR) -MMD $(EXT2CP_SRCS) -o $@

-include $(EXT2CP_DEP)