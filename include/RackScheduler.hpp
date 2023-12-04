#ifndef RackScheduler_DEFINED
#define RackScheduler_DEFINED

#include <vector>

#include "CropsData.hpp"
#include "Cultivation.hpp"
#include "Rack.hpp"
#include "RackData.hpp"

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

    void scheduleRack(std::vector<Cultivation> cultivations, int rackId) {
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

#endif // RackScheduler_DEFINED
