#include <iostream>
#include "SDK.h"

using namespace SDK;

int main() {
    Initialize(".");
    std::cerr << "Have " << GetAvailableCameras() <<"\n";
    std::cerr << "Temperature is " << GetTemperatureF().second;
    std::pair<int,int> detector = GetDetector();
    std::cerr << "Detector width is " << detector.first << "\n";

    ShutDown();

    return 0;
}
