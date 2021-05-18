#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <mysql/mysql.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "/storage/irc/GetThermal/source/libuvc/include/libuvc/libuvc.h"

//////////////////////////////////////////////////
//   Global constants
//////////////////////////////////////////////////
static double threshold;
static int thresholdN;


//////////////////////////////////////////////////
//   MYSQL connection
//////////////////////////////////////////////////
MYSQL* conn;


//////////////////////////////////////////////////
//   Callback functions
//
//   This callback function runs once per frame.
// Use it to perform any quick processing you need,
// or have it put the frame into your application's
// input queue. If this function takes too long, 
// you'll start losing frames.
//////////////////////////////////////////////////
void cb(uvc_frame_t* frame, void* ptr);
void cb_mysql(uvc_frame_t* frame, void* ptr);


//////////////////////////////////////////////////
// Definitions of callback functions begin
//////////////////////////////////////////////////
void cb(uvc_frame_t* frame, void* ptr)
{
	// Skipping frames
	int seq = frame -> sequence;
	if ( seq % 9 != 0 )
	{
//		printf("[Info] Skipping current frame at sequence %d\n", seq);
		return;
	}

	// Information of frame
	const int len    = frame -> data_bytes;
	const int width  = frame -> width;
	const int height = frame -> height;
//	printf("callback! length = %u, ptr = %d\n", frame -> data_bytes, (int) ptr);

	// BGR frame
//	uvc_frame_t* bgr = uvc_allocate_frame(width * height * 3);
//	uvc_error_t ret;
//	if ( !bgr )
//	{
//		printf("Unable to allocate BGR frame!");
//		return;
//	}

	// Do the BGR conversion
//	ret = uvc_any2bgr(frame, bgr);
//	if ( ret )
//	{
//		uvc_perror(ret, "uvc_any2bgr");
//		uvc_free_frame(bgr);
//		return;
//	}


	unsigned short* pix = (unsigned short*) frame -> data;
	int i;

	// print pix
//	for ( i = 0; i < 160 * 120; i++ ) printf("%d\n", pix[i]);

	// What time is it now
	struct timeval time;
	gettimeofday(&time, NULL);
//	printf("%d\n", time . tv_sec);

	// File name
	char fileName[64];
	snprintf(fileName, 64, "log/%ld.%06ld-cam%d", time.tv_sec, time.tv_usec, *(int*) ptr);

	FILE* file = fopen(fileName, "a+");
	if ( file == NULL )
	{
		printf("[Error] Cannot open logging file\n");
		return;
	}

	fwrite(pix, len, 1, file);
	fclose(file);
}

void cb_mysql(uvc_frame_t* frame, void* ptr)
{
//	printf("cb_mysql called\n");

	// Skipping frames
	const int seq = frame -> sequence;
	if ( seq % 9 != 0 )
	{
//		printf("[Info] Skipping current frame at sequence %d\n", seq);
		return;
	}

	// Constants
	const int width      = frame -> width;
	const int height     = frame -> height;
	const int nPix       = width * height;
	const int len        = frame -> data_bytes;
	const int bytePerPix = len / nPix;
	int i, nCloud = 0;

	// Length of frame
	printf("callback! length = %u, ptr = %d\n", len, ptr);

	// Get data
	unsigned short* pix = (unsigned short*) frame -> data;

	char* pix_to = (char*) malloc(2*len+1);
	mysql_real_escape_string(conn, pix_to, (char*) pix, len);
//	printf("Length including \\: %d\n", strlen(pix_to));

	// Calculate cloud rate
	for ( i = 0; i < nPix; i++ )
	{
		if ( pix[i] > thresholdN ) nCloud++;
//		printf("%d >? %d\n", pix[i], thresholdN);
	}
	double cloud_rate = 1. * nCloud / nPix;
//	printf("n cloud    = %d\n"    , nCloud          );
//	printf("cloud rate = %.2f\%\n", cloud_rate * 100);

	// What time is it now
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long milliseconds = (unsigned long long)(tv.tv_sec)*1000 + (unsigned long long)(tv.tv_usec)/1000;
//	printf("%llu\n", milliseconds);

	// Image
	cv::Mat img(height, width, CV_16U, pix);
	double min, max;
	cv::minMaxIdx(img, &min, &max);
	printf("%f ~ %f\n", min, max);
	cv::Mat scaled;
//	cv::convertScaleAbs(img, scaled, 255. / (max - min), - min * 255 / (max - min));
	img . convertTo(scaled, CV_16U, 65535. / (max - min), - min * 65535 / (max - min));
	imwrite("result.png", img);
	imwrite("result_s.png", scaled);

	// query
	char* stat = "insert into frames values(%llu,'%s',%.2f,%d)";
	char* query = (char*) malloc(strlen(stat) + 13 + 2*len+1 + 6 + 5);
	unsigned long len_query = snprintf(query, strlen(stat) + 13 + 2*len+1, stat, milliseconds, pix_to, cloud_rate, thresholdN);

	if ( mysql_real_query(conn, query, len_query) )
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
	}

	free(query);
}


//////////////////////////////////////////////////
// Main function
//////////////////////////////////////////////////
int main(int argc, char **argv)
{
	// threshold
	threshold = 0.00; // in degree celcius
	thresholdN = threshold * 100 + 27315;

	// mysql connection
	conn = mysql_init(NULL);
	if ( conn == NULL )
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 1;
	}

	// connect
	if ( mysql_real_connect(conn, "localhost", "irc", "cmb2725K", NULL, 0, NULL, 0) == NULL )
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);
		return 1;
	}

	// create db
	if ( mysql_query(conn, "create database if not exists irc") )
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);
		return 1;
	}

	// use db
	if ( mysql_query(conn, "use irc") )
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);
		return 1;
	}

	// create table
	if ( mysql_query(conn, "create table if not exists `frames` ( `time` BIGINT(19) UNSIGNED NOT NULL , `frame` BLOB(38400) NOT NULL , `cloud_rate` FLOAT UNSIGNED , `threshold` INT UNSIGNED , PRIMARY KEY(`time`) )") )
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 1;
	}

	/* Locates the first attached UVC device, stores in dev */
