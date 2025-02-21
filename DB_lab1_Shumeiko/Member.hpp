#pragma once

struct Member {
    int MemberID;
    int ManagerID;   // foreign key
    char Name[NAME_SIZE];
    char Role[ROLE_SIZE];
    int TasksPerMonth;
    int TasksTotal;
    time_t LastTaskDate;
    bool isDeleted;   // службове поле
};