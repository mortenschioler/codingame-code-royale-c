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
#define CMD_TRAIN_SITE_ID_INDEX 6

#define ERROR	40000
#define WARN	30000
#define INFO	20000
#define DEBUG	10000
#define LOG_LEVEL DEBUG
#define log(LEVEL,...) \
            if (LEVEL >= LOG_LEVEL) fprintf(stderr, __VA_ARGS__);

typedef int log_level;

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
struct game_static gs;

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

int square(int n);
double distance(int x0, int y0, int x1, int x2);

double game_value(struct game *);
void simulate(struct game * g, char * cmd);
void candidates(const struct game *, char * cands);

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

void print_game_static (struct game_static *gs, log_level l) {
	log(l,"num_sites: %d\n", gs->num_sites);
	log(l,"%8s%8s%8s%8s\n", "site_id", "x", "y", "radius");
	struct site_static * s;
	for (int i = 0; i < gs->num_sites; i++) {
		s = &gs->sites[i];
		log(l,"%8d%8d%8d%8d\n", s->site_id, s->x, s->y, s->radius);
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

void load_sites(struct game *g) {
	struct site *sites;
	g->sites = sites = malloc(gs.num_sites * sizeof(struct site));
	for (int i = 0; i < gs.num_sites; i++) {
		load_site(sites++);
	}
}

void load_unit(struct unit *u) {
	scanf("%d%d%d%d%d", &u->x, &u->y, &u->owner, &u->unit_type, &u->health);
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

void load_game (struct game *g) {
	scanf("%d%d", &g->gold, &g->touched_site);
	load_sites(g);
	load_units(g);
}


void free_game(struct game *g) {
	free(g->sites);
	free(g->units);
}

void print_player_info(struct game *g, log_level l) {
	log(l,"Game:\nGold=%d, touched_site=%d, num_units=%d\n", g->gold, g->touched_site, g->num_units);
}

void print_sites (struct game *g, log_level l) {
	log(l,"site_id\tstructure_type\towner\tparam_1\tparam_2%c" , '\n');
	struct site *s;
	for (int i = 0; i < gs.num_sites; i++) {
		s = &g->sites[i];
		log(l,"%d\t%d\t%d\t%d\t%d\n", s->site_id, s->structure_type, s->owner, s->param_1, s->param_2);
	}
}

void print_units (struct game *g, log_level l) {
	log(l,"x\ty\towner\tunit_type\thealth%c", '\n');
	struct unit u;
	for (int i = 0; i < g->num_units; i++) {
		u = g->units[i];
		log(l,"%d\t%d\t%d\t%d\t%d\n", u.x, u.y, u.owner, u.unit_type, u.health);
	}
}

// print turn-specific information
void print_game (struct game *g, log_level l) {
	print_player_info(g, l);
	print_sites (g, l);
	print_units (g, l);
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
        default: log(ERROR,"%s", "CMD TYPE ERROR\n"); exit(1);
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

void panic_fallthrough() {
	log(ERROR,"%s", "PANIC: Fallthrough");
	exit(1);
}

char * base_build_cmd_str (int structure_type, int creep_type) {
    switch (structure_type) {
        case (STRUCTURE_TYPE_BARRACKS):
        switch (creep_type) {
            case CREEP_TYPE_KNIGHT: return "BUILD 00 BARRACKS-KNIGHT\nTRAIN\n";
            case CREEP_TYPE_ARCHER: return "BUILD 00 BARRACKS-ARCHER\nTRAIN\n";
            case CREEP_TYPE_GIANT: return "BUILD 00 BARRACKS-GIANT\nTRAIN\n";
            default: panic_fallthrough();
        }
        case (STRUCTURE_TYPE_TOWER): return "BUILD 00 TOWER\nTRAIN\n";
        default: panic_fallthrough();
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

void set_site_id (char * cmd, int site_id) {
	format_decimal(&cmd[CMD_TRAIN_SITE_ID_INDEX], site_id);
}

void build_cmd (char * dest, int site_id, int structure_type, int creep_type) {
    char * base_str = base_build_cmd_str(structure_type, creep_type);
    strcpy(dest, base_str);
    set_site_id(dest, site_id);
}

void candidates(const struct game * g, char * cmd) {
    // For each site, build each building
    for (int site_id = 0; site_id < gs.num_sites; site_id++) {
        build_cmd(cmd, site_id, STRUCTURE_TYPE_BARRACKS, CREEP_TYPE_KNIGHT);
        while(*cmd++);
		build_cmd(cmd, site_id, STRUCTURE_TYPE_BARRACKS, CREEP_TYPE_ARCHER);
        while(*cmd++);
		build_cmd(cmd, site_id, STRUCTURE_TYPE_BARRACKS, CREEP_TYPE_GIANT);
        while(*cmd++);
		build_cmd(cmd, site_id, STRUCTURE_TYPE_TOWER, NONE);
        while(*cmd++);
    }
}

int main()
{

	load_game_static(&gs);
	print_game_static(&gs, DEBUG);

    char cands[100000];
	// game loop
	while (1) {
        long t1,t2;

		struct game game;
		load_game(&game);
        // print_game(&game, DEBUG);

        t1 = ms_epoch();

		char * best_strategy = "WAIT\nTRAIN\n";
        double best_value = game_value(&game);
        
        candidates(&game, cands);
        
    	struct game game2;
		for (char * cmd = cands; *cmd != '\0'; ) {
            log(DEBUG,"Copying game...%c", ' ');
            copy_game(&game, &game2);
            log(DEBUG,"Simulating...%c", ' ');
            simulate(&game2, cmd);
            log(DEBUG,"Calculating game value...%c", ' ');
            double value = game_value(&game2);
			log(DEBUG,"Done. \n Simulated value of [%s] is %f ", cmd, value);
            if(value > best_value) {
                log(DEBUG,"> %f. New best strategy found.\n", best_value);
                best_value = value;
                best_strategy = cmd;
            } else {
				log(DEBUG,"<= %f. Strategy is rejected.\n", best_value);
			}
            free_game(&game2);
            while(*cmd++ != '\0');
        }

        t2 = ms_epoch();

        log(DEBUG,"Elapsed time for game round: %ld ms\n", t2-t1);
		printf("%s", best_strategy);

		free_game(&game);
	}

	return 0;
}