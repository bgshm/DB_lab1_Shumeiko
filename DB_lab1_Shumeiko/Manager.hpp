#pragma once

// ���������, �� ������������� ������ (��������� �������� ���� ��� ���������)
struct Manager {
    int ManagerID;
    char Name[NAME_SIZE];
    bool isDeleted;   // �������� ����
};