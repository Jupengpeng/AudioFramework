include $(top_srcdir)/freeverb/src.make
include $(top_srcdir)/libsamplerate2/src.make

if BUILD_STATIC
AM_LDFLAGS = -static
endif

INCLUDES = -I$(F_SRC) -I$(top_srcdir) -DLIBFV3_DOUBLE \
	-I$(S_SRC) -I$(top_srcdir)/libsamplerate2 -DLIBSRATE2_DOUBLE

if BUILD_DOUBLE
noinst_LTLIBRARIES = libfv3_double.la libsr2_double.la
else
noinst_LTLIBRARIES =
endif

libfv3_double_la_SOURCES = $(FV3_SRC)

libsr2_double_la_SOURCES = $(SR2_SRC)
