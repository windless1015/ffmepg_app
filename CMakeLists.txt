CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(ffmpegApp LANGUAGES C CXX)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")


#set qt5 and openmesh libraries directory
SET(Qt5_DIR D:/Librarys/Qt/Qt5.12.10/5.12.10/msvc2015_64/lib/cmake/Qt5)

SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} Qt5_DIR)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTOUIC_SEARCH_PATHS src/ui)

#FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Gui Multimedia MultimediaWidgets REQUIRED)

FIND_PACKAGE(Qt5 COMPONENTS Core Gui REQUIRED)
SET(FFMPEG_LIB_DIRS 
	D:/Librarys/ffmpeg/ffmpeg)
#SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} FFmpeg)

SET(AVCODEC_LIBRARY ${FFMPEG_LIB_DIRS}/lib/avcodec.lib)
SET(AVFORMAT_LIBRARY ${FFMPEG_LIB_DIRS}/lib/avformat.lib)
SET(AVUTIL_LIBRARY  ${FFMPEG_LIB_DIRS}/lib/avutil.lib)
SET(AVDEVICE_LIBRARY  ${FFMPEG_LIB_DIRS}/lib/avdevice.lib)
SET(SWSCALE_LIBRARY  ${FFMPEG_LIB_DIRS}/lib/swscale.lib)

#opencv
SET(OPENCV_DIR 
	D:/projects/ultras/libs_Make/ThirdPartyLibs/OpenCVInstall/include
)

SET(OPENCV_LIB_DIR 
	D:/projects/ultras/libs_Make/ThirdPartyLibs/OpenCVInstall/x64/vc14/lib
)
SET(HIGHGUI_LIBRARY  ${OPENCV_LIB_DIR}/opencv_highgui420d.lib)


#group source files, header files, resource files and ui files
SET(FFMPEGAPP_SRCS
	main.cpp
	src/encode_video.c
)

SET(FFMPEGAPP_HDRS

)


#add the resource files into qt project
QT5_ADD_RESOURCES(QRC_FILES ${QRC_SOURCE_FILES})


#integrate all the files to generate the executable program
ADD_EXECUTABLE( ${CMAKE_PROJECT_NAME}
  ${FFMPEGAPP_SRCS}
  ${FFMPEGAPP_HDRS}

)

include_directories(${CMAKE_PROJECT_NAME} 
	${FFMPEG_LIB_DIRS}/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${OPENCV_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
	${AVCODEC_LIBRARY} 
	${AVFORMAT_LIBRARY} 
	${AVUTIL_LIBRARY} 
	${AVDEVICE_LIBRARY}
	${SWSCALE_LIBRARY}
	Qt5::Core
	Qt5::Gui
	#${HIGHGUI_LIBRARY}
	
	debug ${OPENCV_LIB_DIR}/opencv_highgui420d.lib
	optimized ${OPENCV_LIB_DIR}/opencv_highgui420.lib
	
	debug ${OPENCV_LIB_DIR}/opencv_core420d.lib
	optimized ${OPENCV_LIB_DIR}/opencv_core420.lib   
	
	debug ${OPENCV_LIB_DIR}/opencv_videoio420d.lib
	optimized ${OPENCV_LIB_DIR}/opencv_videoio420.lib
	
	debug ${OPENCV_LIB_DIR}/opencv_imgproc420d.lib
	optimized ${OPENCV_LIB_DIR}/opencv_imgproc420.lib

)
message(STATUS "${AVDEVICE_LIBRARY}")
message(STATUS "${HIGHGUI_LIBRARY}")