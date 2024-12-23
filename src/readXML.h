#include <tinyxml2.h>
#include <iostream>
#include <map>
#include <vector>

struct FaceData {
    std::string colorCode;
    std::string positionCode;
    std::string textureType;
};

std::vector<FaceData> readXML();
void updateXML(const std::vector<FaceData>& faceDatas);
std::vector<std::string> readActionsFromXML();