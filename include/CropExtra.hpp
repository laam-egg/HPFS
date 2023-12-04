#ifndef CropExtra_DEFINED
#define CropExtra_DEFINED

#include "Crop.hpp"

struct CropExtra {
    Crop * crop;
    int extraTimes;

    CropExtra(Crop * crop, int extraTimes) {
        this->crop = crop;
        this->extraTimes = extraTimes;
    }
};

#endif // CropExtra_DEFINED
