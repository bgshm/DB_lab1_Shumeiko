#pragma once
// Константи для імен файлів
const char* MASTER_FILE = "managers.dat";
const char* SLAVE_FILE = "members.dat";

// Фіксовані розміри для символьних полів (для двійкового сховища з фіксованим записом)
const int NAME_SIZE = 256;
const int ROLE_SIZE = 101;

// Структури, що представляють записи (включаючи службове поле для видалення)
struct Manager {
    int ManagerID;
    char Name[NAME_SIZE];
    bool isDeleted;   // службове поле
};

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