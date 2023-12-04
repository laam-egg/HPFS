#ifndef Crop_DEFINED
#define Crop_DEFINED

#include <string>
#include <vector>

class Crop {
public:
    std::string cropCode;
    std::string cropName;
    std::string familyName;
    int cultivationTimeUnits;
    bool needSunlight;
    double harvestedAmount;
    double followingHarvestedAmount;
    int multipleHarvestTimes;
    std::vector <double> pricePerWeeks;

    Crop(std::string cropCode, std::string cropName, std::string familyName, int cultivationTimeUnits, bool needSunlight, double harvestedAmount, int multipleHarvestTimes) {
        this->cropCode = cropCode;
        this->cropName = cropName;
        this->familyName = familyName;
        this->cultivationTimeUnits = cultivationTimeUnits;
        this->needSunlight = needSunlight;
        this->harvestedAmount = harvestedAmount;
        this->multipleHarvestTimes = multipleHarvestTimes;
    }

    void setPrices(std::vector<double> prices) {
        this->pricePerWeeks = prices;
    }
};

#endif // Crop_DEFINED
