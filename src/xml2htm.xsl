<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >

<!-- Convert GSview XML help to XHTML -->

<!--
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format">
-->

<xsl:output method="html" />

<xsl:template match="/">
	<xsl:apply-templates />	
</xsl:template>

<xsl:template match="help">
	<html>
	<head>
	<xsl:comment>Automatically produced using xml2htm.xsl</xsl:comment>
	<title>
	    <xsl:value-of select="helptitle" />
	</title>
	</head>
	<body>
	<h1>
	<xsl:value-of select="helptitle" />
	</h1>
	<xsl:apply-templates select="topic"/>	
  	</body></html>
</xsl:template>

<xsl:template match="topic">
	<hr> </hr>
	<a> <xsl:attribute name="name">
	<xsl:value-of select="@id" />
	</xsl:attribute>
	</a>

<!--
	<xsl:for-each select="key">
	<a> <xsl:attribute name="name">
	<xsl:value-of select="." />
	</xsl:attribute>
	</a>
	</xsl:for-each>
-->

<!-- Try to figure out a way to get level automatically 
<tt>Level
<xsl:number level="any" count="topic" format="01" />
</tt>
-->

<!-- Look at topic level and set heading level accordingly -->
	<xsl:choose>
	<xsl:when test="@level = 1">
	<h1>
	    <xsl:value-of select="name" />
	</h1>
	</xsl:when>
	<xsl:when test="@level = 2">
	<h1>
	    <xsl:value-of select="name" />
	</h1>
	</xsl:when>
	<xsl:when test="@level = 3">
	<h2>
	    <xsl:value-of select="name" />
	</h2>
	</xsl:when>
	<xsl:otherwise>
	<h3>
	    <xsl:value-of select="name" />
	</h3>
	</xsl:otherwise>
	</xsl:choose>

	<xsl:apply-templates select="body" />	
	<xsl:for-each select="topic">
	     <a> 
	     <xsl:attribute name="href">#<xsl:value-of select="@id" />
	     </xsl:attribute>
	     <xsl:value-of select="name" />
	     </a><br></br>
	</xsl:for-each>
	<xsl:apply-templates select="topic" />	
</xsl:template>

<xsl:template match="body">
	<xsl:apply-templates />
</xsl:template>

<xsl:template match="p">
	<p>
	<xsl:apply-templates />
	</p>
</xsl:template>

<xsl:template match="b">
	<b>
	<xsl:apply-templates />
	</b>
</xsl:template>

<xsl:template match="fixed">
	<pre>
	<xsl:apply-templates />
	</pre>
</xsl:template>

<xsl:template match="file">
	<i>
	<xsl:apply-templates />
	</i>
</xsl:template>

<xsl:template match="link">
	<a> 
	<xsl:attribute name="href">#<xsl:value-of select="@href" />
	</xsl:attribute>
	<xsl:apply-templates />
	</a>
</xsl:template>


</xsl:stylesheet>
