###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS                                             =   -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES                                                =   ../Includes/libARNetwork/ARNETWORK_Manager.h          \
                                                                ../Includes/libARNetwork/ARNETWORK_Error.h            \
                                                                ../Includes/libARNetwork/ARNETWORK_IOBufferParam.h    \
                                                                ../Includes/libARNetwork/ARNetwork.h

# The sources to add to the library and to add to the source distribution
SOURCE_FILES                                                =   $(HEADER_FILES)                         \
                                                                ../Sources/ARNETWORK_Buffer.c           \
                                                                ../Sources/ARNETWORK_RingBuffer.c       \
                                                                ../Sources/ARNETWORK_DataDescriptor.h   \
                                                                ../Sources/ARNETWORK_IOBufferParam.c    \
                                                                ../Sources/ARNETWORK_IOBuffer.c         \
                                                                ../Sources/ARNETWORK_Sender.c           \
                                                                ../Sources/ARNETWORK_Receiver.c         \
                                                                ../Sources/ARNETWORK_Manager.c          \
                                                                ../Sources/ARNETWORK_Error.c            \
                                                                ../Sources/ARNETWORK_Buffer.h           \
                                                                ../Sources/ARNETWORK_RingBuffer.h       \
                                                                ../Sources/ARNETWORK_IOBuffer.h         \
                                                                ../Sources/ARNETWORK_Sender.h           \
                                                                ../Sources/ARNETWORK_Receiver.h


# The library names to build (note we are building static and shared libs)
if DEBUG_MODE
lib_LTLIBRARIES                                             =   libarnetwork_dbg.la
libarnetwork_dbg_ladir                                      =   $(includedir)/libARNetwork
libarnetwork_dbg_la_HEADERS                                 =   $(HEADER_FILES)
libarnetwork_dbg_la_SOURCES                                 =   $(SOURCE_FILES)
else
lib_LTLIBRARIES                                             =   libarnetwork.la
libarnetwork_ladir                                          =   $(includedir)/libARNetwork
libarnetwork_la_HEADERS                                     =   $(HEADER_FILES)
libarnetwork_la_SOURCES                                     =   $(SOURCE_FILES)
endif

check_PROGRAMS                                              =   ../TestBench/autoTest/autoTest                      \
                                                                ../TestBench/bidirectionalTest/bidirectionalTest    \
                                                                ../TestBench/defaultTest/defaultTest

___TestBench_autoTest_autoTest_SOURCES                      =   ../TestBench/autoTest/autoTest.c
___TestBench_bidirectionalTest_bidirectionalTest_SOURCES    =   ../TestBench/bidirectionalTest/bidirectionalTest.c
___TestBench_defaultTest_defaultTest_SOURCES                =   ../TestBench/defaultTest/defaultTest.c
if DEBUG_MODE
___TestBench_autoTest_autoTest_LDADD                        =   -larsal_dbg                 \
                                                                -larnetworkal_dbg  \
                                                                libarnetwork_dbg.la
___TestBench_bidirectionalTest_bidirectionalTest_LDADD      =   -larsal_dbg                 \
                                                                -larnetworkal_dbg  \
                                                                libarnetwork_dbg.la
___TestBench_defaultTest_defaultTest_LDADD                  =   -larsal_dbg                 \
                                                                -larnetworkal_dbg  \
                                                                libarnetwork_dbg.la
else
___TestBench_autoTest_autoTest_LDADD                        =   -larsal                 \
                                                                -larnetworkal  \
                                                                libarnetwork.la
___TestBench_bidirectionalTest_bidirectionalTest_LDADD      =   -larsal                 \
                                                                -larnetworkal  \
                                                                libarnetwork.la
___TestBench_defaultTest_defaultTest_LDADD                  =   -larsal                 \
                                                                -larnetworkal  \
                                                                libarnetwork.la
endif

CLEAN_FILES                                                 =   libarnetwork.la       \
                                                                libarnetwork_dbg.la

EXTRA_DIST                                                  =   bootstrap

# Ensure the distribution of the doxygen configuration file
# Doxygen support
if HAVE_DOXYGEN
include doxygen.am
EXTRA_DIST                                                  +=  doxygen.ac  \
                                                                doxygen.am  \
                                                                $(DX_CONFIG)
endif
