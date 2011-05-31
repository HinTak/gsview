<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >

<!-- 
 Convert GSview XML help to RTF for Windows Help.
 This requires some strict formatting of the XML file
 to avoid duplicate spaces in the RTF file.
-->


<!-- remove html tags -->
<xsl:output method="text" />

<xsl:template match="/">{\rtf1\ansi \deff0{\fonttbl{\f0\fswiss Arial;}{\f1\fmodern Courier New;}}
<xsl:apply-templates />	
}
</xsl:template>

<xsl:template match="help">
<xsl:apply-templates select="topic"/>	
</xsl:template>

<xsl:template match="topic">
{<xsl:apply-templates select="browse" />
${\footnote $ <xsl:value-of select="name" />}
{\b \fs24 <xsl:value-of select="name" />}\plain\par
#{\footnote # <xsl:value-of select="@id" />}
<xsl:for-each select="key">K{\footnote K <xsl:value-of select="." />}</xsl:for-each>
<xsl:apply-templates select="body" />	
<xsl:for-each select="topic">
\par{\uldb <xsl:value-of select="name" />}{\v <xsl:value-of select="@id" />}</xsl:for-each>
}{\plain \page}
<xsl:apply-templates select="topic" />	
</xsl:template>

<xsl:template match="body">
<xsl:apply-templates />
</xsl:template>

<xsl:template match="p">
\par
\pard \plain \qj \fs20 \f0
<xsl:apply-templates />
\par
</xsl:template>

<xsl:template match="b">{\b <xsl:apply-templates />}</xsl:template>

<xsl:template match="fixed">
{\f1 \fs20 <xsl:apply-templates />}
</xsl:template>

<xsl:template match="file">
{\b <xsl:apply-templates />}
</xsl:template>

<xsl:template match="link">
{\b <xsl:apply-templates />}
</xsl:template>

<!--
<xsl:template match="browse">
+{\footnote + <xsl:apply-templates />}</xsl:template>
-->
<xsl:template match="browse">
+{\footnote + <xsl:number level="multiple" count="topic" format="01." />}</xsl:template>


</xsl:stylesheet>
