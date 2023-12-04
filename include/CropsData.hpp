#ifndef CropsData_DEFINED
#define CropsData_DEFINED

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "Crop.hpp"
#include "Cultivation.hpp"
#include "CSVRow.hpp"

class CropsData {
public:
    int numberOfCrops;
    int numberOfWeeks;
    std::vector <Crop*> crops;
    std::vector <Cultivation> cultivations;
    CropsData() {}

    void getCropsDataFromFile(std::string fileName) {
        std::ifstream file(fileName.c_str());
        CSVRow row;

        file >> row;
        while(file >> row){
            int splitter = row[5].find("-");
            Crop * newCrop = new Crop(row[0], row[1], row[2], stoi(row[3]), stoi(row[4]), stod(row[5].substr(1, splitter - 1)), stoi(row[6]));
            crops.push_back(newCrop);
        }
        numberOfCrops = crops.size();
        file.close();
    }

    void getCropsPriceFromFile(std::string fileName) {
        std::ifstream file(fileName.c_str());
        CSVRow row;
        file >> row;

        while (file >> row) {
            std::vector<double> prices;

            for(int i = 1; i <= numberOfCrops; ++i) {
                for(int k = 0; k < 4; ++k) {
                    crops[i - 1]->pricePerWeeks.push_back(stod(row[i]));
                }
            }
        }
        numberOfWeeks = 40; //(crops[0]->pricePerWeeks).size();
        file.close();
    }

    void buildCultivations() {
        for (Crop * crop : crops) {
            for(int week = 0; week < numberOfWeeks; ++week) {
                if (week + 1 < crop->cultivationTimeUnits) {
                    continue;
                }
                double avgRevenue = crop->pricePerWeeks[week] * crop->harvestedAmount / crop->cultivationTimeUnits;
                cultivations.push_back(Cultivation(avgRevenue, crop, week));
            }
        }
        std::sort(cultivations.begin(), cultivations.end(),
            [](const Cultivation & fi, Cultivation & se) -> bool
            {
                if (fi.avgRevenue != se.avgRevenue) {
                    return fi.avgRevenue < se.avgRevenue;
                }
                else {
                    return fi.crop->cultivationTimeUnits > se.crop->cultivationTimeUnits;
                }
            });

        int fastedCultivationBy;
        for (size_t i = 0; i < cultivations.size(); ++i) {
            if (i == 0) {
                fastedCultivationBy = cultivations[i].crop->cultivationTimeUnits;
            }
            else {
                fastedCultivationBy = std::min(fastedCultivationBy, cultivations[i].crop->cultivationTimeUnits);
            }
            cultivations[i].fastedCultivationBy = fastedCultivationBy;
        }
    }
};

#endif // CropsData_DEFINED
