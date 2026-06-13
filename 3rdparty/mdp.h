#ifndef MDP_H_
#define MDP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

typedef enum {
	MDP_TOK_EOF,
	MDP_TOK_HEADING,
	MDP_TOK_ORD_LIST,
	MDP_TOK_BLOCK_CODE,
	MDP_TOK_UNORD_LIST,
	MDP_TOK_QUOTE,
	MDP_TOK_2NL,
	_MDP_TOK_INLINE_START,
	MDP_TOK_BEG_LINK,
	MDP_TOK_END_LINK,
	MDP_TOK_STRONG,
	MDP_TOK_EMPHASIS,
	MDP_TOK_INLINE_CODE,
	MDP_TOK_CHAR,
	MDP_TOK_NL,
	_MDP_TOK_INLINE_END,
} MDP_TokenKind;

typedef struct {
	MDP_TokenKind kind;
	union {
		struct {
			const char *lang;
			const char *code;
		} block_code;
		const char *inline_code;
		char chr;
		unsigned data;
	} as;
} MDP_Token;

typedef struct {
	MDP_Token *tokens;
	size_t count;
} MDP_Parser;

typedef enum {
/* Container */
	MDP_NODE_DOC,
	MDP_NODE_HEADING,
	MDP_NODE_PARAGRAPH,
	MDP_NODE_ORD_LIST,
	MDP_NODE_UNORD_LIST,
	MDP_NODE_BLOCK_CODE,
	MDP_NODE_QUOTE,

/* Inline */
	MDP_NODE_LINK,
	MDP_NODE_IMAGE,
	MDP_NODE_STRONG,
	MDP_NODE_EMPHASIS,
	MDP_NODE_INLINE_CODE,
	MDP_NODE_TEXT,
	MDP_NODE_NL,

/* Helper */
	MDP_NODE_ORD_LIST_ITEM,
} MDP_NodeKind;

typedef struct MDP_Node MDP_Node;
struct MDP_Node {
	MDP_NodeKind kind;
	MDP_Node *body;
	MDP_Node *next;

	union {
		struct {
			unsigned level;
			MDP_Node *title;
		} heading;
		struct {
			const char *lang;
			const char *code;
		} block_code;
		struct {
			const char *link;
			const char *desc;
		} link;
		unsigned idx;
		const char *inline_code;
		const char *text;
	} as;
};

#define mdp_node_foreach(vn, list) \
	for (MDP_Node *vn = list; vn; vn = vn->next)

MDP_Token *mdp_lex(const char *stream);
MDP_Node *mdp_parse(MDP_Token *stream);

#endif // MDP_H_

#ifdef MDP_IMPLEMENTATION

#define _MDP_DA(T)       \
	struct {             \
		T *items;        \
		size_t count;    \
		size_t capacity; \
	}

#define _mdp_da_append(da, item)                               \
	do {                                                       \
		if ((da)->capacity == 0) (da)->capacity = 256;         \
		if ((da)->count >= (da)->capacity - 1 || !(da)->items) \
			(da)->items = realloc((da)->items, sizeof(*(da)->items) * ((da)->capacity *= 2)); \
		(da)->items[(da)->count++] = (item);                   \
	} while (0)

#define mdp_da_free(da)         \
	do {                        \
		if ((da)->items) {      \
			free((da)->items);  \
			(da)->items = NULL; \
			(da)->count = 0;    \
			(da)->capacity = 0; \
		}                       \
	} while (0)

/* Lexer stage */

typedef _MDP_DA(MDP_Token) _MDP_Tokens;

void _mdp_tokens_append(_MDP_Tokens *toks, MDP_Token tok) {
	_mdp_da_append(toks, tok);
}

