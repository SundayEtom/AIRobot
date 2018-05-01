#ifndef AIROBOT_HPP
#define AIROBOT_HPP

#include <string>
#include <fstream>
#include <regex>

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


class Identifier{
    private:
        string name;
        string type;
        string content;

    public:
        Identifier(string name){
            this->name = name;
            type = "unknown";
            content = "";
        }
        Identifier(string name, string type){
            this->name = name;
            this->type = type;
            content = "";
        }
        Identifier(string name, string type, string content){
            this->name = name;
            this->type = type;
            this->content = content;
        }
        Identifier(void) : name{""}, type{""}, content{""}{}

        void setName(string name){ this->name = name; }
        void setType(string type){ this->type = type; }
        void setContent(string content){ this->content = content; }

        string getName(void){ return name; }
        string getType(void){ return type; }
        string getContent(void){ return content; }

        bool is_null(void){
            if(name.empty()) return true;
            return false;
        }
};



class Prompt{
    private:
        string original;
        string regex_str;
        vector<Identifier> identifiers;

    public:
        Prompt(string tmp) : original{tmp}{
            regex pattern{"<([[:alpha:]]+)([[:s:]]+as[[:s:]]*=[[:s:]]*\"([[:alnum:]]*)\"){0,1}>"};
            regex_str = original;

            for(sregex_iterator p(original.begin(), original.end(), pattern); p != sregex_iterator{}; ++p){
                string sub = (*p)[0];

                string idtype = (*p)[1];
                string idname = (*p)[3];
                if(idname.empty())
                    continue;
                Identifier ident(idname, idtype);
                identifiers.push_back(ident);
            }
            regex numpat{"<number.*?>"};
            regex strpat{"<string.*?>"};
            regex oppat{"<mathop.*?>"};

            regex_str = regex_replace(regex_str, numpat, "[()[:s:]]*([[:d:]]+)[()[:s:]]*");
            regex_str = regex_replace(regex_str, strpat, "(.*?)");
            regex_str = regex_replace(regex_str, oppat, "[()[:s:]]*([+*-/%]{1})[()[:s:]]*");
        }
        Prompt(void) : original{""}, regex_str{""}{
            // This prompt is empty
        }

        bool is_empty(void){
            if(original.empty()) return true;
            return false;
        }

        vector<Identifier> getIdentifiers(void){ return identifiers; }

        Identifier getIdentifier(string name){
            Identifier identifier;
            for(Identifier id : getIdentifiers())
                if(id.getName() == name){
                    identifier = id;
                    break;
                }
            return identifier;
        }

        string getPattern(void){ return regex_str; }
        string getOriginal(void){ return original; }
};


class Response{
    private:
        string content;

    public:
        Response(string c) : content{c}{}
        Response(void) : content{""}{}

        bool iscode(void){
            regex code_pattern{"(.*?)<code>(.*?)</code>(.*?)"};

            smatch matches;
            if(regex_match(content, matches, code_pattern))
                return true;
            return false;
        }

        string getContent(void){
            return content;
        }

        string getCodeContent(void){
            regex code_pattern{"(.*?)<code>(.*?)</code>(.*?)"};
            smatch matches;
            if(regex_match(content, matches, code_pattern))
                return matches[2];
            return "";
        }

        void setContent(string c){
            this->content = c;
        }

        string format(void){
            string nl{"<br>"}, tb{"<tb>"};
            string newstr = getCodeContent();
            newstr = regex_replace(newstr, regex("[[:blank:]]{2,}"), "");
            newstr = regex_replace(newstr, regex(nl), "\n");
            newstr = regex_replace(newstr, regex(tb), "\t");

            return newstr;
        }
};


class Category{
    private:
        vector<Prompt> prompts;
        vector<Response> responses;
        string name;
        string language;

    public:
        Category(string name, initializer_list<Prompt> ps, initializer_list<Response> rs, string lang="python"){
            this->name = name;
            this->language = lang;
            for(Prompt p : ps) prompts.push_back(p);
            for(Response r : rs) responses.push_back(r);
        }
        Category(string name, Prompt p, Response r, string lang="python"){
            this->name = name;
            this->language = lang;
            prompts.push_back(p);
            responses.push_back(r);
        }
        Category(string n="", string lang="python") : name{n}, language{lang}{}


        void print(void){
            cout << "Name:     " << name << endl;
            for(Prompt p : prompts)
                cout << "Prompt:   " << p.getOriginal() << endl;
            for(Response r : responses)
                cout << "Response: " << r.getContent() << endl;
        }

