<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >

<!-- 
 Convert GSview XML help to OS/2 IPF.
 This requires some strict formatting of the XML file
 to avoid duplicate spaces in the IPF file.
 Problems with current version:
 - All colons in text must be replaced by &colon.
   To implement this, we should replace : by :: before
   the translation, then replace :: by &colon.
   after the translation.
 - Links within text can't be done directly because we don't 
   know the target res number.  Put res=name and replace
   this by target res number during post processing.
-->

<!-- remove html tags -->
<xsl:output method="text" />

<xsl:template match="/">:userdoc.<xsl:apply-templates />
:euserdoc.
</xsl:template>

<xsl:template match="help">
:prolog.
:title.<xsl:value-of select="helptitle" />
:docprof toc=1234.
:eprolog.
<xsl:apply-templates select="topic"/>	
</xsl:template>

<xsl:template match="topic">:h1 res=<xsl:value-of select="browse" /> name='<xsl:value-of select="@id" />'.<xsl:value-of select="name" />
:p.
:i1. <xsl:value-of select="name" />
<xsl:apply-templates select="body" />:sl compact.<xsl:for-each select="topic">
:li.:link reftype=hd res=<xsl:value-of select="browse" />.<xsl:value-of select="name" />:elink.</xsl:for-each>
:esl.

<xsl:apply-templates select="topic" />	
</xsl:template>

<xsl:template match="body">
<xsl:apply-templates />
</xsl:template>

<xsl:template match="p">
:p.<xsl:apply-templates />
</xsl:template>

<xsl:template match="b">:hp2.<xsl:apply-templates />:ehp2.</xsl:template>

<xsl:template match="fixed">
:cgraphic.
 <xsl:apply-templates />
:ecgraphic.
</xsl:template>

<xsl:template match="file">:hp2<xsl:apply-templates />}:ehp2.</xsl:template>

<!-- We can't do links because we don't know the browse number of the target
<xsl:template match="link">
:link reftype=hd res=<xsl:value-of select="@href" />.<xsl:apply-templates />:elink.</xsl:template>
-->
<xsl:template match="link"><xsl:apply-templates /></xsl:template>

<xsl:template match="browse" />


</xsl:stylesheet>
