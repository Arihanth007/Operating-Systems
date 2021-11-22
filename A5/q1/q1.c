#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#define NUM_COURSES_PER_STUDENT 3
#define COURSE_NAME_SIZE 15
#define LAB_NAME_SIZE 15

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
lab *rlabs;
course *courses;

pthread_t *t_courses = NULL, *t_students = NULL;

// lab_tas has the information about the remaining courses a TA can take and if the TA is busy
pthread_mutex_t **lab_tas_lock;
pthread_cond_t **lab_tas_available;
int ***lab_tas;

// courses_spots is the number of available spots in a course after it is allocated a TA
// -1 if it has been withdrawn
pthread_mutex_t *courses_spots_lock;
pthread_cond_t *courses_spots_available;
int *courses_spots;

// student_status is the status of a student
// if >= 0, it is the course where the student is attending a tutorial or has registered
// -i if the student is waiting for course i to be allocated
int *student_status;
pthread_mutex_t *student_status_lock;
pthread_cond_t *student_status_available;

void *simulate_student(void *args)
{
    student p_student = *(student *)args;

    sleep(p_student.fill_time);
    printf("Student %d has filled in preferences for course registration\n", p_student.id);

    int current_pref = -1;        // preference number - 0,1,2
    int current_pref_course = -1; // course id of the currently preferred course
    int i;
    for (i = 0; i < 3; i++)
    {
        pthread_mutex_lock(&courses_spots_lock[p_student.preferences[i]]);
        if (courses_spots[p_student.preferences[i]] != -1 && current_pref_course == -1)
        {
            current_pref = i;
            current_pref_course = courses_spots[p_student.preferences[i]];
        }
        pthread_mutex_unlock(&courses_spots_lock[p_student.preferences[i]]);
    }

    while (current_pref > -1)
    {

        // printf("Student %d is looking for course %s\n", p_student.id, courses[current_pref_course].name);

        int course_withdrawn = 0;

        pthread_mutex_lock(&student_status_lock[p_student.id]);
        student_status[p_student.id] = -1 * (current_pref_course + 1); // student_status has -pref_course, meaning the student is waiting
        pthread_mutex_unlock(&student_status_lock[p_student.id]);

        pthread_mutex_lock(&courses_spots_lock[current_pref_course]);
        // wait for spots in the course to open up if there are no spots
        while (courses_spots[current_pref_course] == 0)
        {
            pthread_cond_wait(&courses_spots_available[current_pref_course], &courses_spots_lock[current_pref_course]);
        }
        // if this thread wakes up and the course is withdrawn, save the bool course_withdrawn
        // if the thread wakes up because of spots being available
        if (courses_spots[current_pref_course] > 0)
        {
            courses_spots[current_pref_course]--;
        }
        else
        {
            course_withdrawn = 1;
        }
        pthread_mutex_unlock(&courses_spots_lock[current_pref_course]);

        if (!course_withdrawn)
        {
            printf("Student %d has been allocated a seat in course %s\n", p_student.id, courses[current_pref_course].name);

            pthread_mutex_lock(&student_status_lock[p_student.id]);
            student_status[p_student.id] = current_pref_course + 1;
            pthread_mutex_unlock(&student_status_lock[p_student.id]);

            pthread_mutex_lock(&student_status_lock[p_student.id]);
            pthread_cond_wait(&student_status_available[p_student.id], &student_status_lock[p_student.id]);
            pthread_mutex_unlock(&student_status_lock[p_student.id]);

            // choose to finalize or withdraw from course
            float x = (float)rand() / (float)(RAND_MAX / 1.0);
            if (x < courses[current_pref_course].interest * p_student.calibre)
            {
                printf("Student %d has selected course %s permanently\n", p_student.id, courses[current_pref_course].name);
                pthread_mutex_lock(&student_status_lock[p_student.id]);
                student_status[p_student.id] = 0;
                pthread_mutex_unlock(&student_status_lock[p_student.id]);
                // printf("Student thread %d has ended\n", p_student.id);
                return NULL;
            }
            else
            {
                printf("Student %d has withdrawn from course %s\n", p_student.id, courses[current_pref_course].name);
                course_withdrawn = 1;
            }
        }
        if (course_withdrawn)
        {
            int previous_pref_course = current_pref_course;
            // get next course if present course isn't available
            for (i = current_pref + 1; i < 3; i++)
            {
                pthread_mutex_lock(&courses_spots_lock[p_student.preferences[i]]);
                if (courses_spots[p_student.preferences[i]] != -1)
                {
                    current_pref = i;
                    current_pref_course = courses_spots[p_student.preferences[i]];
                }
                pthread_mutex_unlock(&courses_spots_lock[p_student.preferences[i]]);
            }
            if (previous_pref_course == current_pref_course)
            {
                current_pref = -1;
            }
            else
            {
                printf("Student %d has changed current preference from %s to %s\n", p_student.id, courses[previous_pref_course].name, courses[current_pref_course].name);
            }
        }
    }

    if (current_pref == -1)
    {
        printf("Student %d did not get any of their preferred courses\n", p_student.id);
    }

    // printf("Student thread %d has ended\n", p_student.id);

    return NULL;
}

