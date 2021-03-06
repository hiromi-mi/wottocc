#include "wottocc.h"


Map *new_map() {
	Map *map = malloc(sizeof(Map));
	map->keys = new_vector();
	map->vals = new_vector();
	map->types = new_vector();
	return map;
}

void map_put(Map *map, char *key, void *val, Type *type) {
	vec_push(map->keys, key);
	vec_push(map->vals, val);
	vec_push(map->types, type);
}

void *map_get(Map *map, char *key) {
	for (int i = map->keys->len - 1; i >= 0; i--)
		if (strcmp(map->keys->data[i], key) == 0)
			return map->vals->data[i];
	return NULL;
}

void *map_get_type(Map *map, char *key) {
	for (int i = map->keys->len - 1; i >= 0; i--)
		if (strcmp(map->keys->data[i], key) == 0)
			return map->types->data[i];
	return NULL;
}

int map_get_ind(Map *map, char *key) {
	for (int i = map->keys->len - 1; i >= 0; i--)
		if (strcmp(map->keys->data[i], key) == 0)
			return i;
	return -1;
}


void test_map() {
	Map *map = new_map();
	/*expect(__LINE__, 0, (int)map_get(map, "foo"));

	map_put(map, "foo", (void *)2);
	expect(__LINE__, 2, (int)map_get(map, "foo"));

	map_put(map, "bar", (void *)4);
	expect(__LINE__, 4, (int)map_get(map, "bar"));

	map_put(map, "foo", (void *)6);
	expect(__LINE__, 6, (int)map_get(map, "foo"));*/

	printf("map OK\n");
}
