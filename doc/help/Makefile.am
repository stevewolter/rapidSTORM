HTMLHELP_XSLT_FLAGS= \
    --stringparam manifest.in.base.dir 1

noinst_HEADERS = rapidstorm_help_file.h

doc_DATA = $(builddir)/rapidstorm.chm

HTMLHELP_FILES = rapidstorm.css rapidstorm-job-status.PNG \
    rapidstorm-output-box.PNG rapidstorm-step-by-step-2.PNG \
    SbSa.PNG
EXTRA_DIST = rapidstorm.xml rapidstorm.xsl rapidstorm_pre.xsl $(HTMLHELP_FILES)

.hhp.chm : 
	for i in $(HTMLHELP_FILES); do $(RM) $(builddir)/$$i; $(LN_S) $(srcdir)/$$i $(builddir)/$$i; done
	grep -q -v '#' alias.h || { echo DUMMY_ALIAS=index.html >> alias.h; echo '#define DUMMY_ALIAS 23' >> context.h; } # We need at least one non-anchored alias so aliasing works.
	'$(HHC)' '$<' || test "$$?" = '1' # Ignore the error exit code; it is normal due to superfluous HHC warnings

.PRECIOUS : rapidstorm.hhp

.xml.hhp :
	PREXSL=$(<:.xml=_pre.xsl); \
	if test -e $$PREXSL; then $(XSLTPROC) $(XSLT_FLAGS) $$PREXSL $<; else cat $<; fi | \
	$(XSLTPROC) $(XSLT_FLAGS) $(HTMLHELP_XSLT_FLAGS) \
	    --stringparam htmlhelp.hhp $*.hhp \
	    --stringparam htmlhelp.hhc $*.hhc \
	    --stringparam htmlhelp.hhk $*.hhk \
	    --stringparam htmlhelp.chm $*.chm \
	    $(<:.xml=.xsl) -

rapidstorm_help_file.h :
	echo 'namespace dStorm { static const char *HelpFileName = "$(PACKAGE)/rapidstorm.chm"; }' > $@
