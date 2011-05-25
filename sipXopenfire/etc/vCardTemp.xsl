<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml"/>

<xsl:template match="/contact-information">
<vCard xmlns='vcard-temp'>
<ORG>
   <ORGNAME><xsl:value-of select="companyName" /></ORGNAME>
   <ORGUNIT> <xsl:value-of select="jobDept" /></ORGUNIT>
</ORG>
<TITLE><xsl:value-of select="jobTitle" /></TITLE>
<xsl:apply-temlates/>
    <!--<FN>Peter Saint-Andre</FN>
    <N>
      <FAMILY>Saint-Andre</FAMILY>
      <GIVEN>Peter</GIVEN>
      <MIDDLE/>
    </N>
    <NICKNAME>stpeter</NICKNAME>
    <URL>http://www.xmpp.org/xsf/people/stpeter.shtml</URL>
    <BDAY>1966-08-06</BDAY>-->


<!--<ROLE>Patron Saint</ROLE>-->
<!-- <TEL><WORK/><VOICE/><NUMBER><xsl:value-of select="" /></NUMBER></TEL> -->
<TEL><HOME/><VOICE/><NUMBER><xsl:value-of select="homePhoneNumber" /></NUMBER></TEL>
<TEL><HOME/><FAX/><NUMBER/></TEL>
<TEL><HOME/><MSG/><NUMBER/></TEL>
<TEL><WORK/><FAX/><xsl:value-of select="faxNumber" /><NUMBER/></TEL>
<EMAIL><INTERNET/><PREF/><USERID><xsl:value-of select="emailAddress" /></USERID></EMAIL>
<JABBERID><xsl:value-of select="imId" /></JABBERID>
</vCard>
</xsl:template>

<xsl:template match="/contact-information/officeAddress">
<!-- <TEL><WORK/><MSG/><NUMBER/></TEL>  -->
<ADR>
<WORK/>
<!-- <EXTADD>Suite 600</EXTADD>-->
<STREET> <xsl:value-of select="street" /></STREET>
<LOCALITY><xsl:value-of select="city" /></LOCALITY>
<REGION><xsl:value-of select="state" /></REGION>
<PCODE><xsl:value-of select="zip" /></PCODE>
<CTRY><xsl:value-of select="country" /></CTRY>
</ADR>
</xsl:template>

<xsl:template match="/contact-information/homeAddress">
<ADR>
<HOME/>
<EXTADD/>
<STREET> <xsl:value-of select="street" /></STREET>
<LOCALITY><xsl:value-of select="city" /></LOCALITY>
<REGION><xsl:value-of select="state" /></REGION>
<PCODE><xsl:value-of select="zip" /></PCODE>
<CTRY><xsl:value-of select="country" /></CTRY>
</ADR>
</xsl:template>
</xsl:stylesheet>
