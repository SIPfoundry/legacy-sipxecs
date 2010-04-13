<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:strip-space elements="*"/>
<xsl:output method="text"/>

<xsl:template match="testsuite">
<xsl:variable name="bad" select="sum((@errors|@failures))"/>
<xsl:if test="$bad > 0">
testcase: <xsl:value-of select="@name"/>
--------------------------------------------------------------------------------
<xsl:apply-templates select="testcase"/>

</xsl:if>
</xsl:template>

<xsl:template match="failure">
<xsl:value-of select="@type"/>
<xsl:value-of select="text()"/>
</xsl:template>

</xsl:stylesheet>