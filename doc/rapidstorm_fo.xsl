<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
                xmlns:fo="http://www.w3.org/1999/XSL/Format" >
   <xsl:import href="http://docbook.sourceforge.net/release/xsl-ns/current/fo/docbook.xsl"/>

   <xsl:param name="section.autolabel" select="1"/>
   <xsl:param name="paper.type">A4</xsl:param>
   <xsl:param name="fop1.extensions">1</xsl:param>
   <xsl:param name="double.sided">1</xsl:param>
   <xsl:param name="appendix.autolabel">0</xsl:param>

   <xsl:template match="relatedtopicsinfo"/>

    <xsl:template match="guilabel">
        <xsl:call-template name="inline.boldseq"/>
    </xsl:template>

</xsl:stylesheet>
