#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <ctime>
#include <iomanip>
#include "Utils.hpp"
using namespace std;

// Helper: записує Manager запис в кінці MASTER_FILE
void insertManager(const Manager& m) {
    fstream file(MASTER_FILE, ios::in | ios::out | ios::binary);
    if (!file) { // файл може не існувати -> створюємо його
        file.open(MASTER_FILE, ios::out | ios::binary);
    }
    file.seekp(0, ios::end);
    file.write(reinterpret_cast<const char*>(&m), sizeof(Manager));
    file.close();
}

// Helper: записує Member запис в кінці SLAVE_FILE
void insertMember(const Member& cm) {
    fstream file(SLAVE_FILE, ios::in | ios::out | ios::binary);
    if (!file) {
        file.open(SLAVE_FILE, ios::out | ios::binary);
    }
    file.seekp(0, ios::end);
    file.write(reinterpret_cast<const char*>(&cm), sizeof(Member));
    file.close();
}

// Helper: зчитує запис Manager за його номером (починаючи з 0).
bool getManagerRecord(int recNo, Manager& m) {
    ifstream file(MASTER_FILE, ios::binary);
    if (!file) return false;
    file.seekg(recNo * sizeof(Manager), ios::beg);
    file.read(reinterpret_cast<char*>(&m), sizeof(Manager));
    return file.gcount() == sizeof(Manager);
}

// Helper: оновлює запис Manager за номером запису
bool updateManagerRecord(int recNo, const Manager& m) {
    fstream file(MASTER_FILE, ios::in | ios::out | ios::binary);
    if (!file) return false;
    file.seekp(recNo * sizeof(Manager), ios::beg);
    file.write(reinterpret_cast<const char*>(&m), sizeof(Manager));
    return true;
}

// Helper: зчитує всі підлеглі записи, пов'язані з даним ManagerID
std::vector<Member> getSlaveRecordsForManager(int managerID) {
    vector<Member> members;
    ifstream file(SLAVE_FILE, ios::binary);
    if (!file) return members;
    Member cm;
    while (file.read(reinterpret_cast<char*>(&cm), sizeof(Member))) {
        if (!cm.isDeleted && cm.ManagerID == managerID)
            members.push_back(cm);
    }
    return members;
}

// Helper: оновлює запис Member за номером запису (позицією у файлі)
bool updateMemberRecord(int recNo, const Member& cm) {
    fstream file(SLAVE_FILE, ios::in | ios::out | ios::binary);
    if (!file) return false;
    file.seekp(recNo * sizeof(Member), ios::beg);
    file.write(reinterpret_cast<const char*>(&cm), sizeof(Member));
    return true;
}

// get-m: виводить master запис за номером запису 'recNo'
void command_get_m(int recNo) {
    Manager m;
    if (getManagerRecord(recNo, m) && !m.isDeleted) {
        cout << "Manager Record (" << recNo << "): ID=" << m.ManagerID
            << ", Name=" << m.Name << "\n";
    }
    else {
        cout << "Manager record not found or deleted.\n";
    }
}

// get-s: виводить всі slave записи для заданого запису manager (за ідентифікатором ManagerID)
void command_get_s(int managerID) {
    vector<Member> members = getSlaveRecordsForManager(managerID);
    cout << "Member records for ManagerID=" << managerID << ":\n";
    for (const auto& cm : members) {
        char dateBuffer[26];
        ctime_s(dateBuffer, sizeof(dateBuffer), &cm.LastTaskDate);
        cout << "  MemberID=" << cm.MemberID << ", Name=" << cm.Name
            << ", Role=" << cm.Role << ", TasksPerMonth=" << cm.TasksPerMonth
            << ", TasksTotal=" << cm.TasksTotal
            << ", LastTaskDate=" << dateBuffer;
    }
    if (members.empty())
        cout << "  No slave records found.\n";
}

// del-m: позначає master запис як видалений
void command_del_m(int recNo) {
    Manager m;
    if (!getManagerRecord(recNo, m)) {
        cout << "Manager record not found.\n";
        return;
    }
    m.isDeleted = true;
    updateManagerRecord(recNo, m);
    // Також позначає всі slave записи для цього ManagerID як видалені
    fstream file(SLAVE_FILE, ios::in | ios::out | ios::binary);
    if (!file) return;
    Member cm;
    int pos = 0;
    while (file.read(reinterpret_cast<char*>(&cm), sizeof(Member))) {
        if (!cm.isDeleted && cm.ManagerID == m.ManagerID) {
            cm.isDeleted = true;
            file.seekp(pos * sizeof(Member), ios::beg);
            file.write(reinterpret_cast<const char*>(&cm), sizeof(Member));
            file.seekg((pos + 1) * sizeof(Member), ios::beg);
        }
        pos++;
    }
    cout << "Manager (and its slave records) deleted.\n";
}