        string respond(string prompt){
            int res_count = responses.size();
            if(res_count <= 0){
                return "No response.";
            }

            for(Prompt& p : prompts){
                smatch matches;
                string patstr{p.getPattern()};
                if(patstr.empty())
                    continue;
                regex pat{patstr};

                if(regex_search(prompt, matches, pat)){
                    srand((int)time(0));
                    int index = rand() % res_count;
                    Response res = responses.at(index);

                    if(res.iscode()){
                        string command;
                        string file = "intepreter";

                        if(language == "python"){ file += ".py"; command = "python intepreter.py"; }
                        else if(language == "cpp"){ file += ".cpp"; command = "g++ intepreter.cpp -o Intepreter ./Intepreter"; }
                        else{ language = "python"; file += ".py"; }

                        ofstream os(file);
                        if(os){
                            int match_index = 1;

                            for(Identifier& id : p.getIdentifiers()){
                                id.setContent(matches[match_index++]);

                                if(id.getType() == "number"){
                                    if(language == "python") os << id.getName() << " = " << id.getContent() << endl;
                                    if(language == "cpp") os << "double " << id.getName() << " = " << id.getContent() << ";" << endl;
                                }
                                else if(id.getType() == "string"){
                                    if(language == "python") os << id.getName() << " = \"" << id.getContent() << "\"" << endl;
                                    if(language == "cpp") os << "string " << id.getName() << " = \"" << id.getContent() << "\";" << endl;
                                }
                                else if(id.getType() == "mathop"){
                                    if(language == "python") os << id.getName() << " = '" << id.getContent() << "'"<< endl;
                                    if(language == "cpp") os << "char " << id.getName() << " = '" << id.getContent() << "';" << endl;
                                }
                            }
                            string code_content = res.format();
                            if(regex_search(code_content, regex("<prompt/>"))){
                                if(regex_search(prompt, matches, pat))
                                    os << "\nprint(" << matches[0] << ")" << endl;
                            }
                            else os << endl << code_content << endl;

                            os.close();
                            system(command.c_str());

                            return "";
                        }
                    } else return res.getContent();
                }
                else{
                    cout << "Regex was not generated." << endl;
                    return "";
                }
            }
        }

        Prompt getPrompt(string original){
            Prompt prompt;
            for(Prompt p : prompts){
                smatch matches;
                regex pat{p.getPattern()};

                if(regex_match(original, matches, pat)){
                    prompt = p;
                    break;
                }
            }
            return prompt;
        }

        void addPrompt(Prompt p){ prompts.push_back(p); }
        void addResponse(Response r){ responses.push_back(r); }
        void setName(string name){ this->name = name; }
        void setLanguage(string lang){ this->language = lang; }
        string getName(void){ return this->name; }
        string getLanguage(void){ return this->language; }
};


class Robot{
    private:
        vector<Category> categories;

    public:
        Robot(string xmlfile){
            XMLDom dom(xmlfile, "robot", true);
            string language = dom.getAttribute("using");
            int cat_index = 0;

            for(XMLDom cat : dom.getChildren({"category"})){
                string cat_name = cat.getAttribute("name");
                if(cat_name.length() <= 0){
                    cat_name = "category_" + to_string(cat_index);
                    cat_index++;
                }
                Category newCategory(cat_name, language);

                for(XMLDom child : cat.getChildren({"prompt", "response"})){
                    if(!child.is_empty()){
                        if(child.getName() == "prompt"){
                            string cont = "";

                            XMLDom anyof = child.getChild("anyof");
                            if(!anyof.is_empty()){
                                for(XMLDom option : anyof.getChildren({"option"})){
                                    cont = option.getContent();
                                    Prompt prompt(cont);
                                    newCategory.addPrompt(prompt);
                                }
                            }
                            else{
                                cont = child.getContent();
                                Prompt prompt(cont);
                                newCategory.addPrompt(prompt);
                            }
                        }
                        else if(child.getName() == "response"){
                            Response response;
                            string cont = "";

                            XMLDom random = child.getChild("random");
                            if(!random.is_empty()){
                                for(XMLDom option : random.getChildren({"option"})){
                                    cont = option.getContent();
                                    response.setContent(cont);
                                    newCategory.addResponse(response);
                                    //cout << newCategory.respond("56 + 7") << endl;
                                }
                            }
                            else{
                                cont = child.getContent();
                                response.setContent(cont);
                                newCategory.addResponse(response);
                            }
                        }
                    }
                }
                categories.push_back(newCategory);
            }
        }

        void run(void){
            string prompt;
            while(true){
                cout << "Ask Me: ";
                getline(cin, prompt);
                if(prompt == "quit")
                    break;
                for(Category& cat : categories){
                    cout << cat.respond(prompt) << endl;
                }
            }
        }
};

#endif
