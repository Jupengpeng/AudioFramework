if BUILD_STATIC
AM_LDFLAGS = -static
endif

noinst_LTLIBRARIES = libsamplerate2.la
libsamplerate2includedir = ${includedir}/libsamplerate2
libsamplerate2include_HEADERS = \
	samplerate2.h samplerate2_t.h samplerate2_common.h type_float.h
INCLUDES = -I$(top_srcdir)
COEFF_HDRS = high_qual_coeffs.h mid_qual_coeffs.h fastest_coeffs.h \
slow_high_qual_coeffs.h slow_mid_qual_coeffs.h

noinst_HEADERS = common.h common_c.h float_cast.h $(COEFF_HDRS)
libsamplerate2_la_SOURCES = \
	samplerate_common.c \
	src_common.c \
	$(noinst_HEADERS)

if BUILD_SRCNEWCOEFFS
INCLUDES += -DSRCNEWCOEFFS
endif
