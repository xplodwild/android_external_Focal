# -*- Makefile -*-
#
# name:     Makefile.scm
# synopsis: Bootstap the GNU AutoTools configuration after a SCM check out.


.PHONY: default
default: all


.PHONY: all
all:
	aclocal -I m4
	autoheader
	automake --add-missing
	autoconf


.PHONY: clean
clean:
	rm -rf autom4te.cache
	rm -f INSTALL Makefile.in aclocal.m4 \
	      config.guess config.h.in config.sub configure \
	      depcomp install-sh mdate-sh missing \
	      texinfo.tex
