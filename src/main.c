#include <stdio.h>
#include <dirent.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HTMPL_IMPLEMENTATION
#include "../3rdparty/htmpl.h"

typedef struct {
	int id;
	char *title;
	char *link;
} Post;

char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

/* ID is a 4 digits long integer */
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

/* index_tmpl(Post *posts, size_t count) */
#include INDEX_TEMPLATE

int main(void) {
	Post posts[9999];
	size_t count = 0;

	DIR *dir = opendir("./posts");
	if (dir == NULL) {
		fprintf(stderr, "Error opening directory\n");
		return 1;
	}

	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		char *file = ent->d_name;
		size_t len = strlen(file);

		/* Filter */
		if (len < 7) continue;
		int id = extract_id(file);
		if (file[len-3] != '.' ||
			file[len-2] != 'm' ||
			file[len-1] != 'd' ||
			id == -1
		) continue;

		char md[1024];
		char html[1024];
		char link[1024];
		char cmd[1024];

		sprintf(md, "./posts/%s", file);
		sprintf(html, "./pages/%s.html", file);

		const char *mdf = "./build/md2html '%s' '%s'";
		sprintf(cmd, mdf, md, html);

		char *name = xstrdup(file);
		name[len-3] = '\0';
		name += 5;

		posts[count++] = (Post){
			.id = id,
			.title = name,
			.link = xstrdup(html),
		};

		printf("CMD: %s\n", cmd);
		if (system(cmd) != 0) {
			return 1;
		}
	}
	closedir(dir);

	/* Sorting */
	while (true) {
		bool sorted = true;
		for (size_t i = 0; i < count - 1; i++) {
			if (posts[i].id < posts[i+1].id) {
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
