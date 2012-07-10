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
            <xsl:processing-instruction name="dbhh">
              <xsl:text>topicname="HELP_</xsl:text>
              <xsl:value-of select="@topic"/>
              <xsl:text>" </xsl:text>
              <xsl:text>topicid="</xsl:text>
              <xsl:value-of select="count(preceding::elemtable:elem[@topic])+1000"/>
              <xsl:text>"</xsl:text>
            </xsl:processing-instruction>
        <glossterm><xsl:value-of select="@desc"/>
            </glossterm>
        <glossdef>
            <xsl:apply-templates select="node()|text()"/>
        </glossdef>
    </glossentry>
  </xsl:template>
  <xsl:template match="elemtable:title">
    <title id="{@topic}"><xsl:value-of select="@desc"/></title>
  </xsl:template>
  <xsl:template match="elemtable:elemref">
    <guilabel><xref linkend="{@linkend}"/></guilabel>
  </xsl:template>
</xsl:stylesheet>
