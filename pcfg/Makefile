# Makefile for ~/research/inside-outside
#
#  (c) Mark Johnson, 20th October, 2012

CC = gcc
CFLAGS = $(GCCFLAGS) -O6 -Wall -fomit-frame-pointer -ffast-math -fstrict-aliasing -fmerge-all-constants -finline-functions
LDFLAGS = $(GCCLDFLAGS)
CXXFLAGS =  ${CFLAGS}
EXEC =
#
# debugging
#
# CFLAGS = -g -O -MMD -Wall -ffast-math -fstrict-aliasing $(GCCFLAGS) ${TAO_PETSC_INCLUDE} 
# LDFLAGS = -g -Wall
# EXEC = valgrind --tool=memcheck
#
# profiling
#
# CFLAGS = -g -pg -O6 -MMD -Wall -ffast-math -fno-default-inline -fno-inline $(GCCFLAGS)
# CFLAGS = -g -pg -O -MMD -Wall -ffast-math $(GCCFLAGS)
# LDFLAGS = -g -pg $(GCCLDFLAGS)

SOURCES = bayes.cc io.c ioh.c hash-string.c mmm.c tree.c grammar.c vindex.c expected-counts.c
TARGETS = bayes io ioh
OBJECTS = $(patsubst %.l,%.o,$(patsubst %.c,%.o,$(SOURCES:%.cc=%.o)))

top: io

# testvb does Variational Bayes inference on the toy grammar in testengger.lt
# and the strings in testeng.yld.
#
.PHONY: testvb
testvb: io
	$(EXEC) io -a 1 -V -d 1000 -g testengger-bayes.lt testeng.yld 

# testio does inside-outside inference on the toy grammar in testengger.lt
# and the strings in testeng.yld.
#
.PHONY: testio
testio: io
	$(EXEC) io -d 1000 -g testengger.lt testeng.yld 
	$(EXEC) io -c -d 1000 -g testengger.lt testeng-counts.yld


io: io.o hash-string.o mmm.o tree.o grammar.o vindex.o expected-counts.o digamma.o
	gcc $^ $(LDFLAGS) -lm -o $@

io.o: io.c local-trees.h hash-string.h hash-templates.h \
	hash.h grammar.h expected-counts.h digamma.h

expected-counts.o : expected-counts.c expected-counts.h local-trees.h \
	hash-templates.h hash.h vindex.h mmm.h grammar.h

grammar.o: grammar.c local-trees.h hash-string.h hash.h hash-templates.h \
	grammar.h mmm.h 

tree.o: tree.c tree.h hash-string.h hash.h mmm.h local-trees.h

vindex.o: vindex.c vindex.h mmm.h hash.h hash-templates.h local-trees.h

hash-string.o : hash-string.c hash-string.h hash.h hash-templates.h \
	mmm.h local-trees.h

mmm.o : mmm.c mmm.h 

digamma.o: digamma.h

.PHONY: real-clean
real-clean: clean

.PHONY: clean
clean:
	rm -fr *.a *.d *.o *~ $(TARGETS)

# this command tells GNU make to look for dependencies in *.d files
-include $(patsubst %.l,%.d,$(patsubst %.c,%.d,$(SOURCES:%.cc=%.d)))
