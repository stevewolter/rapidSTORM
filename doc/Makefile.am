CHAPTERS = rapidstorm ui usage-examples input-options output-options errors fundamentals

PIXEL_GRAPHICS = rapidstorm-job-status.PNG rapidstorm-output-box.PNG \
    rapidstorm-step-by-step-2.PNG 
TEX_GRAPHICS = polynomial_3d_psf.tex polynomial_3d_parabolic_conversion.tex polynomial_3d_holtzer_conversion.tex
GENERATED_GRAPHICS = $(TEX_GRAPHICS:.tex=.png)
GRAPHICS = $(PIXEL_GRAPHICS) $(GENERATED_GRAPHICS)
EXTRA_DIST = $(TEX_GRAPHICS) $(PIXEL_GRAPHICS) $(CHAPTERS:=.xml) \
    rapidstorm_pre.xsl rapidstorm_html.xsl rapidstorm_fo.xsl tex2im

if BUILD_PDF_DOCUMENTATION
    PDF_TARGET = manual.pdf
else
    PDF_TARGET = 
endif

doc_DATA = $(GRAPHICS) manual.html $(PDF_TARGET)

manual.xml : $(CHAPTERS:=.xml)
manual.pdf : $(GRAPHICS) rapidstorm_fo.xsl
manual.html : $(GRAPHICS) rapidstorm_html.xsl

XSLT_FLAGS ?= 
XSLT_FLAGS_HTML ?= $(XSLT_FLAGS) 
XSLT_FLAGS_FO ?= $(XSLT_FLAGS) 

manual.xml : rapidstorm_pre.xsl
	$(XSLTPROC) $(XSLT_FLAGS) --xinclude -o $@ $(srcdir)/rapidstorm_pre.xsl $(srcdir)/rapidstorm.xml

.xml.html : rapidstorm_html.xsl
	$(XSLTPROC) $(XSLT_FLAGS_HTML) -o $@ $(srcdir)/rapidstorm_html.xsl $<

.xml.fo : rapidstorm_fo.xsl
	$(XSLTPROC) $(XSLT_FLAGS_FO) -o $@ $(srcdir)/rapidstorm_fo.xsl $<

.fo.pdf : foprc
	fop -c foprc $< $@

.tex.png :
	bash $(srcdir)/tex2im -r 150x150 -o $@ $^

CLEANFILES = $(GENERATED_GRAPHICS) manual.xml $(PDF_TARGET) manual.html manual.fo
