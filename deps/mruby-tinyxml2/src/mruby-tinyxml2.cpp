#include "mruby.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/value.h"
#include "mruby/string.h"
#include "tinyxml2.h"

using namespace tinyxml2;

static void
xml_object_free(mrb_state *mrb, void *ptr)
{
}

static void
xml_document_free(mrb_state *mrb, void *ptr)
{
  delete static_cast<XMLDocument*>(ptr);
}

static struct mrb_data_type xml_node_type            = { "XMLNode",           xml_object_free   };
static struct mrb_data_type xml_document_type        = { "XMLDocument",       xml_document_free };
static struct mrb_data_type xml_element_type         = { "XMLElement",        xml_object_free   };
static struct mrb_data_type xml_const_attribute_type = { "XMLConstAttribute", xml_object_free   };
static struct mrb_data_type xml_attribute_type       = { "XMLAttribute",      xml_object_free   };
static struct mrb_data_type xml_comment_type         = { "XMLComment",        xml_object_free   };
static struct mrb_data_type xml_text_type            = { "XMLText",           xml_object_free   };
static struct mrb_data_type xml_declaration_type     = { "XMLDeclaration",    xml_object_free   };
static struct mrb_data_type xml_unknown_type         = { "XMLUnknown",        xml_object_free   };

static struct RClass *tinyxml2_module;
static struct RClass *xml_node_class;
static struct RClass *xml_document_class;
static struct RClass *xml_element_class;
static struct RClass *xml_const_attribute_class;
static struct RClass *xml_attribute_class;
static struct RClass *xml_comment_class;
static struct RClass *xml_text_class;
static struct RClass *xml_declaration_class;
static struct RClass *xml_unknown_class;

static mrb_sym
xml_error_to_symbol(mrb_state *mrb, XMLError error)
{
  switch (error) {
  case XML_SUCCESS:                        return mrb_intern_cstr(mrb, "XML_SUCCESS");
  case XML_NO_ATTRIBUTE:                   return mrb_intern_cstr(mrb, "XML_NO_ATTRIBUTE");
  case XML_WRONG_ATTRIBUTE_TYPE:           return mrb_intern_cstr(mrb, "XML_WRONG_ATTRIBUTE_TYPE");
  case XML_ERROR_FILE_NOT_FOUND:           return mrb_intern_cstr(mrb, "XML_ERROR_FILE_NOT_FOUND");
  case XML_ERROR_FILE_COULD_NOT_BE_OPENED: return mrb_intern_cstr(mrb, "XML_ERROR_FILE_COULD_NOT_BE_OPENED");
  case XML_ERROR_FILE_READ_ERROR:          return mrb_intern_cstr(mrb, "XML_ERROR_FILE_READ_ERROR");
  case XML_ERROR_ELEMENT_MISMATCH:         return mrb_intern_cstr(mrb, "XML_ERROR_ELEMENT_MISMATCH");
  case XML_ERROR_PARSING_ELEMENT:          return mrb_intern_cstr(mrb, "XML_ERROR_PARSING_ELEMENT");
  case XML_ERROR_PARSING_ATTRIBUTE:        return mrb_intern_cstr(mrb, "XML_ERROR_PARSING_ATTRIBUTE");
  case XML_ERROR_IDENTIFYING_TAG:          return mrb_intern_cstr(mrb, "XML_ERROR_IDENTIFYING_TAG");
  case XML_ERROR_PARSING_TEXT:             return mrb_intern_cstr(mrb, "XML_ERROR_PARSING_TEXT");
  case XML_ERROR_PARSING_CDATA:            return mrb_intern_cstr(mrb, "XML_ERROR_PARSING_CDATA");
  case XML_ERROR_PARSING_COMMENT:          return mrb_intern_cstr(mrb, "XML_ERROR_PARSING_COMMENT");
  case XML_ERROR_PARSING_DECLARATION:      return mrb_intern_cstr(mrb, "XML_ERROR_PARSING_DECLARATION");
  case XML_ERROR_PARSING_UNKNOWN:          return mrb_intern_cstr(mrb, "XML_ERROR_PARSING_UNKNOWN");
  case XML_ERROR_EMPTY_DOCUMENT:           return mrb_intern_cstr(mrb, "XML_ERROR_EMPTY_DOCUMENT");
  case XML_ERROR_MISMATCHED_ELEMENT:       return mrb_intern_cstr(mrb, "XML_ERROR_MISMATCHED_ELEMENT");
  case XML_ERROR_PARSING:                  return mrb_intern_cstr(mrb, "XML_ERROR_PARSING");
  case XML_CAN_NOT_CONVERT_TEXT:           return mrb_intern_cstr(mrb, "XML_CAN_NOT_CONVERT_TEXT");
  case XML_NO_TEXT_NODE:                   return mrb_intern_cstr(mrb, "XML_NO_TEXT_NODE");
  default:
    mrb_raise(mrb, E_RUNTIME_ERROR, "Undefined XMLError code");
  }
  return 0;
}

