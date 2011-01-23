
all:mrproper SFhash.o sfhash

sfhash : SFhash_use.c SFhash.o
	gcc $^ -o $@

SFhash.o : SFhash.c
	gcc -c $^ -o $@


clean:
	rm -f *.o
mrproper : clean
	rm -f sfhash
	
