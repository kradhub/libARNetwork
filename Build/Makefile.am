###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS = -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES	=	../Includes/libNetwork/buffer.h \
					../Includes/libNetwork/ringBuffer.h \
					../Includes/libNetwork/inOutBuffer.h\
					../Includes/libNetwork/sender.h \
					../Includes/libNetwork/receiver.h \
					../Includes/libNetwork/network.h \
					../Includes/libNetwork/common.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES 	= 	$(HEADER_FILES)		\
					../Sources/buffer.c \
					../Sources/ringBuffer.c	\
					../Sources/inOutBuffer.c\
					../Sources/sender.c \
					../Sources/receiver.c \
					../Sources/network.c
					
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



check_PROGRAMS = 	../TestBench/test1 \
					../TestBench/test2/manager \
					../TestBench/test2/repeater 
					
___TestBench_test1_SOURCES 	= ../TestBench/test1.c
___TestBench_test2_manager_SOURCES 	= ../TestBench/test2/mainManager.c
___TestBench_test2_repeater_SOURCES = ../TestBench/test2/mainRepeater.c
if DEBUG_MODE
___TestBench_test1_LDADD = 	-lsal  \
							libnetwork_dbg.la
___TestBench_test2_manager_LDADD = 	-lsal  \
									libnetwork_dbg.la
___TestBench_test2_repeater_LDADD = -lsal  \
									libnetwork_dbg.la
else
___TestBench_test1_LDADD 	= 	-lsal \
								libnetwork.la
___TestBench_test2_manager_LDADD = 	-lsal  \
									libnetwork.la
___TestBench_test2_repeater_LDADD = -lsal  \
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
