#include "scheduler.h"

// Global private data
static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint32_t taskID_gen;
static uint32_t current_task_count;

// Global public data
uint8_t Error_code_G;

/**
 * What SCH_Init does:
 * Clear, initialize SCH_tasks_G
 * Reset task ID generator value
 * Reset system error status
 */
void SCH_Init(void)
{
    // clear all tasks currently in queue
    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++)
    {
        SCH_tasks_G[i].pTask = 0;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
        SCH_tasks_G[i].TaskID = NO_TASK_ID;
    }
    // then reinitialize
    current_task_count = 0;
    taskID_gen = 1;
    Error_code_G = ERROR_SCH_OK;
}

/**
 * Adding to the task array with Delta List approach
 * in a Delta List, every task in the queue only stores its wait time relative to the task before it
 */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD)
{
    if (pFunction == NULL || current_task_count >= SCH_MAX_TASKS)
        return NO_TASK_ID;

    uint32_t delay_ticks = DELAY / TICK_PERIOD;
    uint32_t period_ticks = PERIOD / TICK_PERIOD;

    uint32_t remaining_delay = delay_ticks;
    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++)
    {
        if (SCH_tasks_G[i].TaskID == NO_TASK_ID || remaining_delay < SCH_tasks_G[i].Delay)
        {
            // shift every task starting at i one slot to the right
            for (uint32_t j = current_task_count; j > i; j--)
                SCH_tasks_G[j] = SCH_tasks_G[j - 1];

            // add task here at i
            SCH_tasks_G[i].pTask = pFunction;
            SCH_tasks_G[i].Delay = remaining_delay;
            SCH_tasks_G[i].Period = period_ticks;
            SCH_tasks_G[i].RunMe = 0;
            SCH_tasks_G[i].TaskID = taskID_gen++;

            // change the relative delay of the task at i+1
            if (i + 1 <= current_task_count)
                SCH_tasks_G[i + 1].Delay -= remaining_delay;

            // increment current task count
            current_task_count++;

            return SCH_tasks_G[i].TaskID;
        }
        else
        {
            // decrement remaining_delay because the task to be added now runs after the task at i
            remaining_delay -= SCH_tasks_G[i].Delay;
        }
    }

    return NO_TASK_ID;
}

/**
 *
 */
uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == NO_TASK_ID || current_task_count == 0)
    {
        Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
        return RETURN_ERROR;
    }
    // search for task with taskID in the array
    for (uint32_t i = 0; i < current_task_count; i++)
    {
        if (SCH_tasks_G[i].TaskID == taskID)
        {
            if (i + 1 < current_task_count)
                SCH_tasks_G[i + 1].Delay += SCH_tasks_G[i].Delay;

            for (uint32_t j = i; j < current_task_count - 1; j++)
                SCH_tasks_G[j] = SCH_tasks_G[j + 1];

            SCH_tasks_G[current_task_count - 1].pTask = 0;
            SCH_tasks_G[current_task_count - 1].TaskID = NO_TASK_ID;
            SCH_tasks_G[current_task_count - 1].Delay = 0;
            SCH_tasks_G[current_task_count - 1].Period = 0;
            SCH_tasks_G[current_task_count - 1].RunMe = 0;
            current_task_count--;

            return RETURN_NORMAL;
        }
    }
    // if not found
    Error_code_G = ERROR_SCH_CANNOT_DELETE_TASK;
    return RETURN_ERROR;
}

/**
 *
 */
void SCH_Update(void)
{
    if (SCH_tasks_G[0].pTask != 0)
    {
        if (SCH_tasks_G[0].Delay > 0)
            SCH_tasks_G[0].Delay--;
        if (SCH_tasks_G[0].Delay == 0)
            SCH_tasks_G[0].RunMe++;
    }
}

/**
 *
 */
void SCH_Dispatch_Tasks(void)
{
    if (current_task_count == 0 || SCH_tasks_G[0].RunMe == 0)
        return;

    if (SCH_tasks_G[0].RunMe > 0)
    {
        // 1. Run the task
        (*SCH_tasks_G[0].pTask)();
        SCH_tasks_G[0].RunMe--;

        // 2. Only delete/reschedule when all pending runs are cleared!
        if (SCH_tasks_G[0].RunMe == 0)
        {
            if (SCH_tasks_G[0].Period == 0)
                SCH_Delete_Task(SCH_tasks_G[0].TaskID);
            else
            {
                void (*pFunction)(void) = SCH_tasks_G[0].pTask;
                uint32_t period_ms = SCH_tasks_G[0].Period * TICK_PERIOD;
                SCH_Delete_Task(SCH_tasks_G[0].TaskID);
                SCH_Add_Task(pFunction, period_ms, period_ms);
            }
            // Handling simultaneous arrivals
            if (current_task_count > 0 && SCH_tasks_G[0].Delay == 0)
                SCH_tasks_G[0].RunMe = 1;
        }
    }
}