CHAPTERS = rapidstorm ui usage-examples reference-manual fundamentals

PIXEL_GRAPHICS = rapidstorm-job-status.PNG rapidstorm-output-box.PNG \
    rapidstorm-step-by-step-2.PNG SbSa.PNG
TEX_GRAPHICS = polynomial_3d_psf.tex polynomial_3d_parabolic_conversion.tex polynomial_3d_holtzer_conversion.tex
GENERATED_GRAPHICS = $(TEX_GRAPHICS:.tex=.png)
GRAPHICS = $(PIXEL_GRAPHICS) $(GENERATED_GRAPHICS)
EXTRA_DIST = $(TEX_GRAPHICS) $(PIXEL_GRAPHICS) $(CHAPTERS:=.pxml) \
    rapidstorm_pre.xsl rapidstorm_html.xsl rapidstorm_fo.xsl tex2im

if BUILD_PDF_DOCUMENTATION
    PDF_TARGET = rapidstorm.pdf
else
    PDF_TARGET = 
endif

doc_DATA = $(GRAPHICS) rapidstorm.html $(PDF_TARGET)

rapidstorm.fo rapidstorm.html : $(CHAPTERS:=.xml)
rapidstorm.pdf : $(GRAPHICS)

XSLT_FLAGS ?= 
XSLT_FLAGS_HTML ?= $(XSLT_FLAGS) --xinclude
XSLT_FLAGS_FO ?= $(XSLT_FLAGS) --xinclude --stringparam fop1.extensions 1 --stringparam double.sided 1

.pxml.xml : $(srcdir)/rapidstorm_pre.xsl
	$(XSLTPROC) $(XSLT_FLAGS) -o $@ $(srcdir)/rapidstorm_pre.xsl $<

.xml.html :
	$(XSLTPROC) $(XSLT_FLAGS_HTML) -o $@ $(srcdir)/$(<:.xml=_html.xsl) $<

.xml.fo :
	$(XSLTPROC) $(XSLT_FLAGS_FO) -o $@ $(srcdir)/$(<:.xml=_fo.xsl) $<

.fo.pdf : foprc
	fop -c foprc $< $@

%.png : $(srcdir)/%.tex
	bash $(srcdir)/tex2im -r 150x150 -o $@ $<

CLEANFILES = $(GENERATED_GRAPHICS) $(CHAPTERS:=.xml) $(PDF_TARGET) rapidstorm.html rapidstorm.fo