// del-s: позначає slave запис як видалений за номером запису у slave файлі
void command_del_s(int recNo) {
    ifstream fileIn(SLAVE_FILE, ios::binary);
    if (!fileIn) {
        cout << "Slave file not found.\n";
        return;
    }
    // Спочатку зчитує запис
    Member cm;
    fileIn.seekg(recNo * sizeof(Member), ios::beg);
    fileIn.read(reinterpret_cast<char*>(&cm), sizeof(Member));
    fileIn.close();
    if (cm.isDeleted) {
        cout << "Record already deleted.\n";
        return;
    }
    cm.isDeleted = true;
    updateMemberRecord(recNo, cm);
    cout << "Slave record deleted.\n";
}

// update-m: оновлює поле master запису (для простоти оновлює Name)
void command_update_m(int recNo, const string& newName) {
    Manager m;
    if (!getManagerRecord(recNo, m) || m.isDeleted) {
        cout << "Manager record not found or deleted.\n";
        return;
    }
    strcpy_s(m.Name, NAME_SIZE, newName.c_str());
    updateManagerRecord(recNo, m);
    cout << "Manager record updated.\n";
}

// update-s: оновлює поле slave запису (для простоти оновлює Role)
void command_update_s(int recNo, const string& newRole) {
    ifstream fileIn(SLAVE_FILE, ios::binary);
    if (!fileIn) {
        cout << "Slave file not found.\n";
        return;
    }
    Member cm;
    fileIn.seekg(recNo * sizeof(Member), ios::beg);
    fileIn.read(reinterpret_cast<char*>(&cm), sizeof(Member));
    fileIn.close();
    if (cm.isDeleted) {
        cout << "Slave record not found or deleted.\n";
        return;
    }
    strcpy_s(cm.Role, ROLE_SIZE, newRole.c_str());
    updateMemberRecord(recNo, cm);
    cout << "Slave record updated.\n";
}

// calc-m: підраховує кількість master записів (невидалених)
void command_calc_m() {
    ifstream file(MASTER_FILE, ios::binary);
    if (!file) {
        cout << "Master file not found.\n";
        return;
    }
    int count = 0;
    Manager m;
    while (file.read(reinterpret_cast<char*>(&m), sizeof(Manager))) {
        if (!m.isDeleted)
            count++;
    }
    cout << "Number of master records: " << count << "\n";
}

// calc-s: підраховує slave записи загалом і для кожного master
void command_calc_s() {
    ifstream file(SLAVE_FILE, ios::binary);
    if (!file) {
        cout << "Slave file not found.\n";
        return;
    }
    int total = 0;
    // Map managerID -> count
    vector<pair<int, int>> perManager;
    Member cm;
    while (file.read(reinterpret_cast<char*>(&cm), sizeof(Member))) {
        if (!cm.isDeleted) {
            total++;
            bool found = false;
            for (auto& p : perManager) {
                if (p.first == cm.ManagerID) {
                    p.second++;
                    found = true;
                    break;
                }
            }
            if (!found) {
                perManager.push_back({ cm.ManagerID, 1 });
            }
        }
    }
    cout << "Total number of slave records: " << total << "\n";
    for (const auto& p : perManager) {
        cout << "ManagerID " << p.first << " has " << p.second << " slave record(s).\n";
    }
}

// ut-m: утиліта для друку всіх master записів, включаючи службові поля
void command_ut_m() {
    ifstream file(MASTER_FILE, ios::binary);
    if (!file) {
        cout << "Master file not found.\n";
        return;
    }
    cout << "=== All Master Records ===\n";
    Manager m;
    int recNo = 0;
    while (file.read(reinterpret_cast<char*>(&m), sizeof(Manager))) {
        cout << "Record " << recNo << ": "
            << "ManagerID=" << m.ManagerID << ", Name=" << m.Name
            << ", isDeleted=" << m.isDeleted << "\n";
        recNo++;
    }
}

