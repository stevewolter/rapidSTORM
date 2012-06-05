<!DOCTYPE xsl:stylesheet [
<!ENTITY lf '<xsl:text xmlns:xsl="http://www.w3.org/1999/XSL/Transform">&#xA;</xsl:text>'>
]>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 xmlns:d="http://docbook.org/ns/docbook"
 version="1.0"
 exclude-result-prefixes="d"
>
   <xsl:import href="http://docbook.sourceforge.net/release/xsl-ns/current/htmlhelp/htmlhelp.xsl"/>
   <xsl:param name="htmlhelp.title" select="'rapidSTORM online help'"/>
   <xsl:param name="htmlhelp.button.next" select="1"/>
   <xsl:param name="htmlhelp.button.previous" select="1"/>
   <xsl:param name="htmlhelp.hhc.show.root" select="1"/>
   <xsl:param name="htmlhelp.use.hhk" select="1"/>
   <xsl:param name="section.autolabel" select="0"/>
   <xsl:param name="chapter.autolabel" select="0"/>
   <xsl:param name="part.autolabel" select="0"/>
   <xsl:param name="book.autolabel" select="0"/>
   <xsl:param name="use.id.as.filename" select="1"/>
   <xsl:param name="suppress.navigation" select="'0'"/>
   <xsl:param name="chunk.first.sections" select="1"/>
   <xsl:param name="bibliography.collection">bib4wbbt.xml</xsl:param>

</xsl:stylesheet>
