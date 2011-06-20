<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml"/>

<xsl:template match="/vCard">
<contact-information>
<jobTitle><xsl:value-of select="TITLE"/></jobTitle>
<xsl:apply-templates/>
<imID><xsl:value-of select="JABBERID" /></imID>
</contact-information>
</xsl:template>

<xsl:template match="/vCard/homeAddress">
  <homeAddress>
    <street><xsl:value-of select="STREET" /></street>
    <city><xsl:value-of select="LOCALITY" /></city>
    <country><xsl:value-of select="CTRY" /></country>
    <state><xsl:value-of select="REGION" /></state>
    <zip><xsl:value-of select="PCODE" /></zip>
  </homeAddress>
</xsl:template>

<xsl:template match="/vCard/ORG">
  <jobDept><xsl:value-of select="ORGUNIT" /></jobDept>
  <companyName><xsl:value-of select="ORGNAME" /></companyName>
</xsl:template>

<xsl:template match="/vCard/ADDR">
  <officeAddress>
    <street><xsl:value-of select="STREET" /></street>
    <city><xsl:value-of select="LOCALITY" /></city>
    <country><xsl:value-of select="CTRY" /></country>
    <state><xsl:value-of select="REGION" /></state>
    <zip><xsl:value-of select="PCODE" /></zip>
  </officeAddress>
</xsl:template>

<xsl:template match="/vCard/TEL">
  <cellPhoneNumber><xsl:value-of select="NUMBER"/></cellPhoneNumber>
</xsl:template>

<xsl:template match="/vCard/EMAIL">
  <emailAddress><xsl:value-of select="USERID" /></emailAddress>
</xsl:template>

</xsl:stylesheet>
