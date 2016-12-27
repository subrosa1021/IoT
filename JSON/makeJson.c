#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main(){
	char encoded[] = "base62 code";

	FILE *fp = fopen("example.json", "w");

	fprintf(fp, "{\n");
	fprintf(fp, "	\"requests\": [{\n");
	fprintf(fp, "		\"image\": {\n");
	fprintf(fp, "			\"content\": \"%s\" },\n", encoded);
	fprintf(fp, "			\"features\": [{\n");
	fprintf(fp, "				\"tpye\": \"LABEL_DETECTION\", \"maxResults\": 1\n");
	fprintf(fp, "			}]\n");
	fprintf(fp, "	}]\n");
	fprintf(fp, "}\n");

	fclose(fp);

	return 0;
}
