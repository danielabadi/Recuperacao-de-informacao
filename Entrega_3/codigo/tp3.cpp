#include <fstream>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include "struct_aux.hpp"
#include "gumbo.h"
#include <locale>
#include <algorithm>
#include <codecvt>

std::ofstream final_file { "data.txt" };

std::string clean(std::string content){
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> transform;
    std::wstring ws_content = transform.from_bytes(content);
    for (std::wstring::iterator it = ws_content.begin(); it != ws_content.end(); ++it){
        if(((*it >= 65 && *it <= 90) ||
            (*it >= 97 && *it <= 122) ||
            (*it >= 192 && *it <= 195) ||
            (*it >= 199 && *it <= 206) ||
            (*it >= 210 && *it <= 213) ||
            (*it >= 217 && *it <= 219) ||
            (*it >= 224 && *it <= 227) ||
            (*it >= 231 && *it <= 238) ||
            (*it >= 242 && *it <= 245) ||
            (*it >= 249 && *it <= 251) ||
            (*it == 45))){
                *it = towlower(*it);
            } else {
                *it = ' ';
            }
    }
    std::string cont = transform.to_bytes(ws_content);
    return cont;
}
std::unordered_map<std::string, struct_aux> mymap;
void get_size_hash(){
    unsigned int size = 0;
    std::unordered_map<std::string, struct_aux>::iterator it;
    for(it = mymap.begin(); it != mymap.end(); ++it){
        size += it->first.size();
        size += it->second.size_struct();
    }
    final_file << "Tamanho do índice invertido: " << size/1000 << " Kbytes. \n";
}
void print_hash(){
    final_file << "Quantidade de termos na coleção: "<<mymap.size() << '\n';
    std::unordered_map<std::string, struct_aux>::iterator it;
    for(it = mymap.begin(); it != mymap.end(); ++it){
        final_file << it->first << "-> número de documentos que contêm a palavra: " << it->second.return_size() << ". Média de aparições nos documentos: " << it->second.appear() << "\n";
    }
}
void add_hash(std::string word, int num_doc, int pos){
    std::unordered_map<std::string, struct_aux>::iterator it = mymap.find(word);
    if(it == mymap.end()){
        struct_aux aux;
        aux.add(num_doc, pos);
        mymap.insert({word, aux});
    } else {
        it->second.add(num_doc, pos);
    }
}
static std::string cleantext(GumboNode* node) {
  if (node->type == GUMBO_NODE_TEXT) {
    return std::string(node->v.text.text);
  } else if (node->type == GUMBO_NODE_ELEMENT &&
             node->v.element.tag != GUMBO_TAG_SCRIPT &&
             node->v.element.tag != GUMBO_TAG_STYLE) {
    std::string contents = "";
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
      const std::string text = cleantext((GumboNode*) children->data[i]);
      if (i != 0 && !text.empty()) {
        contents.append(" ");
      }
      contents.append(text);
    }
    return clean(contents);
  } else {
    return "";
  }
}
void parser(char *argv[]){
    std::ofstream file { "out.txt" };
    const char* filename = argv[1];

    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in) {
      std::cout << "File " << filename << " not found!\n";
      exit(EXIT_FAILURE);
    }

    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    GumboOutput* output = gumbo_parse(contents.c_str());
    file << cleantext(output->root) << std::endl;
    file.close();
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}
void read_doc(){
    std::ifstream file { "out.txt" };
    if(!file.is_open()){
        std::cout << "Não foi possivel encontrar o arquivo" << '\n';
        return;
    }
    std::string words;
    int num_doc = 0;
    int pos = 0;
    while(file >> words){
        if(words == "newdoc"){
            num_doc++;
            pos=0;
            continue;
        }
        add_hash(words, num_doc, pos);
        pos+=words.size();
    }
    final_file << "Número de documentos: " << num_doc << '\n';

    file.close();

}

int main(int argc, char** argv) {
    std::setlocale(LC_CTYPE, "en_US.UTF-8");
    if (argc < 2) {
        std::cout << "Argumentos insuficientes \n";
        return 0;
    }
    parser(argv);
    read_doc();
    get_size_hash();
    print_hash();
    final_file.close();
    return 0;
}
