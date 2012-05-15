javabuilddir = classes
javadir = $(datadir)/java
java_DATA = rapidSTORM.jar
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
   SliderNumberField.java DStorm.java 
JARCONTENT = logo.png
JAVASOURCES = $(SOURCES) DefaultPath.java

EXTRA_DIST=$(SOURCES) $(JARCONTENT) stock-hchain-24.png stock-hchain-broken-24.png \
    DefaultPath.java.in 

CLEANFILES = DefaultPath.java

rapidSTORM.jar : $(JAVASOURCES) manifest
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -d $(javabuilddir) $(filter %.java,$^)
	ln -f $(srcdir)/*.png $(javabuilddir)
	$(JAR) -cmf manifest $@ -C $(javabuilddir) .

DefaultPath.java : $(srcdir)/DefaultPath.java.in
	$(SED) -e 's:[@]prefix[@]:$(prefix):g' \
	    -e 's:[@]sysconfdir[@]:$(sysconfdir):g' \
	    $< > $@

clean-local :
	$(RM) -r $(javabuilddir) rapidSTORM.jar
