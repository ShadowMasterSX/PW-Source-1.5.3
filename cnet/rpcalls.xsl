<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/application">
    <html>
    <head>
        <title>RPCALLS</title>
    </head>
    <body>
    	<xsl:apply-templates select="service"></xsl:apply-templates>
	</body>
	</html>
</xsl:template>

<xsl:variable name="ucase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'" />
<xsl:variable name="lcase" select="'abcdefghijklmnopqrstuvwxyz'" />

<xsl:template match="service">
	<a>
		<xsl:attribute name="href"><xsl:value-of select="translate(@name, $ucase, $lcase)"/>\<xsl:value-of select="translate(@name, $ucase, $lcase)"/>.cpp</xsl:attribute>
		<B><xsl:value-of select="@name"/></B>
	</a><br></br>
	State:
		<xsl:apply-templates select="state"></xsl:apply-templates>
	<br></br>
	Manager:
		<xsl:apply-templates select="manager"></xsl:apply-templates>
	<br></br><ui></ui>
	Protocol:
		<xsl:apply-templates select="protocol"></xsl:apply-templates>
	<br></br><ui></ui>
	RPC:<xsl:apply-templates select="rpc"></xsl:apply-templates>
	<br></br>
	<br></br>

</xsl:template>

<xsl:template match="service/protocol">
	<li>
	<a>
		<xsl:attribute name="href"><xsl:value-of select="translate(../@name, $ucase, $lcase)"/>\<xsl:value-of select="translate(@ref, $ucase, $lcase)"/>.hpp</xsl:attribute>
		<xsl:value-of select="@ref" />
	</a>
	</li>
</xsl:template>
<xsl:template match="service/rpc">
	<li>
	<a>
		<xsl:attribute name="href"><xsl:value-of select="translate(../@name, $ucase, $lcase)"/>\<xsl:value-of select="translate(@ref, $ucase, $lcase)"/>.hrp</xsl:attribute>
		<xsl:value-of select="@ref" />
	</a>
	</li>
</xsl:template>
<xsl:template match="service/manager">
	<li>
	<a>
		<xsl:attribute name="href"><xsl:value-of select="translate(../@name, $ucase, $lcase)"/>\<xsl:value-of select="translate(@name, $ucase, $lcase)"/><xsl:value-of select="translate(@type, $ucase, $lcase)"/>.hpp</xsl:attribute>
		<xsl:value-of select="@name"/><xsl:value-of select="@type"/>.hpp
	</a><img width='10' height='0'></img>
	<a>
		<xsl:attribute name="href"><xsl:value-of select="translate(../@name, $ucase, $lcase)"/>\<xsl:value-of select="translate(@name, $ucase, $lcase)"/><xsl:value-of select="translate(@type, $ucase, $lcase)"/>.cpp</xsl:attribute>
		<xsl:value-of select="@name"/><xsl:value-of select="@type"/>.cpp
	</a><img width='10' height='0'></img>
	</li>
</xsl:template>
<xsl:template match="service/state">
	<xsl:value-of select="@ref"/><img width='10' height='0'></img>
</xsl:template>
</xsl:stylesheet>

 
