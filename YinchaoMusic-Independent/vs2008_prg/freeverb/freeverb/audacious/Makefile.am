SUBDIRS =
if BUILD_AUDACIOUS
lib_LTLIBRARIES = libfreeverb3.la librevmodel.la libimpulser2.la libmbcomp.la libstenh.la libstrev.la	
else
lib_LTLIBRARIES =
endif

libdir = $(audacious_effect_plugin_dir)

I_LIBS =  $(gtk2_LIBS) \
	$(audacious_LIBS) \
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

INCLUDES = -I$(top_srcdir) \
	-I$(top_srcdir)/libgdither \
	$(samplerate_CFLAGS) \
	$(audacious_CFLAGS) \
	$(fftw3f_CFLAGS) \
	$(fftw3_CFLAGS) \
	$(fftw3l_CFLAGS) \
	$(gtk2_CFLAGS) \
	-I$(top_srcdir)/xmms/ \
	-DAUDACIOUS

if BUILD_PLUGDOUBLE
INCLUDES += -DPLUGDOUBLE
endif

if ENABLE_PLUGINIT
INCLUDES += -DPLUGINIT
endif

if AUDACIOUS140
INCLUDES += -DAUDACIOUS140
endif

PLUGINSRCDIR = $(top_srcdir)/xmms

libfreeverb3_la_LIBADD = $(I_LIBS)

libfreeverb3_la_SOURCES = \
	$(PLUGINSRCDIR)/freeverb3.cpp

librevmodel_la_LIBADD = $(I_LIBS)

librevmodel_la_SOURCES = \
	$(PLUGINSRCDIR)/revmodel.cpp

libimpulser2_la_LIBADD = $(I_LIBS) \
	$(top_srcdir)/libgdither/libgdither.la \
	$(sndfile_LIBS)

libimpulser2_la_SOURCES = \
	$(PLUGINSRCDIR)/impulser2.cpp \
	$(PLUGINSRCDIR)/wave.xpm

libmbcomp_la_LIBADD = $(I_LIBS)

libmbcomp_la_SOURCES = \
	$(PLUGINSRCDIR)/compressor.cpp

libstenh_la_LIBADD = $(I_LIBS)

libstenh_la_SOURCES = \
	$(PLUGINSRCDIR)/stenh.cpp

libstrev_la_LIBADD = $(I_LIBS)

libstrev_la_SOURCES = \
	$(PLUGINSRCDIR)/strev.cpp