static mrb_value
xml_node_alloc(mrb_state *mrb, XMLNode *ptr)
{
  if (ptr) {
    struct RData *node = mrb_data_object_alloc(mrb, xml_node_class,
					       ptr, &xml_node_type);
    return mrb_obj_value(node);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_const_attribute_alloc(mrb_state *mrb, const XMLAttribute *ptr)
{
  if (ptr) {
    struct RData *attribute = mrb_data_object_alloc(mrb, xml_const_attribute_class,
						    const_cast<XMLAttribute*>(ptr),
						    &xml_const_attribute_type);
    return mrb_obj_value(attribute);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_sym
xml_whitespace_to_symbol(mrb_state *mrb, Whitespace whitespace)
{
  switch (whitespace) {
  case PRESERVE_WHITESPACE: return mrb_intern_cstr(mrb, "PRESERVE_WHITESPACE");
  case COLLAPSE_WHITESPACE: return mrb_intern_cstr(mrb, "COLLAPSE_WHITESPACE");
  default:
    mrb_raise(mrb, E_RUNTIME_ERROR, "Undefined Whitespace code");
  }
  return 0;
}

/* XMLNode */
static mrb_value
xml_node_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_node_type;
  DATA_PTR(self) = NULL;
  mrb_raise(mrb, E_RUNTIME_ERROR, "constructor is private");
  return self;
}

static mrb_value
xml_node_to_element(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLElement *ptr = node->ToElement();
  if (ptr) {
    struct RData *element = mrb_data_object_alloc(mrb, xml_element_class,
						  ptr, &xml_element_type);
    return mrb_obj_value(element);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_to_text(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLText *ptr = node->ToText();
  if (ptr) {
    struct RData *text = mrb_data_object_alloc(mrb, xml_text_class,
					       ptr, &xml_text_type);
    return mrb_obj_value(text);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_to_comment(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLComment *ptr = node->ToComment();
  if (ptr) {
    struct RData *comment = mrb_data_object_alloc(mrb, xml_comment_class,
					       ptr, &xml_comment_type);
    return mrb_obj_value(comment);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_to_document(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLDocument *ptr = node->ToDocument();
  if (ptr) {
    struct RData *document = mrb_data_object_alloc(mrb, xml_document_class,
					           ptr, &xml_document_type);
    return mrb_obj_value(document);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_to_declaration(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLDeclaration *ptr = node->ToDeclaration();
  if (ptr) {
    struct RData *declaration = mrb_data_object_alloc(mrb, xml_declaration_class,
					           ptr, &xml_declaration_type);
    return mrb_obj_value(declaration);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_to_unknown(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLUnknown *ptr = node->ToUnknown();
  if (ptr) {
    struct RData *unknown = mrb_data_object_alloc(mrb, xml_unknown_class,
						  ptr, &xml_unknown_type);
    return mrb_obj_value(unknown);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_value(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  return mrb_str_new_cstr(mrb, node->Value());
}

static mrb_value
xml_node_set_value(mrb_state *mrb, mrb_value self)
{
  char *val;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &val);
  node->SetValue(val, false);
  return mrb_nil_value();
}

static mrb_value
xml_node_parent(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLNode *ptr = node->Parent();
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_no_children(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  return mrb_bool_value(node->NoChildren());
}

static mrb_value
xml_node_first_child(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLNode *ptr = node->FirstChild();
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_first_child_element(mrb_state *mrb, mrb_value self)
{
  char *name = NULL;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "|z", &name);
  XMLElement *ptr = node->FirstChildElement(name);
  if (ptr) {
    struct RData *element = mrb_data_object_alloc(mrb, xml_element_class,
						  ptr, &xml_element_type);
    return mrb_obj_value(element);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_last_child(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLNode *ptr = node->LastChild();
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_last_child_element(mrb_state *mrb, mrb_value self)
{
  char *name = NULL;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "|z", &name);
  XMLElement *ptr = node->LastChildElement(name);
  if (ptr) {
    struct RData *element = mrb_data_object_alloc(mrb, xml_element_class,
						  ptr, &xml_element_type);
    return mrb_obj_value(element);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_previous_sibling(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLNode *ptr = node->PreviousSibling();
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_previous_sibling_element(mrb_state *mrb, mrb_value self)
{
  char *name = NULL;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "|z", &name);
  XMLElement *ptr = node->PreviousSiblingElement(name);
  if (ptr) {
    struct RData *element = mrb_data_object_alloc(mrb, xml_element_class,
						  ptr, &xml_element_type);
    return mrb_obj_value(element);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_next_sibling(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLNode *ptr = node->NextSibling();
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_next_sibling_element(mrb_state *mrb, mrb_value self)
{
  char *name = NULL;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "|z", &name);
  XMLElement *ptr = node->NextSiblingElement(name);
  if (ptr) {
    struct RData *element = mrb_data_object_alloc(mrb, xml_element_class,
						  ptr, &xml_element_type);
    return mrb_obj_value(element);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_node_insert_end_child(mrb_state *mrb, mrb_value self)
{
  mrb_value child;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "o", &child);
  if (!mrb_obj_is_kind_of(mrb, child, xml_node_class)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "expect XMLNode");
  }

  XMLNode *ptr = node->InsertEndChild(static_cast<XMLNode*>(DATA_PTR(child)));
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_link_end_child(mrb_state *mrb, mrb_value self)
{
  mrb_value child;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "o", &child);
  if (!mrb_obj_is_kind_of(mrb, child, xml_node_class)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "expect XMLNode");
  }

  XMLNode *ptr = node->LinkEndChild(static_cast<XMLNode*>(DATA_PTR(child)));
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_insert_first_child(mrb_state *mrb, mrb_value self)
{
  mrb_value child;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "o", &child);
  if (!mrb_obj_is_kind_of(mrb, child, xml_node_class)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "expect XMLNode");
  }

  XMLNode *ptr = node->InsertFirstChild(static_cast<XMLNode*>(DATA_PTR(child)));
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_insert_after_child(mrb_state *mrb, mrb_value self)
{
  mrb_value prev_child;
  mrb_value child;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "oo", &prev_child, &child);
  if (!mrb_obj_is_kind_of(mrb, child, xml_node_class) &&
      !mrb_obj_is_kind_of(mrb, prev_child, xml_node_class)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "expect XMLNode");
  }

  XMLNode *ptr = node->InsertAfterChild(static_cast<XMLNode*>(DATA_PTR(prev_child)),
					static_cast<XMLNode*>(DATA_PTR(child)));
  return xml_node_alloc(mrb, ptr);
}

static mrb_value
xml_node_delete_children(mrb_state *mrb, mrb_value self)
{
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  node->DeleteChildren();
  return mrb_nil_value();
}

static mrb_value
xml_node_delete_child(mrb_state *mrb, mrb_value self)
{
  mrb_value child;
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  mrb_get_args(mrb, "o", &child);
  if (!mrb_obj_is_kind_of(mrb, child, xml_node_class)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "expect XMLNode");
  }

  node->DeleteChild(static_cast<XMLNode*>(DATA_PTR(child)));
  return mrb_nil_value();
}

static mrb_value
xml_node_print(mrb_state *mrb, mrb_value self)
{
  mrb_bool compact = false;
  mrb_get_args(mrb, "|b", &compact);
  XMLNode *node = static_cast<XMLNode*>(DATA_PTR(self));
  XMLPrinter printer(NULL, static_cast<bool>(compact));
  node->Accept(&printer);
  return mrb_str_new_cstr(mrb, printer.CStr());
}

/* XMLDocument */
static mrb_value
xml_document_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_document_type;
  DATA_PTR(self) = NULL;

  XMLDocument *doc = new XMLDocument();
  DATA_PTR(self) = doc;

  return self;
}

static mrb_value
xml_document_to_document(mrb_state *mrb, mrb_value self)
{
  return self;
}

static mrb_value
xml_document_parse(mrb_state *mrb, mrb_value self)
{
  char *xml;
  int len = 0;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "s", &xml, &len);
  XMLError error = doc->Parse(xml, static_cast<size_t>(len));
  return mrb_symbol_value(xml_error_to_symbol(mrb, error));
}

static mrb_value
xml_document_load_file(mrb_state *mrb, mrb_value self)
{
  char *filename;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &filename);
  XMLError error = doc->LoadFile(filename);
  return mrb_symbol_value(xml_error_to_symbol(mrb, error));
}

static mrb_value
xml_document_save_file(mrb_state *mrb, mrb_value self)
{
  char *filename;
  mrb_bool compact = false;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "z|b", &filename, &compact);
  XMLError error = doc->SaveFile(filename, static_cast<bool>(compact));
  return mrb_symbol_value(xml_error_to_symbol(mrb, error));
}

static mrb_value
xml_document_process_entities(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  return mrb_bool_value(doc->ProcessEntities());
}

static mrb_value
xml_document_whitespace_mode(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_sym whitespace = xml_whitespace_to_symbol(mrb, doc->WhitespaceMode());
  return mrb_symbol_value(whitespace);
}

static mrb_value
xml_document_has_bom(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  return mrb_bool_value(doc->HasBOM());
}

static mrb_value
xml_document_set_bom(mrb_state *mrb, mrb_value self)
{
  mrb_bool use_bom;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "b", &use_bom);
  doc->SetBOM(static_cast<bool>(use_bom));
  return mrb_nil_value();
}

static mrb_value
xml_document_root_element(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  struct RData *element = mrb_data_object_alloc(mrb, xml_element_class,
						doc->RootElement(), &xml_element_type);
  return mrb_obj_value(element);
}

static mrb_value
xml_document_new_element(mrb_state *mrb, mrb_value self)
{
  char *name;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  struct RData *element = mrb_data_object_alloc(mrb, xml_element_class,
						doc->NewElement(name), &xml_element_type);
  return mrb_obj_value(element);
}

static mrb_value
xml_document_new_comment(mrb_state *mrb, mrb_value self)
{
  char *str;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &str);
  struct RData *comment = mrb_data_object_alloc(mrb, xml_comment_class,
						doc->NewComment(str), &xml_comment_type);
  return mrb_obj_value(comment);
}

static mrb_value
xml_document_new_text(mrb_state *mrb, mrb_value self)
{
  char *str;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &str);
  struct RData *text = mrb_data_object_alloc(mrb, xml_text_class,
						doc->NewText(str), &xml_text_type);
  return mrb_obj_value(text);
}

static mrb_value
xml_document_new_declaration(mrb_state *mrb, mrb_value self)
{
  char *str = NULL;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "|z", &str);
  struct RData *declaration = mrb_data_object_alloc(mrb, xml_declaration_class,
						doc->NewDeclaration(str), &xml_declaration_type);
  return mrb_obj_value(declaration);
}

static mrb_value
xml_document_new_unknown(mrb_state *mrb, mrb_value self)
{
  char *str;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &str);
  struct RData *unknown = mrb_data_object_alloc(mrb, xml_unknown_class,
						doc->NewUnknown(str), &xml_unknown_type);
  return mrb_obj_value(unknown);
}

static mrb_value
xml_document_delete_node(mrb_state *mrb, mrb_value self)
{
  mrb_value node;
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  mrb_get_args(mrb, "o", &node);
  if (!mrb_obj_is_kind_of(mrb, node, xml_node_class)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "expect XMLNode");
  }
  doc->DeleteNode(static_cast<XMLNode*>(DATA_PTR(node)));
  return mrb_nil_value();
}

static mrb_value
xml_document_error(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  return mrb_bool_value(doc->Error());
}

static mrb_value
xml_document_error_id(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  XMLError error = doc->ErrorID();
  return mrb_symbol_value(xml_error_to_symbol(mrb, error));
}

static mrb_value
xml_document_get_error_str1(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  return mrb_str_new_cstr(mrb, doc->GetErrorStr1());
}

static mrb_value
xml_document_get_error_str2(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  return mrb_str_new_cstr(mrb, doc->GetErrorStr2());
}

static mrb_value
xml_document_print_error(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  doc->PrintError();
  return mrb_nil_value();
}

static mrb_value
xml_document_clear(mrb_state *mrb, mrb_value self)
{
  XMLDocument *doc = static_cast<XMLDocument*>(DATA_PTR(self));
  doc->Clear();
  return mrb_nil_value();
}

/* XMLElement */
static mrb_value
xml_element_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_element_type;
  DATA_PTR(self) = NULL;
  mrb_raise(mrb, E_RUNTIME_ERROR, "constructor is private");
  return self;
}

static mrb_value
xml_element_name(mrb_state *mrb, mrb_value self)
{
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  return mrb_str_new_cstr(mrb, element->Name());
}

static mrb_value
xml_element_set_name(mrb_state *mrb, mrb_value self)
{
  char *val;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &val);
  element->SetName(val, false);
  return mrb_nil_value();
}

static mrb_value
xml_element_to_element(mrb_state *mrb, mrb_value self)
{
  return self;
}

static mrb_value
xml_element_attribute(mrb_state *mrb, mrb_value self)
{
  char *name = NULL;
  char *value = NULL;
  const char *attr;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z|z", &name, &value);
  attr = element->Attribute(name, value);
  if (attr) {
    return mrb_str_new_cstr(mrb, attr);
  }
  else {
    return mrb_nil_value();
  }
}

static mrb_value
xml_element_int_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  int value;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  XMLError error = element->QueryIntAttribute(name, &value);
  if (error == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  else if (error == XML_NO_ATTRIBUTE) {
    return mrb_nil_value();
  }
  return mrb_fixnum_value(value);
}

static mrb_value
xml_element_unsigned_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  unsigned int value;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  XMLError error = element->QueryUnsignedAttribute(name, &value);
  if (error == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  else if (error == XML_NO_ATTRIBUTE) {
    return mrb_nil_value();
  }
  return mrb_fixnum_value(value);
}

static mrb_value
xml_element_bool_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  bool value;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  XMLError error = element->QueryBoolAttribute(name, &value);
  if (error == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  else if (error == XML_NO_ATTRIBUTE) {
    return mrb_nil_value();
  }
  return mrb_bool_value(value);
}

static mrb_value
xml_element_double_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  double value;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  XMLError error = element->QueryDoubleAttribute(name, &value);
  if (error == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  else if (error == XML_NO_ATTRIBUTE) {
    return mrb_nil_value();
  }
  return mrb_float_value(mrb, value);
}

static mrb_value
xml_element_float_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  float value;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  XMLError error = element->QueryFloatAttribute(name, &value);
  if (error == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  else if (error == XML_NO_ATTRIBUTE) {
    return mrb_nil_value();
  }
  return mrb_float_value(mrb, value);
}

static mrb_value
xml_element_set_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  mrb_value value;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "zo", &name, &value);

  switch (mrb_type(value)) {
  case MRB_TT_FIXNUM:
    element->SetAttribute(name, static_cast<int>(mrb_fixnum(value)));
    break;

  case MRB_TT_FLOAT:
    element->SetAttribute(name, static_cast<float>(mrb_float(value)));
    break;

  case MRB_TT_TRUE:
  case MRB_TT_FALSE:
    /* FALLTHROUGH */
    element->SetAttribute(name, static_cast<bool>(mrb_bool(value)));
    break;

  case MRB_TT_STRING:
    element->SetAttribute(name, static_cast<const char*>(mrb_string_value_cstr(mrb, &value)));
    break;

  default:
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid type");
  }

  return mrb_nil_value();
}

static mrb_value
xml_element_delete_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  element->DeleteAttribute(name);
  return mrb_nil_value();
}

static mrb_value
xml_element_first_attribute(mrb_state *mrb, mrb_value self)
{
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  return xml_const_attribute_alloc(mrb, element->FirstAttribute());
}

static mrb_value
xml_element_find_attribute(mrb_state *mrb, mrb_value self)
{
  char *name;
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  mrb_get_args(mrb, "z", &name);
  const XMLAttribute* attr = static_cast<const XMLElement*>(element)->FindAttribute(name);
  return xml_const_attribute_alloc(mrb, attr);
}

static mrb_value
xml_element_get_text(mrb_state *mrb, mrb_value self)
{
  XMLElement *element = static_cast<XMLElement*>(DATA_PTR(self));
  return mrb_str_new_cstr(mrb, element->GetText());
}

/* XMLAttribute */
static mrb_value
xml_attribute_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_attribute_type;
  DATA_PTR(self) = NULL;
  mrb_raise(mrb, E_RUNTIME_ERROR, "constructor is private");
  return self;
}

static mrb_value
xml_attribute_name(mrb_state *mrb, mrb_value self)
{
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  return mrb_str_new_cstr(mrb, attribute->Name());
}

static mrb_value
xml_attribute_value(mrb_state *mrb, mrb_value self)
{
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  return mrb_str_new_cstr(mrb, attribute->Value());
}

static mrb_value
xml_attribute_next(mrb_state *mrb, mrb_value self)
{
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  return xml_const_attribute_alloc(mrb, attribute->Next());
}

static mrb_value
xml_attribute_int_value(mrb_state *mrb, mrb_value self)
{
  int value;
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  if (attribute->QueryIntValue(&value) == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  return mrb_fixnum_value(value);
}

static mrb_value
xml_attribute_unsigned_value(mrb_state *mrb, mrb_value self)
{
  unsigned int value;
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  if (attribute->QueryUnsignedValue(&value) == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  return mrb_fixnum_value(value);
}

static mrb_value
xml_attribute_bool_value(mrb_state *mrb, mrb_value self)
{
  bool value;
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  if (attribute->QueryBoolValue(&value) == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  return mrb_bool_value(value);
}

static mrb_value
xml_attribute_double_value(mrb_state *mrb, mrb_value self)
{
  double value;
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  if (attribute->QueryDoubleValue(&value) == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  return mrb_float_value(mrb, value);
}

static mrb_value
xml_attribute_float_value(mrb_state *mrb, mrb_value self)
{
  float value;
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  if (attribute->QueryFloatValue(&value) == XML_WRONG_ATTRIBUTE_TYPE) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "wrong attribute type");
  }
  return mrb_float_value(mrb, value);
}

static mrb_value
xml_attribute_set_attribute(mrb_state *mrb, mrb_value self)
{
  mrb_value value;
  XMLAttribute *attribute = static_cast<XMLAttribute*>(DATA_PTR(self));
  mrb_get_args(mrb, "o", &value);

  switch (mrb_type(value)) {
  case MRB_TT_FIXNUM:
    attribute->SetAttribute(static_cast<int>(mrb_fixnum(value)));
    break;

  case MRB_TT_FLOAT:
    attribute->SetAttribute(static_cast<float>(mrb_float(value)));
    break;

  case MRB_TT_TRUE:
  case MRB_TT_FALSE:
    /* FALLTHROUGH */
    attribute->SetAttribute(static_cast<bool>(mrb_bool(value)));
    break;

  case MRB_TT_STRING:
    attribute->SetAttribute(static_cast<const char*>(mrb_string_value_cstr(mrb, &value)));
    break;

  default:
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid type");
  }

  return mrb_nil_value();
}

/* XMLComment */
static mrb_value
xml_comment_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_comment_type;
  DATA_PTR(self) = NULL;
  mrb_raise(mrb, E_RUNTIME_ERROR, "constructor is private");
  return self;
}

static mrb_value
xml_comment_to_comment(mrb_state *mrb, mrb_value self)
{
  return self;
}

/* XMLText */
static mrb_value
xml_text_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_text_type;
  DATA_PTR(self) = NULL;
  mrb_raise(mrb, E_RUNTIME_ERROR, "constructor is private");
  return self;
}

static mrb_value
xml_text_to_text(mrb_state *mrb, mrb_value self)
{
  return self;
}

static mrb_value
xml_text_set_cdata(mrb_state *mrb, mrb_value self)
{
  mrb_bool is_cdata;
  XMLText *text = static_cast<XMLText*>(DATA_PTR(self));
  mrb_get_args(mrb, "b", &is_cdata);
  text->SetCData(static_cast<bool>(is_cdata));
  return mrb_nil_value();
}

static mrb_value
xml_text_cdata(mrb_state *mrb, mrb_value self)
{
  XMLText *text = static_cast<XMLText*>(DATA_PTR(self));
  return mrb_bool_value(text->CData());
}

/* XMLDeclaration */
static mrb_value
xml_declaration_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_declaration_type;
  DATA_PTR(self) = NULL;
  mrb_raise(mrb, E_RUNTIME_ERROR, "constructor is private");
  return self;
}

