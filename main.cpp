#include <QApplication>
#include <stdio.h> 

#include "libavcodec/avcodec.h";
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"

#include "src/camera.h"
int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	Camera camera;
	camera.show();

	//printf("%s\n", avcodec_configuration());
	//system("pause");
	return app.exec();
}



