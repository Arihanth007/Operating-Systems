#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define NUM_OF_SEMAPHORES 3
#define PERSON_NAME_SIZE 15
#define LAB_NAME_SIZE 15
#define PTHREAD_COND_SIZE sizeof(pthread_cond_t)
#define PTHREAD_MUTEX_SIZE sizeof(pthread_mutex_t)
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

typedef struct person
{
    char name[PERSON_NAME_SIZE];
    char sem_types[NUM_OF_SEMAPHORES];
    char fan_type;
    int num_sems;
    int id;
    int group_no;
    int reach_time;
    int patience_time;
    int num_goals;
} person;
person *people;

typedef struct arg_struct
{
    person arg_person;
    char zone;
} args;
args *team_args;

typedef struct team
{
    char team_type;
    int id;
    int num_chances;
    int *time_from_previous_chance;
    float *probability_goal;
} team;
team *teams;

pthread_t *t_people, *t_teams;
sem_t h_zone, a_zone, n_zone;
int h_capacity, a_capacity, n_capacity, spectating_time, num_people = 0;

// variables of the number of goals that each team has scored
int goals[2];
pthread_cond_t goals_changed[2];
pthread_mutex_t goals_lock[2];

// variables to keep track of the zones that a person is in: H,A,N,D(Didn't find any zone),E(Entrance),G(Gone)
char *person_in_zone;
pthread_mutex_t *person_zone_lock;
pthread_cond_t *person_moved;

int char_cmp(char *input_char, char check_char[])
{
    char temp[2];
    temp[0] = *(char *)input_char;
    temp[1] = '\0';
    // printf("String: %s\nOther: %s\n", temp, check_char);
    return strcmp(temp, check_char);
}

void *search_in_n_zone(void *input_person)
{
    args arguments = *(args *)input_person;
    person p_person = arguments.arg_person;

    int id = p_person.id, patience = p_person.patience_time;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += patience;

    // wait for a seat to be allocated in h_zone for the patience time
    int isPatience = sem_timedwait(&n_zone, &ts);
    pthread_mutex_lock(&person_zone_lock[id]);
    if (isPatience == -1 && errno == ETIMEDOUT)
    {
        // if the person was at the entrance till now, change the zone info so that he didn't get any zone
        if (char_cmp(&person_in_zone[id], "E") == 0)
            person_in_zone[id] = 'D';
    }
    else
    {
        // if the person was at the entrance or didn't get any seat from the other zone threads, allocate this zone
        if (char_cmp(&person_in_zone[id], "E") == 0 || char_cmp(&person_in_zone[id], "D") == 0)
        {
            person_in_zone[id] = 'N';
            printf(GRN "%s (%c) got a seat in zone N" RESET "\n", p_person.name, p_person.fan_type);
        }
        // if the person got allocated some zone, or left even, increment the semaphore since the person didn't use a seat
        else
            sem_post(&n_zone);
    }
    pthread_mutex_unlock(&person_zone_lock[id]);

    return NULL;
}

void *search_in_a_zone(void *input_person)
{
    args arguments = *(args *)input_person;
    person p_person = arguments.arg_person;

    int id = p_person.id, patience = p_person.patience_time;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += patience;

    // wait for a seat to be allocated in h_zone for the patience time
    int isPatience = sem_timedwait(&a_zone, &ts);
    pthread_mutex_lock(&person_zone_lock[id]);
    if (isPatience == -1 && errno == ETIMEDOUT)
    {
        // if the person was at the entrance till now, change the zone info so that he didn't get any zone
        if (char_cmp(&person_in_zone[id], "E") == 0)
            person_in_zone[id] = 'D';
    }
    else
    {

        // if the person was at the entrance or didn't get any seat from the other zone threads, allocate this zone
        if (char_cmp(&person_in_zone[id], "E") == 0 || char_cmp(&person_in_zone[id], "D") == 0)
        {
            person_in_zone[id] = 'A';
            printf(GRN "%s (%c) got a seat in zone A" RESET "\n", p_person.name, p_person.fan_type);
        }
        // if the person got allocated some zone, or left even, increment the semaphore since the person didn't use a seat
        else
            sem_post(&a_zone);
    }
    pthread_mutex_unlock(&person_zone_lock[id]);

    return NULL;
}