MDP_Token *mdp_lex(const char *stream) {
	_MDP_Tokens toks = {0};
	size_t count = 0;
	goto start;

#define append(kd, ch) \
	_mdp_tokens_append(&toks, (MDP_Token){.kind = kd, .as.chr = ch})
#define append_data(kd, dt) \
	_mdp_tokens_append(&toks, (MDP_Token){.kind = kd, .as.data = dt});
#define CHN() stream[count++]
#define CHP() stream[count]
#define CHI(i) stream[count+i]

	while (true) {
		switch (CHP()) {
		case '\0':
			goto finish;

		case '[':
			append(MDP_TOK_BEG_LINK, CHN());
			break;

		case ']':
			append(MDP_TOK_END_LINK, CHN());
			break;

		case '`':
			CHN();
			if (CHI(0) == '`' && CHI(1) == '`') {
				CHN(); CHN();
				_MDP_DA(char) lang = {0};
				while (CHP() != '\n' && CHP() != '\0')
					_mdp_da_append(&lang, CHN());
				_mdp_da_append(&lang, '\0');
				CHN();

				_MDP_DA(char) code = {0};
				while (
					!(CHI(0) == '`' &&
					CHI(1) == '`' &&
					CHI(2) == '`') &&
					CHP() != '\0'
				) _mdp_da_append(&code, CHN());
				code.items[code.count-1] = '\0';

				if (CHP() != '\0') {
					CHN(); CHN(); CHN();
				}

				_mdp_tokens_append(&toks, (MDP_Token){
					.kind = MDP_TOK_BLOCK_CODE,
					.as.block_code.lang = lang.items,
					.as.block_code.code = code.items,
				});
			} else {
				_MDP_DA(char) code = {0};
				while (CHI(0) != '`' && CHP() != '\0')
					_mdp_da_append(&code, CHN());
				_mdp_da_append(&code, '\0');
				if (CHP() != '\0') CHN();
				_mdp_tokens_append(&toks, (MDP_Token){
					.kind = MDP_TOK_INLINE_CODE,
					.as.inline_code = code.items,
				});
			}
			break;

		case '_':
			CHN();
			if (CHP() == '_') {
				append(MDP_TOK_STRONG, CHN());
			} else {
				append(MDP_TOK_EMPHASIS, '_');
			} break;

		case '*':
			CHN();
			if (CHP() == '*') {
				append(MDP_TOK_STRONG, CHN());
			} else {
				append(MDP_TOK_EMPHASIS, '*');
			} break;

		case '\n':
			CHN();
			if (CHP() == '\n') append(MDP_TOK_2NL, CHN());
			else append(MDP_TOK_NL, '\n');
		start:
			while (CHP() == ' ') CHN();

			switch (CHP()) {
			case '\0':
				goto finish;

			case '#':
				unsigned i;
				for (i = 0; CHP() == '#'; i++) CHN();
				append_data(MDP_TOK_HEADING, i);
				while (CHP() == ' ') CHN();
				break;

			case '>':
				append(MDP_TOK_QUOTE, CHN());
				while (CHP() == ' ') CHN();
				break;

			case '*':
			case '-':
				append(MDP_TOK_UNORD_LIST, CHN());
				while (CHP() == ' ') CHN();
				break;

			default:
				if (isdigit(CHP())) {
					char str[16] = {0};
					size_t cnt = 0;
					while (isdigit(CHP()))
						str[cnt++] = CHN();

					if (CHP() == '.') {
						append_data(MDP_TOK_ORD_LIST, atoi(str));
						CHN();
						while (CHP() == ' ') CHN();
					} else {
						for (size_t i = 0; i < cnt; i++) {
							append(MDP_TOK_CHAR, str[i]);
						}
					}
				}
			}
			break;

		default:
			append(MDP_TOK_CHAR, CHN());
		}
	}

finish:
	append(MDP_TOK_EOF, 0);

#undef append
#undef append_data
#undef CHN
#undef CHP
#undef CHI

	return toks.items;
}

