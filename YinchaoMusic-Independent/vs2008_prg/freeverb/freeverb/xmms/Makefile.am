SUBDIRS =
if BUILD_XMMS
lib_LTLIBRARIES = libfreeverb3.la librevmodel.la libimpulser2.la libmbcomp.la libstenh.la libstrev.la
else
lib_LTLIBRARIES =
endif

libdir = @XMMS_EFFECT_PLUGIN_DIR@

I_LIBS = @GTK_LIBS@ @XMMS_LIBS@ \
	$(top_srcdir)/freeverb/libfreeverb3.la

if BUILD_PLUGDOUBLE
I_LIBS += $(fftw3_LIBS)
else
I_LIBS += $(fftw3f_LIBS)
endif

libfreeverb3_la_LDFLAGS = -module -avoid-version

librevmodel_la_LDFLAGS = -module -avoid-version

libimpulser2_la_LDFLAGS = -module -avoid-version

libmbcomp_la_LDFLAGS = -module -avoid-version

libstenh_la_LDFLAGS = -module -avoid-version

INCLUDES = @XMMS_CFLAGS@ \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgdither \
	$(fftw3f_CFLAGS) \
	$(fftw3_CFLAGS) \
	$(fftw3l_CFLAGS) \
	-DXMMS

if BUILD_PLUGDOUBLE
INCLUDES += -DPLUGDOUBLE
endif

if ENABLE_PLUGINIT
INCLUDES += -DPLUGINIT
endif

libfreeverb3_la_LIBADD = $(I_LIBS)

libfreeverb3_la_SOURCES = \
	freeverb3.cpp xmms_defs.h

librevmodel_la_LIBADD = $(I_LIBS)

librevmodel_la_SOURCES = \
	revmodel.cpp

libimpulser2_la_LIBADD = $(I_LIBS) \
	$(top_srcdir)/libgdither/libgdither.la \
	$(sndfile_LIBS)

libimpulser2_la_SOURCES = \
	CFileLoader_t.hpp \
	CFileLoader.hpp \
	impulser2.cpp wave.xpm

libmbcomp_la_LIBADD = $(I_LIBS)

libmbcomp_la_SOURCES = \
	libxmmsplugin.hpp \
	compressor.cpp

libstenh_la_LIBADD = $(I_LIBS)

libstenh_la_SOURCES = \
	libxmmsplugin.hpp \
	stenh.cpp

libstrev_la_LIBADD = $(I_LIBS)

libstrev_la_SOURCES = \
	libxmmsplugin.hpp \
	strev.cpp
