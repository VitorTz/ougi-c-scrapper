#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "../include/structure/path.h"
#include "../include/parser.h"


static string_t html;

/* =========================================================================
 * Helpers
 * ========================================================================= */

/* Conta nós filhos do tipo elemento (ignora texto/comentário) */
static size_t count_element_children(og_node_t *node) {
    size_t n = 0;
    for (size_t i = 0; i < node->children.count; i++) {
        if (node->children.items[i]->type == OG_NODE_ELEMENT) n++;
    }
    return n;
}

/* Retorna o primeiro filho do tipo texto */
static og_node_t *first_text_child(og_node_t *node) {
    for (size_t i = 0; i < node->children.count; i++) {
        if (node->children.items[i]->type == OG_NODE_TEXT)
            return node->children.items[i];
    }
    return NULL;
}

static const char* html_load(const char* test_name) {
  path_t root = path_create("res/html-test");
  path_append(&root, test_name);
  path_change_extension(&root, ".html");
  html = path_read_text(&root);
  return string_cstr(&html);
}

static void html_free() {
  string_free(&html);
}

static og_node_t* parse(const char* test_name) {
  const char* text = html_load(test_name);
  return og_parse_html(text);
}

/* =========================================================================
 * Test 1 — Estrutura básica e parsing de atributos
 * ========================================================================= */

static void test_basic_structure(void) {
  og_node_t *root = parse("test_basic_structure");

  assert(root != NULL);
  assert(strcmp(root->tag, "#root") == 0);

  /* html > head + body */
  og_node_t *html_node = og_node_find(root, "html", NULL, NULL);
  assert(html_node != NULL);
  assert(count_element_children(html_node) == 2);

  og_node_t *title = og_node_find(root, "title", NULL, NULL);
  assert(title != NULL);
  og_node_t *title_text = first_text_child(title);
  assert(title_text != NULL);
  assert(strcmp(title_text->text, "Página") == 0);

  og_node_t *p = og_node_find(root, "p", "intro", NULL);
  assert(p != NULL);
  assert(strcmp(og_node_get_attr(p, "id"), "intro") == 0);
  assert(strcmp(og_node_get_attr(p, "class"), "lead") == 0);

  og_node_t *p_text = first_text_child(p);
  assert(p_text != NULL);
  assert(strcmp(p_text->text, "Olá mundo") == 0);

  /* atributo inexistente deve retornar NULL */
  assert(og_node_get_attr(p, "data-x") == NULL);

  og_node_free(root);
  printf("[-] test_basic_structure passed.\n");
}

/* =========================================================================
 * Test 2 — Void elements e self-closing
 * ========================================================================= */

static void test_void_and_selfclose(void) {
    og_node_t *root = parse("test_void_and_selfclose");
    og_node_t *div  = og_node_find(root, "div", NULL, NULL);
    assert(div != NULL);

    /* void elements não criam filhos dentro de si */
    og_node_t *img = og_node_find(div, "img", NULL, NULL);
    assert(img != NULL);
    assert(img->children.count == 0);
    assert(strcmp(og_node_get_attr(img, "src"), "banner.webp") == 0);
    assert(strcmp(og_node_get_attr(img, "alt"), "banner") == 0);

    og_node_t *input = og_node_find(div, "input", NULL, NULL);
    assert(input != NULL);
    assert(strcmp(og_node_get_attr(input, "type"), "text") == 0);
    assert(strcmp(og_node_get_attr(input, "value"), "ok") == 0);
    assert(input->children.count == 0);

    /* br e hr devem existir como filhos do div */
    og_node_t *br = og_node_find(div, "br", NULL, NULL);
    assert(br != NULL);
    og_node_t *hr = og_node_find(div, "hr", NULL, NULL);
    assert(hr != NULL);

    og_node_free(root);
    printf("[-] test_void_and_selfclose passed.\n");
}

/* =========================================================================
 * Test 3 — Raw elements (script / style)
 * ========================================================================= */