void *search_in_h_zone(void *input_person)
{
    args arguments = *(args *)input_person;
    person p_person = arguments.arg_person;

    int id = p_person.id, patience = p_person.patience_time;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += patience;

    // wait for a seat to be allocated in h_zone for the patience time
    int isPatience = sem_timedwait(&h_zone, &ts);
    pthread_mutex_lock(&person_zone_lock[id]);
    if (isPatience == -1 && errno == ETIMEDOUT) // patience runs out
    {
        // if the person was at the entrance till now,
        // change the zone info so that he didn't get any zone
        if (char_cmp(&person_in_zone[id], "E") == 0)
            person_in_zone[id] = 'D';
    }
    else
    {
        // if the person was at the entrance or didn't get any seat from the other zone threads, allocate this zone
        if (char_cmp(&person_in_zone[id], "E") == 0 || char_cmp(&person_in_zone[id], "D") == 0)
        {
            person_in_zone[id] = 'H';
            printf(GRN "%s (%c) got a seat in zone H" RESET "\n", p_person.name, p_person.fan_type);
        }
        // if the person got allocated some zone, or left even, increment the semaphore since the person didn't use a seat
        else
            sem_post(&h_zone);
    }
    pthread_mutex_unlock(&person_zone_lock[id]);

    return NULL;
}

void *simulate_person_in_game(void *input_person)
{
    args arguments = *(args *)input_person;
    person p_person = arguments.arg_person;

    int type_of_fan = -1;
    // if the person is a A fan, then look at the goals of the Home team
    if (char_cmp(&p_person.fan_type, "A") == 0)
        type_of_fan = 0;
    // if the person is a H fan, then look at the goals of the Away team
    else if (char_cmp(&p_person.fan_type, "H") == 0)
        type_of_fan = 1;
    // person is neutral, doesn't get enraged
    if (type_of_fan == -1)
        return NULL;

    pthread_mutex_lock(&goals_lock[type_of_fan]);
    // wait till the goals of the opposing team doesn't enrage
    while (goals[type_of_fan] < p_person.num_goals)
    {
        if (goals[type_of_fan] >= 0)
            pthread_cond_wait(&goals_changed[type_of_fan], &goals_lock[type_of_fan]);
        else
            break;
    }
    if (goals[type_of_fan] >= p_person.num_goals)
        printf(RED "%s got enraged and went to the exit" RESET "\n", p_person.name);
    pthread_mutex_unlock(&goals_lock[type_of_fan]);
    pthread_cond_broadcast(&person_moved[p_person.id]);

    return NULL;
}

void *simulate_person_seat(void *input_person)
{
    person p_person = *(person *)input_person;
    int id = p_person.id;

    pthread_t threads[NUM_OF_SEMAPHORES]; // n, a, h
    if (char_cmp(&p_person.fan_type, "N") == 0)
    {
        pthread_create(&threads[0], NULL, search_in_n_zone, &team_args[id]);
        pthread_join(threads[0], NULL);
        pthread_create(&threads[1], NULL, search_in_a_zone, &team_args[id]);
        pthread_join(threads[1], NULL);
        pthread_create(&threads[2], NULL, search_in_h_zone, &team_args[id]);
        pthread_join(threads[2], NULL);
    }
    else if (char_cmp(&p_person.fan_type, "A") == 0)
    {
        pthread_create(&threads[1], NULL, search_in_a_zone, &team_args[id]);
        pthread_join(threads[1], NULL);
    }
    else if (char_cmp(&p_person.fan_type, "H") == 0)
    {
        pthread_create(&threads[0], NULL, search_in_n_zone, &team_args[id]);
        pthread_join(threads[0], NULL);
        pthread_create(&threads[2], NULL, search_in_h_zone, &team_args[id]);
        pthread_join(threads[2], NULL);
    }

    return NULL;
}

