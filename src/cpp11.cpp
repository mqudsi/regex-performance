#include <stdio.h>
#include <string.h>
#include <regex>
#include <iostream>

#include "main.h"

typedef std::basic_regex<char> cregex;

static void * get_cpp11_object(char* pattern)
{
    try {
        // auto flags = std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::nosubs;
        auto flags = std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::nosubs;
        if (strncmp(pattern, "(?i)", strlen("(?i)")) == 0) {
            flags |= std::regex_constants::icase;
            pattern = pattern + strlen("(?i)");
        }
        return new cregex(pattern,  flags);
    }
    catch (...) {
        // std::cerr << "Error compiling regex pattern: " << pattern << std::endl;
        // return new cregex("^$", std::regex_constants::egrep | std::regex_constants::optimize);
        return nullptr;
    }
}

static void free_cpp11_object(void* obj)
{
    delete (cregex*)obj;
}

static int search_all_cpp11(void* obj, char* subject, int subject_len)
{
    auto begin = std::cregex_iterator(subject, subject + subject_len, *(cregex*)obj);
    auto end   = std::cregex_iterator();

    return std::distance(begin, end);
}

extern "C" int cpp11_find_all(char* pattern, char* subject, int subject_len, int repeat, struct result * res)
{
    void * obj = get_cpp11_object(pattern);
    TIME_TYPE start, end = 0;
    int found = 0;

    if (!obj) {
        printf("C++11 compilation failed\n");
        return -1;
    }

    double * times = (double*) std::calloc(repeat, sizeof(double));
    int const times_len = repeat;

    do {
        GET_TIME(start);
        found = search_all_cpp11(obj, subject, subject_len);
        GET_TIME(end);
        times[repeat - 1] = TIME_DIFF_IN_MS(start, end);

    } while (--repeat > 0);


    res->matches = found;
    get_mean_and_derivation(times, times_len, res);

    free_cpp11_object(obj);
    free(times);

    return 0;
}
