C_TESTS = checkLargeVector Reconstruction vectorTest testSmoothing \
	testGaussSmoothing testOpening testMaximumSearch cimgIOException \
	dllExceptionTest cimgIOException dllExceptionTest
TESTS = $(srcdir)/checkDestruction.sh $(C_TESTS) 
noinst_LIBRARIES = libSTORMdebug.a libdllExceptionTestDLL.a
check_PROGRAMS = $(C_TESTS)
bin_PROGRAMS = 
EXTRA_PROGRAMS = imageConverter \
   testGaussFit testSpotFinder testSpotFitter \
   timeAnalyzer showFrustratingImage \
   laeuterung huntBug histogramFitCounts \
   findGoodSigma traceWorm compressionPhaseTimes \
   runSTORM \
   guessSigma integratedBesselTable fitIntegratedBessel \
   squaredBesselIntegral measureSpotFitter \
   measurePixelVariance showCandidates histogramSpotStrengths \
   correlateSTMs badlySeperable countImages noisyImage \
   outputSmoothedImage processRawFreeFits printFitJudgeStatistics \
   testMedian listTest

libSTORMdebug_a_SOURCES = ColorImage.cpp ColorImage.h foreach.h CandidateScanner.h

LDADD_BASIC =
LDFLAGS_BASIC = 
CPPFLAGS_BASIC = -D_POSIX_SOURCE -Wall -Wextra \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I$(srcdir)/../dStorm -I$(srcdir)/../dStorm/dStorm

LDFLAGS_BASIC += $(ANDOR_SDK_LDFLAGS)
CPPFLAGS_BASIC += $(ANDOR_SDK_CPPFLAGS)
LDADD_BASIC += $(ANDOR_SDK_LIBS)

LDFLAGS_BASIC += $(LIBLIBFITPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBLIBFITPP_CPPFLAGS)
LDADD_BASIC += $(LIBLIBFITPP_LIBS) 

LDFLAGS_BASIC += $(LIBANDORCAMERA_LDFLAGS)
CPPFLAGS_BASIC += $(LIBANDORCAMERA_CPPFLAGS)
LDADD_BASIC += $(LIBANDORCAMERA_LIBS)

LDFLAGS_BASIC += $(LIBDSTORM_LDFLAGS)
CPPFLAGS_BASIC += $(LIBDSTORM_CPPFLAGS)
LDADD_BASIC += $(LIBDSTORM_LIBS)

LDFLAGS_BASIC += $(LIBSIMPARM_LDFLAGS)
CPPFLAGS_BASIC += $(LIBSIMPARM_CPPFLAGS)
LDADD_BASIC += $(LIBSIMPARM_LIBS)

LDFLAGS_BASIC += $(LIBCOMMONCPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBCOMMONCPP_CPPFLAGS)
LDADD_BASIC += $(LIBCOMMONCPP_LIBS)

LDFLAGS_BASIC += $(LIBLOCPREC_LDFLAGS)
CPPFLAGS_BASIC += $(LIBLOCPREC_CPPFLAGS)
LDADD_BASIC += $(LIBLOCPREC_LIBS)

LDFLAGS_BASIC += $(LIBDATACPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBDATACPP_CPPFLAGS)
LDADD_BASIC += $(LIBDATACPP_LIBS)

LDFLAGS_BASIC += $(LIBCIMG_LDFLAGS)
CPPFLAGS_BASIC += $(LIBCIMG_CPPFLAGS)
LDADD_BASIC += $(LIBCIMG_LIBS)

LIBRARY_OBJECTS =
LIBRARY_OBJECTS += $(LIBCIMG_OBJECTS)
LIBRARY_OBJECTS += $(LIBCOMMONCPP_OBJECTS)
LIBRARY_OBJECTS += $(LIBDATACPP_OBJECTS)
LIBRARY_OBJECTS += $(LIBSIMPARM_OBJECTS)
LIBRARY_OBJECTS += $(LIBANDORCAMERA_OBJECTS)
LIBRARY_OBJECTS += $(LIBCIMGBUFFER_OBJECTS)
LIBRARY_OBJECTS += $(LIBLOCPREC_OBJECTS)
LIBRARY_OBJECTS += $(LIBDSTORM_OBJECTS)
LIBRARY_OBJECTS += $(LIBCIMG_OBJECTS)

