<?xml version="1.0"?>
<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/application">
    <html>
    <body>
	<xsl:apply-templates select="table"></xsl:apply-templates>
	<br/>
	<xsl:apply-templates select="query"></xsl:apply-templates>
	<br/><br/>
	<xsl:apply-templates select="procedure"></xsl:apply-templates>
	<br/>
    </body>
    </html>
</xsl:template>
<xsl:template match="procedure">
	CREATE PROCEDURE <xsl:value-of select="@name"/><br/>
	<xsl:for-each select="parameter">
		<img width='20' height='1'/>@<xsl:value-of select="@name"/>&#x20;<xsl:value-of select="@sql-type"/>&#x20;
		<xsl:if test="@out='true'">out</xsl:if>
		<xsl:if test="position()!=last()"><br/></xsl:if>
	</xsl:for-each>
<PRE>AS
BEGIN<xsl:value-of select="."/>END
</PRE>
</xsl:template>

<xsl:template match="query">
	<xsl:value-of select="@name"/>:<br/>
	<img width='20' height='1'/><xsl:value-of select="select/@name"/>:(cache size <xsl:value-of select="select/cache/@size"/>, timeout <xsl:value-of select="select/cache/@timeout"/>)<br/>
	<img width='30' height='1'/>
	SELECT <xsl:if test="select/@unique='true'">DISTINCT </xsl:if>
	<xsl:for-each select="column">
		<xsl:if test="@compute!=''"><xsl:value-of select="@compute"/>&#x20;AS&#x20;<xsl:value-of select="@name"/></xsl:if>
		<xsl:if test="@column!=''"><xsl:value-of select="@column"/>&#x20;AS&#x20;<xsl:value-of select="@name"/></xsl:if>
		<xsl:if test="position()!=last()">,</xsl:if>&#x20;
	</xsl:for-each>
	FROM 
	<xsl:for-each select="table">
		<xsl:value-of select="@name"/>&#x20;<xsl:value-of select="@alias"/>
		<xsl:if test="position()!=last()">,</xsl:if>&#x20;
	</xsl:for-each>
	<xsl:value-of select="select/@condition"/>
	<br/>
</xsl:template>

<xsl:template match="table">
	CREATE TABLE <font color="red"><xsl:value-of select="@name"/></font>(<br/>
	<xsl:for-each select="column"><img width='20' height='1'/>
		<font color="red"><xsl:value-of select="@name"/></font>&#x20;
		<xsl:value-of select="@sql-type"/>&#x20;
		<xsl:if test="@not-null='true'">NOT NULL</xsl:if>
		<xsl:if test="position()!=last()">,</xsl:if><br/>
	</xsl:for-each>

	<img width='20' height='1'/>PRIMARY KEY (<xsl:value-of select="primarykey/@column"/>)
	<br/>)<br/>
	
	<xsl:for-each select="index">
		CREATE <xsl:if test="@unique='true'">UNIQUE</xsl:if>
		INDEX <xsl:value-of select="@name"/> ON <xsl:value-of select="../@name"/> (<xsl:value-of select="@column"/>)<br/>
	</xsl:for-each>

	<xsl:for-each select="insert">
		<xsl:value-of select="@name"/>: 
		<xsl:choose>
			<xsl:when test="@condition">
				INSERT INTO <xsl:value-of select="../@name"/>&#x20;<xsl:value-of select="@condition" /><br/>
			</xsl:when>
			<xsl:otherwise>
				INSERT INTO <xsl:value-of select="../@name"/> (
				<xsl:for-each select="../column">
					<xsl:value-of select="@name"/>
					<xsl:if test="position()!=last()">,</xsl:if>&#x20;
				</xsl:for-each>
				) VALUES (
				<xsl:for-each select="../column">
					?<xsl:if test="position()!=last()">,</xsl:if>&#x20;
				</xsl:for-each>
				)<br/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:for-each>
	
	<xsl:for-each select="delete">
		<xsl:value-of select="@name"/>: DELETE FROM <xsl:value-of select="../@name"/> &#x20;<xsl:value-of select="@condition"/><br/>
	</xsl:for-each>

	<xsl:for-each select="update">
		<xsl:value-of select="@name"/>: UPDATE <xsl:value-of select="../@name"/> SET <xsl:value-of select="@column"/> &#x20; <xsl:value-of select="@condition"/><br/>
	</xsl:for-each>
	<br/>
	
</xsl:template>

</xsl:stylesheet>
