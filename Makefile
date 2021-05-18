
CC=gcc


daemon:
	$(CC) -o irc_daemon irc_daemon.C -I/usr/include/opencv4 \
	  	-I/storage/irc/GetThermal/source/libuvc/build/include \
	  	-lmysqlclient -L/storage/irc/GetThermal/source/libuvc/build \
	  	-luvc -lopencv_core -lopencv_imgcodecs -lpthread

module:
	$(CC) -o irc_module irc_module.C -I/usr/include/opencv4 \
	  	-I/storage/irc/GetThermal/source/libuvc/build/include \
	  	-lmysqlclient -L/storage/irc/GetThermal/source/libuvc/build \
	  	-luvc -lopencv_core -lopencv_imgcodecs


single: 
	$(CC) -o singlecam singlecam.C -I/usr/include/opencv4 \
	 	-I/storage/irc/GetThermal/source/libuvc/build/include \
		-lmysqlclient -L/storage/irc/GetThermal/source/libuvc/build \
		-luvc -lopencv_core -lopencv_imgcodecs

example: 
	$(CC) -o example example_new.C -I/usr/include/opencv4 \
		-I/storage/irc/GetThermal/source/libuvc/build/include \
		-lmysqlclient -L/storage/irc/GetThermal/source/libuvc/build \
		-luvc -lopencv_core -lopencv_imgcodecs

test:
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 0015002c-5119-3038-3732-333700000000 1 &
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 0013001c-5113-3437-3335-373400000000 2 &
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 00070029-5102-3038-3835-393400000000 3 &
	LD_LIBRARY_PATH=/storage/irc/GetThermal/source/libuvc/build/ ./singlecam 8010800b-5113-3437-3335-373400000000 4 &

clean:
	rm irc_module
	rm irc_daemon
	rm singlecam
	rm example
