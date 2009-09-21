javabuilddir = classes
javadir = $(datadir)/java
java_DATA = twiddler.jar
SOURCES = ConfigEntryBool.java ConfigEntryButton.java \
   ConfigEntryCheckbox.java ConfigEntryChoice.java ConfigEntryChooser.java \
   ConfigEntryDouble.java ConfigEntryFile.java \
   ConfigEntry.java ConfigEntryNumber.java ConfigEntryProgress.java \
   ConfigEntryString.java ConfigEntryTextbox.java ConfigEntryTrigger.java \
   ConfigEntryUnsignedLong.java ConfigEntryLong.java ConfigSet.java \
   Twiddler.java Main.java MessageDialogMaker.java \
   PipingThread.java ConfigObject.java ConfigIO.java \
   Acknowledgement.java TreeComponent.java HelpManager.java
EXTRA_DIST=$(SOURCES) manifest

twiddler.jar : $(SOURCES)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -d $(javabuilddir) $^
	$(JAR) -cmf $(srcdir)/manifest $@ -C $(javabuilddir) .
