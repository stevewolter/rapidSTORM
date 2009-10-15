<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
 version="1.0">
   <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/htmlhelp/htmlhelp.xsl"/>
   <xsl:param name="htmlhelp.title" select="'rapidSTORM online help'"/>
   <xsl:param name="htmlhelp.button.next" select="1"/>
   <xsl:param name="htmlhelp.button.previous" select="1"/>
   <xsl:param name="htmlhelp.hhc.show.root" select="0"/>
   <xsl:param name="htmlhelp.use.hhk" select="1"/>
   <!-- <xsl:param name="generate.toc" select="'article nop'"/>  -->
   <xsl:param name="section.autolabel" select="0"/>
   <xsl:param name="chapter.autolabel" select="0"/>
   <xsl:param name="part.autolabel" select="0"/>
   <xsl:param name="book.autolabel" select="0"/>
   <xsl:param name="use.id.as.filename" select="1"/>
   <xsl:param name="htmlhelp.default.topic" select="'help_on_help.html'"/>
   <xsl:param name="html.stylesheet" select="'rapidstorm.css'"/>
   <xsl:param name="suppress.navigation" select="'0'"/>
   <xsl:param name="chunk.first.sections" select="1"/>

   <xsl:template name="preface.titlepage.separator">
      <xsl:call-template name="related_topics_link"/></xsl:template>
   <xsl:template name="part.titlepage.separator">
      <xsl:call-template name="related_topics_link"/></xsl:template>
   <xsl:template name="chapter.titlepage.separator">
      <xsl:call-template name="related_topics_link"/></xsl:template>
   <xsl:template name="section.titlepage.separator">
      <xsl:if test="not(ancestor::section)">
        <xsl:call-template name="related_topics_link"/>
      </xsl:if>
   </xsl:template>

   <xsl:template name="related_topics_link">
      <span class="navigation_item">
      [<a href="#related">Related Topics</a>]
      </span>
      <hr/>
   </xsl:template>

   <xsl:template name="make-link">
      <xsl:param name="next"/>
      <xsl:param name="text"/>
      <xsl:if test="count($next)!=0">
      <span class="navigation_item">
      <xsl:text>[</xsl:text>
      <a>
        <xsl:attribute name="href">
        <xsl:call-template name="href.target">
	  <xsl:with-param name="object" select="$next"/>
        </xsl:call-template>
        </xsl:attribute>
        <xsl:value-of select="$text"/>
      </a>
      <xsl:text>]</xsl:text>
      </span>
      </xsl:if>
   </xsl:template>
   <xsl:template name="header.navigation">
      <xsl:param name="prev"/>
      <xsl:param name="next"/>

      <a id="top_of_page_anchor"/>
      <xsl:if test="ancestor::part">
          <div class="title part_header">  
          <xsl:value-of select="ancestor::part/title"/>
          </div>
          <xsl:if test="ancestor::chapter">
             <div class="title chapter_header">
                <xsl:value-of select="ancestor::chapter/title"/>
             </div>
          </xsl:if>
      </xsl:if>
<!--
      <div class="navigation_container">
      <xsl:call-template name="make-link">
         <xsl:with-param name="next" select="$prev"/>
         <xsl:with-param name="text" select="'Previous'"/>
      </xsl:call-template>
      <xsl:call-template name="make-link">
         <xsl:with-param name="next" select="parent::*"/>
         <xsl:with-param name="text" select="'Up'"/>
      </xsl:call-template>
      <xsl:call-template name="make-link">
         <xsl:with-param name="next" select="$next"/>
         <xsl:with-param name="text" select="'Next'"/>
      </xsl:call-template>
      </div>
-->

   </xsl:template>
   <xsl:template name="footer.navigation">
      <xsl:param name="next"/>
      <xsl:param name="prev"/>
      <hr/>
      <span id="related_topics">Related topics:</span>
      <span id="top_of_page_link">[<a href="#top_of_page_anchor">Top</a>]</span>
      <p id="support_link">[<a href="support.html">Support</a>]</p>
   </xsl:template>

   <xsl:template match="navel">
      [<i><xsl:value-of select="*|text()"/></i>]
   </xsl:template>

   <xsl:template match="menu">
     <b><xsl:apply-templates select="node()"/></b>
   </xsl:template>
   <xsl:template match="menuitem">
     <b><xsl:apply-templates select="node()"/></b>
   </xsl:template>
</xsl:stylesheet>
