#include "readXML.h"
#include <vector>

std::vector<FaceData> readXML() {
    tinyxml2::XMLDocument doc;
    doc.LoadFile("./data/EloMaluco.xml");

    tinyxml2::XMLElement* root = doc.FirstChildElement("EloMaluco");
    tinyxml2::XMLElement* estadoAtual = root->FirstChildElement("EstadoAtual");

    std::vector<FaceData> faceDatas;

    for (tinyxml2::XMLElement* row = estadoAtual->FirstChildElement("row"); row != nullptr; row = row->NextSiblingElement("row")) {
        for (tinyxml2::XMLElement* col = row->FirstChildElement("col"); col != nullptr; col = col->NextSiblingElement("col")) {
            std::string faceCode = col->GetText() ? col->GetText() : "";
            std::string textureType = col->Attribute("texture") ? col->Attribute("texture") : "link";
            
            FaceData faceData;
            faceData.colorCode = faceCode.length() >= 2 ? faceCode.substr(0, 2) : faceCode;
            faceData.positionCode = faceCode.length() >= 4 ? faceCode.substr(2, 2) : faceCode.substr(2, 1);
            faceData.textureType = textureType;
            
            faceDatas.push_back(faceData);
        }
    }

    return faceDatas;
}


void updateXML(const std::vector<FaceData>& faceDatas) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile("./data/EloMaluco.xml");

    tinyxml2::XMLElement* root = doc.FirstChildElement("EloMaluco");
    tinyxml2::XMLElement* estadoAtual = root->FirstChildElement("EstadoAtual");

    int index = 0;
    for (tinyxml2::XMLElement* row = estadoAtual->FirstChildElement("row"); row != nullptr; row = row->NextSiblingElement("row")) {
        for (tinyxml2::XMLElement* col = row->FirstChildElement("col"); col != nullptr; col = col->NextSiblingElement("col")) {
            const FaceData& faceData = faceDatas[index];
            std::string faceCode = faceData.colorCode + faceData.positionCode;
            col->SetText(faceCode.c_str());
            col->SetAttribute("texture", faceData.textureType.c_str());
            index++;
        }
    }

    doc.SaveFile("./data/EloMaluco.xml");
}

std::vector<std::string> readActionsFromXML() {
    std::vector<std::string> actions;
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile("./data/EloMaluco.xml") == tinyxml2::XML_SUCCESS) {
        tinyxml2::XMLElement* root = doc.FirstChildElement("EloMaluco");
        if (root) {
            tinyxml2::XMLElement* acoesElement = root->FirstChildElement("Acoes");
            if (acoesElement) {
                for (tinyxml2::XMLElement* acaoElement = acoesElement->FirstChildElement("acao"); acaoElement; acaoElement = acaoElement->NextSiblingElement("acao")) {
                    const char* acaoText = acaoElement->GetText();
                    if (acaoText) {
                        actions.push_back(acaoText);
                    }
                }
            }
        }
    }
    return actions;
}
