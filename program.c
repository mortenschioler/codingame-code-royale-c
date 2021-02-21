#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define NUM_SITES_MAX 50
#define OWNER_FRIENDLY 0
#define NONE -1

struct site_static;
struct game_static;
struct game;
struct site;
struct unit;
struct command;

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

struct site {
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
};

struct unit {
	int x;
	int y;
	int owner;
	int unit_type;
	int health;
};

struct game {
	int gold;
	int touched_site;
	int num_units;
	struct site *sites;
	struct unit *units;
};

enum barracks_type {
	KNIGHT, ARCHER, GIANT
};

void load_game_static (struct game_static *);
void print_game_static (struct game_static *);
void print_game (struct game *);
void load_game(struct game *);
void load_sites(struct game *);
void load_site(struct site *);
void load_units(struct game *);
void load_unit(struct unit *);
void free_game(struct game *);
void copy_game (struct game * from, struct game * to);

int square(int n);
double distance(int x0, int y0, int x1, int x2);

double game_value(struct game *);
void simulate(struct game * g, char * cmd);
void candidates(struct game *, char ** cands);



struct game_static gs;

int main()
{

	load_game_static(&gs);
	print_game_static(&gs);

	// game loop
	while (1) {
		struct game game;
		load_game(&game);

		char * best_strategy = "WAIT\nTRAIN\n";
		char * cands[1000];
		candidates(&game, cands);
		double value = -1000000;
		double val;
		for (char ** c = cands; **c != '\0'; c++) {
			struct game sim;
			copy_game (&game, &sim);
			simulate(&sim, *c);
			val = game_value(&sim);
			if (val > value) {
				best_strategy = *c;
				value = val;
			}
		}
		printf("%s", best_strategy);

		print_game(&game);
		free_game(&game);
	}

	return 0;
}

void load_game_static (struct game_static *gs) {
	scanf("%d", &gs->num_sites);
	struct site_static * s;
	for (int i = 0; i < gs->num_sites; i++) {
		s = &gs->sites[i];
		scanf("%d%d%d%d", &s->site_id, &s->x, &s->y, &s->radius);
	}
}

void print_game_static (struct game_static *gs) {
	fprintf(stderr, "num_sites: %d\n", gs->num_sites);
	fprintf(stderr, "%8s%8s%8s%8s\n", "site_id", "x", "y", "radius");
	struct site_static * s;
	for (int i = 0; i < gs->num_sites; i++) {
		s = &gs->sites[i];
		fprintf(stderr, "%8d%8d%8d%8d\n", s->site_id, s->x, s->y, s->radius);
	}
}

void load_game (struct game *g) {
	scanf("%d%d", &g->gold, &g->touched_site);
	load_sites(g);
	load_units(g);
}

void load_sites(struct game *g) {
	struct site *sites;
	g->sites = sites = malloc(gs.num_sites * sizeof(struct site));
	for (int i = 0; i < gs.num_sites; i++) {
		load_site(sites++);
	}
}

void load_site(struct site *s) {
	scanf("%d%d%d%d%d%d%d",
			&s->site_id,
			&s->ignore_1,
			&s->ignore_2,
			&s->structure_type,
			&s->owner,
			&s->param_1,
			&s->param_2);
}

void load_units(struct game *g) {
	struct unit *units;
	int num_units;
	scanf("%d", &num_units);
	g->num_units = num_units;
	g->units = units = malloc(num_units * sizeof(struct unit));
	for (int i = 0; i < num_units; i++) {
		load_unit(units++);
	}
}

void load_unit(struct unit *u) {
	scanf("%d%d%d%d%d", &u->x, &u->y, &u->owner, &u->unit_type, &u->health);
}

void free_game(struct game *g) {
	free(g->sites);
	free(g->units);
}

void print_player_info(struct game *g) {
	fprintf(stderr, "Game:\nGold=%d, touched_site=%d, num_units=%d\n", g->gold, g->touched_site, g->num_units);
}

void print_sites (struct game *g) {
	fprintf(stderr, "site_id\tstructure_type\towner\tparam_1\tparam_2\n");
	struct site *s;
	for (int i = 0; i < gs.num_sites; i++) {
		s = &g->sites[i];
		fprintf(stderr, "%d\t%d\t%d\t%d\t%d\n", s->site_id, s->structure_type, s->owner, s->param_1, s->param_2);
	}
}

void print_units (struct game *g) {
	fprintf(stderr, "x\ty\towner\tunit_type\thealth\n");
	struct unit u;
	for (int i = 0; i < g->num_units; i++) {
		u = g->units[i];
		fprintf(stderr, "%d\t%d\t%d\t%d\t%d\n", u.x, u.y, u.owner, u.unit_type, u.health);
	}
}

// print turn-specific information
void print_game (struct game *g) {
	print_player_info(g);
	print_sites (g);
	print_units (g);
}

int square (int n) {
	return n * n;
}

double distance (int x0, int y0, int x1, int y1) {
	return sqrt(square(x1 - x0) + square(y1 - y0));
}

int count_sites (struct game * g, int (*f)(struct site)) {
	int acc = 0;
	for (int i = 0; i < gs.num_sites; i++) {
		acc += (*f)(g->sites[i]);
	}
	return acc;
}

int site_weight (struct site s) {
	return (s.structure_type != NONE) * (s.owner == OWNER_FRIENDLY);
}

char * as_word (enum barracks_type type) {
	switch (type) {
		case KNIGHT: return "KNIGHT";
		case ARCHER: return "ARCHER";
		case GIANT: return "GIANT";
	}
}

enum barracks_type find_barracks_type (char * cmd) {
	if (strstr(cmd, "KNIGHT")) return KNIGHT;
	if (strstr(cmd, "ARCHER")) return ARCHER;
	// FIXME bad fallthrough (maybe get rid of enum)
	return GIANT;
}

void copy_game (struct game * from, struct game * to) {
	*to = *from;
}

double game_value(struct game * g) {
	return count_sites(g, site_weight);
}

int parse_site_id (char * s) {
	int n1, n2;
	n1 = *s - '0';
	n2 = *(++s);
	if (n2 == ' ') {
		return n1;
	} else {
		return 10*n1 + n2;
	}
}

void simulate(struct game * g, char * cmd) {
	if (strncmp ("BUILD ", cmd, 6) == 0) {
		int site_id = parse_site_id(cmd + 6 * sizeof (char));
		g->sites[site_id].structure_type = 1;
	}
}

void candidates(struct game * g, char ** cands) {
	int i = 0;
	char * str = "BUILD 0 BARRACKS-KNIGHT\nTRAIN\n";
	for (int site_id = 0; site_id < 10; site_id++) {
		str[6] = site_id - '0';
		cands[i++] = str;
	}

	cands[0] = str;
	cands[1] = "";
	cands[0] = "";
}
