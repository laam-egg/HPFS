#include <bits/stdc++.h>

using namespace std;

const int SUNLIGHT = 1;
const int NO_SUNLIGHT = 0;

class CSVRow
{
    public:
        std::string operator[](std::size_t index) const
        {
            return std::string(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
        }
        std::size_t size() const
        {
            return m_data.size() - 1;
        }
        void readNextRow(std::istream& str)
        {
            std::getline(str, m_line);
            m_data.clear();
            m_data.emplace_back(-1);
            std::string::size_type pos = 0;
            while((pos = m_line.find(',', pos)) != std::string::npos)
            {
                m_data.emplace_back(pos);
                ++pos;
            }
            // This checks for a trailing comma with no data after it.
            pos   = m_line.size();
            m_data.emplace_back(pos);
        }
        std::string         m_line;
        std::vector<int>    m_data;
};

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}

class Crop {
public:
    string cropCode;
    string cropName; 
    string familyName;
    int cultivationTimeUnits;
    bool needSunlight;
    double harvestedAmount;
    double followingHarvestedAmount;
    int multipleHarvestTimes;
    vector <double> pricePerWeeks;

    Crop(string cropCode, string cropName, string familyName, int cultivationTimeUnits, bool needSunlight, double harvestedAmount, int multipleHarvestTimes) {
        this->cropCode = cropCode;
        this->cropName = cropName;
        this->familyName = familyName;
        this->cultivationTimeUnits = cultivationTimeUnits;
        this->needSunlight = needSunlight;
        this->harvestedAmount = harvestedAmount;
        this->multipleHarvestTimes = multipleHarvestTimes;
    }

    void setPrices(vector<double> prices) {
        this->pricePerWeeks = prices;
    }
};

class Cultivation {
public:
    double avgRevenue;
    double totalRevenue;
    Crop* crop;
    int harvest_week;
    int fastedCultivationBy;

    Cultivation(int avgRevenue, Crop * crop, int harvest_week) {
        this->avgRevenue = avgRevenue;
        this->crop = crop;
        this->harvest_week = harvest_week;
        this->totalRevenue = crop->pricePerWeeks[harvest_week] * crop->harvestedAmount ;
    }

    int start_week() {
        return harvest_week - crop->cultivationTimeUnits + 1;
    }
};

class CropsData {
public:
    int numberOfCrops;
    int numberOfWeeks;
    vector <Crop*> crops;
    vector <Cultivation> cultivations;
    CropsData() {}  

