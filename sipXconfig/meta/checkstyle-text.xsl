<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" omit-xml-declaration="yes"/>

<xsl:strip-space elements="*" />

<xsl:template match="/">
Coding Style Check Results
--------------------------
Total files checked: <xsl:number level="any" value="count(descendant::file)"/>
  Files with errors: <xsl:number level="any" value="count(descendant::file[error[@severity = 'error']])"/>
       Total errors: <xsl:number level="any" value="count(descendant::error)"/>
<xsl:text>
    
</xsl:text>           
    <xsl:apply-templates/>
</xsl:template>

<xsl:template match="error[@severity = 'error']">
	<xsl:value-of select="../@name"/>:<xsl:value-of select="@line"/>:<xsl:value-of select="@column"/><xsl:text> - </xsl:text><xsl:value-of select="@message"/>
            <xsl:text>
</xsl:text>    
</xsl:template>

<xsl:template match="file"><xsl:apply-templates/></xsl:template>

</xsl:stylesheet>
