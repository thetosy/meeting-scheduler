//
// Created by Praise Olukilede on 4/16/23.
//
#include "timeslotsFuncs.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <cctype>
#include <algorithm>

#define FALSE 0
#define TRUE 1

using namespace std;

void createTimeSlots(a_timeslot *timeslot, char *s) {
    int i = 0;
    int x;
    int brackets = 0;
    int first = 0;
    int interval_count = 0;
    int keep_running = TRUE;
    while (keep_running) {
        if (isspace(s[i])) {
            i++;
            continue;
        }
        if (s[i] == '[') {
            brackets++;
        } else if (s[i] == ']') {
            brackets--;
        }
        if (brackets == 2) {
            string number = "";
            for (x = i + 1; isdigit(s[x]); x++) {
                number += s[x];
            }
            if (first == 0) {
                timeslot->times[interval_count].start = stoi(number);
                first = 1;
            } else {
                timeslot->times[interval_count].end = stoi(number);
                first = 0;
                interval_count++;
            }
            i = x;
        } else {
            if (brackets == 0 && interval_count <= 10) {
                timeslot->count = interval_count;
                interval_count = 0;
                keep_running = FALSE;
            }
            i++;
        }
    }
}

void findCommonTimeSlots(char *listNames, unordered_map<string, a_timeslot *> data, a_timeslot *group) {
    string names = string(listNames);
    //fprintf(stdout, "here4\n");
    names.erase(remove_if(names.begin(), names.end(), [](char c) { return isspace(c); }), names.end());
    //fprintf(stdout, "here5\n");
    char copy[strlen(names.c_str())];
    strcpy(copy, names.c_str());
    //fprintf(stdout, "here6\n");
    char *start_ptr = copy;
    char *delimeter;
    //int index = 0;
    for (;;) {
        delimeter = strchr(start_ptr, ',');
        //fprintf(stdout, "here7.1\n");
        if (delimeter != NULL) {
            //delimeter
            *delimeter++ = '\0';
        }
        //fprintf(stdout, "here1\n");
        timeslots *p = data[string(start_ptr)];
        //fprintf(stdout, "here7.2\n");
        findTimeSlots(p, group);
        //fprintf(stdout, "here7\n");
        //fprintf(stdout, "here2\n");
        start_ptr = delimeter;
        if (delimeter == NULL) {
            break;
        }
    }
}

void findTimeSlots(a_timeslot *user, a_timeslot *res) {
    if (res->count == 0) {
        setTimeSlotsEquals(user, res);
        //fprintf(stdout, "here6.5\n");
        return;
    }
    a_timeslot result;
    //fprintf(stdout, "here6.6\n");
    memset(&result, 0, sizeof(result));
    int entry_count = 0;
    for (int i = 0; i < user->count; i++) {
        for (int j = 0; j < res->count; j++) {
            //fprintf(stdout, "here5\n");
            if (user->times[i].end > res->times[j].start && user->times[i].start < res->times[j].end) {
                result.times[entry_count].start = (user->times[i].start > res->times[j].start) ? user->times[i].start
                                                                                                : res->times[j].start;
                result.times[entry_count].end = (user->times[i].end < res->times[j].end) ? user->times[i].end
                                                                                          : res->times[j].end;
                entry_count++;
            }
        }
    }
    result.count = entry_count;
    setTimeSlotsEquals(&result, res);
}

void setTimeSlotsEquals(a_timeslot *a, a_timeslot *b) {
    // set b to equal a
    int i = 0;
    while (1) {
        if (i == a->count) {
            break;
        }
        b->times[i].start = a->times[i].start;
        b->times[i].end = a->times[i].end;
        i++;
    }
    b->count = i;
}

void convertTimeslotsToString(char* s, a_timeslot* y){
    int i = 0;
    string x = "[";
    while(1){
        if(i == y->count){
            break;
        }
        string start = to_string(y->times[i].start);
        string end = to_string(y->times[i].end);
        if(i == 0){
            x = x + "[" + start + "," + end + "]";
        } else {
            x = x + ",[" + start + "," + end + "]";
        }
        i++;
    }
    x = x + "]";
    strcpy(s, x.c_str());
}
