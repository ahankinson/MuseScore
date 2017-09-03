//
//  exportmei.hpp
//  mscore
//
//  Created by Andrew Hankinson on 2017-01-15.
//
//

#ifndef __EXPORTMEI_H__
#define __EXPORTMEI_H__

#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "pugixml/pugixml.hpp"


namespace Ms
{
    class ExportMEI {
        Score* _score;
        shared_ptr<pugi::xml_document> _xdoc;
        pugi::xml_node _xroot;
        int _currId;

    public:
        ExportMEI(Score* s);
        ~ExportMEI();
        void write(QIODevice* dev);
        Score* score();
    private:
        std::string generateId();
        void scoreToXML();
        void createHeader();
        void createMusic();
        void createScoreDef(pugi::xml_node score);
        void createSection(pugi::xml_node score);
        void createMeasure(Measure* m, pugi::xml_node parentNode);
        void createMeasureStaff(Staff* s, Measure* m, pugi::xml_node parentNode);
        void createElement(Element* el, pugi::xml_node parentNode);
        pugi::xml_node generateXMLElement(std::string name, pugi::xml_node parentnode);
        void addAttribute(pugi::xml_node node, std::string name, std::string value);
    };
    
    bool saveMei(Score* score, const QString& name);
}

#endif /* exportmei_hpp */
