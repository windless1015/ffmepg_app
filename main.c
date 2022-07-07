#include <stdio.h> 

#include "libavcodec/avcodec.h";
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"

int main()
{
		printf("%s\n", avcodec_configuration());
		system("pause");
		return 0;
}