void *simulate_person_enter_exit(void *input_person)
{
    args arguments = *(args *)input_person;
    person p_person = arguments.arg_person;
    int id = p_person.id, rt = p_person.reach_time;

    // person comes to the gate at time reach_time
    sleep(rt);
    printf(RED "%s has reached the stadium" RESET "\n", p_person.name);

    // run the thread where the person is simulated to wait for a seat in a zone and watch the match
    pthread_t thread;
    pthread_create(&thread, NULL, simulate_person_seat, &arguments);
    pthread_join(thread, NULL);

    // if the person did not find any seat
    int isFound_seat = 0;
    pthread_mutex_lock(&person_zone_lock[id]);
    if (char_cmp(&person_in_zone[id], "D") != 0 && char_cmp(&person_in_zone[id], "G") != 0)
        isFound_seat = 1;
    pthread_mutex_unlock(&person_zone_lock[id]);

    if (isFound_seat)
    {
        // set timespec for the spectating time
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        // person found a seat and is watching the game
        pthread_create(&thread, NULL, simulate_person_in_game, &arguments);

        pthread_mutex_lock(&person_zone_lock[id]);
        ts.tv_sec += spectating_time;
        // simulate_person will signal the conditional variable if the person wants to leave
        if (pthread_cond_timedwait(&person_moved[id], &person_zone_lock[id], &ts) == ETIMEDOUT)
            printf(YEL "%s watched the match for %d seconds and is leaving" RESET "\n", p_person.name, spectating_time);
        pthread_mutex_unlock(&person_zone_lock[id]);

        // Once the person wants to leave, increment the semaphore
        pthread_mutex_lock(&person_zone_lock[id]);
        if (char_cmp(&person_in_zone[id], "H") == 0)
            sem_post(&h_zone);
        else if (char_cmp(&person_in_zone[id], "A") == 0)
            sem_post(&a_zone);
        else if (char_cmp(&person_in_zone[id], "N") == 0)
            sem_post(&n_zone);
        person_in_zone[id] = 'G';
        pthread_mutex_unlock(&person_zone_lock[id]);

        printf(MAG "%s left the stadium" RESET "\n", p_person.name);
    }
    else
    {
        printf(BLU "%s did not find a seat in any of the zones" RESET "\n", p_person.name);
        printf(MAG "%s left the stadium" RESET "\n", p_person.name);
    }

    return NULL;
}

void *simulate_team(void *input_team)
{
    team p_team = *(team *)input_team;

    int i;
    for (i = 0; i < p_team.num_chances; i++)
    {
        // choose whether the team has scored a goal or not
        float x = p_team.probability_goal[i] - ((float)rand() / (float)(RAND_MAX / 1.0));
        int id = p_team.id;
        // sleep until the time elapses till the next chance to score a goal
        sleep(p_team.time_from_previous_chance[i]);

        if (x >= 0)
        {
            // team has scored a goal
            pthread_mutex_lock(&goals_lock[id]);
            goals[id]++;
            printf(CYN "Team %c has scored goal number %d" RESET "\n", p_team.team_type, goals[id]);
            pthread_mutex_unlock(&goals_lock[id]);
        }
        else
        {
            printf(CYN "Team %c missed their chance to score a goal" RESET "\n", p_team.team_type);
        }
        // broadcast after every goal scoring chance, even if the team did not score
        pthread_cond_broadcast(&goals_changed[id]);
    }

    return NULL;
}

void allocate_memory(int isDatastruct)
{
    if (isDatastruct == 1)
    {
        people = malloc(sizeof(person) * 1);
        team_args = malloc(sizeof(args) * 1);
        teams = malloc(sizeof(team) * 2);
        int i;
        for (i = 0; i < 2; i++)
        {
            teams[i].probability_goal = malloc(sizeof(float) * 1);
            teams[i].time_from_previous_chance = malloc(sizeof(int) * 1);
        }
    }
    else if (isDatastruct == 0)
    {
        person_moved = malloc(sizeof(pthread_cond_t) * num_people);
        person_in_zone = malloc(sizeof(char) * num_people);
        person_zone_lock = malloc(sizeof(pthread_mutex_t) * num_people);
        t_people = malloc(sizeof(pthread_t) * num_people);
        t_teams = malloc(sizeof(pthread_t) * 2);
    }
}

