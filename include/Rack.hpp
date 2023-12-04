#ifndef Rack_DEFINED
#define Rack_DEFINED

#include <vector>
#include "Crop.hpp"
#include "Cultivation.hpp"

class Rack {
public:
    int rackId;
    int numberOfTimeSlot;
    int numAvailableTimeSlot;
    bool haveSunlight;
    std::vector<Cultivation> cultivations;
    std::vector<Crop*> cropAtTimeSlot;
    std::vector<Rack*> adjacentRacks;

    Rack(int rackId, int numberOfTimeSlot) {
        this->rackId = rackId;
        this->numberOfTimeSlot = numberOfTimeSlot;
        this->numAvailableTimeSlot = numberOfTimeSlot;
        cropAtTimeSlot = std::vector<Crop*>(numberOfTimeSlot, nullptr);
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

#endif // Rack_DEFINED
