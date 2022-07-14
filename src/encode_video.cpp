//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//

//
//


#include <stdio.h> 
#include <iostream>
extern "C" {
#include "libavcodec/avcodec.h";
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/buffersink.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
}

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
	FILE *outfile)
{
	int ret;

	/* send the frame to the encoder */
	if (frame)
		printf("Send frame %ld\n", frame->pts);

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		printf("Write packet %ld, size=%5d)\n", pkt->pts, pkt->size);
		fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt);
	}
}

int main_test(int argc, char **argv)
{
	const int output_w = 1432;
	const int output_h = 730;

	const char *filename, *codec_name;
	const AVCodec *codec;
	AVCodecContext *c = NULL;
	int i, ret, x, y;
	FILE *f;
	AVFrame *frame;
	AVPacket *pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	/*if (argc <= 2) {
		fprintf(stderr, "Usage: %s <output file> <codec name>\n", argv[0]);
		exit(0);
	}
	filename = argv[1];
	codec_name = argv[2];*/

	filename = "D:/official_encodeExample.mp4";

	/* find the mpeg1video encoder */
	//codec = avcodec_find_encoder_by_name(codec_name);

	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		fprintf(stderr, "Codec '%s' not found\n", codec_name);
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	pkt = av_packet_alloc();
	if (!pkt)
		exit(1);

	/* put sample parameters */  // reference: https://blog.csdn.net/benkaoya/article/details/79558896
	c->bit_rate = 4000000; //4Mbps
	/* resolution must be a multiple of two */
	c->width = output_w;
	c->height = output_h;
	/* frames per second */
	AVRational tbs = {1, 30 };
	AVRational fr = { 30,1 };
	c->time_base = tbs;
	c->framerate = fr;

	/* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	/* open it */
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) 
	{
		char errBuf[1024];
		char* errStr = av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, ret);
		fprintf(stderr, "Could not open codec: %s\n", errStr);
		exit(1);
	}

	f = fopen(filename, "wb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;

	ret = av_frame_get_buffer(frame, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}





	// Open webcam
	cv::VideoCapture video_capture(0);
	if (!video_capture.isOpened()) {
		std::cout << "Failed to open VideoCapture." << std::endl;
		return 2;
	}
	video_capture.set(cv::CAP_PROP_FRAME_WIDTH, output_w);
	video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, output_h);
	// Allocate cv::Mat with extra bytes (required by AVFrame::data)
	std::vector<uint8_t> imgbuf(output_h * output_w * 4 + 16);
	cv::Mat image(output_h, output_w, CV_8UC4, imgbuf.data(), output_w * 4);

	/* encode video */
	bool is_flushing = false;
	for(;;)
	{
		fflush(stdout);

		if (!is_flushing) 
		{
			// Read video capture image
			video_capture >> image;
			cv::imshow("press ESC to exit", image);
			if (cv::waitKey(33) == 0x1b)
				is_flushing = true;
		}

		/* Make sure the frame data is writable.
		On the first round, the frame is fresh from av_frame_get_buffer()
		and therefore we know it is writable.
		But on the next rounds, encode() will have called
		avcodec_send_frame(), and the codec may have kept a reference to
		the frame in its internal structures, that makes the frame
		unwritable.
		av_frame_make_writable() checks that and allocates a new buffer
		for the frame only if necessary.
		*/
		ret = av_frame_make_writable(frame);
		if (ret < 0)
			exit(1);

		/* Prepare a dummy image.
		In real code, this is where you would have your own logic for
		filling the frame. FFmpeg does not care what you put in the
		frame.
		*/
		if (!is_flushing) 
		{
			//const int stride[] = { static_cast<int>(image.step[0]) };
			//sws_scale(sws_context, &image.data, stride, 0, image.rows, frame->data, frame->linesize);
			//frame->pts = frame_pts++; // Set presentation timestamp

			frame->pts = i;
		}
		/* encode the image */
		encode(c, frame, pkt, f);
	}

	/* flush the encoder */
	encode(c, NULL, pkt, f);

	/* Add sequence end code to have a real MPEG file.
	It makes only sense because this tiny examples writes packets
	directly. This is called "elementary stream" and only works for some
	codecs. To create a valid file, you usually need to write packets
	into a proper file format or protocol; see muxing.c.
	*/
	if (codec->id == AV_CODEC_ID_H264)
		fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	return 0;
}
