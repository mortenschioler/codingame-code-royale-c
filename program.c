#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define NUM_SITES_MAX 50

struct site_static;
struct game_static;
struct game;
struct site;
struct unit;

struct site_static {
	int site_id;
	int x;
	int y;
	int radius;
};

struct game_static {
	int num_sites;
	// Static data will not be passed to functions,
	// and there shall be exactly one global copy 
	// since the data is read-only, so a fixed size
	// array is used.
	struct site_static sites[NUM_SITES_MAX];
};

void load_game_static (struct game_static *);
void print_game_static (struct game_static *);

int main()
{
	struct game_static gs;
	load_game_static(&gs);
	print_game_static(&gs);
	
	// game loop
	while (1) {
		int gold;
		// -1 if none
		int touched_site;
		scanf("%d%d", &gold, &touched_site);
		for (int i = 0; i < gs.num_sites; i++) {
			int site_id;
			// used in future leagues
			int ignore_1;
			// used in future leagues
			int ignore_2;
			// -1 = No structure, 2 = Barracks
			int structure_type;
			// -1 = No structure, 0 = Friendly, 1 = Enemy
			int owner;
			int param_1;
			int param_2;
			scanf("%d%d%d%d%d%d%d", &site_id, &ignore_1, &ignore_2, &structure_type, &owner, &param_1, &param_2);
		}
		int num_units;
		scanf("%d", &num_units);
		for (int i = 0; i < num_units; i++) {
			int x;
			int y;
			int owner;
			// -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
			int unit_type;
			int health;
			scanf("%d%d%d%d%d", &x, &y, &owner, &unit_type, &health);
		}

		// Write an action using printf(). DON'T FORGET THE TRAILING \n
		// To debug: fprintf(stderr, "Debug messages...\n");


		// First line: A valid queen action
		// Second line: A set of training instructions
		printf("WAIT\n");
		printf("TRAIN\n");
	}

	return 0;
}

void load_game_static (struct game_static * gs) {
	scanf("%d", &gs->num_sites);
	struct site_static * s;
	for (int i = 0; i < gs->num_sites; i++) {
		s = &gs->sites[i];
		scanf("%d%d%d%d", &s->site_id, &s->x, &s->y, &s->radius);
	}
}

void print_game_static (struct game_static * gs) {
	fprintf(stderr, "num_sites: %d\n", gs->num_sites);
	fprintf(stderr, "%8s%8s%8s%8s\n", "site_id", "x", "y", "radius");
	struct site_static * s;
	for (int i = 0; i < gs->num_sites; i++) {
		s = &gs->sites[i];
		fprintf(stderr, "%8d%8d%8d%8d\n", s->site_id, s->x, s->y, s->radius);
	}
}
