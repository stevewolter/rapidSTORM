<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
                xmlns:fo="http://www.w3.org/1999/XSL/Format" >
   <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/fo/docbook.xsl"/>

   <xsl:param name="section.autolabel" select="1"/>
   <xsl:template match="relatedtopicsinfo"/>

    <xsl:template match="guilabel">
        <xsl:call-template name="inline.boldseq"/>
    </xsl:template>

</xsl:stylesheet>