void *simulate_course(void *args)
{
    course p_course = *(course *)args;

    while (1)
    {

        int chosen_ta_lab_id = -1;
        int chosen_ta_id = -1;
        // go through each of the eligible labs
        int lab_no, i;
        for (lab_no = 0; lab_no < p_course.num_labs; lab_no++)
        {
            int lab_id = p_course.labs[lab_no];

            // go through each of the TAs in the lab
            int ta_num;
            for (ta_num = 0; ta_num < rlabs[lab_id].num_tas; ta_num++)
            {

                pthread_mutex_lock(&lab_tas_lock[lab_id][ta_num]);
                // if a TA is eligible to take a tutorial but is currently taking a tutorial for some other course
                // a broadcast will wake the condition
                while (lab_tas[lab_id][ta_num][0] > 0 && lab_tas[lab_id][ta_num][1] <= 0)
                {
                    pthread_cond_wait(&lab_tas_available[lab_id][ta_num], &lab_tas_lock[lab_id][ta_num]);
                }
                // if it does not wait, or when it successfully comes out of the wait, then choose this ta for the course
                if (lab_tas[lab_id][ta_num][0] > 0 && lab_tas[lab_id][ta_num][1] > 0)
                {
                    lab_tas[lab_id][ta_num][0]--;   // the ta used up a course
                    lab_tas[lab_id][ta_num][1] = 0; // the ta is now busy conducting a tutorial here
                    chosen_ta_lab_id = lab_id;
                    chosen_ta_id = ta_num;
                }
                pthread_mutex_unlock(&lab_tas_lock[lab_id][ta_num]);

                // stop going through the tas if you chose one already
                if (chosen_ta_id != -1)
                    break;
            }
            // stop going through the tas if you chose one already
            if (chosen_ta_id != -1)
                break;
        }

        // no ta was found for this course, the course is now removed
        if (chosen_ta_id == -1)
        {
            pthread_mutex_lock(&courses_spots_lock[p_course.id]);
            courses_spots[p_course.id] = -1;
            pthread_mutex_unlock(&courses_spots_lock[p_course.id]);
            pthread_cond_broadcast(&courses_spots_available[p_course.id]);
            printf("Course %s does not have any TA's eligible and is removed from course offerings\n", p_course.name);
            // printf("Course thread %s has ended\n", p_course.name);
            return NULL;
        }

        pthread_mutex_lock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);
        printf("TA %d from lab %s has been allocated to course %s for his %dst TA ship\n", chosen_ta_id, rlabs[chosen_ta_lab_id].name, p_course.name, rlabs[chosen_ta_lab_id].max_courses - lab_tas[chosen_ta_lab_id][chosen_ta_id][0]);
        pthread_mutex_unlock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);

        int left_spots_in_lab = 0;
        for (i = 0; i < rlabs[chosen_ta_lab_id].num_tas; i++)
        {
            pthread_mutex_lock(&lab_tas_lock[chosen_ta_lab_id][i]);
            left_spots_in_lab += lab_tas[chosen_ta_lab_id][i][0];
            pthread_mutex_unlock(&lab_tas_lock[chosen_ta_lab_id][i]);
        }
        if (left_spots_in_lab == 0)
        {
            printf("Lab %s no longer has mentors available for TAship\n", rlabs[chosen_ta_lab_id].name);
        }

        // open tut_slots slots for the tutorial
        int tut_slots = (rand() % p_course.course_max_slots) + 1;
        pthread_mutex_lock(&courses_spots_lock[p_course.id]);
        courses_spots[p_course.id] = tut_slots;
        pthread_mutex_unlock(&courses_spots_lock[p_course.id]);

        printf("Course %s has been allocated %d seats\n", p_course.name, tut_slots);

        int waiting_students = tut_slots;
        int filled_seats = 0;
        while ((filled_seats < 1) || (waiting_students != 0 && filled_seats < tut_slots))
        {
            pthread_cond_broadcast(&courses_spots_available[p_course.id]);

            waiting_students = 0;
            filled_seats = 0;
            int i;
            for (i = 0; i < num_students; i++)
            {
                pthread_mutex_lock(&student_status_lock[i]);
                if (student_status[i] == -1 * (p_course.id + 1))
                    waiting_students += 1;
                if (student_status[i] == (p_course.id + 1))
                {
                    filled_seats += 1;
                    // printf("Found that student %d filled %s as value %d\n", i, p_course.name, student_status[i]);
                }
                pthread_mutex_unlock(&student_status_lock[i]);
            }

            if (waiting_students == 0)
                break; // if noone is waiting, stop
        }

        filled_seats = 0;
        for (i = 0; i < num_students; i++)
        {
            pthread_mutex_lock(&student_status_lock[i]);
            if (student_status[i] == (p_course.id + 1))
                filled_seats += 1;
            pthread_mutex_unlock(&student_status_lock[i]);
        }

        printf("TA %d has started tutorial for Course %s with %d seats filled out of %d\n", chosen_ta_id, p_course.name, filled_seats, tut_slots);

        pthread_mutex_lock(&courses_spots_lock[p_course.id]);
        courses_spots[p_course.id] = 0;
        pthread_mutex_unlock(&courses_spots_lock[p_course.id]);

        sleep(3); // the ta is conducting a tutorial

        for (i = 0; i < num_students; i++)
        {
            pthread_mutex_lock(&student_status_lock[i]);
            if (student_status[i] == (p_course.id + 1))
            {
                student_status[i] = 0;
                pthread_cond_broadcast(&student_status_available[i]);
            }
            pthread_mutex_unlock(&student_status_lock[i]);
        }

        printf("TA %d from lab %s has completed the tutorial and left the course %s\n", chosen_ta_id, rlabs[chosen_ta_lab_id].name, p_course.name);

        pthread_mutex_lock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);
        lab_tas[chosen_ta_lab_id][chosen_ta_id][1] = 1; // the ta is now available to ta some other course
        pthread_cond_broadcast(&lab_tas_available[chosen_ta_lab_id][chosen_ta_id]);
        pthread_mutex_unlock(&lab_tas_lock[chosen_ta_lab_id][chosen_ta_id]);
    }

    // printf("Course thread %s has ended\n", p_course.name);

    return NULL;
}

