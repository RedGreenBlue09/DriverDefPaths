#include <stdint.h>

typedef struct {
	char* sName;
	char* sContent;
} xml_attribute;

typedef struct xml_node_tag {

	struct xml_node_tag* pParent;

	size_t nChildren;
	struct xml_node_tag** apChildren;

	char* sName;
	char* sContent;
	size_t nAttribute;
	xml_attribute** apAttribute;

} xml_node;

xml_node* XmlNodeCreate();
void XmlNodeFree(xml_node* pNode);
void XmlNodeAddChildren(xml_node* pParentNode, xml_node* pChildrenNode);
void XmlNodeAddAttribute(xml_node* pParentNode, xml_attribute* pNewAttribute);
xml_attribute* XmlNodeFindAttribute(xml_node* pParentNode, const char* sAttributeName);

xml_attribute* XmlAttributeCreate();
void XmlAttributeFree(xml_attribute* pAttribute);
