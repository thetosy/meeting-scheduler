//
// Created by Praise Olukilede on 3/20/23.
//

#ifndef SOCKET_PROJECT_TIMESLOTSFUNCS_H
#define SOCKET_PROJECT_TIMESLOTSFUNCS_H
#include <unordered_map>
#include <string.h>
#include <string>

using namespace std;

struct interval{
    int start;
    int end;
};

typedef struct timeslots{
    int count;
    struct interval times[10];
} a_timeslot;


extern void createTimeSlots(a_timeslot *, char*);

extern void findCommonTimeSlots(char*, unordered_map<string, a_timeslot*>, a_timeslot*);

extern void findTimeSlots(a_timeslot*, a_timeslot*);

extern void setTimeSlotsEquals(a_timeslot*, a_timeslot*);

extern void convertTimeslotsToString(char*, a_timeslot*);


#endif //SOCKET_PROJECT_TIMESLOTSFUNCS_H
