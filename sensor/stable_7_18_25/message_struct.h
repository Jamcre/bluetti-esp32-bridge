#ifndef MESSAGE_STRUCT_H
#define MESSAGE_STRUCT_H

typedef struct message_struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} message_struct;

extern message_struct myData;
extern message_struct myData_copy;
extern int stored_min;

#endif