void mdp_dump_tokens(MDP_Token *toks) {
	for (size_t i = 0; toks[i].kind != MDP_TOK_EOF; i++) {
		switch (toks[i].kind) {
		case MDP_TOK_STRONG:      printf("STRONG\n");      break;
		case MDP_TOK_NL:          printf("NL\n");          break;
		case MDP_TOK_2NL:         printf("2NL\n");         break;
		case MDP_TOK_QUOTE:       printf("QUOTE\n");       break;
		case MDP_TOK_INLINE_CODE: printf("INLINE_CODE\n"); break;
		case MDP_TOK_BLOCK_CODE:  printf("BLOCK_CODE\n");  break;
		case MDP_TOK_EMPHASIS:    printf("EMPHASIS\n");    break;
		case MDP_TOK_UNORD_LIST:  printf("UNORD_LIST\n");  break;
		case MDP_TOK_EOF:         printf("EOF\n");         break;
		case MDP_TOK_HEADING:
			printf("HEADING(%u)\n", toks[i].as.data);
			break;
		case MDP_TOK_ORD_LIST:
			printf("ORD_LIST(%u)\n", toks[i].as.data);
			break;
		case MDP_TOK_CHAR:
			char ch = toks[i].as.chr;
			if (ch == '\n') printf("CHAR(NL)\n");
			else printf("CHAR(%c)\n", ch);
		}
	}
}

/* Parser stage */

#define next(p) (p)->tokens[(p)->count++]
#define peek(p) (p)->tokens[(p)->count]
#define peek2(p) (p)->tokens[(p)->count+1]
#define is_chr(p, ch) \
	(peek(p).kind == MDP_TOK_CHAR && peek(p).as.chr == (ch))

#define node(kind, ...) _mdp_node(kind, &(MDP_Node){__VA_ARGS__})
MDP_Node *_mdp_node(MDP_NodeKind kind, MDP_Node *node) {
	MDP_Node *n = malloc(sizeof(MDP_Node));
	*n = *node;
	n->kind = kind;
	return n;
}

void _mdp_node_append(MDP_Node **head, MDP_Node *n) {
	if (!*head) {
		*head = n;
		return;
	}

	MDP_Node *cur = *head;
	while (cur->next)
		cur = cur->next;
	cur->next = n;
}

bool _mdp_is_tok_inline(MDP_Token tok) {
	return tok.kind > _MDP_TOK_INLINE_START &&
	       tok.kind < _MDP_TOK_INLINE_END;
}

void _mdp_remove_nl(MDP_Node *bf_last, MDP_Node *last) {
	if (bf_last != NULL && last != NULL) {
		if (last->kind == MDP_NODE_NL) {
			bf_last->next = NULL;
			free(last);
		}
	}
}

MDP_Node *_mdp_parse(MDP_Parser *p, bool is_inline);

MDP_Node *_mdp_parse_paragraph(MDP_Parser *p) {
	MDP_Node *n = node(MDP_NODE_PARAGRAPH, 0);
	MDP_Node *bf_last = NULL, *last = NULL;
	bool is_empty = true;
	while (_mdp_is_tok_inline(peek(p))) {
		MDP_Node *c = _mdp_parse(p, true);
		if (!c) { next(p); continue; }
		if (c->kind != MDP_NODE_NL) is_empty = false;
		if (!is_empty) _mdp_node_append(&n->body, c);
		bf_last = last;
		last = c;
	}
	if (!n->body || is_empty)
		return NULL;
	_mdp_remove_nl(bf_last, last);
	if (peek(p).kind == MDP_TOK_2NL)
		next(p);
	return n;
}

MDP_Node *_mdp_parse_text(MDP_Parser *p) {
	_MDP_DA(char) sb = {0};
	while (peek(p).kind == MDP_TOK_CHAR)
		_mdp_da_append(&sb, next(p).as.chr);
	_mdp_da_append(&sb, '\0');
	return node(MDP_NODE_TEXT, .as.text = sb.items);
}

