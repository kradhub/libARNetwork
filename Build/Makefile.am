###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS = -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES	=	../Includes/libNetwork/buffer.h \
					../Includes/libNetwork/ringBuffer.h \
					../Includes/libNetwork/ioBuffer.h\
					../Includes/libNetwork/sender.h \
					../Includes/libNetwork/receiver.h \
					../Includes/libNetwork/manager.h \
					../Includes/libNetwork/common.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES 	= 	$(HEADER_FILES)		\
					../Sources/buffer.c \
					../Sources/ringBuffer.c	\
					../Sources/ioBuffer.c\
					../Sources/sender.c \
					../Sources/receiver.c \
					../Sources/manager.c
					
# The library names to build (note we are building static and shared libs)
if DEBUG_MODE
lib_LTLIBRARIES 		= libnetwork_dbg.la
libnetwork_dbg_ladir 		= $(includedir)/libNetwork
libnetwork_dbg_la_HEADERS 	= $(HEADER_FILES)
libnetwork_dbg_la_SOURCES 	= $(SOURCE_FILES)
else
lib_LTLIBRARIES 		= libnetwork.la
libnetwork_ladir 		= $(includedir)/libNetwork
libnetwork_la_HEADERS 	= $(HEADER_FILES)
libnetwork_la_SOURCES 	= $(SOURCE_FILES)
endif



check_PROGRAMS = 	../TestBench/autoTest \
					../TestBench/test2/manager \
					../TestBench/test2/repeater \
					../TestBench/test3/managerT3 \
					../TestBench/test3/repeaterT3 \
					../TestBench/test4/test4  
					
___TestBench_autoTest_SOURCES 	= ../TestBench/autoTest.c
___TestBench_test2_manager_SOURCES 	= ../TestBench/test2/mainManager.c
___TestBench_test2_repeater_SOURCES = ../TestBench/test2/mainRepeater.c
___TestBench_test3_managerT3_SOURCES 	= ../TestBench/test3/mainManagerT3.c
___TestBench_test3_repeaterT3_SOURCES = ../TestBench/test3/mainRepeaterT3.c
___TestBench_test4_test4_SOURCES = ../TestBench/test4/mainT4.c
if DEBUG_MODE
___TestBench_autoTest_LDADD = 	-lsal_dbg  \
							libnetwork_dbg.la
___TestBench_test2_manager_LDADD = 	-lsal_dbg  \
									libnetwork_dbg.la
___TestBench_test2_repeater_LDADD = -lsal_dbg  \
									libnetwork_dbg.la
___TestBench_test3_managerT3_LDADD = 	-lsal_dbg  \
									libnetwork_dbg.la
___TestBench_test3_repeaterT3_LDADD = -lsal_dbg  \
									libnetwork_dbg.la
___TestBench_test4_test4_LDADD = -lsal_dbg  \
									libnetwork_dbg.la
else
___TestBench_autoTest_LDADD 	= 	-lsal \
								libnetwork.la
___TestBench_test2_manager_LDADD = 	-lsal  \
									libnetwork.la
___TestBench_test2_repeater_LDADD = -lsal  \
									libnetwork.la
___TestBench_test3_managerT3_LDADD = 	-lsal  \
									libnetwork.la
___TestBench_test3_repeaterT3_LDADD = -lsal  \
									libnetwork.la
___TestBench_test4_test4_LDADD = -lsal  \
								libnetwork.la
endif

CLEAN_FILES = libnetwork.la libnetwork_dbg.la

EXTRA_DIST = bootstrap

# Ensure the distribution of the doxygen configuration file
# Doxygen support
if HAVE_DOXYGEN
include doxygen.am
EXTRA_DIST += doxygen.ac doxygen.am $(DX_CONFIG)
endif
