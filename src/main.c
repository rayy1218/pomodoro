#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ncurses.h>
#include <string.h>

struct pomodoroArg {
    time_t work_second, rest_second, period_num;
};

struct workRet {
    bool end;
    time_t time_pass;
};

struct restRet {
    bool overtime, end;
    time_t time_pass;
};

int getKeyEvent() {
    timeout(0);
    return getch();
}

void timeToString(char *str, int str_maxsize, time_t sec) {
    int format_sec = sec % 60, 
        format_minute = sec / 60;
    
    snprintf(str, 100, "%i minutes %i second", format_minute, format_sec);
}

time_t pauseTimer() {
    while (true) {
        if (getch() == 'p') {return time(NULL);}
    }
}

void renderTimerWork(int current_period, int total_period, time_t sec_total, time_t sec_left) {
    WINDOW *canvas = newwin(10, 30, 0, 0);
    box(canvas, 0, 0);
    
    char total_string[30], left_string[30];
    timeToString(total_string, 30, sec_total);
    timeToString(left_string, 30, sec_left);
    
    mvwprintw(canvas, 0, 1, "%i/%i", current_period, total_period);
    mvwprintw(canvas, 1, 1, "Work - %s", total_string);
    mvwprintw(canvas, 2, 1, "%s left", left_string);

    mvwprintw(canvas, 4, 1, "[p] pause/continue");
    mvwprintw(canvas, 5, 1, "[Enter] end period");
    mvwprintw(canvas, 8, 1, "[q] quit");
     
    wrefresh(canvas);

    delwin(canvas);
}

void renderTimerRest(int current_period, int total_period, time_t sec_total, time_t sec_pass) {
    WINDOW *canvas = newwin(10, 30, 0, 0);
    box(canvas, 0, 0);
    
    char total_string[30], pass_string[30];
    timeToString(total_string, 30, sec_total);
    timeToString(pass_string, 30, sec_pass);
    
    mvwprintw(canvas, 0, 1, "%i/%i", current_period, total_period);
    mvwprintw(canvas, 1, 1, "Rest - %s", total_string);
    mvwprintw(canvas, 2, 1, "%s pass", pass_string);
    mvwprintw(canvas, 4, 1, "[p] pause/continue");
    mvwprintw(canvas, 5, 1, "[o] restart as overtime");
    mvwprintw(canvas, 6, 1, "[Enter] end period");
    mvwprintw(canvas, 8, 1, "[q] quit");
    
    wrefresh(canvas);

    delwin(canvas);
}

struct workRet timerWork(int current_period, int total_period, time_t sec_total) {
    struct workRet ret;
    time_t before, now, end;
    before = time(NULL), now = before, end = now + sec_total;

    do {
        time_t sec_left = end - now;
        now = time(NULL);
        if (now > before) {
            renderTimerWork(current_period, total_period, sec_total, sec_left);
            before = now;
        }

        int key = getKeyEvent();
        //Respond key event
        switch (key) {
            case 10: 
                ret.time_pass = sec_total - sec_left;
                ret.end = false;

                return ret;
            
            case 'q':
                ret.time_pass = sec_total - sec_left;
                ret.end = true;

                return ret;

            case 'p':
                now = pauseTimer();
                end = now + sec_left;
                break;
        }
    }
    while (now < end);
    
    ret.end = false;
    ret.time_pass = sec_total;
    return ret;
}

struct restRet timerRest(int current_period, int total_period, time_t sec_total) {
    struct restRet ret;

    time_t start = time(NULL);
    time_t before, now;
    before = time(NULL), now = before;

    do {
        time_t sec_pass = now - start;
        now = time(NULL);
        if (now > before) {
            renderTimerRest(current_period, total_period, sec_total, sec_pass);
            before = now;
        }

        int key = getKeyEvent();
        //Respond key event
        switch (key) { 
            case 10: 
                ret.end = false;
                ret.overtime = false;
                ret.time_pass = sec_pass;

                return ret;
            
            case 'o': 
                ret.end = false;
                ret.overtime = true;
                ret.time_pass = sec_pass;

                return ret;

            case 'q':
                ret.end = true;
                ret.overtime = false;
                ret.time_pass = sec_pass;

                return ret;

            case 'p':
                now = pauseTimer();
                start += (now - before);
                break;
        }
    }
    while (true);
}

void startPomodoro(struct pomodoroArg arg) {
    initscr();
    noecho();
    curs_set(0);
    
    time_t work_time[arg.period_num], rest_time[arg.period_num - 1],
           overtime[arg.period_num];
    bool end = false;

    for (int i = 0; i < arg.period_num; i++) {overtime[i] = 0;}

    for (int i = 0; i < arg.period_num; i++) {
        if (end) {
            work_time[i] = 0;
        }
        else {
            struct workRet ret = timerWork(i + 1, arg.period_num, arg.work_second);
            work_time[i] = ret.time_pass;
            end = ret.end;
        }

        if (end) {
            rest_time[i] = 0;
        }
        else if (i < arg.period_num - 1) {
            struct restRet ret = timerRest(i + 1, arg.period_num, arg.rest_second);
            if (ret.overtime) {
                overtime[i] = ret.time_pass;
                rest_time[i] = timerRest(i + 1, arg.period_num, arg.rest_second).time_pass;
            }
            else {
                rest_time[i] = ret.time_pass;
            }

            end = ret.end;
        }
    }

    endwin();

    time_t total_work = 0, total_rest = 0, total_overtime = 0;
    printf("Summary: \n");
    for (int i = 0; i < arg.period_num; i++) {
        printf("Period %i\n", i + 1);

        printf("Work time(sec): %i\n", work_time[i]);
        total_work += work_time[i];
        
        if (overtime[i] > 0) {
            printf("Overtime: %i\n", overtime[i]);
            total_overtime += overtime[i];
        }

        if (i < arg.period_num - 1) {
            printf("Rest time(sec): %i\n", rest_time[i]);
            total_rest += rest_time[i];
        }
    }

    printf("\nTotal work(sec): %i\n", total_work);
    printf("Total rest(sec): %i\n", total_rest);
}

int main(int argc, char *argv[]) {
    const int OPTION_NUM = 2;
    char *options[] = {"start", "help"};
    int i;
    for (i = 0; i < OPTION_NUM; i++) {
        if (strcmp(argv[1], options[i]) == 0) {break;}
    }
    
    switch (i) {
        case 0:
            if (argc != 5) {
                printf("Unexpected number of argument\n");
                return 0;
            }

            struct pomodoroArg arg;
            arg.work_second = atoi(argv[2]) * 60;
            arg.rest_second = atoi(argv[3]) * 60;
            arg.period_num = atoi(argv[4]);
            startPomodoro(arg);
            break;

        case 1:
            printf("Manual: pomodoro start [Work length(minute)] [Rest length(minute)] [Number of period]");
            return 0;

        default:
            printf("Unknown command: %s\n", argv[1]);
            return 0;
    }

    return 0;
}

