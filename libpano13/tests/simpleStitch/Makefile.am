EXTRA_DIST = 051221_6054_750.jpg 051221_6055_750.jpg \
	     CMakeLists.txt \
             ChangeLog README.txt simple.pto simple.txt

MAINTAINERCLEANFILES = \
	Makefile.in

SUBDIRS = reference

all:

check: test

test:
	perl ../panoAutomateTest.pl simple.txt

verify:
	perl ../panoAutomateTest.pl -t simple.txt

clean-local:
	rm -f testOutput* _PTSti* tests/*.* temp.txt

