<?xml version="1.0"?>

<xsl:stylesheet version="1.0" exclude-result-prefixes="exsl mods"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:exsl="http://exslt.org/common" 
    xmlns="http://docbook.org/ns/docbook" 
    xmlns:mods="http://www.loc.gov/mods/v3">

<xsl:template match="mods:modsCollection">
    <bibliography>
        <xsl:apply-templates/>
    </bibliography>
</xsl:template>

<xsl:template match="mods:mods">
    <biblioentry id="{mods:identifier[@type='citekey']/text()}">
        <xsl:apply-templates/>
    </biblioentry>
</xsl:template>

<xsl:template match="mods:name">
    <xsl:choose>
        <xsl:when test="mods:role/mods:roleTerm = 'author'">
            <author>
                <xsl:apply-templates/>
            </author>
        </xsl:when>
        <xsl:when test="mods:role/mods:roleTerm = 'degree grantor'">
            <corpname><xsl:value-of select="mods:namePart"/></corpname>
        </xsl:when>
    </xsl:choose>
</xsl:template>

<xsl:template match="mods:namePart">
    <xsl:choose>
        <xsl:when test="@type='given' and preceding-sibling::mods:namePart[@type='given']">
            <othername><xsl:apply-templates/></othername>
        </xsl:when>
        <xsl:when test="@type='given'">
            <firstname><xsl:apply-templates/></firstname>
        </xsl:when>
        <xsl:when test="@type='family'">
            <surname><xsl:apply-templates/></surname>
        </xsl:when>
    </xsl:choose>
</xsl:template>

<xsl:template match="mods:titleInfo">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="mods:title">
    <title><xsl:apply-templates/></title>
</xsl:template>

<xsl:template match="mods:subTitle">
    <subtitle><xsl:apply-templates/></subtitle>
</xsl:template>

<xsl:template match="mods:role"/>
<xsl:template match="mods:typeOfResource"/>
<xsl:template match="mods:relatedItem">
    <xsl:choose>
        <xsl:when test="@type='host'">
            <biblioset relation="journal">
                <xsl:apply-templates/>
            </biblioset>
        </xsl:when>
    </xsl:choose>
</xsl:template>

<xsl:template match="mods:genre"/>

<xsl:template match="mods:originInfo">
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="mods:dateIssued">
    <date><xsl:value-of select="text()"/></date>
</xsl:template>

<xsl:template match="mods:note"/>

<xsl:template match="mods:language"/>

<xsl:template match="mods:issuance"/>

<xsl:template match="mods:publisher">
    <publisher>
        <publishername><xsl:value-of select="text()"/></publishername>
        <xsl:if test="../mods:place/mods:placeTerm">
            <address><xsl:value-of select="../mods:place/mods:placeTerm/text()"/></address>
        </xsl:if>
    </publisher>
</xsl:template>

<xsl:template match="mods:place"/>

<xsl:template match="mods:edition">
    <edition><xsl:value-of select="text()"/></edition>
</xsl:template>

<xsl:template match="mods:physicalDescription"/>

<xsl:template match="mods:abstract">
    <abstract><xsl:apply-templates/></abstract>
</xsl:template>
<xsl:template match="mods:subject"/>
<xsl:template match="mods:location">
    <xsl:apply-templates/>
</xsl:template>
<xsl:template match="mods:url">
    <biblioid class="uri"><xsl:value-of select="text()"/></biblioid>
</xsl:template>
<xsl:template match="mods:identifier">
    <xsl:choose>
        <xsl:when test="@type='citekey'"/>
        <xsl:otherwise>
            <biblioid class="{@type}">
                <xsl:value-of select="text()"/>
            </biblioid>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>
<xsl:template match="mods:part">
    <xsl:apply-templates mode="part"/>
</xsl:template>
<xsl:template match="mods:date" mode="part">
    <pubdate><xsl:value-of select="text()"/></pubdate>
</xsl:template>
<xsl:template match="mods:detail" mode="part">
    <xsl:choose>
        <xsl:when test="@type='volume'"><volumenum><xsl:value-of select="mods:number"/></volumenum></xsl:when>
        <xsl:when test="@type='number'"><issuenum><xsl:value-of select="mods:number"/></issuenum></xsl:when>
    </xsl:choose>
</xsl:template>
<xsl:template match="mods:extent" mode="part">
    <pagenums>
        <xsl:choose>
            <xsl:when test="mods:start and mods:end">
                <xsl:value-of select="mods:start"/><xsl:text>-</xsl:text><xsl:value-of select="mods:end"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="mods:start"/>
            </xsl:otherwise>
        </xsl:choose>
    </pagenums>
</xsl:template>

<xsl:template match="mods:*">
    <xsl:message terminate="yes">Unrecognized element <xsl:copy-of select="."/></xsl:message>
</xsl:template>

</xsl:stylesheet>