MDP_Node *_mdp_parse(MDP_Parser *p, bool is_inline) {
	if (!is_inline) {
		switch (peek(p).kind) {
		case MDP_TOK_HEADING: {
			unsigned level = next(p).as.data;
			MDP_Node *n = node(
				MDP_NODE_HEADING,
				.as.heading.level = level);
			while (_mdp_is_tok_inline(peek(p))) {
				MDP_Node *c = _mdp_parse(p, true);
				if (!c) { next(p); continue; }
				if (c->kind == MDP_NODE_NL) { free(c); break; }
				_mdp_node_append(&n->as.heading.title, c);
			}
			while (
				!(peek(p).kind == MDP_TOK_HEADING &&
				peek(p).as.data <= level) &&
				peek(p).kind != MDP_TOK_EOF
			) {
				MDP_Node *c = _mdp_parse(p, false);
				if (!c) { next(p); continue; }
				_mdp_node_append(&n->body, c);
			}
			return n;
		}

		case MDP_TOK_BLOCK_CODE: {
			return node(MDP_NODE_BLOCK_CODE,
				.as.block_code.lang = peek(p).as.block_code.lang,
				.as.block_code.code = next(p).as.block_code.code,
			);
		}

		case MDP_TOK_QUOTE: {
			MDP_Node *n = node(MDP_NODE_QUOTE, 0);
			MDP_Node *bf_last = NULL, *last = NULL;
			while (peek(p).kind == MDP_TOK_QUOTE) {
				next(p);
				while (_mdp_is_tok_inline(peek(p))) {
					MDP_Node *c = _mdp_parse(p, true);
					if (!c) { next(p); continue; }
					_mdp_node_append(&n->body, c);
					bf_last = last;
					last = c;
					if (c->kind == MDP_NODE_NL) {
						break;
					}
				}
			}
			_mdp_remove_nl(bf_last, last);
			return n;
		}

		case MDP_TOK_UNORD_LIST: {
			MDP_Node *n = node(MDP_NODE_UNORD_LIST, 0);
			while (peek(p).kind == MDP_TOK_UNORD_LIST) {
				next(p);
				MDP_Node *c = _mdp_parse(p, false);
				if (!c) { next(p); continue; }
				_mdp_node_append(&n->body, c);
			}
			return n;
		}

		case MDP_TOK_ORD_LIST: {
			MDP_Node *n = node(MDP_NODE_ORD_LIST, 0);
			while (peek(p).kind == MDP_TOK_ORD_LIST) {
				unsigned idx = next(p).as.data;
				MDP_Node *c = _mdp_parse(p, false);
				if (!c) { next(p); continue; }
				_mdp_node_append(
					&n->body,
					node(MDP_NODE_ORD_LIST_ITEM,
						.as.idx = idx,
						.body = c,
					)
				);
			}
			return n;
		}

		default:
			return _mdp_parse_paragraph(p);
		}
	} else {
		switch (peek(p).kind) {
		/* [n->body](n->as.link) */
		case MDP_TOK_BEG_LINK: {
			MDP_Node *n = node(MDP_NODE_LINK, 0);
			next(p);
			while (
				peek(p).kind != MDP_TOK_END_LINK &&
				peek(p).kind != MDP_TOK_EOF
			) {
				MDP_Node *c = _mdp_parse(p, true);
				if (!c) { next(p); continue; }
				_mdp_node_append(&n->body, c);
			}
			next(p);
			if (!is_chr(p, '('))
				return NULL;
			next(p);
			_MDP_DA(char) link = {0};
			_MDP_DA(char) desc = {0};
			while (peek(p).kind == MDP_TOK_CHAR) {
				if (peek(p).as.chr == ')') break;
				if (peek(p).as.chr == '"') {
					next(p);
					while (peek(p).kind == MDP_TOK_CHAR) {
						if (peek(p).as.chr == '"') break;
						_mdp_da_append(&desc, next(p).as.chr);
					}
					next(p);
				} else {
					_mdp_da_append(&link, next(p).as.chr);
				}
			}
			_mdp_da_append(&link, '\0');
			_mdp_da_append(&desc, '\0');
			n->as.link.link = link.items;
			n->as.link.desc = desc.items;
			if (!is_chr(p, ')'))
				return NULL;
			next(p);
			return n;
		}

		/* `n->body` */
		case MDP_TOK_INLINE_CODE:
			return node(MDP_NODE_INLINE_CODE,
				.as.inline_code = next(p).as.inline_code);

		/* __n->body__ | _n->body_ */
		case MDP_TOK_STRONG:
		case MDP_TOK_EMPHASIS: {
			MDP_TokenKind kind = peek(p).kind;
			MDP_NodeKind nk =
				kind == MDP_TOK_STRONG   ? MDP_NODE_STRONG   :
				kind == MDP_TOK_EMPHASIS ? MDP_NODE_EMPHASIS : 0;
			MDP_Node *n = node(nk, 0);
			next(p); while (
				peek(p).kind != kind &&
				peek(p).kind != MDP_TOK_EOF
			) {
				MDP_Node *c = _mdp_parse(p, true);
				if (!c) { next(p); continue; }
				_mdp_node_append(&n->body, c);
			}
			next(p);
			return n;
		}

		case MDP_TOK_NL:
			next(p);
			return node(MDP_NODE_NL, 0);

		case MDP_TOK_CHAR:
			if (is_chr(p, '!') && peek2(p).kind == MDP_TOK_BEG_LINK) {
				next(p);
				MDP_Node *n = _mdp_parse(p, true);
				if (n) n->kind = MDP_NODE_IMAGE;
				return n;
			}

			return _mdp_parse_text(p);
		}
	}

	next(p);
	return NULL;
}

