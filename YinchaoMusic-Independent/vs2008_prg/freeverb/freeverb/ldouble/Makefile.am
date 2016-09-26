include $(top_srcdir)/freeverb/src.make
include $(top_srcdir)/libsamplerate2/src.make

if BUILD_STATIC
AM_LDFLAGS = -static
endif

INCLUDES = -I$(F_SRC) -I$(top_srcdir) -DLIBFV3_LDOUBLE \
	-I$(S_SRC) -I$(top_srcdir)/libsamplerate2 -DLIBSRATE2_LDOUBLE

if BUILD_LDOUBLE
noinst_LTLIBRARIES = libfv3_ldouble.la libsr2_ldouble.la
else
noinst_LTLIBRARIES =
endif

libfv3_ldouble_la_SOURCES = $(FV3_SRC)

libsr2_ldouble_la_SOURCES = $(SR2_SRC)
