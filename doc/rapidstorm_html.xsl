<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 version="1.0">
   <xsl:import href="http://docbook.sourceforge.net/release/xsl-ns/current/html/docbook.xsl"/>

   <xsl:param name="ignore.image.scaling" select="1"/>
   <xsl:param name="section.autolabel" select="1"/>
   <xsl:param name="glossary.sort" select="1"/>
   <xsl:param name="section.label.includes.component.label" select="1"/>
   <xsl:param name="bibliography.collection">bib4wbbt.xml</xsl:param>
   <xsl:template match="relatedtopicsinfo"/>
    <xsl:template match="guilabel">
        <xsl:call-template name="inline.boldseq"/>
    </xsl:template>


</xsl:stylesheet>
