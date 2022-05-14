# build and run various internal code verifiers


THINGS=flags


all: $(THINGS) run

flags: flags.c
	$(BUILD) -c flags.c
	$(LINK) -lmarkdown -o flags flags.o

run:
	@for x in $(THINGS); do ./$$x || exit 1; done

clean:
	rm -f $(THINGS)
