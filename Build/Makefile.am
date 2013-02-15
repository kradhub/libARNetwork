###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS                                             =   -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES                                                =   ../Includes/libNetwork/manager.h            \
                                                                ../Includes/libNetwork/frame.h              \
                                                                ../Includes/libNetwork/status.h             \
                                                                ../Includes/libNetwork/deportedData.h       \
                                                                ../Includes/libNetwork/paramNewIoBuffer.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES                                                =   $(HEADER_FILES)                 \
                                                                ../Sources/buffer.c             \
                                                                ../Sources/ringBuffer.c         \
                                                                ../Sources/paramNewIoBuffer.c   \
                                                                ../Sources/ioBuffer.c           \
                                                                ../Sources/sender.c             \
                                                                ../Sources/receiver.c           \
                                                                ../Sources/manager.c            \
                                                                ../Sources/buffer.h             \
                                                                ../Sources/ringBuffer.h         \
                                                                ../Sources/ioBuffer.h           \
                                                                ../Sources/sender.h             \
                                                                ../Sources/receiver.h


# The library names to build (note we are building static and shared libs)
if DEBUG_MODE
lib_LTLIBRARIES                                             =   libnetwork_dbg.la
libnetwork_dbg_ladir                                        =   $(includedir)/libNetwork
libnetwork_dbg_la_HEADERS                                   =   $(HEADER_FILES)
libnetwork_dbg_la_SOURCES                                   =   $(SOURCE_FILES)
else
lib_LTLIBRARIES                                             =   libnetwork.la
libnetwork_ladir                                            =   $(includedir)/libNetwork
libnetwork_la_HEADERS                                       =   $(HEADER_FILES)
libnetwork_la_SOURCES                                       =   $(SOURCE_FILES)
endif

check_PROGRAMS                                              =   ../TestBench/autoTest/autoTest                      \
                                                                ../TestBench/bidirectionalTest/bidirectionalTest    \
                                                                ../TestBench/defaultTest/defaultTest

___TestBench_autoTest_autoTest_SOURCES                      =   ../TestBench/autoTest/autoTest.c
___TestBench_bidirectionalTest_bidirectionalTest_SOURCES    =   ../TestBench/bidirectionalTest/bidirectionalTest.c
___TestBench_defaultTest_defaultTest_SOURCES                =   ../TestBench/defaultTest/defaultTest.c
if DEBUG_MODE
___TestBench_autoTest_autoTest_LDADD                        =   -larsal_dbg         \
                                                                libnetwork_dbg.la
___TestBench_bidirectionalTest_bidirectionalTest_LDADD      =   -larsal_dbg         \
                                                                libnetwork_dbg.la
___TestBench_defaultTest_defaultTest_LDADD                  =   -larsal_dbg         \
                                                                libnetwork_dbg.la
else
___TestBench_autoTest_autoTest_LDADD                        =   -larsal         \
                                                                libnetwork.la
___TestBench_bidirectionalTest_bidirectionalTest_LDADD      =   -larsal         \
                                                                libnetwork.la
___TestBench_defaultTest_defaultTest_LDADD                  =   -larsal         \
                                                                libnetwork.la
endif

CLEAN_FILES                                                 =   libnetwork.la       \
                                                                libnetwork_dbg.la

EXTRA_DIST                                                  =   bootstrap

# Ensure the distribution of the doxygen configuration file
# Doxygen support
if HAVE_DOXYGEN
include doxygen.am
EXTRA_DIST                                                  +=  doxygen.ac  \
                                                                doxygen.am  \
                                                                $(DX_CONFIG)
endif
