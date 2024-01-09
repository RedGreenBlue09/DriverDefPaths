#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <expat.h>

#include "GuardedMalloc.h"
#include "XmlNode.h"

static void XMLCALL StartElementCallback(void* pUserData, const XML_Char* sName, const XML_Char** asAttributes) {
	xml_node** ppParentNode = pUserData;
	xml_node* pNewNode = XmlNodeCreate();
	XmlNodeAddChildren(*ppParentNode, pNewNode);
	*ppParentNode = pNewNode;

	size_t NameSize = (strlen(sName) + 1) * sizeof(*sName);
	pNewNode->sName = malloc_guarded(NameSize);
	memcpy(pNewNode->sName, sName, NameSize);

	for (int i = 0; asAttributes[i]; i += 2) {
		size_t AttributeNameSize = (strlen(asAttributes[i]) + 1) * sizeof(*asAttributes);
		size_t AttributeContentSize = (strlen(asAttributes[i + 1]) + 1) * sizeof(*asAttributes);

		xml_attribute* pNewAttribute = XmlAttributeCreate();
		XmlNodeAddAttribute(pNewNode, pNewAttribute);

		pNewAttribute->sName = malloc_guarded(AttributeNameSize);
		memcpy(pNewAttribute->sName, asAttributes[i], AttributeNameSize);

		pNewAttribute->sContent = malloc_guarded(AttributeContentSize);
		memcpy(pNewAttribute->sContent, asAttributes[i + 1], AttributeContentSize);
	}
}

static void XMLCALL EndElementCallback(void* pUserData, const XML_Char* sName) {
	xml_node** ppCurrentNode = pUserData;
	*ppCurrentNode = (*ppCurrentNode)->pParent;
}

static void XMLCALL CharacterDataCallback(void* pUserData, const XML_Char* sContent, int Length) {
	xml_node* pCurrentNode = *(xml_node**)pUserData;
	pCurrentNode->sContent = malloc_guarded((Length + 1) * sizeof(*pCurrentNode->sContent));
	memcpy(pCurrentNode->sContent, sContent, Length * sizeof(*pCurrentNode->sContent));
	pCurrentNode->sContent[Length] = '\0';
}

int main(int argc, char** argv) {

	if (argc < 2) {
		fprintf(stderr, "No definition file specified.\n");
		return 1;
	}

	FILE* DefinitionFile = fopen(argv[1], "rb");
	if (!DefinitionFile) {
		fprintf(stderr, "Unable to open the file \"%s\":\n", argv[1]);
		fprintf(stderr, "%s\n", strerror(errno));
		return 1;
	}
	fseek(DefinitionFile, 0, SEEK_END);
	size_t DefinitionFileSize = ftell(DefinitionFile);
	fseek(DefinitionFile, 0, SEEK_SET);

	XML_Parser Parser = XML_ParserCreate(NULL);
	if (!Parser) {
		fprintf(stderr, "Unable to create parser.\n");
		return 1;
	}
	
	xml_node* pRootNode = XmlNodeCreate();
	XML_SetUserData(Parser, &pRootNode);
	XML_SetElementHandler(Parser, StartElementCallback, EndElementCallback);
	XML_SetCharacterDataHandler(Parser, CharacterDataCallback);

	void* pTextBuffer = XML_GetBuffer(Parser, (int)DefinitionFileSize);
	if (!pTextBuffer) {
		fprintf(stderr, "Unable to allocate memory.\n");
		XML_ParserFree(Parser);
		return 1;
	}

	DefinitionFileSize = fread(pTextBuffer, 1, DefinitionFileSize, DefinitionFile);
	if (ferror(DefinitionFile)) {
		fprintf(stderr, "Unable to read from file.\n");
		XML_ParserFree(Parser);
		return 1;
	}

	if (XML_ParseBuffer(Parser, (int)DefinitionFileSize, 1) == XML_STATUS_ERROR) {
		fprintf(stderr,
			"Parse error at line %iu:\n"
			"%s\n",
			XML_GetCurrentLineNumber(Parser),
			XML_ErrorString(XML_GetErrorCode(Parser)));
		XML_ParserFree(Parser);
		return 1;
	}

	// OMG. Maybe callback function is better?

	for (size_t i = 0; i < pRootNode->nChildren; ++i) {
		if (strcmp(pRootNode->apChildren[i]->sName, "FeatureManifest") == 0) {

			xml_node* FeatureManifestNode = pRootNode->apChildren[i];
			for (size_t j = 0; j < FeatureManifestNode->nChildren; ++j) {
				if (strcmp(FeatureManifestNode->apChildren[j]->sName, "Drivers") == 0) {

					xml_node* DriversNode = FeatureManifestNode->apChildren[j];
					for (size_t k = 0; k < DriversNode->nChildren; ++k) {
						if (strcmp(DriversNode->apChildren[k]->sName, "BaseDriverPackages") == 0) {

							xml_node* BaseDriverPackagesNode = DriversNode->apChildren[k];
							for (size_t l = 0; l < BaseDriverPackagesNode->nChildren; ++l) {
								if (strcmp(BaseDriverPackagesNode->apChildren[l]->sName, "DriverPackageFile") == 0) {

									xml_node* DriverPackageFileNode = BaseDriverPackagesNode->apChildren[l];
									char* DriverPath = XmlNodeFindAttribute(DriverPackageFileNode, "Path")->sContent;
									char* DriverName = XmlNodeFindAttribute(DriverPackageFileNode, "Name")->sContent;
									if (!DriverPath || !DriverName)
										break;
									printf("%s\\%s\n", DriverPath, DriverName);

								}
							}

						}
					}
				}

			}

		}
	}

	XmlNodeFree(pRootNode);
	XML_ParserFree(Parser);
	return 0;
}