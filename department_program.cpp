#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Check if the input department is in the registered list
bool isDepartmentRegistered(const string& department, const vector<string>& departments) {
    return find(departments.begin(), departments.end(), department) != departments.end();
}

// Print the contents of the simulated database
void printDatabase(const vector<vector<string>>& database) {
    cout << "\nCurrent Database Contents:\n";
    for (size_t i = 0; i < database.size(); ++i) {
        cout << "Row " << i + 1 << ": ";
        for (const auto& field : database[i]) {
            cout << field << " ";
        }
        cout << endl;
    }
}

int main() {
    vector<string> departments = {"HR", "Finance", "IT", "Marketing"}; // Registered departments
    vector<vector<string>> database; // Simulated database represented as a 2D vector
    string department, dataToEnter;

    cout << "1. INPUT INTERNAL DATA  2. REQUEST INVESTMENT DATA" << endl;
    cout << "Client is running for data input...\n" << endl;

    while (true) {
        // Ask the user to input a department
        cout << "What is your department? (Type 'end' to terminate): ";
        getline(cin, department); // Get user input

        if (department == "end") {
            cout << "Terminating the program..." << endl;
            break;
        }

        // Check if the department is registered
        if (isDepartmentRegistered(department, departments)) {
            cout << "Department '" << department << "' is registered." << endl;
        } else {
            cout << "Department '" << department << "' is not registered. Try again." << endl;
            continue;
        }

        // Ask the user to input data
        cout << "What data do you want to enter? (Type 'end' to terminate): ";
        getline(cin, dataToEnter);

        if (dataToEnter == "end") {
            cout << "Terminating the program..." << endl;
            break;
        }

        // Simulate saving data to the database
        vector<string> newRow = {department, dataToEnter};
        database.push_back(newRow); // Add the new row to the simulated database

        cout << "Data saved successfully!" << endl;

        // Display the contents of the database
        printDatabase(database);

        cout << "\nEnter new data or type 'end' to terminate." << endl;
    }

    return 0;
}
