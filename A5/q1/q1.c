#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define NUM_COURSES_PER_STUDENT 3
#define COURSE_NAME_SIZE 15
#define LAB_NAME_SIZE 15
#define PTHREAD_COND_SIZE sizeof(pthread_cond_t)
#define PTHREAD_MUTEX_SIZE sizeof(pthread_mutex_t)
#define OCCUPIED 0
#define FREE 1
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

pthread_t *t_courses = NULL, *t_students = NULL;

// student_status is the status of a student
// if >= 0, it is the course where the student is attending a tutorial or has registered
// -i if the student is waiting for course i to be allocated
int *student_status;
pthread_cond_t *student_status_available;
pthread_mutex_t *student_status_lock;

// courses_spots is the number of available spots in a course after it is allocated a TA
// -1 if it has been withdrawn
int *courses_spots;
pthread_cond_t *courses_spots_available;
pthread_mutex_t *courses_spots_lock;

// lab_tas has the information about the remaining courses a TA can take and if the TA is busy
int ***lab_tas;
pthread_cond_t **lab_tas_available;
pthread_mutex_t **lab_tas_lock;
typedef struct student
{
    int id;
    int preferences[NUM_COURSES_PER_STUDENT];
    float calibre;
    int fill_time;
} student;

typedef struct course
{
    int id;
    int *labs;
    float interest;
    int num_labs;
    int course_max_slots;
    char name[LAB_NAME_SIZE];
} course;

typedef struct lab
{
    int id;
    int num_tas;
    int max_courses;
    char name[COURSE_NAME_SIZE];
} lab;

int num_students, num_labs, num_courses;
student *students;
course *courses;
lab *rlabs;

void student_waitfor_ta(int lab_id, int ta_num)
{
    while (lab_tas[lab_id][ta_num][1] < FREE)
    {
        if (lab_tas[lab_id][ta_num][0] < 1)
            break;
        else
            pthread_cond_wait(&lab_tas_available[lab_id][ta_num], &lab_tas_lock[lab_id][ta_num]);
    }
}