AM_CPPFLAGS = $(LIBCIMGBUFFER_CPPFLAGS) $(CPPFLAGS_BASIC)
AM_LDFLAGS = $(LDFLAGS_BASIC) $(LIBCIMGBUFFER_LDFLAGS) 
LDADD = libSTORMdebug.a $(LIBRARY_OBJECTS) $(LDADD_BASIC) $(LIBCIMGBUFFER_LIBS) 

traceWorm_SOURCES = traceWorm.cpp 
testGaussFit_SOURCES = testGaussFit.cpp
testSpotFinder_SOURCES = testSpotFinder.cpp
testSpotFitter_SOURCES = testSpotFitter.cpp
timeAnalyzer_SOURCES = timeAnalyzer.cpp
showFrustratingImage_SOURCES = showFrustratingImage.cpp
laeuterung_SOURCES = laeuterung.cpp
huntBug_SOURCES = huntBug.cpp
histogramFitCounts_SOURCES = histogramFitCounts.cpp
findGoodSigma_SOURCES = findGoodSigma.cpp
compressionPhaseTimes_SOURCES = compressionPhaseTimes.cpp ../src/Garage.cpp ../src/logo.cpp
imageConverter_SOURCES = imageConverter.cpp
imageConverter_LDFLAGS = $(AM_LDFLAGS) -static
guessSigma_SOURCES = guessSigma.cpp
integratedBesselTable_SOURCES = integratedBesselTable.cpp
fitIntegratedBessel_SOURCES = fitIntegratedBessel.cpp
squaredBesselIntegral_SOURCES = squaredBesselIntegral.cpp
checkLargeVector_SOURCES = checkLargeVector.cpp
Reconstruction_SOURCES = Reconstruction.cpp
vectorTest_SOURCES = vectorTest.cpp
testSmoothing_SOURCES = testSmoothing.cpp
testGaussSmoothing_SOURCES = testGaussSmoothing.cpp
testOpening_SOURCES = testOpening.cpp
testMaximumSearch_SOURCES = testMaximumSearch.cpp
measureSpotFitter_SOURCES = measureSpotFitter.cpp
measurePixelVariance_SOURCES = measurePixelVariance.cpp
showCandidates_SOURCES = showCandidates.cpp ../src/Garage.cpp ../src/logo.cpp
histogramSpotStrengths_SOURCES = histogramSpotStrengths.cpp ../src/Garage.cpp
correlateSTMs_SOURCES = correlateSTMs.cpp
badlySeperable_SOURCES = badlySeperable.cpp
countImages_SOURCES = countImages.cpp
noisyImage_SOURCES = noisyImage.cpp
outputSmoothedImage_SOURCES = outputSmoothedImage.cpp
processRawFreeFits_SOURCES = processRawFreeFits.cpp
printFitJudgeStatistics_SOURCES = printFitJudgeStatistics.cpp
testMedian_SOURCES = testMedian.cpp
listTest_SOURCES = listTest.cpp
cimgIOException_SOURCES = cimgIOException.cpp
cimgIOException_LDFLAGS = $(LDFLAGS_BASIC)
cimgIOException_CPPFLAGS = $(CPPFLAGS_BASIC)
cimgIOException_LDADD = $(LDADD_BASIC)

dllExceptionTest_SOURCES = shared-dll-exception-test/main.cpp
dllExceptionTest_LDADD = dllExceptionTestDLL.a
libdllExceptionTestDLL_a_SOURCES = shared-dll-exception-test/library.cpp
#libdllExceptionTestDLL_a_LDFLAGS = -no-undefined -version-info 1:0:0
libdllExceptionTestDLL_a_LIBADD = $(LIBOBJS)


