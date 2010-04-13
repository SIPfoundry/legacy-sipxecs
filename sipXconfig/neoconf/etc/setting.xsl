<?xml version="1.0" encoding="UTF-8"?>
<xs:stylesheet version="1.0" xmlns:xs="http://www.w3.org/1999/XSL/Transform">
	
	<xs:output method="html"/>

	<xs:template match="/">
		<html>
			<head>
		    <link rel="stylesheet" type="text/css" href="http://scm.sipfoundry.org/rep/sipXconfig/main/web/context/css/sipxconfig.css"/>
				<STYLE type="text/css">
					table {
				    background: #fff;
				    border-collapse: collapse;
				    border: 1px;
				    border-style : solid;
				    border-color : #ccc;  
					}					
				  thead {
			      background-color: #699;
					}					
					td {
				    border: 1px;
				    border-style : solid;
				    border-color : #ccc;  
				    border-collapse: collapse;
				    padding-bottom: 3px;
				    padding-top: 3px;
					}
				</STYLE>								
				<!--xs:apply-templates mode="head"/-->
			</head>	
			<body>
				<xs:apply-templates mode="body"/>
			</body>
		</html>
	</xs:template>
		
	<xs:template match="model/group" mode="body">
		<h2><xs:value-of select="label"/></h2>
		<p><xs:value-of select="@name"/></p>
		<br/>
		<table>
			<thead>
				<tr>
					<th>Name</th>
					<th>Label</th>
					<th>Description</th>					
				</tr>	
			</thead>	
			<tbody>
				<xs:apply-templates select="setting" mode="body"/>
			</tbody>	
		</table>
	</xs:template>
	
	<xs:template match="model/group/setting" mode="body">
		<tr>
			<td><xs:value-of select="@name"/></td>
			<td><xs:value-of select="label"/></td>
			<td>
				<xs:value-of select="description"/><br/>
				<em>Default value: <xs:value-of select="value"/></em><br/>
				<em>Type: <xs:apply-templates select="type" mode="body"/></em>
			</td>
		</tr>			
	</xs:template>

	<xs:template match="model/group/setting/type/integer" mode="body">
		integer<br/>
		min:<xs:value-of select="@min"/><br/>
		max:<xs:value-of select="@max"/><br/>
	</xs:template>

	<xs:template match="model/group/setting/type/string" mode="body">
		string
	</xs:template>
			
	<xs:template match="model/type" mode="body"/>		
		
</xs:stylesheet>