#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "main.h"

// CONFIGURATION/MUST BE ADJUSTED FOR EACH NEW PROJECT
#define SCH_MAX_TASKS 40
#define NO_TASK_ID 0
#define TICK_PERIOD 10 // in milliseconds

// RETURN CODE
#define RETURN_ERROR 1
#define RETURN_NORMAL 0

typedef enum
{
    ERROR_SCH_OK = 0,
    ERROR_SCH_TOO_MANY_TASKS,
    ERROR_SCH_CANNOT_DELETE_TASK,
} SCH_Error_t;

typedef struct
{
    // Pointer to the task (must be a "void (void)" function)
    void (*pTask)(void);
    uint32_t Delay;  // counter variable, counting down, in ticks
    uint32_t Period; // original cycle time, in ticks
    uint32_t RunMe;  // counter variable, scheduling = add 1, dispatching = subtract 1
    uint32_t TaskID; // Incrementally unique
} sTask;

// Required
void SCH_Init(void);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t SCH_Delete_Task(uint32_t taskID);
//

uint32_t get_time(void);
#endif