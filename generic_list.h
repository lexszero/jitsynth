#ifndef ADT_LIST

#define ADT_LIST
#define list_new(type) type##_new()
#define list_add_head(type, name, data) type##_add_head(name, data)
#define list_add_tail(type, name, data) type##_add_tail(name, data)
#define list_delete(type, name, item) type##_delete(name, item)
#define list_foreach(name, item) for (item = name->head; item; item = item->next) 
#define list_foreach_safe(name, item, nextitem, func) for (item = name->head; item; item = nextitem) { nextitem = item->next; func; }
#define list_free(type, name) type##_free(name)

#define _unused __attribute__((unused))
#define LIST(NAME, TYPE) \
typedef struct NAME##item_t { \
	TYPE data; \
	struct NAME##item_t *next, *prev; \
} NAME##item_t; \
\
typedef struct NAME##_t { \
	pthread_mutex_t mutex; \
	NAME##item_t *head, *tail; \
} NAME##_t; \
\
static _unused NAME##_t * NAME##_new() { \
	NAME##_t *t = calloc(1, sizeof(NAME##_t)); \
	return t; \
} \
\
static _unused NAME##item_t * NAME##_add_head(NAME##_t *t, TYPE data) { \
	assert(t); \
	mutex_lock(*t); \
	NAME##item_t *item = calloc(1, sizeof(NAME##item_t)); \
	item->data = data; \
	if (t->head) { \
		item->next = t->head; \
		t->head->prev = item; \
	} \
	t->head = item; \
	if (! t->tail) \
		t->tail = item; \
	mutex_unlock(*t); \
	LOGF("add head=%p", t->head); \
	return item; \
} \
\
static _unused NAME##item_t * NAME##_add_tail(NAME##_t *t, TYPE data) { \
	assert(t); \
	mutex_lock(*t); \
	NAME##item_t *item = calloc(1, sizeof(NAME##item_t)); \
	item->data = data; \
	if (t->tail) { \
		item->prev = t->tail; \
		t->tail->next = item; \
	} \
	t->tail = item; \
	if (! t->head) \
		t->head = item; \
	mutex_unlock(*t); \
	LOGF("add tail=%p", t->head); \
	return item; \
} \
\
static _unused void NAME##_delete(NAME##_t *t, NAME##item_t *item) { \
	assert(t); \
	assert(item); \
	mutex_lock(*t); \
	if (t->head == item) \
		t->head = item->next; \
	if (t->tail == item) \
		t->tail = item->prev; \
	if (item->next) \
		item->next->prev = item->prev; \
	if (item->prev)\
		item->prev->next = item->next; \
	free(item); \
	mutex_unlock(*t); \
	LOGF("deleted"); \
} \
\
static _unused void NAME##_free(NAME##_t *t) { \
	NAME##item_t *cur; \
	mutex_lock(*t); \
	for (cur = t->head; cur; cur = cur->next, free(cur)); \
	mutex_unlock(*t); \
	free(t); \
}

#endif
