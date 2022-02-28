#!/bin/sh

x=200
cnt=0

while true 
do
	x=`expr $x + 1`
	echo "$cnt Writing $x"
	echo ZZ$x:s:{"glossary": { "title": "example glossary", "GlossDiv": { "title": "S", "GlossList": { "GlossEntry": { "ID": "SGML", "SortAs": "SGML", "GlossTerm": "Standard Generalized Markup Language", "Acronym": "SGML", "Abbrev": "ISO 8879-1986", "GlossDef": { "para": "A meta-markup language used to create markup languages such as DocBook.", "GlossSeeAlso": ["GML", "XML"], "hello": "Sejin" }, "GlossSee": "markup" } } }, "HelloDiv": { "title": "V", "GlossList": { "GlossEntry": { "ID": "YGML", "SortAs": "YGML", "GlossTerm": "Non-Standard Generalized Markup Language", "Acronym": "YGML", "Abbrev": "Non ISO", "GlossDef": { "para": "A meta-markup language used to create markup languages such as DocBook.", "GlossSeeAlso": ["YML", "XML", "222", "3333", "ssssssssssssssssssss" ], "hello": "Myungjin" }, "GlossSee": "markup" }, "Bushman": "mochican" }}}} >> $1
	cnt=`expr $cnt + 1`
	if [ "$cnt" = "2000" ]; then exit 1; fi
done