static void test_raw_elements(void) {
  og_node_t *root = parse("test_raw_elements");

  og_node_t *style = og_node_find(root, "style", NULL, NULL);
  assert(style != NULL);
  og_node_t *style_text = first_text_child(style);
  assert(style_text != NULL);
  assert(strstr(style_text->text, ".foo") != NULL);

  og_node_t *script = og_node_find(root, "script", NULL, NULL);
  assert(script != NULL);
  og_node_t *script_text = first_text_child(script);
  assert(script_text != NULL);
  /* conteúdo literal, o "<" não deve virar tag */
  assert(strstr(script_text->text, "var x") != NULL);

  og_node_free(root);
  printf("[-] test_raw_elements passed.\n");
}

/* =========================================================================
 * Test 4 — Comentários e DOCTYPE
 * ========================================================================= */

static void test_comments_and_doctype(void) {
    og_node_t *root = parse("test_comments_and_doctype");
    assert(root != NULL);

    /* DOCTYPE é ignorado, html deve estar presente */
    og_node_t *html_node = og_node_find(root, "html", NULL, NULL);
    assert(html_node != NULL);

    /* comentário dentro de html */
    bool found_comment = false;
    for (size_t i = 0; i < html_node->children.count; i++) {
        og_node_t *c = html_node->children.items[i];
        if (c->type == OG_NODE_COMMENT) {
            found_comment = true;
            assert(strstr(c->text, "corpo") != NULL);
        }
    }
    assert(found_comment);

    og_node_free(root);
    printf("[-] test_comments_and_doctype passed.\n");
}

/* =========================================================================
 * Test 5 — og_node_has_class
 * ========================================================================= */

static void test_has_class(void) {    
  og_node_t *root = parse("test_has_class");

  og_node_t *div = og_node_find(root, "div", NULL, NULL);
  assert(div != NULL);

  assert(og_node_has_class(div, "card"));
  assert(og_node_has_class(div, "highlight"));
  assert(og_node_has_class(div, "active"));
  assert(!og_node_has_class(div, "activ"));    /* prefixo não basta */
  assert(!og_node_has_class(div, "CARD"));     /* case sensitive */
  assert(!og_node_has_class(div, "missing"));

  og_node_t *span = og_node_find(root, "span", NULL, NULL);
  assert(span != NULL);
  assert(og_node_has_class(span, "tag"));
  assert(!og_node_has_class(span, "card"));

  og_node_free(root);
  printf("[-] test_has_class passed.\n");
}

/* =========================================================================
 * Test 6 — og_node_find_all (tag + id + class combinados)
 * ========================================================================= */

static void test_find_all(void) {
  og_node_t *root = parse("test_find_all");

  /* todos os <li> */
  og_node_list_t all_li = og_node_find_all(root, "li", NULL, NULL);
  assert(all_li.count == 4);
  og_node_list_free(&all_li);

  /* <li class="item"> */
  og_node_list_t items = og_node_find_all(root, "li", NULL, "item");
  assert(items.count == 3);
  og_node_list_free(&items);

  /* <li class="active"> */
  og_node_list_t actives = og_node_find_all(root, "li", NULL, "active");
  assert(actives.count == 2);
  og_node_list_free(&actives);

  /* <li id="special"> */
  og_node_t *special = og_node_find(root, "li", "special", NULL);
  assert(special != NULL);
  assert(og_node_has_class(special, "active"));

  /* busca por tag inexistente */
  og_node_list_t none = og_node_find_all(root, "table", NULL, NULL);
  assert(none.count == 0);
  og_node_list_free(&none);

  og_node_free(root);
  printf("[-] test_find_all passed.\n");
}

/* =========================================================================
 * Test 7 — Nesting profundo e tags não fechadas
 * ========================================================================= */

