<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                xmlns:d="http://docbook.org/ns/docbook">
   <xsl:import href="http://docbook.sourceforge.net/release/xsl-ns/current/fo/docbook.xsl"/>
   <xsl:import href="titlepage.xsl"/>

   <xsl:param name="section.autolabel" select="1"/>
   <xsl:param name="paper.type">A4</xsl:param>
   <xsl:param name="fop1.extensions">1</xsl:param>
   <xsl:param name="double.sided">1</xsl:param>
   <xsl:param name="appendix.autolabel">0</xsl:param>
   <xsl:param name="glossary.sort" select="1"/>

   <xsl:template match="relatedtopicsinfo"/>

    <xsl:template match="guilabel">
        <xsl:call-template name="inline.boldseq"/>
    </xsl:template>

<xsl:template name="make.book.tocs">

  <xsl:variable name="lot-master-reference">
    <xsl:call-template name="select.pagemaster">
      <xsl:with-param name="pageclass" select="'lot'"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="toc.params">
    <xsl:call-template name="find.path.params">
      <xsl:with-param name="table" select="normalize-space($generate.toc)"/>
    </xsl:call-template>
  </xsl:variable>

    <xsl:call-template name="page.sequence">
      <xsl:with-param name="master-reference"
                      select="$lot-master-reference"/>
      <xsl:with-param name="element" select="'toc'"/>
      <xsl:with-param name="gentext-key" select="'TableofContents'"/>
      <xsl:with-param name="content">
        <xsl:if test="contains($toc.params, 'toc')">
            <xsl:call-template name="division.toc">
                <xsl:with-param name="toc.title.p" 
                                select="contains($toc.params, 'title')"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="contains($toc.params,'figure') and .//d:figure">
            <xsl:call-template name="list.of.titles">
                <xsl:with-param name="titles" select="'figure'"/>
                <xsl:with-param name="nodes" select=".//d:figure"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="contains($toc.params,'table') and .//d:table">
            <xsl:call-template name="list.of.titles">
                <xsl:with-param name="titles" select="'table'"/>
                <xsl:with-param name="nodes" select=".//d:table"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="contains($toc.params,'example') and .//d:example">
            <xsl:call-template name="list.of.titles">
                <xsl:with-param name="titles" select="'example'"/>
                <xsl:with-param name="nodes" select=".//d:example"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="contains($toc.params,'equation') and 
                        .//d:equation[d:title or d:info/d:title]">
            <xsl:call-template name="list.of.titles">
                <xsl:with-param name="titles" select="'equation'"/>
                <xsl:with-param name="nodes" 
                                select=".//d:equation[d:title or d:info/d:title]"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="contains($toc.params,'procedure') and 
                        .//d:procedure[d:title or d:info/d:title]">
            <xsl:call-template name="list.of.titles">
                <xsl:with-param name="titles" select="'procedure'"/>
                <xsl:with-param name="nodes" 
                                select=".//d:procedure[d:title or d:info/d:title]"/>
            </xsl:call-template>
        </xsl:if>
      </xsl:with-param>
    </xsl:call-template>

</xsl:template>

<xsl:template match="d:address" mode="titlepage.mode">
    <fo:block>
        <xsl:apply-templates mode="titlepage.mode"/>
    </fo:block>
</xsl:template>

<xsl:template match="d:areaspec">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="d:areaset">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="d:area">
    <fo:block id="{@id}"/>
</xsl:template>

<xsl:template match="d:imageobjectco">
    <xsl:apply-templates select="d:imageobject"/>
    <xsl:apply-templates select="d:calloutlist"/>
    <xsl:apply-templates select="d:areaspec"/>
</xsl:template>

</xsl:stylesheet>
