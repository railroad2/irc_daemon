
CC=gcc -x c
FLAGS= -I/usr/include/opencv4 \
	   -I/storage/irc/GetThermal/source/libuvc/build/include \
	   -lmysqlclient -L/storage/irc/GetThermal/source/libuvc/build \
	   -luvc -lopencv_core -lopencv_imgcodecs -lpthread 

LIBROOT=/home/kmlee/usr
INC=-I$(LIBROOT)/include 
LIBLEP=-L$(LIBROOT)/lib -llepton -lm
LIBSOL=-L/usr/local/lib -lSolTrack -lm



daemon: irc_daemon.c
	$(CC) -o irc_daemon irc_daemon.c $(FLAGS) $(LIBSOL) $(LIBLEP) $(INC)

all: daemon module shutter sunpos

module: irc_module.c
	$(CC) -o irc_module irc_module.c $(FLAGS)
	

single: singlecam.c
	$(CC) -o singlecam singlecam.c  $(FLAGS)


shutter: shutter.c
	$(CC) -o shutter shutter.c $(FLAGS) $(INC) $(LIBLEP) 


sunpos: sunpos.c
	$(CC) -o sunpos sunpos.c $(LIBSOL)
	

test:
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 0015002c-5119-3038-3732-333700000000 1 &
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 0013001c-5113-3437-3335-373400000000 2 &
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 00070029-5102-3038-3835-393400000000 3 &
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 8010800b-5113-3437-3335-373400000000 4 &

clean:
	rm irc_module irc_daemon singlecam shutter example sunpos