void *simulate_course(void *args)
{
    int chosen_ta_lab_id, chosen_ta_id;
    course p_course = *(course *)args;

    for (;;)
    {
        chosen_ta_lab_id = -1;
        chosen_ta_id = -1;
        // go through each of the eligible labs
        int lab_no, num_labs = p_course.num_labs;
        int i;
        for (lab_no = 0; lab_no < num_labs; lab_no++)
        {
            int lab_id = p_course.labs[lab_no];
            // go through each of the TAs in the lab
            int ta_num;
            for (ta_num = 0; ta_num < rlabs[lab_id].num_tas; ta_num++)
            {
                int isFoundTA;
                pthread_mutex_lock(&lab_tas_lock[lab_id][ta_num]);
                student_waitfor_ta(lab_id, ta_num);
                // pick a TA for the course
                isFoundTA = 0;
                if (lab_tas[lab_id][ta_num][1] > OCCUPIED && lab_tas[lab_id][ta_num][0] > 0)
                {
                    chosen_ta_lab_id = lab_id;
                    chosen_ta_id = ta_num;
                    lab_tas[lab_id][ta_num][1] = OCCUPIED; // the ta is now busy conducting a tutorial here
                    lab_tas[lab_id][ta_num][0]--;          // the ta used up a course
                    isFoundTA = 1;
                }
                pthread_mutex_unlock(&lab_tas_lock[lab_id][ta_num]);

                // stop going through the tas if you chose one already
                if (isFoundTA)
                    break;
            }
            // stop going through the tas if you chose one already
            if (chosen_ta_id > -1)
                break;
        }

        int id = p_course.id;
        // no ta was found for this course, the course is now removed
        if (chosen_ta_id < 0)
        {
            pthread_mutex_lock(&courses_spots_lock[id]);
            courses_spots[id] = -1;
            pthread_mutex_unlock(&courses_spots_lock[id]);
            pthread_cond_broadcast(&courses_spots_available[id]);
            printf(GRN "Course %s does not have any TA's eligible and is removed from course offerings\n" RESET, p_course.name);
            printf(GRN "Course thread %s has ended\n" RESET, p_course.name);
            return NULL;
        }
        char name[LAB_NAME_SIZE];
        strcpy(name, rlabs[chosen_ta_lab_id].name);
        // ta was found for this course
        pthread_mutex_lock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);
        int taship_num = rlabs[chosen_ta_lab_id].max_courses - lab_tas[chosen_ta_lab_id][chosen_ta_id][0];
        printf(RED "TA %d from lab %s has been allocated to course %s for his %dst TA ship\n" RESET, chosen_ta_id, name, p_course.name, taship_num);
        pthread_mutex_unlock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);

        int tut_slots = (rand() % p_course.course_max_slots) + 1;
        int left_spots_in_lab = 0;
        for (i = 0; i < rlabs[chosen_ta_lab_id].num_tas; i++)
        {
            pthread_mutex_lock(&lab_tas_lock[chosen_ta_lab_id][i]);
            left_spots_in_lab = left_spots_in_lab + lab_tas[chosen_ta_lab_id][i][0];
            pthread_mutex_unlock(&lab_tas_lock[chosen_ta_lab_id][i]);
        }
        if (left_spots_in_lab == 0)
            printf(RED "Lab %s no longer has mentors available for TAship\n" RESET, name);

        // open tut_slots slots for the tutorial
        pthread_mutex_lock(&courses_spots_lock[id]);
        courses_spots[id] = tut_slots;
        pthread_mutex_unlock(&courses_spots_lock[id]);

        printf(GRN "Course %s has been allocated %d seats\n" RESET, p_course.name, tut_slots);

        int filled_seats = 0;
        int waiting_students = tut_slots;
        while ((filled_seats < tut_slots && waiting_students != 0) || (filled_seats < 1))
        {
            pthread_cond_broadcast(&courses_spots_available[id]);

            int oneindex_id = id + 1;
            waiting_students = 0;
            filled_seats = 0;
            int i;
            for (i = 0; i < num_students; i++)
            {
                pthread_mutex_lock(&student_status_lock[i]);
                if (student_status[i] == oneindex_id)
                    filled_seats++;
                if (student_status[i] == -1 * oneindex_id)
                    waiting_students++;
                pthread_mutex_unlock(&student_status_lock[i]);
            }

            if (waiting_students == 0)
                break; // if noone is waiting, stop
        }

        filled_seats = 0;
        int oneindex_id = id + 1;
        for (i = 0; i < num_students; i++)
        {
            pthread_mutex_lock(&student_status_lock[i]);
            if (student_status[i] == oneindex_id)
                filled_seats++;
            pthread_mutex_unlock(&student_status_lock[i]);
        }

        printf(RED "TA %d has started tutorial for Course %s with %d seats filled out of %d\n" RESET, chosen_ta_id, p_course.name, filled_seats, tut_slots);

        pthread_mutex_lock(&courses_spots_lock[id]);
        courses_spots[id] = 0;
        pthread_mutex_unlock(&courses_spots_lock[id]);

        sleep(3); // the ta is conducting a tutorial

        for (i = 0; i < num_students; i++)
        {
            pthread_mutex_lock(&student_status_lock[i]);
            if (student_status[i] == oneindex_id)
            {
                student_status[i] = 0;
                pthread_cond_broadcast(&student_status_available[i]);
            }
            pthread_mutex_unlock(&student_status_lock[i]);
        }

        printf(RED "TA %d from lab %s has completed the tutorial and left the course %s\n" RESET, chosen_ta_id, name, p_course.name);

        pthread_mutex_lock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);
        // Ta is free
        lab_tas[chosen_ta_lab_id][chosen_ta_id][1] = FREE;
        pthread_cond_broadcast(&lab_tas_available[chosen_ta_lab_id][chosen_ta_id]);
        pthread_mutex_unlock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);
    }

    printf(GRN "Course thread %s has ended\n" RESET, p_course.name);

    return NULL;
}

