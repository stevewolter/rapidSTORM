javabuilddir = classes
javadir = $(datadir)/java
java_DATA = twiddler.jar
SOURCES = ConfigEntryBool.java ConfigEntryButton.java \
   ConfigEntryCheckbox.java ConfigEntryChoice.java ConfigEntryChooser.java \
   ConfigEntryFile.java ConfigEntryFloat.java ConfigEntryInteger.java \
   ConfigEntry.java ConfigEntryNumber.java ConfigEntryProgress.java \
   ConfigEntryString.java ConfigEntryTextbox.java ConfigEntryTrigger.java \
   ConfigSet.java \
   Twiddler.java Main.java MessageDialogMaker.java \
   PipingThread.java ConfigObject.java ConfigIO.java \
   Acknowledgement.java TreeComponent.java HelpManager.java \
   ComponentTitledBorder.java FixedWidthPanel.java \
   ConfigEntryFloat.java ConfigMessage.java \
   ConfigURI.java ConfigMenu.java \
   NumberField.java NumberFieldListener.java NumberParser.java \
   SliderNumberField.java

EXTRA_DIST=$(SOURCES) manifest stock-hchain-24.png stock-hchain-broken-24.png

dist_bin_SCRIPTS=replay_twiddler_script.sh twiddler

twiddler.jar : $(SOURCES)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -d $(javabuilddir) $^
	ln -f $(srcdir)/*.png $(javabuilddir)
	$(JAR) -cmf $(srcdir)/manifest $@ -C $(javabuilddir) .

clean-local :
	$(RM) -r $(javabuilddir) twiddler.jar
