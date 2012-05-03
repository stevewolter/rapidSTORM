<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 version="1.0">
  <xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
  </xsl:template>
  <xsl:template match="elemtable">
    <table>
      <xsl:copy-of select="title"/>
      <tgroup cols='2' align='left' colsep='1' rowsep='1'>
        <thead>
          <row><entry>Name</entry>
               <entry>Effect</entry></row>
        </thead>
        <tbody>
         <xsl:apply-templates
            select="elem"/>
        </tbody>
      </tgroup>
    </table>
  </xsl:template>
  <xsl:template match="elem">
    <row>
       <entry>
         <xsl:value-of select="@desc"/>
       </entry>
       <entry>
         <xsl:if test="@topic">
            <anchor id="{@topic}">
            <xsl:processing-instruction name="dbhh">
              <xsl:text>topicname="HELP_</xsl:text>
              <xsl:value-of select="@topic"/>
              <xsl:text>" </xsl:text>
              <xsl:text>topicid="</xsl:text>
              <xsl:value-of select="count(preceding::elem[@topic])+1000"/>
              <xsl:text>"</xsl:text>
            </xsl:processing-instruction>
            </anchor>
         </xsl:if>
         <xsl:apply-templates select="node()"/>
       </entry>
    </row>
  </xsl:template>
</xsl:stylesheet>
