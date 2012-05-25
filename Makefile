# compiler
CC			= clang++

CINCLUDE	= -I src
LDFLAGS		= 
CFLAGS  	= -Wall -msse3 -ffast-math -O2

SOURCES		= 
INCLUDES	= $(SOURCES:.c=.h)
OBJECTS		= $(SOURCES:.c=.o)

BUILDDIR  	= build
BINDIR		= bin
SRCDIR		= src
EXDIR		= example

all: example

# ADMM based optimal power flow solver

example: $(BUILDDIR) $(BINDIR) $(BINDIR)/main 

$(BINDIR)/main: $(BUILDDIR)/main.o #$(LIBDIR)/$(LIBNAME).a
	$(CC) -g $< -o $@ $(LDFLAGS)

$(LIBDIR)/$(LIBNAME).a: $(BUILDDIR)/$(OBJECTS)
	$(AR) rsv $@ $<
  
# compile .cc files in to .o files
$(BUILDDIR)/%.o : $(EXDIR)/%.cc $(SRCDIR)/graph_admm.h
	$(CC) -c -o $@ $(CFLAGS) $(CINCLUDE) $<

$(BUILDDIR):
	mkdir $(BUILDDIR)

$(LIBDIR):
	mkdir $(LIBDIR)

$(BINDIR):
	mkdir $(BINDIR)

# Cleanup tasks
clean:
	rm -rf $(BINDIR) $(LIBDIR) $(BUILDDIR)

.PHONY: clean

