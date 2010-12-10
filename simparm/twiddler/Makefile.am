javabuilddir = classes
javadir = $(datadir)/java
java_DATA = twiddler.jar
SOURCES = ConfigEntryBool.java ConfigEntryButton.java \
   ConfigEntryCheckbox.java ConfigEntryChoice.java ConfigEntryChooser.java \
   ConfigEntryDivider.java ConfigEntryDouble.java ConfigEntryFile.java \
   ConfigEntry.java ConfigEntryNumber.java ConfigEntryProgress.java \
   ConfigEntryString.java ConfigEntryTextbox.java ConfigEntryTrigger.java \
   ConfigEntryUnsignedLong.java ConfigEntryLong.java ConfigSet.java \
   Twiddler.java Main.java MessageDialogMaker.java Tab.java \
   PipingThread.java ConfigObject.java ConfigIO.java DescriptedPanel.java
EXTRA_DIST=$(SOURCES) manifest

twiddler.jar : $(SOURCES)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -d $(javabuilddir) $^
	$(JAR) -cmf $(srcdir)/manifest $@ -C $(javabuilddir) .
