#ifndef Cultivation_DEFINED
#define Cultivation_DEFINED

#include "Crop.hpp"

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

#endif // Cultivation_DEFINED