MDP_Node *mdp_parse(MDP_Token *stream) {
	MDP_Parser p = {.tokens = stream};
	MDP_Node *doc = node(MDP_NODE_DOC, 0);
	while (peek(&p).kind != MDP_TOK_EOF)
		_mdp_node_append(&doc->body, _mdp_parse(&p, false));
	return doc;
}

void mdp_dump_tree(MDP_Node *n, int it, int lv) {
	printf("%*s", it, "");
	switch (n->kind) {
	case MDP_NODE_DOC:
		printf("Document:\n");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_STRONG:
		printf("Strong:\n");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_IMAGE:
		printf("Image(%s|%s):\n", n->as.link.link, n->as.link.desc);
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_LINK:
		printf("Link(%s|%s):\n", n->as.link.link, n->as.link.desc);
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_EMPHASIS:
		printf("Emphasis:\n");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_INLINE_CODE:
		printf("InlineCode: %s\n", n->as.inline_code);
		break;
	case MDP_NODE_PARAGRAPH:
		printf("Paragraph:\n");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_UNORD_LIST:
		printf("UnordList:\n");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_QUOTE:
		printf("Quote:\n");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_ORD_LIST:
		printf("OrdList:\n");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv, lv);
		break;
	case MDP_NODE_BLOCK_CODE:
		printf("BlockCode(%s):\n", n->as.block_code.lang);
		printf("%s\n", n->as.block_code.code);
		break;
	case MDP_NODE_ORD_LIST_ITEM:
		printf("OrdListItem(%u):\n", n->as.idx);
		mdp_dump_tree(n->body, it + lv, lv);
		break;
	case MDP_NODE_HEADING:
		printf("Heading(%u):\n", n->as.heading.level);
		printf("%*sTitle:\n", it + lv, "");
		mdp_node_foreach (c, n->as.heading.title)
			mdp_dump_tree(c, it + lv * 2, lv);
		printf("%*sBody:\n", it + lv, "");
		mdp_node_foreach (c, n->body)
			mdp_dump_tree(c, it + lv * 2, lv);
		break;
	case MDP_NODE_NL:
		printf("NewLine\n");
		break;
	case MDP_NODE_TEXT:
		printf("Text: %s\n", n->as.text);
	}
}

#undef next
#undef peek
#undef peek2
#undef is_chr
#undef node

#endif // MDP_IMPLEMENTATION
