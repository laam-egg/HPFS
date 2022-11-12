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
        this->totalRevenue = crop->pricePerWeeks[harvest_week] * crop->harvestedAmount * 0.95;
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
        cout << fileName << endl;
        file >> row; // read header
        while(file >> row){
            int splitter = row[5].find("-");
            //Debug
            for(int i = 0; i < 7; ++i) {
                cout << row[i] << " ";
            }
            cout << row[5].substr(1, splitter - 1) << endl;
            //End debug
            Crop * newCrop = new Crop(row[0], row[1], row[2], stoi(row[3]), stoi(row[4]), stod(row[5].substr(1, splitter - 1)), stoi(row[6]));
            crops.push_back(newCrop);
        }
        numberOfCrops = crops.size();
    }   

    void getCropsPriceFromFile(string fileName) {
        ifstream file(fileName.c_str());
        CSVRow row;
        cout << fileName << endl;
        file >> row; // read header
            //Debug
            for(int i = 0; i <= numberOfCrops; ++i) {
                cout << row[i] << ",";
            }
            cout << endl; 
            //End debug

        int counter = 0;
        while (file >> row) {
            vector<double> prices;

            //Debug
            for(int i = 0; i <= numberOfCrops; ++i) {
                cout << row[i] << " ";
            }
            cout << endl; 
            //End debug

            for(int i = 1; i <= numberOfCrops; ++i) {
                crops[i - 1]->pricePerWeeks.push_back(stod(row[i]));
            }
        }
        numberOfWeeks = (crops[0]->pricePerWeeks).size();
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

    bool canInsertCultivation(Cultivation cultivation){
        if(cultivation.start_week() < 0) {
            return false; 
        }

        for (int w = cultivation.start_week(); w <= cultivation.harvest_week; ++w) {
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

    bool isValidCultivation(Cultivation cultivation) {
        if (cultivation.crop->needSunlight && !haveSunlight) {
            return false;
        }

        for(Rack * adjacentRack: adjacentRacks) {
            for(int w = cultivation.start_week(); w <= cultivation.harvest_week; ++w) {
                if (isFamily(adjacentRack->cropAtTimeSlot[w], cultivation.crop)) {
                    return false;
                }
            }
        }
        return true;
    }

    void insertCultivation(Cultivation cultivation) {
        cultivations.push_back(cultivation);
        for(int w = cultivation.start_week(); w <= cultivation.harvest_week; ++w) {
            cropAtTimeSlot[w] = cultivation.crop;
        }
        numAvailableTimeSlot -= cultivation.crop->cultivationTimeUnits;
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
    vector <int> racksPermutation; 
    vector <Rack> racks;

    RackData(int numTimeSlot) {
        this->numTimeSlot = numTimeSlot;
    }

    void readRackData(string fileName) {
        ifstream file(fileName.c_str());
        CSVRow row;

        file >> row; // Read header 

        file >> row;
        this->initRackData(row);

        //Debug
        cout << numberOfRack << " " << numRow << " " << numColumn << endl;
        //End debug 

        file >> row; // Read header 
        
        file >> row;
        //Debug
        for (int i = 0; i < numberOfRack; ++i) {
            cout << row[i] << ",";
        }
        cout << endl;
        //End debug
        setSunlightConfig(row);
        buildAdjacentGraph();
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
} ;

class RackScheduler : public RackData {
public:
    CropsData * cropsData;

    double totalRevenue;

    RackScheduler(CropsData * cropsData, int numTimeSlot) : RackData(numTimeSlot){
        this->cropsData = cropsData;
        this->totalRevenue = 0;
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

    void writeResultToFile(string fileName) {
        ofstream file(fileName.c_str());    
        file << "Revenue\n";

        file << this->totalRevenue << "\n";

        file << "Rack/Week";
        for(int week = 1; week <= numTimeSlot; ++week) {
            file << "," << week;
        }
        file << "\n";

        for(int rackId = 0; rackId < numberOfRack; ++rackId) {
            file << rackId + 1;
            for (Crop * crop : racks[rackId].cropAtTimeSlot) {
                file << "," << (crop != nullptr ? crop->cropCode : "");
            }
            file << "\n";
        }
    }
};

class RackSchedulerDP : public RackData {
public:
    const int MAINTAINED = 1;
    const int UN_MAINTAINED = 0;
    CropsData * cropsData;

    double totalRevenue;

    RackSchedulerHPFS_DP(CropsData * cropsData, int numberOfWeeks) : RackData(numberOfWeeks){
        this->cropsData = cropsData;
        this->totalRevenue = 0;
    }
    
    void scheduleRack(int rackId) {
        cout << rackId << endl;
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
            cout << week << " " << maintained << endl;
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

    void writeResultToFile(string fileName) {
        ofstream file(fileName.c_str());    
        file << "Revenue\n";

        file << this->totalRevenue << "\n";

        file << "Rack/Week";
        for(int week = 1; week <= numTimeSlot; ++week) {
            file << "," << week;
        }
        file << "\n";

        for(int rackId = 0; rackId < numberOfRack; ++rackId) {
            file << rackId + 1;
            for (Crop * crop : racks[rackId].cropAtTimeSlot) {
                file << "," << (crop != nullptr ? crop->cropCode : "");
            }
            file << "\n";
        }
    }
};


string getAbsoluteFileName(string fileName) {
    return "c:\\Users\\kurod\\Documents\\CPP\\HPFS\\" + fileName;
}

void scheduleForRackData(string fileName, CropsData * cropsData) {
    string absFile = getAbsoluteFileName(fileName);

    RackScheduler scheduler(cropsData, cropsData->numberOfWeeks);
    scheduler.readRackData(absFile);
    scheduler.runSchedule();
    scheduler.writeResultToFile(getAbsoluteFileName("result_" + fileName)) ;

    RackSchedulerDP DPScheduler(cropsData, cropsData->numberOfWeeks); 
    DPScheduler.readRackData(absFile);
    DPScheduler.runSchedule();
    DPScheduler.writeResultToFile(getAbsoluteFileName("resultDP_" + fileName)) ;
}


int main() {
    string cropDataFile = "New_QH_Crop_Data.csv";
    string priceDataFile = "New_QH_Price_Data.csv";
    
    vector<string> rackDataFiles = {"Rack_Data.csv"}; 

    CropsData * cropData = new CropsData();
    cropData->getCropsDataFromFile(getAbsoluteFileName(cropDataFile));
    cropData->getCropsPriceFromFile(getAbsoluteFileName(priceDataFile));
    cropData->buildCultivations();

    for (string rackDataFile : rackDataFiles) {
        scheduleForRackData(rackDataFile, cropData);
    }
}
