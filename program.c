#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>

#define NUM_SITES_MAX 50
#define OWNER_FRIENDLY 0
#define NONE -1
#define STRUCTURE_TYPE_TOWER 1
#define STRUCTURE_TYPE_BARRACKS 2
#define CREEP_TYPE_KNIGHT 0
#define CREEP_TYPE_ARCHER 1
#define CREEP_TYPE_GIANT 2
#define CMD_TYPE_WAIT 0
#define CMD_TYPE_MOVE 1
#define CMD_TYPE_BUILD 2

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
void candidates(const struct game *, char * cands);

struct game_static gs;

long ms_epoch () {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    unsigned long long millisecondsSinceEpoch =
    (unsigned long long)(tv.tv_sec) * 1000 +
    (unsigned long long)(tv.tv_usec) / 1000;
    return millisecondsSinceEpoch;
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

void copy_game (struct game * from, struct game * to) {
    to->gold = from->gold;
    to->num_units = from->num_units;
    to->touched_site = from->touched_site;
    to->sites = malloc(gs.num_sites * sizeof(struct site));
    for (int i=0;i<gs.num_sites;i++) to->sites[i] = from->sites[i];
    to->units = malloc(from->num_units * sizeof(struct unit));
    for (int i=0;i<from->num_units;i++) to->units[i] = from->units[i];
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

double game_value(struct game * g) {
	return count_sites(g, site_weight);
}

int cmd_type(char * cmd) {
    switch (*cmd) {
        case 'W': return CMD_TYPE_WAIT;
        case 'M': return CMD_TYPE_MOVE;
        case 'B': return CMD_TYPE_BUILD;
        default: fprintf(stderr, "CMD TYPE ERROR\n"); exit(1);
    }
}

int parse_decimal (char * s) {
    // supports string of one or two digits. The string needs not be
    // null-terminated.
	return 10*(s[0] - '0') + (s[1] - '0');
}

char dig(int n) {
    // n must be within range [0-9].
    return n + '0';
}

void simulate(struct game * g, char * cmd) {
    // hack: Only value build commands and assume they
    // are immediately constructed
	if (cmd_type(cmd) == CMD_TYPE_BUILD) {
		int site_id = parse_decimal(&cmd[6]);
		g->sites[site_id].structure_type = STRUCTURE_TYPE_BARRACKS;
        g->sites[site_id].param_2 = CREEP_TYPE_KNIGHT;
        g->sites[site_id].owner = OWNER_FRIENDLY;
	}
}

char * base_build_cmd_str (int structure_type, int creep_type) {
    switch (structure_type) {
        case (STRUCTURE_TYPE_BARRACKS):
        switch (creep_type) {
            case CREEP_TYPE_KNIGHT: return "BUILD 00 BARRACKS-KNIGHT\n";
            case CREEP_TYPE_ARCHER: return "BUILD 00 BARRACKS-ARCHER\n";
            case CREEP_TYPE_GIANT: return "BUILD 00 BARRACKS-GIANT\n";
            default: exit(1);
        }
        case (STRUCTURE_TYPE_TOWER): return "BUILD 00 TOWER\n";
        default: exit(1);
    }
}

void format_decimal (char * dest, int n) {
    // Takes a pointer to two consecutive characters
    // and overwrites them with the ten-part and the one-part of n
    int ones, tens;
    tens = n / 10;
    ones = n % 10;
    dest[0] = dig(tens);
    dest[1] = dig(ones);
}

void build_cmd (char * dest, int site_id, int structure_type, int creep_type) {
    char * base_str = base_build_cmd_str(structure_type, creep_type);
    strcpy(dest, base_str);
    format_decimal(&dest[6], site_id);
}

void candidates(const struct game * g, char * cands) {
    char * cmd = cands;
    // For each site, build each building
    for (int i = 0; i < gs.num_sites; i++) {
        build_cmd(cmd, i, STRUCTURE_TYPE_BARRACKS, CREEP_TYPE_KNIGHT);
        strcat(cmd, "TRAIN\n");
        // move to end of string
        while(*(cmd++));
    }
}

int main()
{

	load_game_static(&gs);
	print_game_static(&gs);

    char cands[10000];

	// game loop
	while (1) {
        long t1,t2;

		struct game game;
		load_game(&game);
        // print_game(&game);

        t1 = ms_epoch();

		char * best_strategy = "WAIT\nTRAIN\n";
        double best_value = game_value(&game);
        
        candidates(&game, cands);
        
		for (char * s = cands; *s != '\0'; ) {
            struct game game2;
            fprintf(stderr, "Copying game... \n");
            copy_game(&game, &game2);
            fprintf(stderr, "Simulating... \n");
            simulate(&game2, s);
            fprintf(stderr, "Calculating game value...\n");
            double value = game_value(&game2);
            fprintf(stderr, "Simulated value of %s is %f.\n", s, value);

            if(value >= best_value) {
                fprintf(stderr, "New best strategy found.\n");
                best_value = value;
                best_strategy = s;
            }
            free_game(&game2);
            while(*s++ != '\0');
        }

        t2 = ms_epoch();

        fprintf(stderr, "Elapsed time for game round: %d ms\n", t2-t1);
		fprintf(stdout,"%s", best_strategy);

		free_game(&game);
	}

	return 0;
}