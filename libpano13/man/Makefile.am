man_MANS = panoinfo.1 PTAInterpolate.1 PTblender.1 PTcrop.1 PTinfo.1 PTmasker.1 PTmender.1 PToptimizer.1 PTroller.1 PTtiff2psd.1 PTtiffdump.1 PTuncrop.1

PACKAGE = 'LIBPANO13 TOOLS'

EXTRA_DIST = panoinfo.pod PTAInterpolate.pod PTblender.pod PTcrop.pod PTinfo.pod PTmasker.pod \
	PTmender.pod PToptimizer.pod PTroller.pod PTtiff2psd.pod PTtiffdump.pod \
	PTuncrop.pod $(man_MANS) \
	CMakeLists.txt

%.1: %.pod
	pod2man --center $(PACKAGE) --release $(PACKAGE_VERSION) $< $@

install-data-local: install-man

MAINTAINERCLEANFILES = Makefile.in