static void test_deep_nesting_and_unclosed() {
  /* Parser deve sobreviver a HTML mal-formado sem crash */
  og_node_t *root = parse("test_deep_nesting_and_unclosed");
  assert(root != NULL);

  og_node_t *strong = og_node_find(root, "strong", NULL, NULL);
  assert(strong != NULL);
  og_node_t *strong_text = first_text_child(strong);
  assert(strong_text != NULL);
  assert(strcmp(strong_text->text, "negrito") == 0);

  og_node_list_t paras = og_node_find_all(root, "p", NULL, NULL);
  assert(paras.count == 2);
  og_node_list_free(&paras);

  og_node_free(root);

  /* tags não fechadas — não deve crashar */
  const char *bad_html = "<div><p>sem fechar<span>span";
  og_node_t *bad_root = og_parse_html(bad_html);
  assert(bad_root != NULL);
  og_node_free(bad_root);

  printf("[-] test_deep_nesting_and_unclosed passed.\n");
}

/* =========================================================================
 * Test 8 — HTML vazio e strings degeneradas
 * ========================================================================= */

static void test_edge_cases() {
    /* string vazia */
    og_node_t *r1 = og_parse_html("");
    assert(r1 != NULL);
    assert(strcmp(r1->tag, "#root") == 0);
    assert(r1->children.count == 0);
    og_node_free(r1);

    /* só texto sem tags */
    og_node_t *r2 = og_parse_html("apenas texto simples");
    assert(r2 != NULL);
    og_node_t *t = first_text_child(r2);
    assert(t != NULL);
    assert(strcmp(t->text, "apenas texto simples") == 0);
    og_node_free(r2);

    /* tag sem atributos e sem conteúdo */
    og_node_t *r3 = og_parse_html("<br>");
    assert(r3 != NULL);
    og_node_t *br = og_node_find(r3, "br", NULL, NULL);
    assert(br != NULL);
    og_node_free(r3);

    printf("[-] test_edge_cases passed.\n");
}

/* =========================================================================
 * Test 9 — Atributos com aspas simples e sem aspas
 * ========================================================================= */

static void test_attribute_variants(void) {
  og_node_t *root = parse("test_attribute_variants");
  og_node_t *a = og_node_find(root, "a", NULL, NULL);
  assert(a != NULL);

  assert(strcmp(og_node_get_attr(a, "href"), "https://example.com") == 0);
  assert(strcmp(og_node_get_attr(a, "data-id"), "42") == 0);

  /* atributo booleano (sem valor) deve existir com valor vazio */
  const char *disabled = og_node_get_attr(a, "disabled");
  assert(disabled != NULL);
  assert(strcmp(disabled, "") == 0);

  og_node_free(root);
  printf("[-] test_attribute_variants passed.\n");
}

/* =========================================================================
 * Test 10 — extract_links
 * ========================================================================= */

static void test_extract_links() {
  og_node_t *root = parse("test_extract_links");
  assert(root != NULL);

  string_t *links = extract_links(root, "li", NULL, "chapter");
  assert(links != NULL);

  /* 3 únicos (duplicata removida, span sem href ignorado) */
  assert(vec_len(links) == 3);

  bool found1 = false, found2 = false, found3 = false;
  for (size_t i = 0; i < (size_t)vec_len(links); i++) {
      if (strcmp(links[i].data, "/chapter/1") == 0) found1 = true;
      if (strcmp(links[i].data, "/chapter/2") == 0) found2 = true;
      if (strcmp(links[i].data, "/chapter/3") == 0) found3 = true;
  }
  assert(found1 && found2 && found3);

  for (size_t i = 0; i < (size_t)vec_len(links); i++) {
    string_free(&links[i]);
  }
  vec_free(links);
  og_node_free(root);
  printf("[-] test_extract_links passed.\n");
}

/* =========================================================================
 * Test 11 — extract_links: nenhum resultado
 * ========================================================================= */

static void test_extract_links_empty(void) {
  og_node_t *root = parse("test_extract_links_empty");
  string_t *links = extract_links(root, "li", NULL, "chapter");
  assert(links == NULL);

  og_node_free(root);
  printf("[-] test_extract_links_empty passed.\n");
}

