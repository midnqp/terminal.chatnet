#include <gc/gc.h>

#include "autofree.h"

#ifndef DEBUG
#define DEBUG 0
#endif
#define _add(a, b) a b
#define debuglog(label, ...)                                                   \
	do {                                                                       \
		if (DEBUG)                                                             \
			printf(_add(_add("\033[104m", label), "\033[0m") __VA_ARGS__);     \
	} while (0);

void **freeable_list;
int freeable_count = 0;

int isptreq(void *p1, void *p2) {
	char addr1[16];
	char addr2[16];
	sprintf(addr1, "%p", p1);
	sprintf(addr2, "%p", p2);
	return strcmp(addr1, addr2) == 0;
}

int freeable_idx(void *ptr) {
	for (int i = 0; i < freeable_count; i++) {
		int iseq = isptreq(freeable_list[i], ptr);
		if (iseq == 1)
			return i;
	}
	return -1;
}

void freeable_remove(void *ptr) {
	int idx = freeable_idx(ptr);
	if (idx != -1) {
		for (int i = idx; i < freeable_count - 1; i++) {
			freeable_list[i] = freeable_list[i + 1];
		}
		freeable_count--;
	}
}
void freeable_add(void *ptr) {
	int idx = freeable_idx(ptr);
	if (idx == -1) {
		size_t sz = (freeable_count + 1) * sizeof(void *);
		// TODO realloc for 5 pointers each time
		freeable_list = realloc(freeable_list, sz);
		freeable_list[freeable_count] = ptr;
		freeable_count++;
	}
}

void dealloc() {
	for (int i = 0; i < freeable_count; i++) {
		if (freeable_list[i] == NULL)
			continue;
		debuglog("dealloc", " now address %p\n", freeable_list[i]);

		free(freeable_list[i]);
		freeable_list[i] = NULL;
		debuglog("dealloc", " done address, remaining %d\n",
				 freeable_count - (i + 1));
	}

	freeable_count = 0;
	debuglog("dealloc", "about to final...\n");
	if (!!freeable_list || (freeable_list != NULL)) {
		debuglog("dealloc (freeable_list)", " address %p\n", freeable_list);
		free(freeable_list);
		freeable_list = NULL;
	}
}

void *alloc(size_t len) {
	void *result = malloc(len);
	debuglog("alloc", " %zu bytes at %p, total %d\n", len, result,
			 freeable_count + 1);
	freeable_add(result);
	return result;
};

void *autofree_realloc(void *ptr, size_t sz) {
	// not having the line below was a major painful bug!
	int autofree_created = freeable_idx(ptr) != -1;
	// since realloc returns new pointer, remove old one
	if (autofree_created)
		freeable_remove(ptr);
	ptr = realloc(ptr, sz);
	// tracking back any autofree-maintained pointer
	if (autofree_created)
		freeable_add(ptr);
	return ptr;
}

void autofree_free(void *ptr) {
	free(ptr);
	freeable_remove(ptr);
}

/*
#ifdef free
#undef free
#define free(x) \
  {             \
	free(x);    \
	x = NULL;   \
  }
#endif

#ifdef malloc
#undef malloc
#define malloc(x) alloc(x)
#endif
*/
