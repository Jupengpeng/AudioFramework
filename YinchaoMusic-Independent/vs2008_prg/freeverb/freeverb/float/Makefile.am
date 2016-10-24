include $(top_srcdir)/freeverb/src.make
include $(top_srcdir)/libsamplerate2/src.make

if BUILD_STATIC
AM_LDFLAGS = -static
endif

INCLUDES = -I$(F_SRC) -I$(top_srcdir) -DLIBFV3_FLOAT \
	-I$(S_SRC) -I$(top_srcdir)/libsamplerate2 -DLIBSRATE2_FLOAT

if BUILD_FLOAT
noinst_LTLIBRARIES = libfv3_float.la libsr2_float.la
else
noinst_LTLIBRARIES =
endif

libfv3_float_la_SOURCES = $(FV3_SRC)

libsr2_float_la_SOURCES = $(SR2_SRC)
