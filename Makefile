OUTDIR ?= out

WARNINGS := \
	-Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wstrict-prototypes \
	-Wmissing-prototypes -Wno-missing-braces -Wno-missing-field-initializers -Wbad-function-cast \
	-Winline -Wundef -Wunreachable-code -Wredundant-decls -Wfloat-equal -Wcast-align \
	-Wcast-qual -Wdeclaration-after-statement -Wmissing-include-dirs -Wnested-externs \
	-Wno-error=format -Wsequence-point -Wswitch -Wwrite-strings

INCDIR := include
SRCDIR := src

EXT2CP      := $(OUTDIR)/ext2cp
EXT2CP_SRCS := $(SRCDIR)/posix/file.c $(SRCDIR)/fs/ext2.c $(SRCDIR)/cp.c
EXT2CP_DEPS := $(EXT2CP).d

EXT2LS      := $(OUTDIR)/ext2ls
EXT2LS_SRCS := $(SRCDIR)/posix/file.c $(SRCDIR)/fs/ext2.c $(SRCDIR)/ls.c
EXT2LS_DEPS := $(EXT2LS).d

.PHONY: all clean

all: $(EXT2LS) $(EXT2CP)

clean:
	rm -rf $(OUTDIR)

$(OUTDIR):
	mkdir -p $@

$(EXT2LS): $(EXT2LS_SRCS) | $(OUTDIR)
	$(CC) $(WARNINGS) -I$(INCDIR) -MMD $(EXT2LS_SRCS) -o $@

$(EXT2CP): $(EXT2CP_SRCS) | $(OUTDIR)
	$(CC) $(WARNINGS) -I$(INCDIR) -MMD $(EXT2CP_SRCS) -o $@

-include $(EXT2CP_DEPS)