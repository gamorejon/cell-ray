CC = g++

ARCH = pentium3

EXTRA_OPTIMIZATIONS = -mmmx -msse -mfpmath=sse # pentium3


FLAGS = -O3 -mtune=$(ARCH) -march=$(ARCH) \
 $(EXTRA_OPTIMIZATIONS) -pipe -fomit-frame-pointer -funroll-all-loops -s 

EasyBMPtoAVI: EasyBMP.o EasyBMPtoAVI.o
	$(CC) $(CFLAGS) EasyBMP.o EasyBMPtoAVI.o -o EasyBMPtoAVI
	
EasyBMP.o: EasyBMP.cpp EasyBMP*.h
	$(CC) $(CFLAGS) -c EasyBMP.cpp

EasyBMPtoAVI.o: EasyBMPtoAVI.cpp
	$(CC) $(CFLAGS) -c EasyBMPtoAVI.cpp

clean: 
	rm -f *.o
	rm -f EasyBMPtoAVI EasyBMPtoAVI.exe

