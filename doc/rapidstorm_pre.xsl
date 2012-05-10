<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 version="1.0" 
 xmlns='http://docbook.org/ns/docbook' 
 xmlns:docbook='http://docbook.org/ns/docbook' 
 xmlns:elemtable="http://super-resolution.de/elemtable">
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>
  <xsl:template match="elemtable:elemtable">
    <glosslist>
      <xsl:copy-of select="docbook:title"/>
      <xsl:apply-templates select="elemtable:elem"/>
    </glosslist>
  </xsl:template>
  <xsl:template match="elemtable:elem">
    <glossentry id="{@topic}">
        <glossterm><xsl:value-of select="@desc"/></glossterm>
        <glossdef><para>
            <xsl:apply-templates select="node()"/>
        </para></glossdef>
    </glossentry>
  </xsl:template>
</xsl:stylesheet>
