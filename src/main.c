#include <stdio.h>
#include <dirent.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HTMPL_IMPLEMENTATION
#include "../3rdparty/htmpl.h"

typedef struct {
	char *title;
	char *link;
} Post;

#include INDEX_TEMPLATE

char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

int extract_id(char *str) {
	int res = 0;
	int pwr = 1000;
	for (int i = 0; i < 4; i++) {
		if (!(str[i] >= '0' && str[i] <= '9'))
			return -1;
		res += pwr * (str[i] - '0');
		pwr /= 10;
	}
	return res;
}

int main(void) {
	Post posts[9999];
	size_t count = 0;

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir("./posts")) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			char *file = ent->d_name;
			if (file[0] >= '0' && file[0] <= '9') {
				char md[1024];
				char html[1024];
				char link[1024];
				char cmd[1024];

				sprintf(md, "./posts/%s", file);
				sprintf(html, "./pages/%s.html", file);
				sprintf(cmd,
					 "./build/md2html '%s' '%s'",
					 md, html);

				posts[count++] = (Post){
					.title = xstrdup(file),
					.link = xstrdup(html),
				};

				printf("CMD: %s\n", cmd);
				if (system(cmd) != 0) {
					fprintf(stderr, "Error generating html\n");
					return 1;
				}
			}
		}
		closedir(dir);
	} else {
		fprintf(stderr, "Error opening directory\n");
		return 1;
	}

	while (true) {
		bool sorted = true;
		for (size_t i = 0; i < count - 1; i++) {
			int cid = extract_id(posts[i].title);
			int nid = extract_id(posts[i+1].title);
			if (cid < nid) {
				Post tmp = posts[i];
				posts[i] = posts[i+1];
				posts[i+1] = tmp;
				sorted = false;
			}
		}
		if (sorted) {
			break;
		}
	}

	FILE *idx = fopen("./index.html", "w");
	fprintf(idx, "%s", index_tmpl(posts, count));
	fclose(idx);
	return 0;
}
