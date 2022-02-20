# CMakeList.txt: scream 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("scream" VERSION 1.0.0)

# --------------------------------------------------------------------------
# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED True)


######################################
#  config setting
######################################

option(LOG_USE_ASYN "LOG_USE_ASYN" ON)
set(SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE)
set(LOG_FILE_NAME "appliaction.log")
set(LOG_THREAD_COUNT 1)

configure_file(config.h.in "${PROJECT_SOURCE_DIR}/include/config.h")

include(./.SET_LOCAL_PATH.cmake)


set(USE_SCREAM 1)
# 包含子项目。
if(USE_SCREAM)
    # 静态库生成方案
    aux_source_directory(./include scream_H)
    add_library(scream
    	STATIC  
		 src/decoder.cpp
		 src/encoder.cpp
		 src/fec.cpp
		 src/grabber.cpp
		 src/jitter-buffer.cpp
		 src/media-packet.cpp
		 src/packet-decoder.cpp
		 src/parser.cpp
		 src/rtpqueue.cpp
		 src/screamreceiver.cpp
		 src/screamRx.cpp
		 src/screamsender.cpp
		 src/screamTx.cpp
		 src/videoenc.cpp
           
        )
    target_include_directories(scream PUBLIC ${PROJECT_SOURCE_DIR}/include)
else()
	add_executable (${PROJECT_NAME} 
		  "test/test.cpp"
		  #"src/converter.cpp"
		  "src/decoder.cpp"
		  "src/encoder.cpp"
		  "src/fec.cpp"
		  "src/grabber.cpp"
		  "src/jitter-buffer.cpp"
		  "src/media-packet.cpp"
		  "src/packet-decoder.cpp"
		  "src/parser.cpp"
		 # "src/player.cpp"
		  "src/rtpqueue.cpp"
		  #"src/scream-header.cpp"
		  "src/screamreceiver.cpp"
		  "src/screamRx.cpp"
		  "src/screamsender.cpp"
		  #"src/scream-trailer.cpp"
		  "src/screamTx.cpp"
		  "src/videoenc.cpp"
	)
endif()
# 将源代码添加到此项目的可执行文件。
#add_executable (scream "scream.cpp" "scream.h")

####################################################
#  FFmpeg lib setting
####################################################

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY "avcodec")

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY "avformat")

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY "avutil")

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY "avdevice")

find_path(AVFILTER_INCLUDE_DIR libavfilter/avfilter.h)
find_library(AVFILTER_LIBRARY "avfilter")

find_path(POSTPROC_INCLUDE_DIR libpostproc/postprocess.h)
find_library(POSTPROC_LIBRARY "postproc")

find_path(SWRESAMPLE_INCLUDE_DIR libswresample/swresample.h)
find_library(SWRESAMPLE_LIBRARY "swresample")

find_path(SWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY "swscale")

# 配置链接库
if(WIN32)
	target_link_libraries( ${PROJECT_NAME}  
		PRIVATE 
		#${PROJECT_SOURCE_DIR}/out/build/x64-Debug/signalingSdk7.lib
			${AVCODEC_LIBRARY}
            ${AVFORMAT_LIBRARY}
            ${AVUTIL_LIBRARY}
            ${AVDEVICE_LIBRARY}
            ${AVFILTER_LIBRARY}
            ${SWRESAMPLE_LIBRARY}
            ${SWSCALE_LIBRARY}
	)
elseif(UNIX)
	target_link_libraries( ${PROJECT_NAME}  
		PRIVATE 
		${PTHEARD_LIBRARY}
			
	)
endif()

target_include_directories( ${PROJECT_NAME}  
		PRIVATE 
			${PROJECT_SOURCE_DIR}/include
			${ASIO_DIR}
			${AVCODEC_INCLUDE_DIR}
			${AVFORMAT_INCLUDE_DIR}
			${AVUTIL_INCLUDE_DIR}
			${AVDEVICE_INCLUDE_DIR}
			${AVFILTER_INCLUDE_DIR}
			${SWRESAMPLE_INCLUDE_DIR}
			${SWSCALE_INCLUDE_DIR}
	)