void *simulate_student(void *args)
{
    student p_student = *(student *)args;

    sleep(p_student.fill_time);
    printf(BLU "Student %d has filled in preferences for course registration\n" RESET, p_student.id);

    int isFoundCourse = 0;
    int current_pref_course = -1; // course id of the currently preferred course
    int current_pref = -1;        // preference number - 0,1,2
    int i;
    for (i = 0; i < 3; i++)
    {
        if (isFoundCourse)
            break;
        pthread_mutex_lock(&courses_spots_lock[p_student.preferences[i]]);
        if (courses_spots[p_student.preferences[i]] != -1)
        {
            current_pref_course = courses_spots[p_student.preferences[i]];
            current_pref = i;
            isFoundCourse = 1;
        }
        pthread_mutex_unlock(&courses_spots_lock[p_student.preferences[i]]);
    }

    // student has a course to take up
    while (current_pref >= 0)
    {
        int one_indexed_pref = current_pref_course + 1, id = p_student.id;
        pthread_mutex_lock(&student_status_lock[id]);
        student_status[id] = -1 * one_indexed_pref; // student_status has -pref_course, meaning the student is waiting
        pthread_mutex_unlock(&student_status_lock[id]);

        int course_withdrawn = 0, isWaiting = 0;
        pthread_mutex_lock(&courses_spots_lock[current_pref_course]);
        isWaiting = 1;
        while (courses_spots[current_pref_course] == 0)
            pthread_cond_wait(&courses_spots_available[current_pref_course], &courses_spots_lock[current_pref_course]);
        isWaiting = 0;
        if (courses_spots[current_pref_course] <= 0)
            course_withdrawn = 1;
        else
            courses_spots[current_pref_course]--;
        pthread_mutex_unlock(&courses_spots_lock[current_pref_course]);

        if (!course_withdrawn)
        {
            printf(BLU "Student %d has been allocated a seat in course %s\n" RESET, id, courses[current_pref_course].name);

            one_indexed_pref = current_pref_course + 1;
            pthread_mutex_lock(&student_status_lock[id]);
            student_status[id] = one_indexed_pref;
            pthread_mutex_unlock(&student_status_lock[id]);

            srand((unsigned int)time(NULL));
            float a = 1.0;
            float x = ((float)rand() / (float)(RAND_MAX)) * a;
            float withdraw_probability = courses[current_pref_course].interest * p_student.calibre;

            pthread_mutex_lock(&student_status_lock[id]);
            pthread_cond_wait(&student_status_available[id], &student_status_lock[id]);
            pthread_mutex_unlock(&student_status_lock[id]);

            // choose to finalize or withdraw from course
            if (x >= withdraw_probability)
            {
                printf(BLU "Student %d has withdrawn from course %s\n" RESET, id, courses[current_pref_course].name);
                course_withdrawn = 1;
            }
            else
            {
                printf(BLU "Student %d has selected course %s permanently\n" RESET, id, courses[current_pref_course].name);
                pthread_mutex_lock(&student_status_lock[id]);
                student_status[id] = 0;
                pthread_mutex_unlock(&student_status_lock[id]);
                printf(BLU "Student thread %d has ended\n" RESET, id);
                return NULL;
            }
        }
        if (course_withdrawn)
        {
            one_indexed_pref = current_pref_course + 1;
            int previous_pref_course = current_pref_course;
            // get next course if present course isn't available
            for (i = one_indexed_pref; i < 3; i++)
            {
                int pref = p_student.preferences[i];
                pthread_mutex_lock(&courses_spots_lock[pref]);
                if (courses_spots[pref] != -1)
                {
                    current_pref_course = courses_spots[pref];
                    current_pref = i;
                }
                pthread_mutex_unlock(&courses_spots_lock[pref]);
            }
            if (previous_pref_course != current_pref_course)
                printf(BLU "Student %d has changed current preference from %s to %s\n" RESET, id, courses[previous_pref_course].name, courses[current_pref_course].name);
            else
            {
                isFoundCourse = 0;
                current_pref = -1;
            }
        }
    }

    if (!isFoundCourse)
        printf(BLU "Student %d did not get any of their preferred courses\n" RESET, p_student.id);

    printf(BLU "Student thread %d has ended\n" RESET, p_student.id);

    return NULL;
}

