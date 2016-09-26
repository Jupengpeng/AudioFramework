SUBDIRS = IR

INCLUDES = \
	-I$(top_srcdir)/libsamplerate2 \
	-I$(top_srcdir)/libgdither \
	-I$(top_srcdir) \
	$(sndfile_CFLAGS) \
	$(jack_CFLAGS) \
	$(fftw3f_CFLAGS) \
	$(fftw3_CFLAGS) \
	$(fftw3l_CFLAGS)

if BUILD_PLUGDOUBLE
INCLUDES += -DPLUGDOUBLE
endif

AM_LDFLAGS = -static

I_LIBS = $(top_srcdir)/libgdither/libgdither.la \
	$(top_srcdir)/freeverb/libfreeverb3.la \
	$(sndfile_LIBS) \
	$(jack_LIBS)

if BUILD_PLUGDOUBLE
I_LIBS += $(fftw3_LIBS)
else
I_LIBS += $(fftw3f_LIBS)
endif

if BUILD_SAMPLE
noinst_PROGRAMS =
bin_PROGRAMS = fv3_benchmark3 fv3_rateconv fv3_impulser
else
noinst_PROGRAMS =
bin_PROGRAMS =
endif

if BUILD_JACK
bin_PROGRAMS += fv3_impulser_jack
endif

fv3_benchmark3_SOURCES = benchmark3.cpp CArg.cpp CArg.hpp

fv3_benchmark3_LDADD = $(I_LIBS)

fv3_rateconv_SOURCES = rateconv.cpp CArg.cpp CArg.hpp

fv3_rateconv_LDADD = $(I_LIBS)

fv3_impulser_SOURCES = impulser.cpp CArg.cpp CArg.hpp

fv3_impulser_LDADD = $(I_LIBS)

fv3_impulser_jack_SOURCES = impulser_jack.cpp CArg.cpp CArg.hpp

fv3_impulser_jack_LDADD = $(I_LIBS)
