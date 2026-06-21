/* ============================================================
 *  string_t.h - string dinamica para C (estilo C++ std::string)
 * ============================================================
 *
 * IMPORTANTE: o arquivo NAO se chama "string.h" de proposito, para
 * nao colidir/confundir com o <string.h> da libc (que continua
 * sendo usado normalmente por dentro da implementacao).
 *
 * Diferente de vector.h/hashmap.h, string_t e' um TIPO POR VALOR
 * (igual std::string em si, nao um std::vector<char>*). Isso
 * significa:
 *
 *     string_t s = string_new();      // equivalente a std::string s;
 *     string_append(&s, "ola");        // equivalente a s += "ola";
 *     printf("%s\n", string_cstr(&s)); // equivalente a s.c_str()
 *     string_free(&s);                  // equivalente ao destrutor
 *
 * Toda funcao que modifica a string recebe string_t* (ponteiro pra
 * struct), igual um metodo nao-const chamado em "this". Funcoes que
 * so leem recebem const string_t*. O campo "data" e' sempre
 * null-terminated, entao da' pra passar string_cstr(&s) direto pra
 * qualquer funcao que espera "const char *".
 *
 * Nao ha excecoes em C: onde std::string lancaria std::out_of_range
 * (ex: posicao invalida em erase/substr), aqui a posicao e'
 * truncada/ignorada silenciosamente (documentado em cada funcao).
 * Falha de alocacao aborta o programa, igual o comportamento padrao
 * de operator new ao nao conseguir memoria.
 *
 * Requer C99+. string_format/string_append_format usam vsnprintf
 * (C99) e __attribute__((format(printf,...))) quando compilado com
 * GCC/Clang, para o compilador checar os argumentos de formato.
 */

#ifndef STRING_T_H
#define STRING_T_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STRING_NPOS ((size_t)-1)   /* equivalente a std::string::npos */

typedef struct {
    char  *data;       /* sempre null-terminated; nunca NULL apos string_new/from/etc. */
    size_t length;       /* numero de bytes, sem contar o '\0' (igual size()/length()) */
    size_t capacity;      /* bytes utilizaveis em 'data', sem contar o '\0' */
} string_t;

#if defined(__GNUC__) || defined(__clang__)
#define STRING__PRINTF_FMT(fmt_idx, args_idx) __attribute__((format(printf, fmt_idx, args_idx)))
#else
#define STRING__PRINTF_FMT(fmt_idx, args_idx)
#endif

/* ---------- construcao / destruicao ---------- */
string_t string_new(void);                                   /* std::string()              */
string_t string_from(const char *cstr);                       /* std::string(const char*)    */
string_t string_from_n(const char *data, size_t n);            /* std::string(const char*, n) */
string_t string_with_capacity(size_t capacity);                /* reserve() upfront            */
string_t string_clone(const string_t *s);                      /* copy constructor             */
string_t string_format(const char *fmt, ...) STRING__PRINTF_FMT(1, 2);
void     string_free(string_t *s);                              /* destrutor; seguro chamar 2x  */

/* ---------- capacidade ---------- */
size_t string_length(const string_t *s);     /* size() / length() */
size_t string_size(const string_t *s);       /* alias              */
size_t string_capacity(const string_t *s);
int    string_empty(const string_t *s);
void   string_reserve(string_t *s, size_t new_capacity);
void   string_shrink_to_fit(string_t *s);
void   string_resize(string_t *s, size_t n, char fill);

/* ---------- acesso a elementos ----------
 * string_at: com checagem de limites, aborta com mensagem se invalido (igual .at()).
 * front/back/cstr/data: sem checagem (igual operator[]/front()/back()/c_str()/data()),
 * UB se a string estiver vazia (front/back) -- comportamento identico ao std::string. */
char        string_at(const string_t *s, size_t index);
char        string_front(const string_t *s);
char        string_back(const string_t *s);
const char *string_cstr(const string_t *s);   /* c_str() */
char       *string_data(string_t *s);          /* data() nao-const; nao escreva alem de length() */

/* ---------- modificadores ---------- */
void string_clear(string_t *s);
void string_push_back(string_t *s, char c);
void string_pop_back(string_t *s);                 /* no-op seguro se vazia (nao e' UB aqui) */
void string_assign(string_t *s, const char *cstr);  /* operator=(const char*) */
void string_assign_n(string_t *s, const char *data, size_t n);
void string_append(string_t *s, const char *cstr);   /* operator+=(const char*) */
void string_append_n(string_t *s, const char *data, size_t n);
void string_append_char(string_t *s, char c);          /* operator+=(char) */
void string_append_string(string_t *s, const string_t *other); /* operator+=(const string&) */
void string_append_format(string_t *s, const char *fmt, ...) STRING__PRINTF_FMT(2, 3);

/* pos deve ser <= length(); comportamento indefinido caso contrario
 * (mesma convencao do vector.h, sem checagem por performance) */
void string_insert(string_t *s, size_t pos, const char *cstr);

/* pos > length(): no-op. len alem do fim: truncado (igual std::string::erase) */
void string_erase(string_t *s, size_t pos, size_t len);

/* pos > length(): no-op. len alem do fim: truncado (igual std::string::replace) */
void string_replace(string_t *s, size_t pos, size_t len, const char *replacement);

void string_swap(string_t *a, string_t *b);

void string_trim(string_t *s);        /* remove espacos das duas pontas, in-place */
void string_trim_left(string_t *s);
void string_trim_right(string_t *s);
void string_to_upper(string_t *s);     /* in-place, ASCII (via toupper) */
void string_to_lower(string_t *s);     /* in-place, ASCII (via tolower) */

/* ---------- operacoes ---------- */
int  string_compare(const string_t *a, const string_t *b);       /* como strcmp: <0, 0, >0 */
int  string_compare_cstr(const string_t *a, const char *cstr);
int  string_equals(const string_t *a, const string_t *b);
int  string_equals_cstr(const string_t *a, const char *cstr);

string_t string_substr(const string_t *s, size_t pos, size_t len); /* substr() -- nova string_t, dono do chamador */
string_t string_concat(const string_t *a, const string_t *b);       /* equivalente a operator+ */

/* retornam STRING_NPOS se nao encontrado, igual std::string::npos */
size_t string_find(const string_t *s, const char *needle, size_t start_pos);
size_t string_find_char(const string_t *s, char c, size_t start_pos);
size_t string_rfind(const string_t *s, const char *needle);

int string_starts_with(const string_t *s, const char *prefix);
int string_ends_with(const string_t *s, const char *suffix);
int string_contains(const string_t *s, const char *needle);

/* ---------- split (sem equivalente direto no std::string) ----------
 * Retorna um vetor (do vector.h) de string_t, separando 's' pelo
 * caractere 'delim'. O array E as strings internas tem vida propria
 * (copias independentes de 's'); libere com string_array_free, que
 * libera cada string_t e depois o array em si.
 *
 *     string_t *partes = string_split(&s, ',');
 *     for (size_t i = 0; i < vec_len(partes); i++)
 *         printf("%s\n", string_cstr(&partes[i]));
 *     string_array_free(&partes);
 */
string_t *string_split(const string_t *s, char delim);

void      string_array_free(string_t **arr);

void string_print(const string_t* s);


#ifdef __cplusplus
}
#endif

#endif /* STRING_T_H */