#ifndef RackSchedulerDP_DEFINED
#define RackSchedulerDP_DEFINED

#include <vector>
#include <algorithm>

#include "Crop.hpp"
#include "CropsData.hpp"
#include "Cultivation.hpp"
#include "Rack.hpp"
#include "RackData.hpp"

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
        std::vector<std::vector<double>> DP(numTimeSlot, std::vector<double>(2, 0));
        // tracer for crop to be harvested by week for optimized revenue
        std::vector<std::vector<Crop*>> tracer(numTimeSlot, std::vector<Crop*>(2, nullptr));

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
                DP[week][MAINTAINED] = std::max(DP[week - 1][MAINTAINED], DP[week - 1][UN_MAINTAINED]);
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

#endif // RackSchedulerDP_DEFINED
