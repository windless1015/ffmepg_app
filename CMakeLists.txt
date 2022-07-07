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

FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Gui Multimedia MultimediaWidgets REQUIRED)

SET(FFMPEG_LIB_DIRS 
	D:/projects/ffmpeg/ffmpeg-shared)
#SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} FFmpeg)

SET(AVCODEC_LIBRARY ${FFMPEG_LIB_DIRS}/lib/avcodec.lib)
SET(AVFORMAT_LIBRARY ${FFMPEG_LIB_DIRS}/lib/avformat.lib)
SET(AVUTIL_LIBRARY  ${FFMPEG_LIB_DIRS}/lib/avutil.lib)
SET(AVDEVICE_LIBRARY  ${FFMPEG_LIB_DIRS}/lib/avdevice.lib)

#group source files, header files, resource files and ui files
SET(FFMPEGAPP_SRCS
	main.cpp
	src/camera.cpp
)

SET(FFMPEGAPP_HDRS
	src/camera.h
)

SET(UI_FILES
	src/ui/camera.ui
)
SET(QRC_SOURCE_FILES 
	src/ui/camera.qrc
)

#add the resource files into qt project
QT5_ADD_RESOURCES(QRC_FILES ${QRC_SOURCE_FILES})


#integrate all the files to generate the executable program
ADD_EXECUTABLE( ${CMAKE_PROJECT_NAME}
  ${FFMPEGAPP_SRCS}
  ${FFMPEGAPP_HDRS}
  #${QRC_FILES}
  ${UI_FILES}
)

include_directories(${CMAKE_PROJECT_NAME} 
	${FFMPEG_LIB_DIRS}/include
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
	${AVCODEC_LIBRARY} 
	${AVFORMAT_LIBRARY} 
	${AVUTIL_LIBRARY} 
	${AVDEVICE_LIBRARY}
	Qt5::Core
	Qt5::Widgets  
	Qt5::Gui
	Qt5::Multimedia
	Qt5::MultimediaWidgets
)
message(STATUS "${AVDEVICE_LIBRARY}")