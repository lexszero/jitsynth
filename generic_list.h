#ifndef ADT_SLIST

#define ADT_SLIST
#define list_new(type) type##_new();
#define list_add_head(type, name, data) type##_add_head(name, data)
#define list_add_tail(type, name, data) type##_add_tail(name, data)
#define list_delete(type, name, item) type##_delete(name, item)
#define list_foreach(name, item) for (item = name->head; item; item = item->next) 
#define list_foreach_safe(name, item, nextitem, func) for (item = name->head; item; item = nextitem) { nextitem = item->next; func; }
#define list_free(type, name) type##_free(name)
#endif

#define _unused __attribute__((unused))

typedef struct NAMEitem_t {
	TYPE data;
	struct NAMEitem_t *next, *prev;
} NAMEitem_t;

typedef struct NAME_t {
	NAMEitem_t *head, *tail;
} NAME_t;

static _unused NAME_t * NAME_new() {
	NAME_t *t = calloc(1, sizeof(NAME_t));
	return t;
}

static _unused NAMEitem_t * NAME_add_head(NAME_t *t, TYPE data) {
	assert(t);

	NAMEitem_t *item = calloc(1, sizeof(NAMEitem_t));
	item->data = data;

	if (t->head) {
		item->next = t->head;
		t->head->prev = item;
	}
	t->head = item;

	if (! t->tail)
		t->tail = item;

	LOGF("add head=%p", t->head);
	return item;
}

static _unused NAMEitem_t * NAME_add_tail(NAME_t *t, TYPE data) {
	assert(t);

	NAMEitem_t *item = calloc(1, sizeof(NAMEitem_t));
	item->data = data;

	if (t->tail) {
		item->prev = t->tail;
		t->tail->next = item;
	}
	t->tail = item;

	if (! t->head)
		t->head = item;

	LOGF("add tail=%p", t->head);
	return item;
}

static _unused void NAME_delete(NAME_t *t, NAMEitem_t *item) {
	assert(t);
	assert(item);

	if (t->head == item)
		t->head = item->next;
	if (t->tail == item)
		t->tail = item->prev;
	if (item->next)
		item->next->prev = item->prev;
	if (item->prev)
		item->prev->next = item->next;
	free(item);

	LOGF("delete");
}

static _unused void NAME_free(NAME_t *t) {
	NAMEitem_t *cur;
	for (cur = t->head; cur; cur = cur->next, free(cur));
	free(t);
}

#undef _unused
