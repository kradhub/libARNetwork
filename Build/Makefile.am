###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################
ACLOCAL_AMFLAGS = -I m4

# The list of header files that belong to the library (to be installed later)
HEADER_FILES	=	../Includes/LibNetWork.h 	

# The sources to add to the library and to add to the source distribution
SOURCE_FILES 	= 	$(HEADER_FILES)		\
					../Sources/LibNetWork.c	
					
# The library names to build (note we are building static and shared libs)
lib_LTLIBRARIES 	= libNetWork.la
libNetWork_ladir 		= $(includedir)/libNetWork
libNetWork_la_HEADERS 	= $(HEADER_FILES)
libNetWork_la_SOURCES 	= $(SOURCE_FILES)

#!! check_PROGRAMS = ../TestBench/test1 ../TestBench/test2
#!! ___TestBench_test1_SOURCES 	= ../TestBench/test1.c
#!! ___TestBench_test2_SOURCES 	= ../TestBench/test2.c
#!! if DEBUG_MODE
#!! ___TestBench_test1_LDADD 	= libNetWork_dbg.la
#!! ___TestBench_test2_LDADD 	= libNetWork_dbg.la
#!! else
#!! ___TestBench_test1_LDADD 	= libNetWork.la
#!! ___TestBench_test2_LDADD 	= libNetWork.la
#!! endif

CLEAN_FILES			= libNetWork.la

EXTRA_DIST = bootstrap

