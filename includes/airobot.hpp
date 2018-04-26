#ifndef AIROBOT_HPP
#define AIROBOT_HPP

#include <string>
#include <fstream>

using namespace std;


class XMLDom{
    private:
        string attributes;
        string content;
        string tag;

    public:
        XMLDom(string source, string tag="robot", bool from_file=false) : attributes{""}, content{""}, tag{""}{
            string xml_text = "";

            if(from_file){
                ifstream is(source);
                if(is){
                    while(!is.eof()){
                        string line;
                        getline(is, line);
                        if(line.find("<?xml") != -1)
                            continue;
                        xml_text += line;
                    }
                    is.close();
                }
            }
            else{
                xml_text = source;
            }

            regex pat{"<" + tag + "([[:s:]]*)(.*?)>(.*?)</" + tag + ">"};
            smatch matches;
            if(regex_search(xml_text, matches, pat)){
                this->tag = tag;
                attributes = matches[2];
                content = matches[3];
            }
        }


        string getAttribute(string attrName){
            regex pat{attrName + "[[:s:]]*=[[:s:]]*\"(.*?)\""};
            smatch matches;
            if(regex_search(attributes, matches, pat)){
                return matches[1];
            }
            return "";
        }



        XMLDom getChild(string name){
            regex pat{"<" + name + "([[:s:]]*)(.*?)>(.*?)</" + name + ">"};
            smatch matches;
            string txt = "";

            if(regex_search(content, matches, pat)){
                txt = matches[0];
            }
            XMLDom node(txt, name);
            return node;
        }


        vector<XMLDom> getChildren(initializer_list<string> tags){
            vector<XMLDom> children;
            if(!content.empty()){
                for(string tag : tags){
                    regex pat{"<" + tag + "([[:s:]]*)(.*?)>(.*?)</" + tag + ">"};
                    for(sregex_iterator p(content.begin(), content.end(), pat); p != sregex_iterator{}; ++p){
                        XMLDom node((*p)[0], tag);
                        children.push_back(node);
                    }
                }
            }
            return children;
        }



        vector<XMLDom> getChildren(string name){
            vector<XMLDom> children;
            if(!content.empty()){
                regex pat{"<" + name + "(.*?)>(.*?)</" + name + ">"};
                for(sregex_iterator p(content.begin(), content.end(), pat); p != sregex_iterator{}; ++p){
                    XMLDom node((*p)[0], name);
                    children.push_back(node);
                }
            }
            return children;
        }


        string getName(void){
            return tag;
        }


        string getContent(void){
            return content;
        }


        bool is_empty(void){
            if(content.empty())
                return true;
            return false;
        }
};

string outputDir = "AIRobotOutput/";
string robotName;
string xml_text = "";
ofstream outputCpp, outputHpp;

bool initialize(XMLDom dom);
bool de_initialize(bool compile);
bool parse_robot(XMLDom dom);
string parse_content(string content);

#endif
