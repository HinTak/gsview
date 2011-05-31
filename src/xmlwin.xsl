<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >

<!-- Convert GSview XML help to XML with only Windows topics -->

<!--
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format">
-->

<xsl:output method="xml" />

<xsl:template match="/">
	<xsl:apply-templates />	
</xsl:template>

<xsl:template match="help">
	<help>
	<xsl:apply-templates />
	</help>
</xsl:template>

<xsl:template match="helptitle">
	<helptitle>
	<xsl:apply-templates />
	</helptitle>
</xsl:template>

<xsl:template match="topic">
	<topic>
	<xsl:attribute name="level">
	<xsl:value-of select="@level" />
	</xsl:attribute>
	<xsl:attribute name="id">
	<xsl:value-of select="@id" />
	</xsl:attribute>
	<xsl:apply-templates />
	</topic>
</xsl:template>

<xsl:template match="browse">
	<browse>
	<xsl:apply-templates />
	</browse>
</xsl:template>

<xsl:template match="name">
	<name>
	<xsl:apply-templates />
	</name>
</xsl:template>

<xsl:template match="key">
	<key>
	<xsl:apply-templates />
	</key>
</xsl:template>


<xsl:template match="body">
	<body>
	<xsl:apply-templates />
	</body>
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
	<fixed>
	<xsl:apply-templates />
	</fixed>
</xsl:template>

<xsl:template match="file">
	<file>
	<xsl:apply-templates />
	</file>
</xsl:template>

<xsl:template match="link">
	<link>
	<xsl:attribute name="href"><xsl:value-of select="@href" />
	</xsl:attribute>
	<xsl:apply-templates />
	</link>
</xsl:template>

<!-- change the following to include OS/2 or x11 topics -->
<xsl:template match="windows"><xsl:apply-templates /></xsl:template>

<xsl:template match="winos2"><xsl:apply-templates /></xsl:template>

<xsl:template match="os2"></xsl:template>

<xsl:template match="x11"></xsl:template>


</xsl:stylesheet>
