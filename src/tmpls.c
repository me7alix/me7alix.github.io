#define HTMPL_IMPLEMENTATION
#include "../3rdparty/htmpl.h"

int main(void) {
	HTMPL_StringBuilder tb = {0};
	tmpls_builder_compile_template(&tb, "./tmpls/index.htmpl");
	tmpls_builder_write(&tb, "./build/index.c");
	tmpls_builder_destroy(&tb);
	return 0;
}