int main(int argc, char *argv[])
{
    scanf("%d %d %d", &num_students, &num_labs, &num_courses);

    courses = malloc(sizeof(course) * num_courses);

    // taking all the inputs for the courses
    int i;
    for (i = 0; i < num_courses; ++i)
    {
        courses[i].id = i;
        scanf("%s %f %d %d", courses[i].name, &courses[i].interest, &courses[i].course_max_slots, &courses[i].num_labs);

        // taking the list of eligible labs for the course
        courses[i].labs = malloc(sizeof(int) * courses[i].num_labs);
        int ii;
        for (ii = 0; ii < courses[i].num_labs; ii++)
        {
            scanf("%d", &courses[i].labs[ii]);
        }
    }

    students = malloc(sizeof(student) * num_students);
    // taking all the inputs for the students
    for (i = 0; i < num_students; ++i)
    {
        students[i].id = i;
        scanf("%f %d %d %d %d", &students[i].calibre, &students[i].preferences[0], &students[i].preferences[1], &students[i].preferences[2], &students[i].fill_time);
    }

    rlabs = malloc(sizeof(lab) * num_labs);
    // taking all the inputs for the labs
    for (i = 0; i < num_labs; ++i)
    {
        rlabs[i].id = i;
        scanf("%s %d %d", rlabs[i].name, &rlabs[i].num_tas, &rlabs[i].max_courses);
    }

    courses_spots_lock = malloc(sizeof(pthread_mutex_t) * num_courses);
    courses_spots_available = malloc(sizeof(pthread_cond_t) * num_courses);
    courses_spots = malloc(sizeof(int) * num_courses);
    for (i = 0; i < num_courses; ++i)
    {
        pthread_mutex_init(&courses_spots_lock[i], NULL);
        pthread_cond_init(&courses_spots_available[i], NULL);
        courses_spots[i] = 0;
    }

    // student_in_tut_lock = malloc(sizeof(pthread_mutex_t) * num_students);
    // student_in_tut_available = malloc(sizeof(pthread_cond_t) * num_students);
    // student_in_tut = malloc(sizeof(int) * num_students);
    // for (i = 0; i < num_students; ++i){
    //     pthread_mutex_init(&student_in_tut_lock[i], NULL);
    //     pthread_cond_init(&student_in_tut_available[i], NULL);
    //     student_in_tut[i] = -1;
    // }

    lab_tas_lock = malloc(sizeof(pthread_mutex_t *) * num_labs);
    lab_tas_available = malloc(sizeof(pthread_cond_t *) * num_labs);
    lab_tas = malloc(sizeof(int **) * num_labs);
    for (i = 0; i < num_labs; ++i)
    {
        lab_tas_lock[i] = malloc(sizeof(pthread_mutex_t) * rlabs[i].num_tas);
        lab_tas_available[i] = malloc(sizeof(pthread_cond_t) * rlabs[i].num_tas);
        lab_tas[i] = malloc(sizeof(int *) * rlabs[i].num_tas);
        int ii;
        for (ii = 0; ii < rlabs[i].num_tas; ii++)
        {
            pthread_mutex_init(&lab_tas_lock[i][ii], NULL);
            pthread_cond_init(&lab_tas_available[i][ii], NULL);
            lab_tas[i][ii] = malloc(sizeof(int) * 2);
            lab_tas[i][ii][0] = rlabs[i].max_courses;
            lab_tas[i][ii][1] = 1;
        }
    }

    student_status_lock = malloc(sizeof(pthread_mutex_t) * num_students);
    student_status_available = malloc(sizeof(pthread_cond_t) * num_students);
    student_status = malloc(sizeof(int) * num_students);
    for (i = 0; i < num_students; ++i)
    {
        student_status[i] = -1 * (students[i].preferences[0] + 1);
        pthread_mutex_init(&student_status_lock[i], NULL);
        pthread_cond_init(&student_status_available[i], NULL);
    }

    // allocate space and create threads
    t_courses = malloc(sizeof(pthread_t) * num_courses);
    t_students = malloc(sizeof(pthread_t) * num_students);
    for (i = 0; i < num_students; i++)
    {
        pthread_create(&t_students[i], NULL, simulate_student, &students[i]);
    }
    for (i = 0; i < num_courses; i++)
    {
        pthread_create(&t_courses[i], NULL, simulate_course, &courses[i]);
    }

    // wait for all threads to finish
    for (i = 0; i < num_courses; i++)
    {
        pthread_join(t_courses[i], NULL);
    }
    for (i = 0; i < num_students; i++)
    {
        pthread_join(t_students[i], NULL);
    }

    // free all malloc'd memory
    for (i = 0; i < num_courses; ++i)
    {
        free(courses[i].labs);
    }
    free(courses);
    free(students);
    free(rlabs);

    // free(student_in_tut_lock);
    // free(student_in_tut_available);
    // free(student_in_tut);

    free(courses_spots_lock);
    free(courses_spots_available);
    free(courses_spots);
    for (i = 0; i < num_labs; ++i)
    {
        int ii;
        for (ii = 0; ii < rlabs[i].num_tas; ii++)
        {
            free(lab_tas[i][ii]);
        }
        free(lab_tas[i]);
        free(lab_tas_lock[i]);
        free(lab_tas_available[i]);
    }
    free(lab_tas_available);
    free(lab_tas_lock);
    free(lab_tas);
    free(student_status);
    free(student_status_lock);
    free(student_status_available);

    free(t_courses);
    free(t_students);

    return 0;
}