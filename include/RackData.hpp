#ifndef RackData_DEFINED
#define RackData_DEFINED

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

#include "CSVRow.hpp"
#include "Rack.hpp"

class RackData {
public:
    int numTimeSlot;
    int numberOfRack;
    int numRow;
    int numColumn;
    double totalRevenue;
    std::vector <int> racksPermutation;
    std::vector <Rack> racks;

    RackData(int numTimeSlot) {
        this->numTimeSlot = numTimeSlot;
        this->totalRevenue = 0;
    }

    void readRackData(std::string fileName) {
        std::ifstream file(fileName);
        if (!file) {
            std::cerr << "ERROR: Could not open file: " << fileName << std::endl;
            exit(1);
        }

        CSVRow row;

        file >> row; // Read header

        file >> row;
        this->initRackData(row);

        file >> row; // Read header

        file >> row;
        setSunlightConfig(row);
        buildAdjacentGraph();
        file.close();
    }

    void initRackData(CSVRow row) {
        this->numberOfRack = stoi(row[0]);
        this->numRow = stoi(row[1]);
        this->numColumn = stoi(row[2]);
        for(int i = 0; i < numberOfRack; ++i)  {
            racks.push_back(Rack(i, numTimeSlot));
        }
        this->racksPermutation = std::vector<int>(numberOfRack);
        for(int i = 0; i < numberOfRack; ++i) {
            racksPermutation[i] = i;
        }
    }

    bool isValidRack(int row, int column) {
        return row >= 0 && row < this->numRow && column >= 0 && column < this->numColumn;
    }

    void buildAdjacentRacks(int row, int column) {
        int rackId = row * this->numColumn + column;

        // U already know what this is
        int hx[] = {0, 0, 1, -1};
        int hy[] = {1, -1, 0, 0};

        for (int i = 0; i < 4; ++i) {
            int adjacentRow = row + hx[i];
            int adjacentColumn = column + hy[i];
            if (isValidRack(adjacentRow, adjacentColumn)) {
                int adjacentRack = adjacentRow * numColumn + adjacentColumn;
                racks[rackId].adjacentRacks.push_back(&racks[adjacentRack]);
            }
        }
    }

    void buildAdjacentGraph() {
        for(int row = 0; row < this->numRow; ++row) {
            for(int column = 0; column < this->numColumn; ++column) {
                buildAdjacentRacks(row, column);
            }
        }
    }

    void setSunlightConfig(CSVRow row) {
        for(int rackId = 0; rackId < numberOfRack; ++rackId) {
            racks[rackId].haveSunlight = stoi(row[rackId]);
        }
    }

    void writeResultToFile(std::string fileName) {
        std::ofstream file(fileName.c_str());
        file << "Revenue\n";

        file << this->totalRevenue << "\n";

        file << "Rack/Week";
        for(int week = 1; week <= numTimeSlot; ++week) {
            file << "," << week;
        }
        file << "\n";

        bool isMaintained = false;
        int counter = 0;
        for(int rackId = 0; rackId < numberOfRack; ++rackId) {
            file << rackId + 1;
            for (Crop * crop : racks[rackId].cropAtTimeSlot) {
                if (crop == nullptr) {
                    if (!isMaintained) {
                        file << ",M";
                        isMaintained = true;
                    } else {
                        file << ",";
                    }
                } else {
                    counter += 1;
                    if (counter == crop->cultivationTimeUnits) {
                        counter = 0;
                        file << ",E";
                    } else {
                        file << "," << crop->cropCode;
                    }
                }
            }
            file << "\n";
        }
        // file.close();
    }
};

#endif // RackData_DEFINED
