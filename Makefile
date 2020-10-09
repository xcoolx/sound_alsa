#aarch64-linux-gnu-gcc    -c -o alsa.o alsa.c
#aarch64-linux-gnu-gcc --sysroot=/home/dmytro/Sandbox/cross_copmiling/alsa/sysroot -o alsa alsa.o -lasound


SYSROOT = --sysroot=/home/dmytro/Sandbox/cross_copmiling/alsa/sysroot
AARCH64_CC = aarch64-linux-gnu-gcc
SOURCES = alsa.c

LIB = -lasound

sound_arm: sound_arm.o
	$(AARCH64_CC) $(SYSROOT) -o $@ $< $(LIB)

sound_arm.o:
	$(AARCH64_CC) -c -o $@ $(SOURCES)

sound: sound.o
	gcc -o $@ $< $(LIB)

sound.o:
	gcc -c -o $@ $(SOURCES)


clean:
	rm -f  *.o alsa sound sound_arm