//	res = uvc_find_device(ctx, &dev, 0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
	const int nCam = 4;
	const int idCam[nCam] = {1, 2, 3, 4};
	const std::string serial[nCam] = {"0015002c-5119-3038-3732-333700000000",  // 1
	                                  "0013001c-5113-3437-3335-373400000000",  // 2
	                                  "00070029-5102-3038-3835-393400000000",  // 3
	                                  "8010800b-5113-3437-3335-373400000000"}; // 4
	uvc_context_t* ctx[nCam];
	uvc_device_t* dev[nCam];
	uvc_device_handle_t* devh[nCam];
	uvc_stream_ctrl_t ctrl[nCam];
	uvc_error_t res[nCam];
	int i;
	/* Initialize a UVC service context. Libuvc will set up its own libusb
	* context. Replace NULL with a libusb_context pointer to run libuvc
	* from an existing libusb context. */
	for ( i = 0; i < nCam; i++ )
	{
		res[i] = uvc_init(&ctx[i], NULL);
		if ( res[i] < 0 )
		{
			uvc_perror(res[i], "uvc_init");
			return res[i];
		}
	}
	puts("UVC initialized");

	for ( i = 0; i < nCam; i++ )
	{
		res[i] = uvc_find_device(ctx[i], &dev[i], 0x1e4e, 0x0100, serial[i] . data());
		if ( res[i] < 0 )
		{
			uvc_perror(res[i], "uvc_find_device"); // no devices found
			return res[i];
		}
	}

	puts("Device found");

	// Try to open the device: requires exclusive access
	for ( i = 0; i < nCam; i++ )
	{
		res[i] = uvc_open(dev[i], &devh[i]);
		if ( res < 0 )
		{
			uvc_perror(res[i], "uvc_open"); // unable to open device
			return res[i];
		}
	}

	puts("Device opened");
	// Print out a message containing all the information that libuvc knows about the device
	for ( i = 0; i < nCam; i++ ) uvc_print_diag(devh[i], stderr);
	// Try to negotiate a 160x120 9 fps Y16 stream profile
	for ( i = 0; i < nCam; i++ )
		res[i] = uvc_get_stream_ctrl_format_size(
			devh[i], &ctrl[i],    // result stored in ctrl
			UVC_FRAME_FORMAT_Y16,
			160, 120, 9);         // width, height, fps

	// Print out the result
	for ( i = 0; i < nCam; i++ )
	{
		uvc_print_stream_ctrl(&ctrl[i], stderr);
		if ( res[i] < 0 )
		{
			uvc_perror(res[i], "get_mode"); /* device doesn't provide a matching stream */

			return res[i];
		}
	}

	// Start the video stream. The library will call user function cb: cb(frame, (void*) 12345)
//	for ( i = 0; i < nCam; i++ )
//	{
////		if ( i == 0 ) res[i] = uvc_start_streaming(devh[i], &ctrl[i], cb, (void*) &idCam[i], 0);
//		res[i] = uvc_start_streaming(devh[i], &ctrl[i], cb, (void*) &idCam[i], 0);
////		res[i] = uvc_start_streaming(devh[i], &ctrl[i], cb_mysql, (void*) 12345, 0);
//		if ( res[i] < 0 )
//		{
//			uvc_perror(res[i], "start_streaming"); /* unable to start stream */
//			std::cout << "Error with Cam " << idCam[i] << std::endl;
//
//			return res[i];
//		}
//
//		puts("Streaming...");
//		uvc_set_ae_mode(devh[i], 2); /* e.g., turn on auto exposure */
//	}
//
//	sleep(10); /* stream for 10 seconds */
//	
//	/* End the stream. Blocks until last callback is serviced */
//	for ( i = 0; i < nCam; i++ )
//	{
//		uvc_stop_streaming(devh[i]);
//		puts("Done streaming.");
//
//		/* Release our handle on the device */
//		uvc_close(devh[i]);
//		puts("Device closed");
//
//		/* Release the device descriptor */
//		uvc_unref_device(dev[i]);
//	}

	for ( i = 0; i < nCam; i++ )
	{
		res[i] = uvc_start_streaming(devh[i], &ctrl[i], cb, (void*) &idCam[i], 0);
		if ( res[i] < 0 )
		{
			uvc_perror(res[i], "start_streaming"); /* unable to start stream */
			std::cout << "Error with Cam " << idCam[i] << std::endl;

			return res[i];
		}
		puts("Streaming...");
		uvc_set_ae_mode(devh[i], 2); /* e.g., turn on auto exposure */
	
		sleep(3);

		uvc_stop_streaming(devh[i]);
		puts("Done streaming.");
	}

	for ( i = 0; i < nCam; i++ )
	{
		/* Release our handle on the device */
		uvc_close(devh[i]);
		puts("Device closed");

		/* Release the device descriptor */
		uvc_unref_device(dev[i]);
	}

	/* Close the UVC context. This closes and cleans up any existing device handles,
	* and it closes the libusb context if one was not provided. */
	for ( i = 0; i < nCam; i++ ) uvc_exit(ctx[i]);
	puts("UVC exited");

	// mysql close
	mysql_close(conn);

	return 0;
}
