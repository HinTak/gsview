<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >

<!-- Convert GSview XML help to Windows HTML Help Index.  -->

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
	<xsl:comment>Automatically produced using xml2hhc.xsl</xsl:comment>
	</head>
	<body>
	<ul>
	<xsl:apply-templates select="topic"/>	
	</ul>
  	</body></html>
</xsl:template>

<xsl:template match="topic">
	<xsl:for-each select="key">
	<li>
	<object> 
	  <xsl:attribute name="type">text/sitemap</xsl:attribute>
	  <param> 
	    <xsl:attribute name="name">Keyword</xsl:attribute>
	    <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
	  </param>
	  <param> 
	    <xsl:attribute name="name">Local</xsl:attribute>
	    <xsl:attribute name="value">html/<xsl:value-of select="../@id" />.htm</xsl:attribute>
	  </param>
	</object>
	</li>
	</xsl:for-each>

	<xsl:apply-templates select="topic" />	
</xsl:template>


</xsl:stylesheet>