    void getCropsDataFromFile(string fileName) {
        ifstream file(fileName.c_str());
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

    void getCropsPriceFromFile(string fileName) {
        ifstream file(fileName.c_str());
        CSVRow row;
        file >> row; 


        int counter = 0;
        while (file >> row) {
            vector<double> prices;

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
        sort(cultivations.begin(), cultivations.end(), 
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
        for(int i = 0; i < cultivations.size(); ++i) {
            if (i == 0) {
                fastedCultivationBy = cultivations[i].crop->cultivationTimeUnits;
            }
            else {
                fastedCultivationBy = min(fastedCultivationBy, cultivations[i].crop->cultivationTimeUnits);
            }
            cultivations[i].fastedCultivationBy = fastedCultivationBy;
        }
    }
};

int a = 0;
class Rack {
public:
    int rackId;
    int numberOfTimeSlot;
    int numAvailableTimeSlot;
    bool haveSunlight;
    vector<Cultivation> cultivations;
    vector<Crop*> cropAtTimeSlot; 
    vector<Rack*> adjacentRacks;

    Rack(int rackId, int numberOfTimeSlot) {
        this->rackId = rackId;
        this->numberOfTimeSlot = numberOfTimeSlot;
        this->numAvailableTimeSlot = numberOfTimeSlot;
        cropAtTimeSlot = vector<Crop*>(numberOfTimeSlot, nullptr);
    }

    bool canInsertCultivation(Cultivation cultivation, int extraHarvest = 0){
        if(cultivation.start_week() < 0) {
            return false; 
        }

        for (int w = cultivation.start_week(); w <= cultivation.harvest_week + extraHarvest; ++w) {
            if (cropAtTimeSlot[w] != nullptr) {
                return false;
            }
        }
        return true;
    }


    bool isFamily(const Crop * fi, const Crop * se) {
        if (fi == nullptr || se == nullptr) 
            return false;
        return (fi->familyName == se->familyName); 
    }

    bool isValidCultivation(Cultivation cultivation, int extraHarvest = 0) {
        if (cultivation.crop->needSunlight && !haveSunlight) {
            return false;
        }

        for(Rack * adjacentRack: adjacentRacks) {
            for(int w = cultivation.start_week(); w <= cultivation.harvest_week + extraHarvest; ++w) {
                // cout << adjacentRack->cropAtTimeSlot.size() << endl;
                if (isFamily(adjacentRack->cropAtTimeSlot[w], cultivation.crop)) {
                    return false;
                }
            }
        }
        return true;
    }

    double insertCultivation(Cultivation cultivation, int extraHarvest = 0) {
        double revenue = 0;
        cultivations.push_back(cultivation);
        for(int w = cultivation.start_week(); w <= cultivation.harvest_week + extraHarvest; ++w) {
            if (w > cultivation.harvest_week) {
                revenue += cultivation.crop->pricePerWeeks[w] * cultivation.crop->harvestedAmount * 0.95;
            }
            cropAtTimeSlot[w] = cultivation.crop;
        }
        numAvailableTimeSlot -= cultivation.crop->cultivationTimeUnits;
        return revenue;
    }

    bool tryCultivation(Cultivation cultivation) {
        if (canInsertCultivation(cultivation)) {
            if (isValidCultivation(cultivation)) {
                insertCultivation(cultivation);
                return true;
            }
        }
        return false;
    }
};

class RackData {
public:
    int numTimeSlot;
    int numberOfRack;
    int numRow;
    int numColumn;
    double totalRevenue;
    vector <int> racksPermutation; 
    vector <Rack> racks;

    RackData(int numTimeSlot) {
        this->numTimeSlot = numTimeSlot;
        this->totalRevenue = 0;
    }

    void readRackData(string fileName) {
        ifstream file(fileName.c_str());
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

    initRackData(CSVRow row) {
        this->numberOfRack = stoi(row[0]);
        this->numRow = stoi(row[1]);
        this->numColumn = stoi(row[2]);
        for(int i = 0; i < numberOfRack; ++i)  {
            racks.push_back(Rack(i, numTimeSlot));
        }
        this->racksPermutation = vector<int>(numberOfRack);
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

    void writeResultToFile(string fileName) {
        ofstream file(fileName.c_str());    
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
} ;

class RackScheduler : public RackData {
public:
    CropsData * cropsData;


    RackScheduler(CropsData * cropsData, int numTimeSlot) : RackData(numTimeSlot){
        this->cropsData = cropsData;
    }

    bool tryInsertCultivationToRack(Cultivation cultivation, Rack * rack) {
        if (rack->tryCultivation(cultivation)) {
            totalRevenue += cultivation.totalRevenue;
            return true;
        }      
        return false;
    }

    void scheduleRack(vector<Cultivation> cultivations, int rackId) {
        Rack * rack = &racks[rackId];
        while(cultivations.size() > 0) {
            Cultivation bestCultivation = cultivations.back();
            if (bestCultivation.fastedCultivationBy > racks[rackId].numAvailableTimeSlot) {
                break;
            }

            if (tryInsertCultivationToRack(bestCultivation, rack)) {
                continue;
            }

            Cultivation forwardCultivation = bestCultivation;
            forwardCultivation.harvest_week -= 1;
            if (tryInsertCultivationToRack(forwardCultivation, rack)) {
                continue;
            }

            Cultivation delayCultivation = bestCultivation;
            delayCultivation.harvest_week += 1;
            if (tryInsertCultivationToRack(delayCultivation, rack)) {
                continue;
            }
            cultivations.pop_back();
        }
    }

    void runSchedule() {
        for(int rackId: racksPermutation) {
            scheduleRack(cropsData->cultivations, rackId);
        }
    }
};

class RackSchedulerDP : public RackData {
public:
    int MAINTAINED = 1;
    int UN_MAINTAINED = 0;
    CropsData * cropsData;

    RackSchedulerDP(CropsData * cropsData, int numberOfWeeks) : RackData(numberOfWeeks){
        this->cropsData = cropsData;
    }
    
    void scheduleRack(int rackId) {
        Rack * rack = &racks[rackId];
        // optimized revenue by weeks
        vector<vector<double>> DP(numTimeSlot, vector<double>(2, 0));
        // tracer for crop to be harvested by week for optimized revenue
        vector<vector<Crop*>> tracer(numTimeSlot, vector<Crop*>(2, nullptr));

        // For through weeks
        for(int week = 0; week < numTimeSlot; ++week) {
            // Intialize optimize revenue and tracer for first week
            if (week == 0) {
                DP[week][MAINTAINED] = DP[week][UN_MAINTAINED] = 0;
                tracer[week][UN_MAINTAINED] = tracer[week][MAINTAINED] = nullptr;
            }
            else {
                // Intialize  for following weeks ...
                // set optimized revenue for this week un-maintained = optimized revenue for last week un-maintained (skip this week)
                DP[week][UN_MAINTAINED] = DP[week - 1][UN_MAINTAINED]; 
                // set optimized revenue for this week maintained = max optimized revenue for last week maintained / un-maintained (skip this week)
                DP[week][MAINTAINED] = max(DP[week - 1][MAINTAINED], DP[week - 1][UN_MAINTAINED]);
            }

            // consider all crops can be harvested on this week
            for(Crop * crop: cropsData->crops) {
                if (crop->cultivationTimeUnits > week + 1) {
                    continue;
                }

                double value = crop->pricePerWeeks[week] * crop->harvestedAmount * 0.95;
                Cultivation cultivation(value, crop, week);
                if(!rack->canInsertCultivation(cultivation) || !rack->isValidCultivation(cultivation)) {
                    continue;
                }

                // if crop need to be cultivated at first week so rack couldn't be maintained yet
                if (crop->cultivationTimeUnits == week + 1) {
                    if(value > DP[week][UN_MAINTAINED]) {
                        DP[week][UN_MAINTAINED] = value; 
                        tracer[week][UN_MAINTAINED] = crop;
                    }
                    continue;
                }

                // if there are some free week before crop cultivated
                for(int maintained = UN_MAINTAINED; maintained <= MAINTAINED; ++maintained) {
                    int prevWeek = week - crop->cultivationTimeUnits;
                    // if we can cultivate this crop and get higher revenue
                    if (DP[prevWeek][maintained] + value > DP[week][maintained]) {
                        DP[week][maintained] = DP[prevWeek][maintained] + value;
                        tracer[week][maintained] = crop;
                    }
                }
            }
        }

        // to trace week back to week
        int week = numTimeSlot - 1;
        // to trace with maintanace condition
        int maintained = MAINTAINED;
        while(week >= 0) {
            // cout << week << " " << maintained << endl;
            if (tracer[week][maintained] != nullptr) {
                Crop * crop = tracer[week][maintained];
                Cultivation cultivation(0, crop, week);
                rack->insertCultivation(cultivation);
                this->totalRevenue += cultivation.totalRevenue;
                week -= crop->cultivationTimeUnits;
            }
            else {
                if(maintained == MAINTAINED && DP[week][1] == DP[week - 1][UN_MAINTAINED]) 
                    maintained = UN_MAINTAINED;
                week -= 1;
            }
        }
    }

    void runSchedule() {
        for(int rackId: racksPermutation) {
            scheduleRack(rackId);
        }
    }
};


struct CropExtra {
    Crop * crop; 
    int extraTimes;

    CropExtra(Crop * crop, int extraTimes) {
        this->crop = crop;
        this->extraTimes = extraTimes;
    }
};

class RackSchedulerDPMultipleHarvest : public RackData {
public:
    int MAINTAINED = 1;
    int UN_MAINTAINED = 0;
    CropsData * cropsData;

    RackSchedulerDPMultipleHarvest(CropsData * cropsData, int numberOfWeeks) : RackData(numberOfWeeks){
        this->cropsData = cropsData;
    }
    
    void scheduleRack(int rackId) {
        Rack * rack = &racks[rackId];
        // optimized revenue by weeks
        vector<vector<double>> DP(numTimeSlot, vector<double>(2, 0));
        CropExtra nul = CropExtra(nullptr, -1);
        // tracer for crop to be harvested by week for optimized revenue
        vector<vector<CropExtra>> tracer(numTimeSlot, vector<CropExtra>(2, nul));

        // For through weeks
        for(int week = 0; week < numTimeSlot; ++week) {
            // Intialize optimize revenue and tracer for first week
            if (week == 0) {
                DP[week][MAINTAINED] = DP[week][UN_MAINTAINED] = 0;
                tracer[week][UN_MAINTAINED] = tracer[week][MAINTAINED] = nul;
            }
            else {
                // Intialize  for following weeks ...
                // set optimized revenue for this week un-maintained = optimized revenue for last week un-maintained (skip this week)
                DP[week][UN_MAINTAINED] = DP[week - 1][UN_MAINTAINED]; 
                // set optimized revenue for this week maintained = max optimized revenue for last week maintained / un-maintained (skip this week)
                DP[week][MAINTAINED] = max(DP[week - 1][MAINTAINED], DP[week - 1][UN_MAINTAINED]);
            }

            // consider all crops can be harvested on this week
            for(Crop * crop: cropsData->crops) {
                double value = 0;
                for (int extraHarvest = 0; extraHarvest <= crop->multipleHarvestTimes; ++extraHarvest) {
                    int totalTimesUnits = crop->cultivationTimeUnits + extraHarvest;

                    if (totalTimesUnits > week + 1) {
                        continue;
                    }
                    
                    value += crop->pricePerWeeks[week - extraHarvest] * crop->harvestedAmount * 0.95;

                    Cultivation cultivation(value, crop, week - extraHarvest);

                    if(!rack->isValidCultivation(cultivation, extraHarvest)) {
                        continue;
                    }

                    // if crop need to be cultivated at first week so rack couldn't be maintained yet
                    if (totalTimesUnits == week + 1) {
                        if(value > DP[week][UN_MAINTAINED]) {
                            DP[week][UN_MAINTAINED] = value; 
                            tracer[week][UN_MAINTAINED] =  CropExtra(crop, extraHarvest);
                        }
                        continue;
                    }

                    // if there are some free week before crop cultivated
                    for(int maintained = UN_MAINTAINED; maintained <= MAINTAINED; ++maintained) {
                        int prevWeek = week - totalTimesUnits;
                        // if we can cultivate this crop and get higher revenue
                        if (DP[prevWeek][maintained] + value > DP[week][maintained]) {
                            DP[week][maintained] = DP[prevWeek][maintained] + value;
                            tracer[week][maintained] =  CropExtra(crop, extraHarvest);
                        }
                    }
                }
            }
        }

        // to trace week back to week
        int week = numTimeSlot - 1;
        // to trace with maintanace condition
        int maintained = MAINTAINED;
        while(week >= 0) {
            // cout << week << " " << maintained << endl;
            if (tracer[week][maintained].extraTimes != -1) {
                CropExtra cropExtra = tracer[week][maintained]; 
                Crop * crop = cropExtra.crop; 
                int extraHarvest = cropExtra.extraTimes;
                Cultivation cultivation(0, crop, week - extraHarvest);
                
                this->totalRevenue += rack->insertCultivation(cultivation, extraHarvest);
                week -= crop->cultivationTimeUnits + extraHarvest;
            }
            else {
                if(maintained == MAINTAINED && (week == 0 || DP[week][1] == DP[week - 1][UN_MAINTAINED]) )
                    maintained = UN_MAINTAINED;
                week -= 1;
            }
        }
        // cout << rackId << endl;
        // for (int i = 0; i < 40; ++i) {
        //     cout << rack->cropAtTimeSlot[i]->cropCode << endl;
        // }
    }

    void runSchedule() {
        for(int rackId: racksPermutation) {
            scheduleRack(rackId);
        }
    }
};

string getAbsoluteFileName(string fileName) {
    return "c:\\Users\\kurod\\Documents\\CPP\\HPFS\\" + fileName;
}


vector <double> regularSchedule; 
vector <double> DPSchedule; 
vector <double> shuffledSchedule; 
vector <double> DPMSchedule;
vector <double> shuffledScheduleMultiple;

void scheduleForRackData(string fileName, CropsData * cropsData, string resultSuffix) {
    // cout << fileName << endl;
    string absFile = getAbsoluteFileName(fileName);

    RackScheduler scheduler(cropsData, cropsData->numberOfWeeks);
    scheduler.readRackData(absFile);
    scheduler.runSchedule();
    scheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix + "\\HPFS\\" + fileName)) ;
    regularSchedule.push_back(scheduler.totalRevenue);

    RackSchedulerDP DPScheduler(cropsData, cropsData->numberOfWeeks); 
    DPScheduler.readRackData(absFile);
    DPScheduler.runSchedule();
    DPScheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix+ "\\DP\\" + fileName)) ;
    DPSchedule.push_back(DPScheduler.totalRevenue);

    // shuffled DP
    vector <int> shuffled_racks;
    for(int i = 0; i < DPScheduler.racksPermutation.size(); ++i) {
        shuffled_racks.push_back(i);
    }

    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(shuffled_racks), std::end(shuffled_racks), rng);
    double maxRevenue = DPScheduler.totalRevenue;
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
    cout << bestScheduler.totalRevenue << endl;
    bestScheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix+ "\\shuffled_DP\\" + fileName)) ;
    shuffledSchedule.push_back(bestScheduler.totalRevenue);

    // DP Multiple Harvest
    RackSchedulerDPMultipleHarvest DPMScheduler(cropsData, cropsData->numberOfWeeks); 
    DPMScheduler.readRackData(absFile);
    DPMScheduler.runSchedule();
    DPMScheduler.writeResultToFile(getAbsoluteFileName("result_" + resultSuffix+ "\\DP_Multiple\\" + fileName)) ;
    DPMSchedule.push_back(DPMScheduler.totalRevenue);


    vector <int> shuffled_racks_2;
    for(int i = 0; i < DPScheduler.racksPermutation.size(); ++i) {
        shuffled_racks_2.push_back(i);
    }
    rng = std::default_random_engine {};
    std::shuffle(std::begin(shuffled_racks_2), std::end(shuffled_racks_2), rng);
    maxRevenue = DPMScheduler.totalRevenue;
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
}

