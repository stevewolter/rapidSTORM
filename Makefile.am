ACLOCAL_AMFLAGS = -I /usr/share/autoconf-archive 

include doxygen.am

EXTRA_DIST = debug.h doxygen.am Doxyfile

if BUILD_LIBFITPP
    LIBFITPP = libfit++
endif
if BUILD_DATACPP
    LIBDATACPP = libdata-c++
endif
if BUILD_READSIF
    READSIF = readsif
endif
if BUILD_SIMPARM
    SIMPARM = simparm
endif
if BUILD_CAMSIM
    CAMSIM = DummyCamera
endif
if BUILD_COMMONCPP
    COMMONCPP = cc++
endif
if BUILD_CIMGBUFFER
    CIMGBUFFER = CImgBuffer
endif
if BUILD_DSTORM
    DSTORM = dStorm
endif
if BUILD_LOCPREC
    LOCPREC = locprec
endif
SUBDIRS = $(COMMONCPP) $(LIBFITPP) $(LIBDATACPP) $(READSIF) $(SIMPARM) \
    $(CAMSIM) AndorCamera $(CIMGBUFFER) $(DSTORM) $(LOCPREC) src test 

dist-jar : all
	make -C src dist-jar
