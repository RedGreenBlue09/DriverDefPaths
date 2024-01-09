#include <string.h>

#include "GuardedMalloc.h"
#include "XmlNode.h"

xml_node* XmlNodeCreate() {
	xml_node* pNode = malloc_guarded(sizeof(*pNode));
	pNode->pParent = NULL;
	pNode->nChildren = 0;
	pNode->apChildren = NULL;

	pNode->sName = NULL;
	pNode->sContent = NULL;
	pNode->nAttribute = 0;
	pNode->apAttribute = NULL;
	return pNode;
};

void XmlNodeFree(xml_node* pNode) {
	// TODO: Free elements
	free(pNode);
}

void XmlNodeAddChildren(xml_node* pParentNode, xml_node* pChildrenNode) {
	if (!pParentNode->apChildren)
		pParentNode->apChildren = malloc_guarded(sizeof(*pParentNode->apChildren));
	else
		pParentNode->apChildren = realloc_guarded( // Dumb O(n^2). TODO: Dynamic array
			pParentNode->apChildren,
			sizeof(*pParentNode->apChildren) * (pParentNode->nChildren + 1)
		);
	pParentNode->apChildren[pParentNode->nChildren] = pChildrenNode;
	pParentNode->nChildren += 1;
	pChildrenNode->pParent = pParentNode;
};

xml_attribute* XmlNodeFindAttribute(xml_node* pParentNode, const char* sAttributeName) {
	for (size_t i = 0; i < pParentNode->nAttribute; ++i)
		if (strcmp(pParentNode->apAttribute[i]->sName, sAttributeName) == 0)
			return pParentNode->apAttribute[i];
	return NULL;
};

void XmlNodeAddAttribute(xml_node* pParentNode, xml_attribute* pNewAttribute) {
	if (!pParentNode->apAttribute)
		pParentNode->apAttribute = malloc_guarded(sizeof(*pParentNode->apAttribute));
	else
		pParentNode->apAttribute = realloc_guarded( // Dumb O(n^2). TODO: Dynamic array
			pParentNode->apAttribute,
			sizeof(*pParentNode->apAttribute) * (pParentNode->nAttribute + 1)
		);
	pParentNode->apAttribute[pParentNode->nAttribute] = pNewAttribute;
	pParentNode->nAttribute += 1;
};

xml_attribute* XmlAttributeCreate() {
	xml_attribute* pAttribute = malloc_guarded(sizeof(*pAttribute));
	pAttribute->sName = NULL;
	pAttribute->sContent = NULL;
	return pAttribute;
};

void XmlAttributeFree(xml_attribute* pAttribute) {
	free(pAttribute);
};
