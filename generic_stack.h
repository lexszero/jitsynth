#ifndef ADT_STACK

#define ADT_STACK

#define stack_new(name) name##_new();
#define stack_push(name, data) name##_push(name, data)
#define stack_pop(name) name##_pop(name)
#define stack_empty(name) name##_empty(name);
#define stack_free(name) name##_free(name);
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

static void NAME_push(NAME_t *t, TYPE data) {
	assert(t);

	NAMEitem_t *item = calloc(1, sizeof(NAMEitem_t));
	item->data = data;
	item->next = t->head;
	t->head = item;
	LOGF("push head=%p", t->head);
}

static TYPE NAME_pop(NAME_t *t) {
	assert(t->head);

	NAMEitem_t *item = t->head;
	t->head = item->next;
	TYPE data = item->data;
	/* Achtung! We don't carry about freeign YOUR data. */
	free(item);
	LOGF("pop head=%p", t->head);
	return data;
}

static bool stack_empty(NAME_t *t) {
	return (t->head != NULL);
}

static void NAME_free(NAME_t *t) {
	NAMEitem_t *cur;
	for (cur = t->head; cur; cur = cur->next, free(cur));
	free(t);
}
