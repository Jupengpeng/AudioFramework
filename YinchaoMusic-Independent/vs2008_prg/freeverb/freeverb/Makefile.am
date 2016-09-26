ACLOCAL_AMFLAGS = -I m4 --install
SUBDIRS = m4 doc patch libgdither \
	float double ldouble libsamplerate2 freeverb \
	xmms bmp audacious samples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = freeverb3.pc

EXTRA_DIST = cvs2cl.pl

m4datadir = $(datadir)/aclocal

dist-hook: CL

CL:
	cvs2cl.pl  --show-dead --no-times --tags --branches --revisions
	cp $(srcdir)/ChangeLog $(distdir)/

