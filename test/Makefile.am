ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

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

libSTORMdebug_a_SOURCES = ColorImage.cpp ColorImage.h foreach.h CandidateScanner.h FitJudger.h

AM_CPPFLAGS = -D_POSIX_SOURCE -Wall -Wextra \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h \
    -I$(top_srcdir)/src 

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


