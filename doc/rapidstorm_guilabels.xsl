<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 version="1.0" 
 xmlns='http://docbook.org/ns/docbook' 
 xmlns:docbook='http://docbook.org/ns/docbook' 
 xmlns:elemtable="http://super-resolution.de/elemtable">
  <xsl:output method="text"/>
  <xsl:template match="/">
<xsl:text>const char *guilabels[][3] = {
</xsl:text>
<xsl:apply-templates />
<xsl:text>};
</xsl:text>
  </xsl:template>
  <xsl:template match="@*|node()">
    <xsl:apply-templates select="@*|node()"/>
  </xsl:template>
  <xsl:template match="elemtable:elemtable">
      <xsl:apply-templates select="elemtable:elem"/>
  </xsl:template>
  <xsl:template match="elemtable:title">
    <xsl:text>{ "</xsl:text>
    <xsl:value-of select="@topic"/>
    <xsl:text>", "</xsl:text>
    <xsl:value-of select="@desc"/>
    <xsl:text>", "" },
</xsl:text>
  </xsl:template>
  <xsl:template match="elemtable:elem">
    <xsl:text>{ "</xsl:text>
    <xsl:value-of select="@topic"/>
    <xsl:text>", "</xsl:text>
    <xsl:value-of select="@desc"/>
    <xsl:text>", "</xsl:text>
    <xsl:value-of select="normalize-space(.)"/>
    <xsl:text>" },
</xsl:text>
  </xsl:template>
</xsl:stylesheet>
