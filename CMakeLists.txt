CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(ffmpegApp LANGUAGES C CXX)

#set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-D__STDC_CONSTANT_MACROS")


SET(FFMPEG_LIB_DIRS 
	D:/projects/ffmpeg/ffmpeg-shared)
#SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} FFmpeg)




SET(AVCODEC_LIBRARY ${FFMPEG_LIB_DIRS}/lib/avcodec.lib)

SET(AVFORMAT_LIBRARY ${FFMPEG_LIB_DIRS}/lib/avformat.lib)


SET(AVUTIL_LIBRARY  ${FFMPEG_LIB_DIRS}/lib/avutil.lib)


SET(AVDEVICE_LIBRARY  ${FFMPEG_LIB_DIRS}/lib/avdevice.lib)


#group source files, header files, resource files and ui files
SET(FFMPEGAPP_SRCS
	main.c
)

SET(FFMPEGAPP_HDRS

)


#find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE REQUIRED)



#integrate all the files to generate the executable program
ADD_EXECUTABLE( ${CMAKE_PROJECT_NAME}
  ${FFMPEGAPP_SRCS}
  ${FFMPEGAPP_HDRS}
)

include_directories(${CMAKE_PROJECT_NAME} 
	${FFMPEG_LIB_DIRS}/include
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
	${AVCODEC_LIBRARY} 
	${AVFORMAT_LIBRARY} 
	${AVUTIL_LIBRARY} 
	${AVDEVICE_LIBRARY}
)
message(STATUS "${AVDEVICE_LIBRARY}")