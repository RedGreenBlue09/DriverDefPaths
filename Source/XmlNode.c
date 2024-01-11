#include <assert.h>
#include <string.h>

#include "GuardedMalloc.h"
#include "XmlNode.h"

xml_node* XmlNodeCreate(const char* sName, const char* sContent) {
	assert(sName != NULL);

	xml_node* pNode = malloc_guarded(sizeof(*pNode));

	pNode->pParent = NULL;
	pNode->nChildren = 0;
	pNode->apChildren = NULL;

	// Copy name
	size_t NameSize = (strlen(sName) + 1) * sizeof(*sName);
	pNode->sName = malloc_guarded(NameSize);
	memcpy(pNode->sName, sName, NameSize);

	// Copy content
	if (sContent) {
		size_t ContentSize = (strlen(sContent) + 1) * sizeof(*sContent);
		pNode->sContent = malloc_guarded(ContentSize);
		memcpy(pNode->sContent, sContent, ContentSize);
	} else
		pNode->sContent = NULL;

	pNode->nAttribute = 0;
	pNode->apAttribute = NULL;
	return pNode;
};

void XmlNodeFree(xml_node* pNode) {
	assert(pNode != NULL);

	for (size_t i = 0; i < pNode->nChildren; ++i)
		XmlNodeFree(pNode->apChildren[i]);
	if (pNode->apChildren)
		free(pNode->apChildren);

	free(pNode->sName);

	if (pNode->sContent)
		free(pNode->sContent);

	for (size_t i = 0; i < pNode->nAttribute; ++i)
		XmlAttributeFree(pNode->apAttribute[i]);
	if(pNode->apAttribute)
		free(pNode->apAttribute);

	free(pNode);
}

/*
void XmlNodeSetContent(xml_node* pNode, const char* sContent) {
	assert(pNode != NULL);

	size_t ContentSize = (strlen(sContent) + 1) * sizeof(*sContent);
	pNode->sContent = realloc_guarded(pNode->sContent, ContentSize);
	memcpy(pNode->sContent, sContent, ContentSize);
}
*/

void XmlNodeAddChildren(xml_node* pParentNode, xml_node* pChildrenNode) {
	assert(pParentNode != NULL);
	assert(pChildrenNode != NULL);

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

xml_node* XmlNodeFindChildren(xml_node* pParentNode, const char* sChildrenName, size_t* iCurrentChildren) {
	assert(pParentNode != NULL);
	assert(sChildrenName != NULL);

	size_t i;
	for (i = iCurrentChildren ? (*iCurrentChildren) : 0; i < pParentNode->nChildren; ++i)
		if (strcmp(pParentNode->apChildren[i]->sName, sChildrenName) == 0) {
			if (iCurrentChildren)
				*iCurrentChildren = i + 1;
			return pParentNode->apChildren[i];
		}

	if (iCurrentChildren)
		*iCurrentChildren = i;

	return NULL;
};

xml_attribute* XmlNodeFindAttribute(xml_node* pParentNode, const char* sAttributeName) {
	assert(pParentNode != NULL);
	assert(sAttributeName != NULL);

	for (size_t i = 0; i < pParentNode->nAttribute; ++i)
		if (strcmp(pParentNode->apAttribute[i]->sName, sAttributeName) == 0)
			return pParentNode->apAttribute[i];
	return NULL;
};

void XmlNodeAddAttribute(xml_node* pParentNode, xml_attribute* pNewAttribute) {
	assert(pParentNode != NULL);
	assert(pNewAttribute != NULL);

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

xml_attribute* XmlAttributeCreate(const char* sName, const char* sContent) {
	assert(sName != NULL);

	xml_attribute* pAttribute = malloc_guarded(sizeof(*pAttribute));

	// Copy name
	size_t NameSize = (strlen(sName) + 1) * sizeof(*sName);
	pAttribute->sName = malloc_guarded(NameSize);
	memcpy(pAttribute->sName, sName, NameSize);

	// Copy content
	if (sContent) {
		size_t ContentSize = (strlen(sContent) + 1) * sizeof(*sContent);
		pAttribute->sContent = malloc_guarded(ContentSize);
		memcpy(pAttribute->sContent, sContent, ContentSize);
	} else
		pAttribute->sContent = NULL;

	return pAttribute;
};

void XmlAttributeFree(xml_attribute* pAttribute) {
	free(pAttribute->sName);
	if (pAttribute->sContent)
		free(pAttribute->sContent);
	free(pAttribute);
};
