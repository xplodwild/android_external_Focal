EXTRA_DIST = 060520_3398.TIF \
	     CMakeLists.txt \
             ChangeLog simple.txt README.TXT

SUBDIRS = reference

MAINTAINERCLEANFILES = \
	Makefile.in

all:

check: test

test:
	perl ../panoAutomateTest.pl simple.txt

verify:
	perl ../panoAutomateTest.pl -t simple.txt

clean-local:
	rm -f testOutput* _PTSti* tests/*.* temp.txt