static mrb_value
xml_declaration_to_declaration(mrb_state *mrb, mrb_value self)
{
  return self;
}

/* XMLUnknown */
static mrb_value
xml_unknown_initialize(mrb_state *mrb, mrb_value self)
{
  DATA_TYPE(self) = &xml_unknown_type;
  DATA_PTR(self) = NULL;
  mrb_raise(mrb, E_RUNTIME_ERROR, "constructor is private");
  return self;
}

static mrb_value
xml_unknown_to_unknown(mrb_state *mrb, mrb_value self)
{
  return self;
}

extern "C" void
mrb_mruby_tinyxml2_gem_init(mrb_state* mrb)
{
  tinyxml2_module             = mrb_define_module(mrb, "TinyXML2");
  xml_node_class              = mrb_define_class_under(mrb, tinyxml2_module, "XMLNode",           mrb->object_class);
  xml_document_class          = mrb_define_class_under(mrb, tinyxml2_module, "XMLDocument",       xml_node_class);
  xml_element_class           = mrb_define_class_under(mrb, tinyxml2_module, "XMLElement",        xml_node_class);
  xml_const_attribute_class   = mrb_define_class_under(mrb, tinyxml2_module, "XMLConstAttribute", mrb->object_class);
  xml_attribute_class         = mrb_define_class_under(mrb, tinyxml2_module, "XMLAttribute",      xml_const_attribute_class);
  xml_comment_class           = mrb_define_class_under(mrb, tinyxml2_module, "XMLComment",        xml_node_class);
  xml_text_class              = mrb_define_class_under(mrb, tinyxml2_module, "XMLText",           xml_node_class);
  xml_declaration_class       = mrb_define_class_under(mrb, tinyxml2_module, "XMLDeclaration",    xml_node_class);
  xml_unknown_class           = mrb_define_class_under(mrb, tinyxml2_module, "XMLUnknown",        xml_node_class);

  MRB_SET_INSTANCE_TT(xml_node_class,              MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_document_class,          MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_element_class,           MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_attribute_class,         MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_comment_class,           MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_text_class,              MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_declaration_class,       MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_unknown_class,           MRB_TT_DATA);
  MRB_SET_INSTANCE_TT(xml_const_attribute_class,   MRB_TT_DATA);

  /* XMLNode */
  mrb_define_method(mrb, xml_node_class,           "initialize",               xml_node_initialize,               ARGS_NONE());
  /* TODO: get_document */
  mrb_define_method(mrb, xml_node_class,           "to_element",               xml_node_to_element,               ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "to_text",                  xml_node_to_text,                  ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "to_comment",               xml_node_to_comment,               ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "to_document",              xml_node_to_document,              ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "to_declaration",           xml_node_to_declaration,           ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "to_unknown",               xml_node_to_unknown,               ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "value",                    xml_node_value,                    ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "set_value",                xml_node_set_value,                ARGS_REQ(1));
  mrb_define_method(mrb, xml_node_class,           "parent",                   xml_node_parent,                   ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "no_children",              xml_node_no_children,              ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "first_child",              xml_node_first_child,              ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "first_child_element",      xml_node_first_child_element,      ARGS_OPT(1));
  mrb_define_method(mrb, xml_node_class,           "last_child",               xml_node_last_child,               ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "last_child_element",       xml_node_last_child_element,       ARGS_OPT(1));
  mrb_define_method(mrb, xml_node_class,           "previous_sibling",         xml_node_previous_sibling,         ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "previous_sibling_element", xml_node_previous_sibling_element, ARGS_OPT(1));
  mrb_define_method(mrb, xml_node_class,           "next_sibling",             xml_node_next_sibling,             ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "next_sibling_element",     xml_node_next_sibling_element,     ARGS_OPT(1));
  mrb_define_method(mrb, xml_node_class,           "insert_end_child",         xml_node_insert_end_child,         ARGS_REQ(1));
  mrb_define_method(mrb, xml_node_class,           "link_end_child",           xml_node_link_end_child,           ARGS_REQ(1));
  mrb_define_method(mrb, xml_node_class,           "link_first_child",         xml_node_insert_first_child,       ARGS_REQ(1));
  mrb_define_method(mrb, xml_node_class,           "link_after_child",         xml_node_insert_after_child,       ARGS_REQ(2));
  mrb_define_method(mrb, xml_node_class,           "delete_children",          xml_node_delete_children,          ARGS_NONE());
  mrb_define_method(mrb, xml_node_class,           "delete_child",             xml_node_delete_child,             ARGS_REQ(1));
  mrb_define_method(mrb, xml_node_class,           "print",                    xml_node_print,                    ARGS_OPT(1));

  /* XMLDocument */
  mrb_define_method(mrb, xml_document_class,       "initialize",               xml_document_initialize,           ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "to_document",              xml_document_to_document,          ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "parse",                    xml_document_parse,                ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "load_file",                xml_document_load_file,            ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "save_file",                xml_document_save_file,            ARGS_REQ(1)|ARGS_OPT(1));
  mrb_define_method(mrb, xml_document_class,       "process_entities",         xml_document_process_entities,     ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "whitespace_mode",          xml_document_whitespace_mode,      ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "has_bom",                  xml_document_has_bom,              ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "set_bom",                  xml_document_set_bom,              ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "root_element",             xml_document_root_element,         ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "new_element",              xml_document_new_element,          ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "new_comment",              xml_document_new_comment,          ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "new_text",                 xml_document_new_text,             ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "new_declaration",          xml_document_new_declaration,      ARGS_OPT(1));
  mrb_define_method(mrb, xml_document_class,       "new_unknown",              xml_document_new_unknown,          ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "delete_node",              xml_document_delete_node,          ARGS_REQ(1));
  mrb_define_method(mrb, xml_document_class,       "error",                    xml_document_error,                ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "error_id",                 xml_document_error_id,             ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "get_error_str1",           xml_document_get_error_str1,       ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "get_error_str2",           xml_document_get_error_str2,       ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "print_error",              xml_document_print_error,          ARGS_NONE());
  mrb_define_method(mrb, xml_document_class,       "clear",                    xml_document_clear,                ARGS_NONE());

  /* XMLElement */
  mrb_define_method(mrb, xml_element_class,        "initialize",               xml_element_initialize,            ARGS_NONE());
  mrb_define_method(mrb, xml_element_class,        "name",                     xml_element_name,                  ARGS_NONE());
  mrb_define_method(mrb, xml_element_class,        "set_name",                 xml_element_set_name,              ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "to_element",               xml_element_to_element,            ARGS_NONE());
  mrb_define_method(mrb, xml_element_class,        "attribute",                xml_element_attribute,             ARGS_REQ(1)|ARGS_OPT(1));
  mrb_define_method(mrb, xml_element_class,        "int_attribute",            xml_element_int_attribute,         ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "unsigned_attribute",       xml_element_unsigned_attribute,    ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "bool_attribute",           xml_element_bool_attribute,        ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "double_attribute",         xml_element_double_attribute,      ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "float_attribute",          xml_element_float_attribute,       ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "set_attribute",            xml_element_set_attribute,         ARGS_REQ(2));
  mrb_define_method(mrb, xml_element_class,        "delete_attribute",         xml_element_delete_attribute,      ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "first_attribute",          xml_element_first_attribute,       ARGS_NONE());
  mrb_define_method(mrb, xml_element_class,        "find_attribute",           xml_element_find_attribute,        ARGS_REQ(1));
  mrb_define_method(mrb, xml_element_class,        "get_text",                 xml_element_get_text,              ARGS_NONE());

  /* XMLAttribute */
  mrb_define_method(mrb, xml_const_attribute_class, "initialize",              xml_attribute_initialize,          ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "name",                    xml_attribute_name,                ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "value",                   xml_attribute_value,               ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "next",                    xml_attribute_next,                ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "int_value",               xml_attribute_int_value,           ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "unsigned_value",          xml_attribute_unsigned_value,      ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "bool_value",              xml_attribute_bool_value,          ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "double_value",            xml_attribute_double_value,        ARGS_NONE());
  mrb_define_method(mrb, xml_const_attribute_class, "float_value",             xml_attribute_float_value,         ARGS_NONE());
  mrb_define_method(mrb, xml_attribute_class,       "set_attribute",           xml_attribute_set_attribute,       ARGS_REQ(1));

  /* XMLComment */
  mrb_define_method(mrb, xml_comment_class,         "initialize",              xml_comment_initialize,            ARGS_NONE());
  mrb_define_method(mrb, xml_comment_class,         "to_comment",              xml_comment_to_comment,            ARGS_NONE());

  /* XMLText */
  mrb_define_method(mrb, xml_text_class,            "initialize",              xml_text_initialize,               ARGS_NONE());
  mrb_define_method(mrb, xml_text_class,            "to_text",                 xml_text_to_text,                  ARGS_NONE());
  mrb_define_method(mrb, xml_text_class,            "set_cdata",               xml_text_set_cdata,                ARGS_REQ(1));
  mrb_define_method(mrb, xml_text_class,            "cdata",                   xml_text_cdata,                    ARGS_NONE());

  /* XMLDeclaration */
  mrb_define_method(mrb, xml_declaration_class,     "initialize",              xml_declaration_initialize,        ARGS_NONE());
  mrb_define_method(mrb, xml_declaration_class,     "to_declaration",          xml_declaration_to_declaration,    ARGS_NONE());

  /* XMLUnknown */
  mrb_define_method(mrb, xml_unknown_class,         "initialize",              xml_unknown_initialize,            ARGS_NONE());
  mrb_define_method(mrb, xml_unknown_class,         "to_unknown",              xml_unknown_to_unknown,            ARGS_NONE());
}

extern "C" void
mrb_mruby_tinyxml2_gem_final(mrb_state* mrb)
{
}
