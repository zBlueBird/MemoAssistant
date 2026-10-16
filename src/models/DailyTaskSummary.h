#pragma once

struct DailyTaskSummary
{
    int total = 0;
    int incomplete = 0;

    int completed() const
    {
        return total - incomplete;
    }
};
