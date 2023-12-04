#ifndef RackSchedulerDPMultipleHarvest_DEFINED
#define RackSchedulerDPMultipleHarvest_DEFINED

#include <vector>

#include "Crop.hpp"
#include "CropExtra.hpp"
#include "CropsData.hpp"
#include "Cultivation.hpp"
#include "Rack.hpp"
#include "RackData.hpp"

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
        std::vector<std::vector<double>> DP(numTimeSlot, std::vector<double>(2, 0));
        CropExtra nul = CropExtra(nullptr, -1);
        // tracer for crop to be harvested by week for optimized revenue
        std::vector<std::vector<CropExtra>> tracer(numTimeSlot, std::vector<CropExtra>(2, nul));

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
                DP[week][MAINTAINED] = std::max(DP[week - 1][MAINTAINED], DP[week - 1][UN_MAINTAINED]);
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

#endif // RackSchedulerDPMultipleHarvest_DEFINED
