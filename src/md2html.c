#define MDP_IMPLEMENTATION
#include "../3rdparty/mdp.h"
#include <stdio.h>

char *read_file(const char *filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) {
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	long filesize = ftell(file);
	rewind(file);
	char *buffer = malloc(filesize + 1);
	if (!buffer) {
		fclose(file);
		return NULL;
	}
	size_t read_size = fread(buffer, 1, filesize, file);
	if (read_size != filesize) {
		free(buffer);
		fclose(file);
		return NULL;
	}
	buffer[filesize] = '\0';
	fclose(file);
	return buffer;
}

#define fiprintf(stream, intd, ...) \
	do { \
		fprintf(f, "%*s", intd, ""); \
		fprintf(f, __VA_ARGS__); \
	} while(0)

void markdown_to_html(FILE *f, MDP_Node *n, int it, int lv) {
	switch (n->kind) {
	case MDP_NODE_DOC: {
		fprintf(f, "<!DOCTYPE html>\n");
		fprintf(f, "<head>\n");
		fprintf(f, "	<link rel=\"stylesheet\" href=\"../style.css\" />\n");
		fprintf(f, "</head>\n");
		fprintf(f, "<body>\n");
		mdp_node_foreach (c, n->body)
			markdown_to_html(f, c, it+lv, lv);
		fprintf(f, "</body>\n");
		fprintf(f, "</html>");
	} break;
	case MDP_NODE_STRONG:
		fprintf(f, "<strong>");
		mdp_node_foreach (c, n->body)
			markdown_to_html(f, c, it+lv, lv);
		fprintf(f, "</strong>");
		break;
	case MDP_NODE_EMPHASIS:
		fprintf(f, "<em>");
		mdp_node_foreach (c, n->body)
			markdown_to_html(f, c, it+lv, lv);
		fprintf(f, "</em>");
		break;
	case MDP_NODE_INLINE_CODE:
		fprintf(f, "<code>%s</code>", n->as.inline_code);
		break;
	case MDP_NODE_PARAGRAPH:
		fiprintf(f, it, "<p>\n");
		fiprintf(f, it+lv, "");
		mdp_node_foreach (c, n->body)
			markdown_to_html(f, c, it+lv, lv);
		fprintf(f,"\n");
		fiprintf(f, it, "</p>\n");
		break;
	case MDP_NODE_UNORD_LIST:
		fiprintf(f, it, "<ul>\n");
		mdp_node_foreach (c, n->body) {
			fiprintf(f, it, "<li>\n");
			markdown_to_html(f, c, it+lv, lv);
			fiprintf(f, it, "</li>\n");
		}
		fiprintf(f, it, "</ul>\n");
		break;
	case MDP_NODE_QUOTE:
		fiprintf(f, it, "<blockquote>\n");
		mdp_node_foreach (c, n->body)
			markdown_to_html(f, c, it+lv, lv);
		fiprintf(f, it, "</blockquote>\n");
		break;
	case MDP_NODE_ORD_LIST:
		fiprintf(f, it, "<ol>\n");
		mdp_node_foreach (c, n->body) {
			fiprintf(f, it, "<li>\n");
			markdown_to_html(f, c, it+lv, lv);
			fiprintf(f, it, "</li>\n");
		}
		fiprintf(f, it, "</ol>\n");
		break;
	case MDP_NODE_IMAGE:
		fprintf(f,
			"<img src=\"%s\" title=\"%s\" style=\"width:100%;\"></img>",
			n->as.link.link,
			n->as.link.desc);
		break;
	case MDP_NODE_LINK:
		fprintf(f,
			"<a href=\"%s\" title=\"%s\">",
			n->as.link.link,
			n->as.link.desc);
		mdp_node_foreach (c, n->body)
			markdown_to_html(f, c, it + lv, lv);
		fprintf(f, "</a>");
		break;
	case MDP_NODE_BLOCK_CODE:
		const char *style =
			"background-color: #181818;"
			"border-radius: 8px;"
			"padding: 8px;";
		fiprintf(f, it, "<div style=\"%s\"><pre style=\"margin: 0;\"><code>%s",
			style, n->as.block_code.code);
		fprintf(f, "</code></pre></div>\n");
		break;
	case MDP_NODE_ORD_LIST_ITEM:
		markdown_to_html(f, n->body, it, lv);
		break;
	case MDP_NODE_HEADING:
		fiprintf(f, it, "<h%u>\n", n->as.heading.level);
		fiprintf(f, it+lv, "");
		mdp_node_foreach (c, n->as.heading.title)
			markdown_to_html(f, c, it+lv, lv);
		fprintf(f,"\n");
		fiprintf(f, it, "</h%u>\n", n->as.heading.level);
		mdp_node_foreach (c, n->body)
			markdown_to_html(f, c, it, lv);
		break;
	case MDP_NODE_NL:
		fprintf(f, "<br/>");
		break;
	case MDP_NODE_TEXT:
		fprintf(f, "%s", n->as.text);
	}
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Not enought arguments\n");
		return 1;
	}

	char *input = read_file(argv[1]);
	FILE *fp = fopen(argv[2], "w");
    if (fp == NULL || input == NULL) {
        printf("Error opening file\n");
        return 1;
    }

	MDP_Token *toks = mdp_lex(input);
	MDP_Node *doc = mdp_parse(toks);
	markdown_to_html(fp, doc, 0, 4);
	fclose(fp);
	return 0;
}