// ut-s: утиліта для друку всіх slave записів, включаючи службові поля
void command_ut_s() {
    ifstream file(SLAVE_FILE, ios::binary);
    if (!file) {
        cout << "Slave file not found.\n";
        return;
    }
    cout << "=== All Slave Records ===\n";
    Member cm;
    int recNo = 0;
    while (file.read(reinterpret_cast<char*>(&cm), sizeof(Member))) {
        char dateBuffer[26];
        ctime_s(dateBuffer, sizeof(dateBuffer), &cm.LastTaskDate);
        cout << "Record " << recNo << ": "
            << "MemberID=" << cm.MemberID << ", ManagerID=" << cm.ManagerID
            << ", Name=" << cm.Name << ", Role=" << cm.Role
            << ", TasksPerMonth=" << cm.TasksPerMonth << ", TasksTotal=" << cm.TasksTotal
            << ", LastTaskDate=" << dateBuffer
            << ", isDeleted=" << cm.isDeleted << "\n";
        recNo++;
    }
}

// Простий командний процесор, який зчитує команди з текстового файлу.
// (Командний файл повинен містити рядки типу «insert-m», «del-s» тощо).
// Кожен рядок вважається командою з параметрами, відокремленими пробілами.
void processCommands(const string& cmdFilename) {
    ifstream cmdFile(cmdFilename);
    if (!cmdFile) {
        cout << "Could not open command file: " << cmdFilename << "\n";
        return;
    }

    string line;
    while (getline(cmdFile, line)) {
        if (line.empty()) continue;
        istringstream iss(line);
        string cmd;
        iss >> cmd;
        if (cmd == "insert-m") {
            // Очікуваний формат: insert-m <ManagerID> <Name>
            Manager m;
            iss >> m.ManagerID;
            string name;
            getline(iss, name);
            // Обрізання пробілів
            size_t start = name.find_first_not_of(" \t");
            if (start != string::npos)
                name = name.substr(start);
            strcpy_s(m.Name, NAME_SIZE, name.c_str());
            m.isDeleted = false;
            insertManager(m);
            cout << "Inserted Manager: " << m.ManagerID << "\n";
        }
        else if (cmd == "insert-s") {
            // Очікуваний формат: insert-s <MemberID> <ManagerID> <Name> <Role> <TasksPerMonth> <TasksTotal> <LastTaskDate>
            // Для простоти, припустимо, що LastTaskDate задано як ціле число (time_t)
            Member cm;
            iss >> cm.MemberID >> cm.ManagerID;
            string name, role;
            iss >> ws;
            iss >> name >> role;
            iss >> cm.TasksPerMonth >> cm.TasksTotal >> cm.LastTaskDate;
            strcpy_s(cm.Name, NAME_SIZE, name.c_str());
            strcpy_s(cm.Role, ROLE_SIZE, role.c_str());
            cm.isDeleted = false;
            insertMember(cm);
            cout << "Inserted Member: " << cm.MemberID << "\n";
        }
        else if (cmd == "get-m") {
            int recNo;
            iss >> recNo;
            command_get_m(recNo);
        }
        else if (cmd == "get-s") {
            int managerID;
            iss >> managerID;
            command_get_s(managerID);
        }
        else if (cmd == "del-m") {
            int recNo;
            iss >> recNo;
            command_del_m(recNo);
        }
        else if (cmd == "del-s") {
            int recNo;
            iss >> recNo;
            command_del_s(recNo);
        }
        else if (cmd == "update-m") {
            int recNo;
            string newName;
            iss >> recNo;
            getline(iss, newName);
            size_t start = newName.find_first_not_of(" \t");
            if (start != string::npos)
                newName = newName.substr(start);
            command_update_m(recNo, newName);
        }
        else if (cmd == "update-s") {
            int recNo;
            string newRole;
            iss >> recNo >> newRole;
            command_update_s(recNo, newRole);
        }
        else if (cmd == "calc-m") {
            command_calc_m();
        }
        else if (cmd == "calc-s") {
            command_calc_s();
        }
        else if (cmd == "ut-m") {
            command_ut_m();
        }
        else if (cmd == "ut-s") {
            command_ut_s();
        }
        else {
            cout << "Unknown command: " << cmd << "\n";
        }
    }
    cmdFile.close();
}

int main() {
    processCommands("commands.txt");
    return 0;
}