vector <int> num_racks = {8, 12, 20, 50, 100, 200, 400};
vector<string> rackDataFiles = {"rack_data\\rack_data_8.csv",
    "rack_data\\rack_data_12.csv",
    "rack_data\\rack_data_20.csv",
    "rack_data\\rack_data_50.csv",
    "rack_data\\rack_data_100.csv",
    "rack_data\\rack_data_200.csv",
    "rack_data\\rack_data_400.csv"};

void export_all_revenues(string resultSuffix) {
    ofstream file(getAbsoluteFileName("all_revenues/" + resultSuffix +  ".csv").c_str()); 
    file << "numracks,data,regular_schedule,DP_schedule,shuffled_DP_schedule" << endl;;
    for(int i = 0; i < rackDataFiles.size(); ++i) {
        file << num_racks[i] << "," << rackDataFiles[i] << "," << regularSchedule[i] << "," << DPSchedule[i] << "," << shuffledSchedule[i] << "," << DPMSchedule[i] << "," << shuffledScheduleMultiple[i] << endl;
    }
    file.close();
}

int main() {
    string cropDataFile = "datasets/D2/New_QH_Crop_Data.csv";
    string priceDataFile = "datasets/D2/New_QH_Price_Data.csv";
    string resultSuffix = "D2";

    CropsData * cropData = new CropsData();
    cropData->getCropsDataFromFile(getAbsoluteFileName(cropDataFile));
    cropData->getCropsPriceFromFile(getAbsoluteFileName(priceDataFile));
    cropData->buildCultivations();

    for (string rackDataFile : rackDataFiles) {
        scheduleForRackData(rackDataFile, cropData, resultSuffix);
    }

    export_all_revenues(resultSuffix);
}
