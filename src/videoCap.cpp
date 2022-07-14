// VideoCapture.cpp: 定义控制台应用程序的入口点。
//FFmpeg Version 4.02

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
}
#include <dshow.h>


// video=Logitech HD Pro Webcam C920      0
AVInputFormat *inputFormat = nullptr;
AVFormatContext *deformatContext = nullptr;
AVCodecContext *vDeCodecContext = nullptr;
AVCodec *vDeCodec = nullptr;
int videoIndex = -1;

// mux
AVStream * vEnstream = nullptr;
AVCodec *vEnCodec = nullptr;
AVCodecContext *vEncodecContext = nullptr;
AVOutputFormat *vOutputFormat = nullptr;
AVFormatContext *enformatContext = nullptr;
SwsContext *scaleContext = nullptr;
AVFrame *enFrame = nullptr;
AVPacket *enPacket = nullptr;

// function
int writeData(AVFrame *frame);
int resourceRecovery();
int FlushEncoder();
int64_t offsetPts = 0;

int Opendevice() {
	AVDictionary *option = NULL;
	inputFormat = const_cast <AVInputFormat*>(av_find_input_format("dshow"));
	int ret = 0;
	ret = av_dict_set_int(&option, "video_device_number", 0, 0);
	if ( ret < 0) {		
		printf("option set video_device_number 0 error %d\n",ret);
		return 0;
	}

	//ret = avformat_open_input(&deformatContext, "video=Logitech HD Pro Webcam C920", inputFormat, &option);

	ret = avformat_open_input(&deformatContext, "video=Microsoft® LifeCam HD-3000", inputFormat, &option);
	if (ret != 0) {
		// error -2 no file or directory
		printf("avformat_open_input error %d\n",ret);
		av_dict_free(&option);
		return -1;
	}
	av_dict_free(&option);

	ret = avformat_find_stream_info(deformatContext, NULL);
	if (ret < 0) {
		printf("avformat_find_stream_info error %d\n", ret);
		return -1;
	}

	for (int index(0); index < deformatContext->nb_streams; index++) {		
		if (deformatContext->streams[index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = index;

			printf("video stream timebase num = %d , den =%d\n",
				deformatContext->streams[index]->time_base.num,
				deformatContext->streams[index]->time_base.den );

			break;
		}
	
	}

	if (videoIndex < 0) {
		printf("can't find video stream");
		return -1;
	}

	vDeCodec = const_cast <AVCodec*>( avcodec_find_decoder(deformatContext->streams[videoIndex]->codecpar->codec_id));
	if (vDeCodec == nullptr) {
		printf("can't find deCoder codec_id %d \n", deformatContext->streams[videoIndex]->codecpar->codec_id);
		return -1;
	}

	vDeCodecContext = avcodec_alloc_context3(vDeCodec);
	avcodec_parameters_to_context(vDeCodecContext, deformatContext->streams[videoIndex]->codecpar);
	printf("codecpar timebase num = %d, den = %d\n", vDeCodecContext->time_base.num, vDeCodecContext->time_base.den);

	ret = avcodec_open2(vDeCodecContext, vDeCodec, NULL);
	if (ret != 0) {
		printf("avcodec_open2 error with %d\n",ret);
		avcodec_free_context(&vDeCodecContext);
		avformat_close_input(&deformatContext);
		return -1;
	}

	return 0;
}


int WriteMP4Tail() {

	FlushEncoder();
	av_write_trailer(enformatContext);
	avio_close(enformatContext->pb);

	return 0;
}


int CreateMP4File(const char *path) {
	vEnCodec = const_cast <AVCodec*>(avcodec_find_encoder_by_name("h264_nvenc"));
	//	if (vEnCodec == nullptr) {
	//		vEnCodec = avcodec_find_encoder_by_name("libx264");
	//	}

	vOutputFormat = const_cast <AVOutputFormat*>(av_guess_format("mp4", NULL, NULL));
	if (vOutputFormat == nullptr) {
		printf("av_guess_format mp4 fail\n");
		return -1;
	}

	enformatContext = avformat_alloc_context();
	enformatContext->oformat = vOutputFormat;

	vEnstream = avformat_new_stream(enformatContext, vEnCodec);



	vEncodecContext = avcodec_alloc_context3(vEnCodec);
	//	vEncodecContext->codec = vEnCodec;

	vEnstream->time_base = deformatContext->streams[videoIndex]->time_base;

	vEncodecContext->height = 960;// vDeCodecContext->height;
	vEncodecContext->width = 1280;// vDeCodecContext->width;
	vEncodecContext->bit_rate = 8000000;
	vEncodecContext->bit_rate_tolerance = 4000000;

	vEncodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	vEncodecContext->gop_size = 56;
	vEncodecContext->max_b_frames = 2;


	vEncodecContext->time_base = vEnstream->time_base;

	vEncodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


	vEncodecContext->me_range = 16;
	vEncodecContext->max_qdiff = 4;
	vEncodecContext->qmin = 10;
	vEncodecContext->qmax = 51;
	vEncodecContext->qcompress = 0.6;

	if (vEncodecContext->codec_id == AV_CODEC_ID_H264) {
		av_opt_set(vEncodecContext->priv_data, "preset", "slow", 0);
	}


	// -22 use h264_nvenc codec with mp4 error  notic ffmpeg error message 
	// I updata GPU drive ,it work 
	int ret = avcodec_open2(vEncodecContext, vEncodecContext->codec, NULL);  
	if (ret != 0) {
		printf("avcodec_open2 error with %d\n", ret);
		goto error;
	}

	ret = avcodec_parameters_from_context(vEnstream->codecpar, vEncodecContext);
	if (ret < 0) {
		printf("avcodec_parameters_from_context error with ret");
		goto error;
	}

	ret = avio_open(&(enformatContext->pb), path, AVIO_FLAG_WRITE);
	if (ret < 0) {
		printf("avio_open error with %d\n", ret);
		goto error;
	}

	ret = avformat_write_header(enformatContext, NULL);
	if (ret < 0) {
		printf("avformat_write_header error with %d\n", ret);
		goto error;
	}

	scaleContext = sws_alloc_context();

	// vDeCodecContext->pix_fmt: ffmpeg suggest don't use AV_PIX_FMT_YUVJ422 P , convert to AV_PIX_FMT_YUV422P
	scaleContext = sws_getContext(vDeCodecContext->width, vDeCodecContext->height, vDeCodecContext->pix_fmt,    
		vEncodecContext->width, vEncodecContext->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	if (scaleContext == nullptr) {
		sws_freeContext(scaleContext);
		printf("sws_getContext error\n");
		goto error;
	}
	
	enFrame = av_frame_alloc();
	enFrame->width = vEncodecContext->width;
	enFrame->height = vEncodecContext->height;
	enFrame->format = vEncodecContext->pix_fmt;
	av_frame_get_buffer(enFrame, 0);   // we don't need calculate linesize ,function fill
	
	enPacket = av_packet_alloc();

	return 0;
error:
	avcodec_free_context(&vEncodecContext);
	avformat_free_context(enformatContext);
	return -1;
}


int ReceiveData(int frame) {
	AVPacket *demux_packet = av_packet_alloc();
	AVFrame *demux_frame = av_frame_alloc();
	bool start = false;
	if (CreateMP4File("D:/fpsT.mp4") < 0) {
		printf("CreateFile error\n");
		return -1;
	}
	double startTime = 0;
	//while (frame --) {
	while (1) 
	{

		int key = 0;
		if (key = GetKeyState(VK_ESCAPE) < 0)
			break;
		printf("key: %d\n", key);
		if (av_read_frame(deformatContext, demux_packet) == 0) {
			if (demux_packet->stream_index == videoIndex) {
				if (avcodec_send_packet(vDeCodecContext, demux_packet) == 0) {
					if (avcodec_receive_frame(vDeCodecContext, demux_frame) == 0) {
						if (!start) {
							start = true;
							offsetPts = av_rescale_q(demux_frame->pts, enformatContext->streams[videoIndex]->time_base, vEncodecContext->time_base);
						}
					//	printf("frame pts	%lf\n", demux_frame->pts *av_q2d(deformatContext->streams[videoIndex]->time_base));
				//		printf("times %lld\n", av_gettime_relative());
						writeData(demux_frame);
					}
					av_frame_unref(demux_frame);
				}
				av_packet_unref(demux_packet);
			}
		}
	}

	// clear decodec buffer
	int ret = avcodec_send_packet(vDeCodecContext, NULL);
	while (!ret) {
		if (avcodec_receive_frame(vDeCodecContext, demux_frame) == 0) {
			
			writeData(demux_frame);
		}
		else
			break;
	//	av_frame_unref(demux_frame);
	//	AVERROR_EOF  there is no new frame
	}

	WriteMP4Tail();

	av_packet_free(&demux_packet);
	av_frame_free(&demux_frame);
	return 0;
}


int FlushEncoder() {
	
	int nframes = 0;
	if (avcodec_send_frame(vEncodecContext, NULL) == 0) {
		while (avcodec_receive_packet(vEncodecContext, enPacket) == 0) {
			nframes++;
			enPacket->stream_index = 0;
			av_packet_rescale_ts(enPacket, vEncodecContext->time_base, enformatContext->streams[0]->time_base);
			av_interleaved_write_frame(enformatContext, enPacket);
		}
	}
	return nframes;
}


int writeData(AVFrame *frame) {
	
	int scaleH = sws_scale(scaleContext, frame->data, frame->linesize, 0,frame->height,enFrame->data, enFrame->linesize); // notice params site  
	
	if (scaleH == vEncodecContext->height) {
		enFrame->pts = av_rescale_q(frame->pts,enformatContext->streams[videoIndex]->time_base,vEncodecContext->time_base) - offsetPts;
		if (avcodec_send_frame(vEncodecContext, enFrame) == 0) {
			if (avcodec_receive_packet(vEncodecContext, enPacket) == 0) {
				enPacket->stream_index = 0;
			//	av_packet_rescale_ts(enPacket, vEncodecContext->time_base, enformatContext->streams[0]->time_base);
				av_interleaved_write_frame(enformatContext, enPacket);
				
			}
		}
		
	}
	else
		printf("scale dst %d, runtime %d\n",vEncodecContext->height, scaleH);
	
	return 0;
}


int resourceRecovery() {

//	avcodec_close(vEncodecContext);
//	avcodec_close(vDeCodecContext);

	// before free Context
	av_packet_unref(enPacket);
	av_frame_unref(enFrame);

	av_packet_free(&enPacket);
	av_frame_free(&enFrame);

	avcodec_free_context(&vDeCodecContext);
	avcodec_free_context(&vEncodecContext);
	

	
	avformat_free_context(deformatContext);
	avformat_free_context(enformatContext);

	sws_freeContext(scaleContext);

	
	return 0;
}

int main()
{

	avdevice_register_all();
	avformat_network_init();
	if (Opendevice() < 0)
		return -1;
	
	ReceiveData(120);

	resourceRecovery();
	
	return 0;
}