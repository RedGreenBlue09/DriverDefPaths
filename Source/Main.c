#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <expat.h>

#include "GuardedMalloc.h"
#include "XmlNode.h"

#define strlen_literal(X) (sizeof(X) / sizeof(*X) - 1)

static void XMLCALL StartElementCallback(void* pUserData, const XML_Char* sName, const XML_Char** asAttributes) {
	xml_node** ppParentNode = pUserData;
	xml_node* pNewNode = XmlNodeCreate(sName, NULL);
	XmlNodeAddChildren(*ppParentNode, pNewNode);
	*ppParentNode = pNewNode;

	for (int i = 0; asAttributes[i]; i += 2) {
		xml_attribute* pNewAttribute = XmlAttributeCreate(asAttributes[i], asAttributes[i + 1]);
		XmlNodeAddAttribute(pNewNode, pNewAttribute);
	}
}

static void XMLCALL EndElementCallback(void* pUserData, const XML_Char* sName) {
	xml_node** ppCurrentNode = pUserData;
	*ppCurrentNode = (*ppCurrentNode)->pParent;
}

static void XMLCALL CharacterDataCallback(void* pUserData, const XML_Char* sContent, int Length) {
	xml_node* pCurrentNode = *(xml_node**)pUserData;
	pCurrentNode->sContent = malloc_guarded(((size_t)Length + 1) * sizeof(*pCurrentNode->sContent));
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
		char sError[256];
		strerror_s(sError, 256, errno);
		fprintf(stderr, "Unable to open the file \"%s\":\n", argv[1]);
		fprintf(stderr, "%s\n", sError);
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
	
	xml_node* pRootNode = XmlNodeCreate("", NULL);
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

	// Get driver files
	xml_node* pFeatureManifestNode;
	size_t i = 0;
	while (pFeatureManifestNode = XmlNodeFindChildren(pRootNode, "FeatureManifest", &i)) {

		xml_node* pDriversNode;
		size_t ii = 0;
		while (pDriversNode = XmlNodeFindChildren(pFeatureManifestNode, "Drivers", &ii)) {

			xml_node* pBaseDriverPackagesNode;
			size_t iii = 0;
			while (pBaseDriverPackagesNode = XmlNodeFindChildren(pDriversNode, "BaseDriverPackages", &iii)) {

				xml_node* pDriverPackageFileNode;
				size_t iv = 0;
				while (pDriverPackageFileNode = XmlNodeFindChildren(pBaseDriverPackagesNode, "DriverPackageFile", &iv)) {

					xml_attribute* pPathAttribute = XmlNodeFindAttribute(pDriverPackageFileNode, "Path");
					xml_attribute* pNameAttribute = XmlNodeFindAttribute(pDriverPackageFileNode, "Name");
					if (!pPathAttribute || !pNameAttribute)
						continue;

					char* sDriverPath = pPathAttribute->sContent;
					char* sDriverName = pNameAttribute->sContent;
					if (!sDriverPath || !sDriverName)
						continue;

					// Remove "$(mspackageroot)".
					// "$(mspackageroot)" is always at the start of the path
					// because that's the only place it makes sense.
					size_t MprLength = strlen_literal("$(mspackageroot)");
					if (strncmp(sDriverPath, "$(mspackageroot)", MprLength) == 0)
						sDriverPath += MprLength;

					// Remove trailing backslash.
					while (sDriverPath[0] == '\\')
						++sDriverPath;

					printf("%s\\%s\n", sDriverPath, sDriverName);

				}

			}

		}

	}

	XmlNodeFree(pRootNode);
	XML_ParserFree(Parser);
	fclose(DefinitionFile);

	return 0;

}