<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >

<!-- Convert GSview XML help to Windows HTML Help Contents.  -->

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
	<li>
	<object> 
	  <xsl:attribute name="type">text/sitemap</xsl:attribute>
	  <param> 
	    <xsl:attribute name="name">Name</xsl:attribute>
	    <xsl:attribute name="value"><xsl:value-of select="name" /></xsl:attribute>
	  </param>
	  <param> 
	    <xsl:attribute name="name">Local</xsl:attribute>
	    <xsl:attribute name="value">html/<xsl:value-of select="@id" />.htm</xsl:attribute>
	  </param>
	</object>
	</li>

	<!-- do following three lines only if there are sub-topics -->
	<xsl:if test="topic">
	<ul>
	<xsl:apply-templates select="topic" />	
	</ul>
	</xsl:if>
</xsl:template>


</xsl:stylesheet>