void take_input()
{
    scanf("%d %d %d", &num_students, &num_labs, &num_courses);

    students = malloc(sizeof(student) * num_students);
    rlabs = malloc(sizeof(lab) * num_labs);
    courses = malloc(sizeof(course) * num_courses);

    // taking all the inputs for the courses
    int i;
    for (i = 0; i < num_courses; i++)
    {
        char name[COURSE_NAME_SIZE];
        float interest;
        int max_slots, no_of_labs;
        scanf("%s %f %d %d", &name, &interest, &max_slots, &no_of_labs);
        strcpy(courses[i].name, name);
        courses[i].interest = interest;
        courses[i].course_max_slots = max_slots;
        courses[i].num_labs = no_of_labs;
        courses[i].id = i;
        courses[i].labs = malloc(sizeof(int) * courses[i].num_labs);
        int ii;
        for (ii = 0; ii < courses[i].num_labs; ii++)
            scanf("%d", &courses[i].labs[ii]);
    }

    // taking all the inputs for the students
    for (i = 0; i < num_students; i++)
    {
        float calibre;
        int pref1, pref2, pref3, start_time;
        scanf("%f %d %d %d %d", &calibre, &pref1, &pref2, &pref3, &start_time);
        students[i].calibre = calibre;
        students[i].preferences[0] = pref1;
        students[i].preferences[1] = pref2;
        students[i].preferences[2] = pref3;
        students[i].fill_time = start_time;
        students[i].id = i;
    }

    // taking all the inputs for the labs
    for (i = 0; i < num_labs; i++)
    {
        char name[LAB_NAME_SIZE];
        int no_of_tas, max_courses;
        // scanf("%s %d %d", &rlabs[i].name, &rlabs[i].num_tas, &rlabs[i].max_courses);
        scanf("%s %d %d", &name, &no_of_tas, &max_courses);
        strcpy(rlabs[i].name, name);
        rlabs[i].num_tas = no_of_tas;
        rlabs[i].max_courses = max_courses;
        rlabs[i].id = i;
    }
}

void init_courses()
{
    int i;
    courses_spots = malloc(sizeof(int) * num_courses);
    courses_spots_available = malloc(PTHREAD_COND_SIZE * num_courses);
    courses_spots_lock = malloc(PTHREAD_MUTEX_SIZE * num_courses);
    for (i = 0; i < num_courses; i++)
    {
        courses_spots[i] = 0;
        pthread_cond_init(&courses_spots_available[i], NULL);
        pthread_mutex_init(&courses_spots_lock[i], NULL);
    }
}

void init_labTAs()
{
    int i;
    lab_tas = malloc(sizeof(int **) * num_labs);
    lab_tas_available = malloc(sizeof(pthread_cond_t *) * num_labs);
    lab_tas_lock = malloc(sizeof(pthread_mutex_t *) * num_labs);
    for (i = 0; i < num_labs; i++)
    {
        int ii;
        lab_tas[i] = malloc(sizeof(int *) * rlabs[i].num_tas);
        lab_tas_available[i] = malloc(PTHREAD_COND_SIZE * rlabs[i].num_tas);
        lab_tas_lock[i] = malloc(PTHREAD_MUTEX_SIZE * rlabs[i].num_tas);
        int alloc_size = 2 * sizeof(int);
        for (ii = 0; ii < rlabs[i].num_tas; ii++)
        {
            lab_tas[i][ii] = malloc(alloc_size);
            lab_tas[i][ii][1] = FREE;
            lab_tas[i][ii][0] = rlabs[i].max_courses;
            pthread_cond_init(&lab_tas_available[i][ii], NULL);
            pthread_mutex_init(&lab_tas_lock[i][ii], NULL);
        }
    }
}

void init_students()
{
    int i;
    student_status = malloc(sizeof(int) * num_students);
    student_status_available = malloc(PTHREAD_COND_SIZE * num_students);
    student_status_lock = malloc(PTHREAD_MUTEX_SIZE * num_students);
    for (i = 0; i < num_students; i++)
    {
        pthread_cond_init(&student_status_available[i], NULL);
        pthread_mutex_init(&student_status_lock[i], NULL);
        student_status[i] = -1 * (students[i].preferences[0] + 1);
    }
}

int main(int argc, char *argv[])
{
    int i;
    take_input();

    init_courses();
    init_labTAs();
    init_students();

    // allocate space and create threads
    t_students = malloc(sizeof(pthread_t) * num_students);
    for (i = 0; i < num_students; i++)
        pthread_create(&t_students[i], NULL, simulate_student, &students[i]);
    t_courses = malloc(sizeof(pthread_t) * num_courses);
    for (i = 0; i < num_courses; i++)
        pthread_create(&t_courses[i], NULL, simulate_course, &courses[i]);

    // wait for all threads to finish
    for (i = 0; i < num_students; i++)
        pthread_join(t_students[i], NULL);
    for (i = 0; i < num_courses; i++)
        pthread_join(t_courses[i], NULL);

    return 0;
}