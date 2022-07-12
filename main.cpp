//#include <QApplication>
#include <stdio.h> 
extern "C" {
#include "libavcodec/avcodec.h";
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

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


int main(int argc, char** argv)
{

	/*printf("%s\n", avcodec_configuration());
	system("pause");*/

	VideoCapture vc1(0);

	// �ж���Ƶ�Ƿ��ʼ���ɹ�
	if (!vc1.isOpened())
	{
		//fprintf(stderr, "failed to open %s\n", "O:\\image\\1.mp4");
		return EXIT_FAILURE;
	}

	// ������Ƶд�����
	string fileName = "O:\\image\\result.avi";
	//VideoWriter writer = VideoWriter(fileName, VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, Size(1280, 720), true);

	// ѭ��������Ƶ��ÿһ֡������ʾ����д��
	Mat frame;
	vc1 >> frame;
	Mat destMat;
	while (!frame.empty())
	{
		imshow("demo", frame);

		/*QImage img = MatToQImage(frame);
		img.save("D:/img.jpeg");*/

		// д����Ƶ֡
		//writer.write(frame);
		vc1 >> frame;
		int k = waitKey(30);
		// ʵ�ְ�����ͣ���˳���Ƶ���Ź���		
		if (k == 27)
			break;
		else if (k == 32)
		{
			while (waitKey(0) != 32)
				waitKey(0);
		}
	}
	// �ͷ���Դ����ջ���
	destroyAllWindows();
	//writer.release();

	system("pause");
	return EXIT_SUCCESS;

}



