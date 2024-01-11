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

	// TODO: XmlNodeFindChildren

	for (size_t i = 0; i < pRootNode->nChildren; ++i) {

		if (strcmp(pRootNode->apChildren[i]->sName, "FeatureManifest") == 0) {

			xml_node* pFeatureManifestNode = pRootNode->apChildren[i];
			for (size_t ii = 0; ii < pFeatureManifestNode->nChildren; ++ii) {

				if (strcmp(pFeatureManifestNode->apChildren[ii]->sName, "Drivers") == 0) {

					xml_node* pDriversNode = pFeatureManifestNode->apChildren[ii];
					for (size_t iii = 0; iii < pDriversNode->nChildren; ++iii) {

						if (strcmp(pDriversNode->apChildren[iii]->sName, "BaseDriverPackages") == 0) {

							xml_node* pBaseDriverPackagesNode = pDriversNode->apChildren[iii];
							for (size_t iv = 0; iv < pBaseDriverPackagesNode->nChildren; ++iv) {

								if (strcmp(pBaseDriverPackagesNode->apChildren[iv]->sName, "DriverPackageFile") == 0) {

									xml_node* DriverPackageFileNode = pBaseDriverPackagesNode->apChildren[iv];
									xml_attribute* pPathAttribute = XmlNodeFindAttribute(DriverPackageFileNode, "Path");
									xml_attribute* pNameAttribute = XmlNodeFindAttribute(DriverPackageFileNode, "Name");
									if (!pPathAttribute || !pNameAttribute)
										break;

									char* sDriverPath = pPathAttribute->sContent;
									char* sDriverName = pNameAttribute->sContent;
									if (!sDriverPath || !sDriverName)
										break;

									// Remove "$(mspackageroot)"
									size_t MprLength = strlen_literal("$(mspackageroot)");
									if (
										(strlen(sDriverPath) >= MprLength) &&
										(strncmp(sDriverPath, "$(mspackageroot)", MprLength) == 0)
									)
										// "$(mspackageroot)" is always at the start of the path
										// because that's the only place it makes sense.
										sDriverPath += MprLength;

									// Remove trailing backslash.
									while (sDriverPath[0] == '\\')
										++sDriverPath;

									printf("%s\\%s\n", sDriverPath, sDriverName);

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
	fclose(DefinitionFile);

	return 0;

}