#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <random>

#include "CropsData.hpp"
#include "RackScheduler.hpp"
#include "RackSchedulerDP.hpp"
#include "RackSchedulerDPMultipleHarvest.hpp"

const int SUNLIGHT = 1;
const int NO_SUNLIGHT = 0;


std::string rootFolder = "c:\\Users\\kurod\\Documents\\CPP\\HPFS";

std::string getAbsoluteFileName(std::string fileName) {

    return rootFolder + "\\" + fileName;
}


std::vector <double> regularSchedule;
std::vector <double> DPSchedule;
std::vector <double> shuffledSchedule;
std::vector <double> DPMSchedule;
std::vector <double> shuffledScheduleMultiple;

void scheduleForRackData(std::string fileName, CropsData * cropsData, std::string resultSuffix) {
    // cout << fileName << endl;
    std::string absFile = getAbsoluteFileName(fileName);

    std::cout << "Scheduling for rack data file: " << fileName << std::endl;
    RackScheduler scheduler(cropsData, cropsData->numberOfWeeks);
    scheduler.readRackData(absFile);
    scheduler.runSchedule();
    scheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix + "\\HPFS\\" + fileName)) ;
    regularSchedule.push_back(scheduler.totalRevenue);
    std::cout << "Finish HPFS heuristic scheduling, revenue: " << scheduler.totalRevenue << std::endl;

    RackSchedulerDP DPScheduler(cropsData, cropsData->numberOfWeeks);
    DPScheduler.readRackData(absFile);
    DPScheduler.runSchedule();
    DPScheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix+ "\\DP\\" + fileName)) ;
    DPSchedule.push_back(DPScheduler.totalRevenue);
    std::cout << "Finish dynamic programing scheduling, revenue: " << DPScheduler.totalRevenue << std::endl;

    // shuffled DP
    std::vector <int> shuffled_racks;
    for (size_t i = 0; i < DPScheduler.racksPermutation.size(); ++i) {
        shuffled_racks.push_back(i);
    }

    auto rng = std::default_random_engine{};
    std::shuffle(std::begin(shuffled_racks), std::end(shuffled_racks), rng);
    // double maxRevenue = DPScheduler.totalRevenue;
    RackSchedulerDP bestScheduler = DPScheduler;
    for(int i = 0; i < 10; ++i) {
        // cout << "Shuffle: " << i << endl;
        std::shuffle(std::begin(shuffled_racks), std::end(shuffled_racks), rng);
        RackSchedulerDP shuffledDPScheduler(cropsData, cropsData->numberOfWeeks);
        shuffledDPScheduler.readRackData(absFile);
        shuffledDPScheduler.racksPermutation = shuffled_racks;
        shuffledDPScheduler.runSchedule();
        if (bestScheduler.totalRevenue <  shuffledDPScheduler.totalRevenue) {
            bestScheduler = shuffledDPScheduler;
        }
    }
    std::cout << "Finish shuffled dynamic programing scheduling, revenue: " << bestScheduler.totalRevenue << std::endl;
    bestScheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix+ "\\shuffled_DP\\" + fileName)) ;
    shuffledSchedule.push_back(bestScheduler.totalRevenue);

    // DP Multiple Harvest
    RackSchedulerDPMultipleHarvest DPMScheduler(cropsData, cropsData->numberOfWeeks);
    DPMScheduler.readRackData(absFile);
    DPMScheduler.runSchedule();
    DPMScheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix+ "\\DP_Multiple\\" + fileName)) ;
    DPMSchedule.push_back(DPMScheduler.totalRevenue);
    std::cout << "Finish multiple harvest dynamic programing scheduling, revenue: " << DPMScheduler.totalRevenue << std::endl;


    std::vector <int> shuffled_racks_2;
    for(size_t i = 0; i < DPScheduler.racksPermutation.size(); ++i) {
        shuffled_racks_2.push_back(i);
    }
    rng = std::default_random_engine {};
    std::shuffle(std::begin(shuffled_racks_2), std::end(shuffled_racks_2), rng);
    // maxRevenue = DPMScheduler.totalRevenue;
    RackSchedulerDPMultipleHarvest bestSchedulerMultiple = DPMScheduler;
    for(int i = 0; i < 10; ++i) {
        // cout << "Shuffle: " << i << endl;
        std::shuffle(std::begin(shuffled_racks_2), std::end(shuffled_racks_2), rng);
        RackSchedulerDP shuffledDPMScheduler(cropsData, cropsData->numberOfWeeks);
        shuffledDPMScheduler.readRackData(absFile);
        shuffledDPMScheduler.racksPermutation = shuffled_racks_2;
        shuffledDPMScheduler.runSchedule();
        if (bestScheduler.totalRevenue <  shuffledDPMScheduler.totalRevenue) {
            bestScheduler = shuffledDPMScheduler;
        }
    }
    bestSchedulerMultiple.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix + "\\shuffled_DP_Multiple\\" + fileName)) ;
    shuffledScheduleMultiple.push_back(bestSchedulerMultiple.totalRevenue);
    std::cout << "Finish shuffled multiple harvest dynamic programing scheduling, revenue: " << bestSchedulerMultiple.totalRevenue << std::endl;
}

std::vector <int> num_racks = {8, 12, 20, 50, 100, 200, 400};
std::vector<std::string> rackDataFiles = {"rack_data\\rack_data_8.csv",
    "rack_data\\rack_data_12.csv",
    "rack_data\\rack_data_20.csv",
    "rack_data\\rack_data_50.csv",
    "rack_data\\rack_data_100.csv",
    "rack_data\\rack_data_200.csv",
    "rack_data\\rack_data_400.csv"};

void export_all_revenues(std::string resultSuffix) {
    std::ofstream file(getAbsoluteFileName("all_revenues/" + resultSuffix +  ".csv"));
    file << "numracks,data,regular_schedule,DP_schedule,shuffled_DP_schedule,multiple_harvest_schedule, suffled_multiple_harvest_schedule" << std::endl;
    for (size_t i = 0; i < rackDataFiles.size(); ++i) {
        file << num_racks[i] << ",";
        file << rackDataFiles[i] << ",";
        file << regularSchedule[i] << ",";
        file << DPSchedule[i] << ",";
        file << shuffledSchedule[i] << ",";
        file << DPMSchedule[i] << ",";
        file << shuffledScheduleMultiple[i] << std::endl;
    }
    file.close();
}

int main() {
    // rootFolder = "c:\\Users\\kurod\\Documents\\CPP\\HPFS";
    rootFolder = ".";
    std::string cropDataFile = "datasets/D2/New_QH_Crop_Data.csv";
    std::string priceDataFile = "datasets/D2/New_QH_Price_Data.csv";
    std::string resultSuffix = "D2";

    CropsData * cropData = new CropsData();
    cropData->getCropsDataFromFile(getAbsoluteFileName(cropDataFile));
    cropData->getCropsPriceFromFile(getAbsoluteFileName(priceDataFile));
    cropData->buildCultivations();

    for (std::string rackDataFile : rackDataFiles) {
        scheduleForRackData(rackDataFile, cropData, resultSuffix);
    }

    export_all_revenues(resultSuffix);
}
