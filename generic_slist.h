#ifndef ADT_SLIST

#define ADT_SLIST
#define slist_new(name) name##_new();
#define slist_add(name, data) name##_add(name, data)
#define slist_foreach(name, item, x) for (item = name->head, x = item->data; item; item = item->next, x = (item) ? item->data : x)
#define slist_free(name) name##_free(name)
#endif

typedef struct NAMEitem_t {
	TYPE data;
	struct NAMEitem_t *next;
} NAMEitem_t;

typedef struct NAME_t {
	NAMEitem_t *head;
} NAME_t;

static NAME_t * NAME_new() {
	NAME_t *t = calloc(1, sizeof(NAME_t));
	return t;
}

static void NAME_add(NAME_t *t, TYPE data) {
	assert(t);

	NAMEitem_t *item = calloc(1, sizeof(NAMEitem_t));
	item->data = data;
	item->next = t->head;
	t->head = item;
	LOGF("add head=%p", t->head);
}

static void NAME_free(NAME_t *t) {
	NAMEitem_t *cur;
	for (cur = t->head; cur; cur = cur->next, free(cur));
	free(t);
}
