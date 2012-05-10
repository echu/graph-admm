# compiler
CC			  = clang
# archiver
AR        = ar

CINCLUDE	= -I src
LDFLAGS		= -L lib -lgadmm
CFLAGS  	= -Wall -msse3 -ffast-math -O3 -std=c99

SOURCES		= graph_admm.c
INCLUDES	= $(SOURCES:.c=.h)
OBJECTS		= $(SOURCES:.c=.o)

BUILDDIR  = build
SRCDIR    = src
LIBDIR    = lib
EXDIR     = example
LIBNAME   = libgadmm

all: example gadmm

# ADMM based optimal power flow solver

example: $(BUILDDIR) gadmm main 

gadmm: $(BUILDDIR) $(LIBDIR) $(LIBDIR)/$(LIBNAME).a

main: $(BUILDDIR)/main.o
	$(CC) -g $< -o $@ $(LDFLAGS)

$(LIBDIR)/$(LIBNAME).a: $(BUILDDIR)/$(OBJECTS)
	$(AR) rsv $@ $<
  
# compile .c files in to .o files
$(BUILDDIR)/%.o : $(EXDIR)/%.c
	$(CC) -c -o $@ $(CFLAGS) $(CINCLUDE) $<

# compile .cpp, .c, and .cc files in to .o files
$(BUILDDIR)/%.o : $(SRCDIR)/%.c $(SRCDIR)/$(INCLUDES)
	$(CC) -c -o $@ $(CFLAGS) $<

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(LIBDIR):
	mkdir $(LIBDIR)

# Cleanup tasks
clean:
	rm -rf bin lib build main

.PHONY: clean

