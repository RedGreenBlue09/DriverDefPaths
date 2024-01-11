#include <stdint.h>

typedef struct {
	char* sName; // Always != NULL
	char* sContent; // Can be NULL
} xml_attribute;

typedef struct xml_node_tag {

	struct xml_node_tag* pParent;

	size_t nChildren;
	struct xml_node_tag** apChildren; // NULL if nChildren == 0

	char* sName; // Always != NULL
	char* sContent; // Can be NULL
	size_t nAttribute;
	xml_attribute** apAttribute; // NULL if nAttribute == 0

} xml_node;

xml_node* XmlNodeCreate(const char* sName, const char* sContent);
void XmlNodeFree(xml_node* pNode);

void XmlNodeAddChildren(xml_node* pParentNode, xml_node* pChildrenNode);
xml_node* XmlNodeFindChildren(xml_node* pParentNode, const char* sChildrenName, size_t* iCurrentChildren);

void XmlNodeAddAttribute(xml_node* pParentNode, xml_attribute* pNewAttribute);
xml_attribute* XmlNodeFindAttribute(xml_node* pParentNode, const char* sAttributeName);

xml_attribute* XmlAttributeCreate(const char* sName, const char* sContent);
void XmlAttributeFree(xml_attribute* pAttribute);
