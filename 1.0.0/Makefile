VERSION = 1.0.0
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c2x -Ofast -g
RM = rm
LINT = cppcheck
LINTFLAGS = --check-level=exhaustive --enable=all --inconclusive --suppress=missingIncludeSystem --verbose

BIN = rens

.PHONY: all clean lint dist self

# Building -------------------------------------

all: $(BIN)

$(BIN): $(BIN).o
	$(CC) $(CFLAGS) $(BIN).o -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Distribution ---------------------------------

dist: clean
dist: all
dist: $(DDIR)/$(BIN).7
	gzip -vk $<
	tar -czvf $(BIN)-$(VERSION).tar.gz $(BIN) $(DDIR)/* ../README.md ../LICENSE --transform 's,^,$(BIN)-$(VERSION)/,'
	md5sum $(BIN)-$(VERSION).tar.gz

self: clean
self: all
self: $(DDIR)/$(BIN).7
	gzip -vk $<
	sudo mkdir /usr/local/man/man7/ -p
	sudo mv $(BIN) /usr/local/bin
	sudo mv $(DDIR)/$(BIN).7.gz /usr/local/man/man7/
	sudo mandb -q

# Other ----------------------------------------

lint: $(BIN).c
	$(LINT) $(LINTFLAGS) $<
clean:
	$(RM) -f $(BIN) *.o

