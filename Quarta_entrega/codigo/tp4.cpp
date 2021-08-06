#include <stdlib.h>
#include <unordered_map>
#include "gumbo.h"
#include <locale>
#include <algorithm>
#include <codecvt>
#include <sstream> 
#include "rapidjson/document.h"
#include <list>
#include <tuple>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <set>
#include <math.h>

std::ofstream urls { "urls.txt" };
std::ofstream ids { "ids.txt" };
std::unordered_map<std::string, int> terms_id;
std::list <std::tuple<int, int, int>> lista;
int id_term = 0;

std::string clean(std::string content){
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> transform;
    std::wstring ws_content = transform.from_bytes(content);
    for (std::wstring::iterator it = ws_content.begin(); it != ws_content.end(); ++it){
        if(((*it >= 48 && *it <= 57) ||
            (*it >= 65 && *it <= 90) ||
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
void print_lista(int id_file){
    std::ofstream terms { "./outputs/"+std::to_string(id_file)+".txt" };
    std::list <std::tuple<int, int, int>>::iterator it;
	for(it = lista.begin(); it != lista.end(); ++it){
		terms << std::get<0>(*it) << " " << std::get<1>(*it) << " " << std::get<2>(*it) << '\n';
    }
    terms.close();
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
void read_doc(std::string html, int doc_id){
    std::istringstream iss(html);
    std::string word;
    int pos = 0;
    while (iss) { 
        iss >> word;
        std::unordered_map<std::string, int>::iterator it = terms_id.find(word);
        if(it == terms_id.end()){
        	ids << id_term << " " << word << "\n";
        	terms_id.insert({word, id_term});
        	lista.push_back(std::make_tuple(id_term, doc_id, pos));
        	id_term++;
        }else{
        	lista.push_back(std::make_tuple(it->second, doc_id, pos));
        } 
        pos+=word.size();
    }
}
bool compare (const std::tuple<int,int,int>& a, const std::tuple<int,int,int>& b){
	return ( std::get<0>(a) < std::get<0>(b) );
}
void parser(char *argv[]){
    const char* filename = argv[1];
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (!in) {
      std::cout << "Arquivo " << filename << " não encontrado!\n";
      exit(EXIT_FAILURE);
    }
    int id_file = 0;
    int doc_id = 1;
    std::string line;
    while (std::getline (in,line)){
        rapidjson::Document document;
        document.Parse<0>(line.c_str());
        urls << doc_id << " " << document["url"].GetString() << '\n';
        GumboOutput* output = gumbo_parse(document["html_content"].GetString());
        read_doc(cleantext(output->root), doc_id);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        if((doc_id)%31252 == 0 && doc_id < 1000064){
            std::cout << doc_id << '\n';
            lista.sort(compare);
        	print_lista(id_file);
        	lista.clear();
            id_file++;
        }
        doc_id++;
    }
    std::cout << doc_id << '\n';
    lista.sort(compare);
    print_lista(id_file);
    lista.clear();
    in.close();
}
void merge_files(){
	int doc1, doc2;
	std::string line1, line2;
	int out_file = 0;
	int num, num2;

	int files = 16, loop_int;
	while(files >= 1){
		loop_int = 1;
		doc1 = 0;
		doc2 = 1;
		out_file = 0;
		while(loop_int <= files){
			std::ifstream in1("./outputs/"+std::to_string(doc1)+".txt");
			std::ifstream in2("./outputs/"+std::to_string(doc2)+".txt");
			std::ofstream out {"./outputs/out"+std::to_string(doc1)+".txt"};
			line1.clear();
			line2.clear();
			while(!in1.eof() || !in2.eof()){
				if(!in1.eof() && !in2.eof()){
					if(line1.empty() && !in1.eof()){
						std::getline (in1,line1);
						std::istringstream iss1(line1);
						iss1 >> num;
					}
					if(line2.empty() && !in2.eof()){
						std::getline (in2,line2);
						std::istringstream iss2(line2);
						iss2 >> num2;
					}
					if(num <= num2){
						while(num <= num2 && !in1.eof()){
							out << line1 << '\n';
							std::getline (in1,line1);
							std::istringstream iss1(line1);
							iss1 >> num;
							if(in1.eof()){
								if(line1 != "\n") out << line1 << '\n';
								if(line2 != "\n") out << line2 << '\n';
								line2.clear();
							}
							if(num > num2){
								if(line2 != "\n") out << line2 << '\n';
								line2.clear();
							}
						}
					} else {
						while(num > num2 && !in2.eof()){
							if(line2 != "\n") out << line2 << '\n';
							std::getline (in2,line2);
							std::istringstream iss2(line2);
							iss2 >> num2;
							if(num <= num2){
								if(line1 != "\n") out << line1 << '\n';
								line1.clear();
							}
							if(in2.eof()){
								if(line2 != "\n") out << line2 << '\n';
								if(line1 != "\n") out << line1 << '\n';
								line1.clear();
							}
						}
					}
				} else if(!in1.eof() && in2.eof()){
					while(std::getline (in1,line1)){
						out << line1 << '\n';
					}
				} else{
					while(std::getline (in2,line2)){
						out << line2 << '\n';
					}
				}
			}

			in1.close();
			in2.close();
			out.close();
			std::string name1 = "./outputs/"+std::to_string(doc1)+".txt";
			std::string name2 = "./outputs/"+std::to_string(doc2)+".txt";
			std::string name3 = "./outputs/out"+std::to_string(doc1)+".txt";
			std::string name4 = "./outputs/"+std::to_string(out_file)+".txt";
			std::remove(name1.c_str());
			std::remove(name2.c_str());
			std::rename(name3.c_str(), name4.c_str());
			out_file++;
			doc1 += 2;
			doc2 += 2;
			loop_int++;
		}
		files = files/2;
	}
}
void read_merged_file(){
	std::unordered_map<int, int> appeared;
	std::unordered_map<int, std::tuple<long int, long int, float>> dic;
	std::set<int> ni;
	std::ifstream in("./outputs/0.txt", std::ios::in | std::ios::binary);
	std::ofstream out("dic.txt");
	std::string line;
	int current_num, last_num = -1, doc;
	long int last_pos = -1, current_pos;
	long int first_pos;
	while(std::getline(in, line)){
		current_pos = in.tellg();
		if(line == "\n") continue;
		std::istringstream iss(line);
		iss >> current_num >> doc;
		if(current_num != last_num){
			if(last_pos == -1){
				first_pos = current_pos;
				last_num = current_num;
			} else{
				dic.insert({last_num, std::make_tuple(first_pos, last_pos, log2(float(1000068)/float(ni.size())))});
				out << last_num << " " <<first_pos << " " << last_pos << " " <<log2(float(1000068)/float(ni.size())) << "\n";
				ni.clear();
				first_pos = current_pos;
				last_num = current_num;
			}
		}
		ni.insert(doc);
		last_pos = current_pos;
	}
	dic.insert({current_num, std::make_tuple(first_pos, last_pos, log2(float(1000068)/float(ni.size())))});
	out << current_num << " " << first_pos << " " << last_pos << " " << log2(float(1000068)/float(ni.size()));
	ni.clear();
}

int main(int argc, char** argv) {
    std::setlocale(LC_CTYPE, "en_US.UTF-8");
    if (argc < 2) {
        std::cout << "Argumentos insuficientes \n";
        return 0;
    }
    parser(argv);
    merge_files();
    read_merged_file();
    urls.close();
    ids.close();
    return 0;
}
