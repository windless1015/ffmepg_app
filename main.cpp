//#include <QApplication>
#include <stdio.h> 
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

#include <string>
#include <iostream>
#include <QImage>
using namespace std;
using namespace cv;

QImage MatToQImage(const cv::Mat& mat)
{
	if (mat.type() == CV_8UC1)
	{
		// Make all channels (RGB) identical via a LUT
		QVector<QRgb> colorTable;
		for (int i = 0; i < 256; i++)
		{
			colorTable.push_back(qRgb(i, i, i));
		}

		// Create QImage from cv::Mat
		const unsigned char *qImageBuffer = (const unsigned char*)mat.data;
		QImage img(qImageBuffer, mat.cols, mat.rows, (int)mat.step, QImage::Format_Indexed8);
		img.setColorTable(colorTable);
		return img;
	}
	else if (mat.type() == CV_8UC3)
	{
		// Create QImage from cv::Mat
		const unsigned char *qImageBuffer = (const unsigned char*)mat.data;
		return QImage(qImageBuffer, mat.cols, mat.rows, (int)mat.step, QImage::Format_RGB888).rgbSwapped();
	}
	else
	{
		return QImage();
	}
}

/*
https://ffmpeg.org/doxygen/trunk/encode_video_8c-example.html
*/


//int main(int argc, char** argv)
//{
//
//	/*printf("%s\n", avcodec_configuration());
//	system("pause");*/
//
//	VideoCapture vc1(0);
//
//	// 判断视频是否初始化成功
//	if (!vc1.isOpened())
//	{
//		//fprintf(stderr, "failed to open %s\n", "O:\\image\\1.mp4");
//		return EXIT_FAILURE;
//	}
//
//	// 声明视频写入对象
//	string fileName = "O:\\image\\result.avi";
//	//VideoWriter writer = VideoWriter(fileName, VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, Size(1280, 720), true);
//
//	// 循环处理视频的每一帧，先显示，再写入
//	Mat frame;
//	vc1 >> frame;
//	Mat destMat;
//	while (!frame.empty())
//	{
//		imshow("demo", frame);
//
//		QImage img = MatToQImage(frame);
//		//img.save("D:/img.jpeg");
//
//		// 写入视频帧
//		//writer.write(frame);
//		vc1 >> frame;
//		int k = waitKey(30);
//		// 实现按键暂停和退出视频播放功能		
//		if (k == 27)
//			break;
//		else if (k == 32)
//		{
//			while (waitKey(0) != 32)
//				waitKey(0);
//		}
//	}
//	// 释放资源，清空缓存
//	destroyAllWindows();
//	//writer.release();
//
//	system("pause");
//	return EXIT_SUCCESS;
//
//}











