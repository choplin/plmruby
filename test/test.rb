# -*- coding: utf-8 -*-
##
# TinyXML2 Test

assert('TinyXML2::XMLDocument#parse') do

  data = <<XMLDOC
<?xml version="1.0" encoding="UTF-8"?>
<!--comment-->
<root>
  <data name="hoge" type="Hello, 世界!">
    <text><![CDATA[ a << b ]]></text>
  </data>
</root>
XMLDOC

  xml = TinyXML2::XMLDocument.new
  assert_equal :XML_SUCCESS, xml.parse(data)

  decl = xml.first_child.to_declaration
  assert_equal "xml version=\"1.0\" encoding=\"UTF-8\"", decl.value

  comment = decl.next_sibling.to_comment
  assert_equal "comment", comment.value

  root = comment.next_sibling.to_element
  data = root.first_child.to_element
  assert_equal "hoge", data.attribute("name")
  assert_equal "Hello, 世界!", data.attribute("type")

  text = data.first_child.to_element.first_child.to_text
  assert_true text.cdata
  assert_equal " a << b ", text.value

end

assert('TinyXML2::XMLDocument#print') do

  data = <<XMLDOC
<?xml version="1.0" encoding="UTF-8"?>
<book type="novel" page="580" read="0">
    <marker from="10" to="12"><![CDATA[こんにちは、world!]]></marker>
    <marker from="11" to="13"/>
    <marker from="12" to="14"/>
    <marker from="13" to="15"/>
    <marker from="14" to="16"/>
    <marker from="15" to="17"/>
</book>
XMLDOC

  xml = TinyXML2::XMLDocument.new

  decl = xml.new_declaration
  xml.insert_end_child decl

  book = xml.new_element "book"
  xml.insert_end_child book
  book.set_attribute "type", "novel"
  book.set_attribute "page", 580
  book.set_attribute "read", false

  for i in 10..15
    marker = xml.new_element "marker"
    marker.set_attribute "from", "#{i}"
    marker.set_attribute "to", "#{i+2}"
    book.insert_end_child marker
  end

  text = xml.new_text "こんにちは、world!"
  text.set_cdata true
  book.first_child.insert_end_child text

  assert_equal data, xml.print

end
