mruby-tinyxml2
==============

mruby bindings for TinyXML-2

```ruby
xml = TinyXML2::XMLDocument.new

decl = xml.new_declaration
xml.insert_end_child decl

bookshelf = xml.new_element "bookshelf"
xml.insert_end_child bookshelf

book = xml.new_element "book"
bookshelf.insert_end_child book
book.set_attribute "type", "novel"
book.set_attribute "page", 580

text = xml.new_text "TinyXML-2"
text.set_cdata true
book.insert_end_child text

# Different from the original TinyXML-2,
# XMLDocument#print returns a string directly, no stdout.

print xml.print
```

#### output
```xml
<?xml version="1.0" encoding="UTF-8"?>
<bookshelf>
    <book type="novel" page="580"><![CDATA[TinyXML-2]]></book>
</bookshelf>
```