int main(int argc, char *argv[])
{
	/*if (argc < 2) {
		std::cout << "Usage:" << std::endl;
		std::cout << "ex) ./ffmpegOpenCV.exe output.mp4" << std::endl;
		return 1;
	}*/
	//const char *output_filename = argv[1];
	const char *output_filename = "D:/testOpencv.mp4";


	// initialize FFmpeg library
	//av_register_all();  ffmpeg4.0以后可以忽略这个函数  https://github.com/leandromoreira/ffmpeg-libav-tutorial/issues/29

	int ret;

	// Output video size
	const int output_w = 1280;
	const int output_h = 720;
	// Output video FPS
	const AVRational dst_fps = { 24, 1 };
	const int64_t bit_rate = 0;

	// Open webcam
	cv::VideoCapture video_capture(0);
	if (!video_capture.isOpened()) {
		std::cerr << "Failed to open VideoCapture." << std::endl;
		return 2;
	}
	video_capture.set(cv::CAP_PROP_FRAME_WIDTH, output_w);
	video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, output_h);

	// Allocate cv::Mat with extra bytes (required by AVFrame::data)
	std::vector<uint8_t> imgbuf(output_h * output_w * 4 + 16);
	cv::Mat image(output_h, output_w, CV_8UC4, imgbuf.data(), output_w * 4);

	// Open FormatContext (for output)
	AVFormatContext *format_context = nullptr;
	ret = avformat_alloc_output_context2(&format_context, nullptr, nullptr, output_filename);
	if (ret < 0) {
		std::cerr << "Failed to alloc memory for output. avformat_alloc_output_context2(" << output_filename << "): ret=" << ret << std::endl;
		return 2;
	}

	// Open output IO context
	ret = avio_open2(&format_context->pb, output_filename, AVIO_FLAG_WRITE, nullptr, nullptr);
	if (ret < 0) {
		std::cerr << "Failed to open IO context. avio_open2(): ret=" << ret << std::endl;
		return 2;
	}

	// Create new video stream to FormatContext
	AVCodec *video_codec = const_cast <AVCodec*>(avcodec_find_encoder(format_context->oformat->video_codec));

	// Create codec context
	AVCodecContext *codec_context = NULL;
	codec_context = avcodec_alloc_context3(video_codec);
	if (!codec_context) {
		std::cerr << "Could not allocate video codec context. avcodec_alloc_context3()" << std::endl;
		return 2;
	}
	// Set parameters
	codec_context->bit_rate = bit_rate;
	codec_context->width = output_w;
	codec_context->height = output_h;
	codec_context->time_base = av_inv_q(dst_fps);
	codec_context->pix_fmt = AV_PIX_FMT_YUV420P; // H.264 should be planar YUV 4:2:0 chroma sampling
	if (format_context->oformat->flags & AVFMT_GLOBALHEADER)
		codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	// Set encode parameters (-preset veryfast -crf 23 -profile:v high -level 4.1)
	AVDictionary *codec_options = nullptr;
	av_dict_set(&codec_options, "preset", "veryfast", 0);
	av_dict_set(&codec_options, "crf", "23", 0);
	av_dict_set(&codec_options, "profile", "high", 0);
	av_dict_set(&codec_options, "level", "4.1", 0);

	// Open video encoder
	ret = avcodec_open2(codec_context, video_codec, &codec_options);
	if (ret < 0) {
		std::cerr << "Failed to open video encorder. avcodec_open2() : ret=" << ret << std::endl;
		return 2;
	}

	std::cout
		<< "output file: " << output_filename << "\n"
		<< "format:      " << format_context->oformat->name << "\n"
		<< "video_codec: " << video_codec->name << "\n"
		<< "bitrate:     " << codec_context->bit_rate << "\n"
		<< "size:        " << output_w << 'x' << output_h << "\n"
		<< "fps:         " << av_q2d(dst_fps) << "\n"
		<< "pixfmt:      " << av_get_pix_fmt_name(codec_context->pix_fmt) << "\n"
		<< std::flush;

	// Create new video stream to FormatContext
	AVStream *video_stream = avformat_new_stream(format_context, video_codec);
	if (!video_stream) {
		std::cerr << "Failed to create video stream. avformat_new_stream()." << std::endl;
		return 2;
	}
	// Set parameters
	video_stream->time_base = av_inv_q(dst_fps);
	video_stream->r_frame_rate = video_stream->avg_frame_rate = dst_fps;

	// Copy parameters from codec context to video stream
	avcodec_parameters_from_context(video_stream->codecpar, codec_context);

	// Write the stream header
	avformat_write_header(format_context, nullptr);

	// Init sample scaler
	SwsContext *sws_context = sws_getCachedContext(
		nullptr, output_w, output_h, AV_PIX_FMT_BGR24,
		output_w, output_h, codec_context->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
	if (!sws_context) {
		std::cerr << "Failed to initialize samplesws_getCachedContext()" << std::endl;
		return 2;
	}

	// Allocate frame buffer for encoding
	AVFrame *frame = av_frame_alloc();
	int buffer_img_size = av_image_get_buffer_size(codec_context->pix_fmt, output_w, output_h, 1);

	uint8_t *buffer = (uint8_t *)av_mallocz(buffer_img_size);
	av_image_fill_arrays(frame->data, frame->linesize, buffer, codec_context->pix_fmt, output_w, output_h, 1);
	av_freep(buffer);
	frame->width = output_w;
	frame->height = output_h;
	frame->format = static_cast<int>(codec_context->pix_fmt);

	// Start encoding
	int64_t frame_pts = 0;
	unsigned encoded_frames = 0;
	bool is_flushing = false;
	do {
		if (!is_flushing) {
			// Read video capture image
			video_capture >> image;
			cv::imshow("press ESC to exit", image);
			if (cv::waitKey(33) == 0x1b)
				is_flushing = true;
		}
		if (!is_flushing) {
			// Convert cv::Mat to AVFrame (OpenCV to FFmpeg)
			const int stride[] = { static_cast<int>(image.step[0]) };
			sws_scale(sws_context, &image.data, stride, 0, image.rows, frame->data, frame->linesize);
			frame->pts = frame_pts++; // Set presentation timestamp
		}
		// Create packet container
		AVPacket packet;
		packet.data = nullptr;
		packet.size = 0;
		av_init_packet(&packet);

		// Send frame (Set NULL if a frame is a flush packet)
		ret = avcodec_send_frame(codec_context, is_flushing ? nullptr : frame);
		if (ret < 0) {
			std::cerr << "Error encoding frame" << std::endl;
			break;
		}
		while (ret >= 0) {
			// Receive packet
			ret = avcodec_receive_packet(codec_context, &packet);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			else if (ret < 0) {
				std::cerr << "Error during encoding" << std::endl;
				break;
			}
			// Set stream index
			packet.stream_index = 0;
			// Rescale timestamp in a packet
			av_packet_rescale_ts(&packet, codec_context->time_base, video_stream->time_base);
			// Write packet to FormatContext
			av_write_frame(format_context, &packet);
			std::cout << encoded_frames << '\r' << std::flush;  // dump progress
			encoded_frames++;
			av_packet_unref(&packet);
		}

	} while (!is_flushing);

	// Finish encoding
	av_write_trailer(format_context);
	std::cout << encoded_frames << " frames have been encoded" << std::endl;

	av_frame_free(&frame);
	avcodec_close(codec_context);
	avio_close(format_context->pb);
	avformat_free_context(format_context);

	return 0;
}