void take_input()
{
    int i, ii;
    int num_groups, num_in_group;

    scanf("%d %d %d", &h_capacity, &a_capacity, &n_capacity);
    scanf("%d", &spectating_time);
    scanf("%d", &num_groups);

    for (i = 1; i < num_groups + 1; i++)
    {
        scanf("%d", &num_in_group);

        for (ii = 0; ii < num_in_group; ii++)
        {
            char name[PERSON_NAME_SIZE], support_teams;
            int reach_time, patience, num_goals;
            people = realloc(people, sizeof(person) * (num_people + 1));
            team_args = realloc(team_args, sizeof(args) * (num_people + 1));

            scanf("%s %c %d %d %d", name, &support_teams, &reach_time, &patience, &num_goals);
            strcpy(people[num_people].name, name);
            people[num_people].fan_type = support_teams;
            people[num_people].reach_time = reach_time;
            people[num_people].patience_time = patience;
            people[num_people].num_goals = num_goals;
            people[num_people].id = num_people;
            people[num_people].group_no = i;
            team_args[num_people].arg_person = people[num_people];

            num_people++;
        }
    }

    int goal_scoring_chances;
    scanf("%d\n", &goal_scoring_chances);

    for (i = 0; i < goal_scoring_chances; i++)
    {
        int prev_time[2] = {0, 0}, inp_time, j = -1;
        char inp_team;
        scanf("%c", &inp_team);
        if (char_cmp(&inp_team, "H") == 0)
            j = 0;
        else if (char_cmp(&inp_team, "A") == 0)
            j = 1;

        if (j == -1)
        {
            printf("Incorrect Team Name Entered\n");
            exit(1);
        }

        teams[j].probability_goal = realloc(teams[j].probability_goal, sizeof(float) * (teams[j].num_chances + 1));
        teams[j].time_from_previous_chance = realloc(teams[j].time_from_previous_chance, sizeof(int) * (teams[j].num_chances + 1));

        int chances = teams[j].num_chances;
        teams[j].num_chances++;

        float goal_prob;
        scanf("%d %f\n", &inp_time, &goal_prob);
        teams[j].probability_goal[chances] = goal_prob;

        // store the time from the previous chance to goal
        teams[j].time_from_previous_chance[chances] = inp_time - prev_time[j];
        prev_time[j] = inp_time;
    }
}

int main(int argc, char *argv[])
{
    allocate_memory(1);
    teams[0].team_type = 'H';
    teams[1].team_type = 'A';
    int i;
    for (i = 0; i < 2; i++)
    {
        teams[i].id = i;
        teams[i].num_chances = 0;
    }
    take_input();
    allocate_memory(0);

    pthread_mutex_init(&goals_lock[0], NULL);
    pthread_mutex_init(&goals_lock[1], NULL);
    pthread_cond_init(&goals_changed[0], NULL);
    pthread_cond_init(&goals_changed[1], NULL);
    goals[0] = 0;
    goals[1] = 0;

    // initialize the semaphores corresponding to the number of seats in each zone
    sem_init(&h_zone, 0, h_capacity);
    sem_init(&a_zone, 0, a_capacity);
    sem_init(&n_zone, 0, n_capacity);

    for (i = 0; i < num_people; i++)
    {
        person_in_zone[i] = 'E';
        pthread_mutex_init(&person_zone_lock[i], NULL);
        pthread_cond_init(&person_moved[i], NULL);
    }

    // create threads
    pthread_create(&t_teams[0], NULL, simulate_team, &teams[0]);
    pthread_create(&t_teams[1], NULL, simulate_team, &teams[1]);
    for (i = 0; i < num_people; i++)
        pthread_create(&t_people[i], NULL, simulate_person_enter_exit, &team_args[i]);

    // wait for the threads to finish
    pthread_join(t_teams[0], NULL);
    pthread_join(t_teams[1], NULL);
    for (i = 0; i < num_people; i++)
        pthread_join(t_people[i], NULL);

    goals[0] = -1;
    pthread_cond_broadcast(&goals_changed[0]);
    pthread_cond_destroy(&goals_changed[0]);
    pthread_mutex_destroy(&goals_lock[0]);
    pthread_cancel(t_teams[0]);
    goals[1] = -1;
    pthread_cond_broadcast(&goals_changed[1]);
    pthread_cond_destroy(&goals_changed[1]);
    pthread_mutex_destroy(&goals_lock[1]);
    pthread_cancel(t_teams[1]);

    sem_destroy(&n_zone);
    sem_destroy(&a_zone);
    sem_destroy(&h_zone);

    return 0;
}
