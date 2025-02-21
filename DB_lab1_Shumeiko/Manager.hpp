#pragma once

// Структури, що представляють записи (включаючи службове поле для видалення)
struct Manager {
    int ManagerID;
    char Name[NAME_SIZE];
    bool isDeleted;   // службове поле
};