/* =========================================================================
 * Test 12 — extract_imgs (src / data-src / trim / skip vazio)
 * ========================================================================= */

static void test_extract_imgs(void) {
  og_node_t *root = parse("test_extract_imgs");
  assert(root != NULL);

  string_t *imgs = extract_imgs(root, "img", NULL, "wp-manga-chapter-img");
  assert(imgs != NULL);

  /* 3 imagens válidas (a última sem atributo é ignorada) */
  assert(vec_len(imgs) == 3);

  /* primeira URL deve ter sido trimada */
  assert(strcmp(imgs[0].data, "https://cdn.example.com/p01.webp") == 0);
  assert(strcmp(imgs[1].data, "https://cdn.example.com/p02.webp") == 0);
  assert(strcmp(imgs[2].data, "https://cdn.example.com/p03.webp") == 0);

  for (size_t i = 0; i < (size_t)vec_len(imgs); i++) {
      string_free(&imgs[i]);
  }
  vec_free(imgs);
  og_node_free(root);
  printf("[-] test_extract_imgs passed.\n");
}

/* =========================================================================
 * Test 13 — extract_imgs: nenhum resultado
 * ========================================================================= */

static void test_extract_imgs_empty(void) {
  const char *html = "<div><p>sem imagens</p></div>";
  og_node_t *root = og_parse_html(html);
  string_t *imgs = extract_imgs(root, "img", NULL, "wp-manga-chapter-img");
  assert(imgs == NULL);

  og_node_free(root);
  printf("[-] test_extract_imgs_empty passed.\n");
}

/* =========================================================================
 * Test 14 — HTML realista de página de manhwa (integração)
 * ========================================================================= */

static void test_integration_manhwa_page(void) {
  og_node_t *root = parse("test_integration_manhwa_page");
  assert(root != NULL);

  /* título */
  og_node_t *title = og_node_find(root, "title", NULL, NULL);
  assert(title != NULL);

  /* 3 capítulos */
  og_node_list_t chapters = og_node_find_all(root, "li", NULL, "wp-manga-chapter");
  assert(chapters.count == 3);
  og_node_list_free(&chapters);

  /* extract_links deve retornar 3 */
  string_t *links = extract_links(root, "li", NULL, "wp-manga-chapter");
  assert(links != NULL);
  assert(vec_len(links) == 3);
  for (size_t i = 0; i < (size_t)vec_len(links); i++) string_free(&links[i]);
  vec_free(links);

  /* extract_imgs deve retornar 2 (src vazio cai para data-src) */
  string_t *imgs = extract_imgs(root, "img", NULL, "wp-manga-chapter-img");
  assert(imgs != NULL);
  assert(vec_len(imgs) == 2);
  assert(strcmp(imgs[0].data, "https://cdn.site.com/opm/200/p001.jpg") == 0);
  assert(strcmp(imgs[1].data, "https://cdn.site.com/opm/200/p002.jpg") == 0);
  for (size_t i = 0; i < (size_t)vec_len(imgs); i++) string_free(&imgs[i]);
  vec_free(imgs);

  /* meta charset */
  og_node_t *meta = og_node_find(root, "meta", NULL, NULL);
  assert(meta != NULL);
  assert(strcmp(og_node_get_attr(meta, "charset"), "UTF-8") == 0);

  og_node_free(root);
  printf("[-] test_integration_manhwa_page passed.\n");
}


int main(void) {
  printf("Running HTML parser tests...\n\n");

  test_basic_structure();
  test_void_and_selfclose();
  test_raw_elements();
  test_comments_and_doctype();
  test_has_class();
  test_find_all();
  test_deep_nesting_and_unclosed();
  test_edge_cases();
  test_attribute_variants();
  test_extract_links();
  test_extract_links_empty();
  test_extract_imgs();
  test_extract_imgs_empty();
  test_integration_manhwa_page();

  html_free();
  printf("\nAll parser tests completed successfully!\n");
  return 0;
}