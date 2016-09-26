include $(top_srcdir)/freeverb/src.make

if BUILD_STATIC
AM_LDFLAGS = -static
endif

INCLUDES = -DLIBFV3_DEFAULT -I$(top_srcdir)
libfreeverbincludedir = ${includedir}/libfreeverb3-2/freeverb
libfreeverbinclude_HEADERS = \
	allpass.hpp \
	allpass_t.hpp \
	biquad.hpp \
	biquad_t.hpp \
	blockDelay.hpp \
	blockDelay_t.hpp \
	comb.hpp \
	comb_t.hpp \
	compmodel.hpp \
	compmodel_t.hpp \
	delay.hpp \
	delay_t.hpp \
	efilter.hpp \
	efilter_t.hpp \
	fir3bandsplit.hpp \
	fir3bandsplit_t.hpp \
	firfilter.hpp \
	firfilter_t.hpp \
	firwindow.hpp \
	firwindow_t.hpp \
	frag.hpp \
	frag_t.hpp \
	fv3_ch_tool.hpp \
	fv3_defs.h \
	fv3_ns_end.h \
	fv3_ns_start.h \
	fv3_type_float.h \
	irbase.hpp \
	irbase_t.hpp \
	irmodel.hpp \
	irmodel_t.hpp \
	irmodel2.hpp \
	irmodel2_t.hpp \
	irmodel2zl.hpp \
	irmodel2zl_t.hpp \
	irmodel3.hpp \
	irmodel3_t.hpp \
	irmodels.hpp \
	irmodels_t.hpp \
	limitmodel.hpp \
	limitmodel_t.hpp \
	revbase.hpp \
	revbase_t.hpp \
	nrev.hpp \
	nrev_t.hpp \
	nrevb.hpp \
	nrevb_t.hpp \
	revmodel.hpp \
	revmodel_t.hpp \
	rms.hpp \
	rms_t.hpp \
	scomp.hpp \
	scomp_t.hpp \
	slimit.hpp \
	slimit_t.hpp \
	slot.hpp \
	slot_t.hpp \
	src.hpp \
	src_t.hpp \
	stenh.hpp \
	stenh_t.hpp \
	strev.hpp \
	strev_t.hpp \
	utils.hpp \
	utils_t.hpp \
	$(top_srcdir)/fv3_config.h

lib_LTLIBRARIES = libfreeverb3.la

COMPATNUM=@LIBTOOL_VERSION_INFO@
libfreeverb3_la_LDFLAGS = -version-info $(COMPATNUM)

libfreeverb3_la_LIBADD = \
	$(top_srcdir)/libsamplerate2/libsamplerate2.la \
	$(fftw3f_LIBS) \
	$(fftw3_LIBS) \
	$(fftw3l_LIBS)

if BUILD_FLOAT
libfreeverb3_la_LIBADD += $(top_srcdir)/float/libfv3_float.la
libfreeverb3_la_LIBADD += $(top_srcdir)/float/libsr2_float.la
endif

if BUILD_DOUBLE
libfreeverb3_la_LIBADD += $(top_srcdir)/double/libfv3_double.la
libfreeverb3_la_LIBADD += $(top_srcdir)/double/libsr2_double.la
endif

if BUILD_LDOUBLE
libfreeverb3_la_LIBADD += $(top_srcdir)/ldouble/libfv3_ldouble.la
libfreeverb3_la_LIBADD += $(top_srcdir)/ldouble/libsr2_ldouble.la
endif

libfreeverb3_la_SOURCES =
