#pragma once
// ��������� ��� ���� �����
const char* MASTER_FILE = "managers.dat";
const char* SLAVE_FILE = "members.dat";

// Գ������ ������ ��� ���������� ���� (��� ��������� ������� � ���������� �������)
const int NAME_SIZE = 256;
const int ROLE_SIZE = 101;

// ���������, �� ������������� ������ (��������� �������� ���� ��� ���������)
struct Manager {
    int ManagerID;
    char Name[NAME_SIZE];
    bool isDeleted;   // �������� ����
};

struct Member {
    int MemberID;
    int ManagerID;   // foreign key
    char Name[NAME_SIZE];
    char Role[ROLE_SIZE];
    int TasksPerMonth;
    int TasksTotal;
    time_t LastTaskDate;
    bool isDeleted;   // �������� ����
};