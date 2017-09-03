//
//  exportmei.cpp
//  mscore
//
//  Created by Andrew Hankinson on 2017-01-15.
//
//

#include <array>
#include "exportmei.h"
#include "pugixml/pugixml.hpp"
#include "libmscore/note.h"


static int MS_PXML_EXPORT_OPTIONS = pugi::format_default;
static std::string MS_MEI_VERSION = "3.0.0";


struct xml_string_writer: pugi::xml_writer {
    std::string result;
    virtual void write(const void* data, size_t size) {
        result += std::string(static_cast<const char*>(data), size);
    }
};


namespace Ms {
    
ExportMEI::ExportMEI(Score* s)
    {
        // constructor
        this->_score = s;
        this->_currId = 1;  // incremental generator for xml:id values.
    }
    
ExportMEI::~ExportMEI()
    {
        // destructor
    }
    
void ExportMEI::write(QIODevice* dev)
    {
        this->_xdoc = std::make_shared<pugi::xml_document>();

        pugi::xml_node decl = this->_xdoc->append_child(pugi::node_declaration);
        decl.append_attribute("version") = "1.0";
        decl.append_attribute("encoding") = "UTF-8";

        this->_xroot = this->_xdoc->append_child("mei");

        pugi::xml_attribute idattrib = this->_xroot.append_attribute("xml:id");
        idattrib.set_value(this->generateId().c_str());

        pugi::xml_attribute vattrib = this->_xroot.append_attribute("meiversion");
        vattrib.set_value(MS_MEI_VERSION.c_str());
        
        this->scoreToXML();
        
        xml_string_writer writer;
        this->_xdoc->save(writer, "\t", MS_PXML_EXPORT_OPTIONS, pugi::encoding_utf8);
        
        dev->write(writer.result.c_str());
    }

Score* ExportMEI::score()
    {
        return this->_score;
    }

void ExportMEI::scoreToXML()
    {
        this->createHeader();
        this->createMusic();
    }

void ExportMEI::createHeader()
    {
        pugi::xml_node meiHeader = this->generateXMLElement("meiHead", this->_xroot);
        
        pugi::xml_node encodingDesc = this->generateXMLElement("encodingDesc", meiHeader);
        pugi::xml_node appInfo = this->generateXMLElement("appInfo", encodingDesc);
        pugi::xml_node application = this->generateXMLElement("application", appInfo);
        this->addAttribute(application, "isodate", QDate::currentDate().toString(Qt::ISODate).toStdString());
        this->addAttribute(application, "version", VERSION);
        
        pugi::xml_node name = this->generateXMLElement("name", application);
        pugi::xml_node nameTxt = name.append_child(pugi::node_pcdata);
        std::string appName = std::string("MuseScore ") + std::string(VERSION);
        nameTxt.set_value(appName.c_str());
    }
    
void ExportMEI::createMusic()
    {
        pugi::xml_node meiMusic = this->generateXMLElement("music", this->_xroot);
        pugi::xml_node body = this->generateXMLElement("body", meiMusic);
        pugi::xml_node mdiv = this->generateXMLElement("mdiv", body);
        pugi::xml_node score = this->generateXMLElement("score", mdiv);

        this->createScoreDef(score);
        this->createSection(score);

    }
    
void ExportMEI::createScoreDef(pugi::xml_node score)
    {
        pugi::xml_node scoreDef = this->generateXMLElement("scoreDef", score);
    }

void ExportMEI::createSection(pugi::xml_node score)
    {
        pugi::xml_node section = this->generateXMLElement("section", score);
        
        for (MeasureBase* mb = this->_score->measures()->first(); mb; mb = mb->next())
        {
            if (mb->type() != ElementType::MEASURE) continue;
            
            Measure* m = static_cast<Measure*>(mb);
            
            this->createMeasure(m, section);
        }
    }
    
void ExportMEI::createMeasure(Measure* m, pugi::xml_node parentNode)
    {
        int nstaves = this->_score->nstaves();
        
        pugi::xml_node xmeas = this->generateXMLElement("measure", parentNode);
        
        for (int s = 0; s < nstaves; ++s) {
            Staff* sobj = this->_score->staff(s);
            this->createMeasureStaff(sobj, m, xmeas);
        }
    }
    
    
/**
 Create the staff entry in a measure.

 @param s
 @param parentNode
 */
void ExportMEI::createMeasureStaff(Staff* s, Measure* m, pugi::xml_node parentNode)
    {
        pugi::xml_node xstf = this->generateXMLElement("staff", parentNode);
        
        // create an array of four slots, one for each possible layer. This will hold the `layer`
        // element and all its children in an xml_node.
        std::array<pugi::xml_node*, VOICES> layerArr;
        
        for (int v = 0; v < VOICES; v++) {
            pugi::xml_node xlyr = this->generateXMLElement("layer", xstf);

            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                Element* el = seg->element(v);
                if (!el) continue;
                
                
                
                pugi::xml_node foo = this->generateXMLElement("foo", xlyr);
//                qDebug("track %i", el->track());
            }
        }
    }

void ExportMEI::createElement(Element *el, pugi::xml_node parentNode)
    {
        switch (el->type()) {
            case ElementType::CHORD:
                Chord* chord = toChord(el);
                std::vector<Note*> nl = chord->notes();
                
                for (Note* note : nl) {
                    note->tpc();
                }
                break;
                
            default:
                break;
        }
    }
    
std::string ExportMEI::generateId()
    {
        return "m-" + std::to_string(this->_currId++);
    }
    
pugi::xml_node ExportMEI::generateXMLElement(std::string name, pugi::xml_node parentnode)
    {
        pugi::xml_node el;
        
        el = parentnode.append_child(name.c_str());
        pugi::xml_attribute idattrib = el.append_attribute("xml:id");
        idattrib.set_value(this->generateId().c_str());
        
        return el;
    }

void ExportMEI::addAttribute(pugi::xml_node node, std::string name, std::string value)
    {
        pugi::xml_attribute at = node.append_attribute(name.c_str());
        at.set_value(value.c_str());
    }
    
bool saveMei(Score* score, const QString& name)
    {
    QFile f(name);

    if (!f.open(QIODevice::WriteOnly))
        return false;

    ExportMEI em(score);
    em.write(&f);
    return f.error() == QFile::NoError;
    }

}

