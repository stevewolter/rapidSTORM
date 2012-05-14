AUTOMAKE_OPTIONS = foreign
CHAPTERS = rapidstorm ui usage-examples input-options output-options errors fundamentals engine_options bibliography

PIXEL_GRAPHICS = rapidstorm-job-status.PNG rapidstorm-output-box.PNG \
    rapidstorm-step-by-step-2.PNG 
TEX_GRAPHICS = polynomial_3d_psf.tex polynomial_3d_parabolic_conversion.tex polynomial_3d_holtzer_conversion.tex
GENERATED_GRAPHICS = $(TEX_GRAPHICS:.tex=.png)
GRAPHICS = $(PIXEL_GRAPHICS) $(GENERATED_GRAPHICS)
EXTRA_DIST = $(TEX_GRAPHICS) $(PIXEL_GRAPHICS) $(CHAPTERS:=.xml) \
    rapidstorm_pre.xsl rapidstorm_html.xsl rapidstorm_fo.xsl tex2im \
    bib4wbbt.bib

if BUILD_PDF_DOCUMENTATION
    PDF_TARGET = manual.pdf
else
    PDF_TARGET = 
endif

doc_DATA = $(GRAPHICS) manual.html $(PDF_TARGET)

manual.xml : $(CHAPTERS:=.xml)
manual.fo : $(GRAPHICS) rapidstorm_fo.xsl titlepage.xsl bib4wbbt.xml
manual.html : $(GRAPHICS) rapidstorm_html.xsl bib4wbbt.xml

XSLT_FLAGS ?= 
XSLT_FLAGS_HTML ?= $(XSLT_FLAGS) 
XSLT_FLAGS_FO ?= $(XSLT_FLAGS) --path "."

bib4wbbt.xml : xml2db5.xsl bib4wbbt.bib
	bib2xml $(filter %.bib,$^) | $(XSLTPROC) $(XSLT_FLAGS) -o $@ $(filter %.xsl,$^) -

titlepage.xsl : make_titlepage.xsl titlepage.xml
	$(XSLTPROC) $(XSLT_FLAGS) -o $@ $^

manual.xml : rapidstorm_pre.xsl
	$(XSLTPROC) $(XSLT_FLAGS) --xinclude -o $@ $(srcdir)/rapidstorm_pre.xsl $(srcdir)/rapidstorm.xml

manual.html : manual.xml
	$(XSLTPROC) $(XSLT_FLAGS_HTML) -o $@ $(srcdir)/rapidstorm_html.xsl $<

manual.fo : manual.xml
	$(XSLTPROC) $(XSLT_FLAGS_FO) -o $@ $(srcdir)/rapidstorm_fo.xsl $<

manual.pdf : manual.fo foprc
	fop -c foprc $< $@

.tex.png :
	bash $(srcdir)/tex2im -r 150x150 -o $@ $^

CLEANFILES = $(GENERATED_GRAPHICS) manual.xml $(PDF_TARGET) manual.html manual.fo
