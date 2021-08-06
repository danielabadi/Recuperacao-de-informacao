#include <stdlib.h>
#include <unordered_map>
#include <locale>
#include <chrono>
#include <codecvt>
#include <sstream> 
#include <tuple>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <math.h>
#include <map>
#include <vector>
#include <utility>
#include <set>

std::unordered_map<std::string, std::tuple<long int, long int, float>> dic;
std::unordered_map<int, std::string> id_term;
std::unordered_map<int, std::string> id_url;
std::map<int, std::vector<float>> tf_idf_docs;
std::map<int, int> doc_f;
std::map<int, std::vector<float>> final_tf_idf_docs;
std::set<std::pair<float, int>> ranking;

std::map<std::string, int> query;
std::vector<float> tf_idf_query;

void fill_terms_id(){
	int id;
	std::string term;
	std::ifstream in("data/ids.txt");
	while(in >> id >> term){
		id_term.insert({id, term});
	}
	in.close();
}

void fill_dic(){
	int id;
	long int begin, end;
	float idf;
	std::string term;
	std::ifstream in("data/dic.txt");
	std::unordered_map<int, std::string>::iterator search;
	while(in >> id >> begin >> end >> idf){
		search = id_term.find(id);
		if (search == id_term.end()){
    		if(id != 35780){
    			break;
    		}
		}
  		else{
    		dic.insert({search->second, std::make_tuple(begin, end, idf)});
  		}
	}
	in.close();
}

void fill_id_url(){
	int id;
	std::string url;
	std::ifstream in("data/urls.txt");
	while(in >> id >> url){
		id_url.insert({id, url});
	}
	in.close();
}

void search(std::string word){
	std::unordered_map<std::string, std::tuple<long int, long int, float>>::iterator pos = dic.find(word);
	if(pos == dic.end()){
		std::cout << "Palavra não encontrada" << "\n";
		return;
	}
	std::ifstream in("data/0.txt", std::ifstream::binary);
	in.seekg(std::get<0>(pos->second));
	int term, doc, position;
	std::string line;
	std::getline(in, line);
	std::istringstream iss(line);
	iss >> term >> doc >> position;
	std::cout << doc << "\n" << term << " " << position << "\n";
	in.close();
}

float get_idf(std::string word){
	std::unordered_map<std::string, std::tuple<long int, long int, float>>::iterator pos = dic.find(word);
	if(pos == dic.end()){
		return 0;
	}
	return std::get<2>(pos->second);
}

void get_f(std::string word){
	int id_word, doc;
	std::string line;
	std::unordered_map<std::string, std::tuple<long int, long int, float>>::iterator pos = dic.find(word);
	if(pos == dic.end()){
		return;
	}
	std::ifstream in("data/0.txt", std::ifstream::binary);
	in.seekg(std::get<0>(pos->second));

	while(in.tellg() <= std::get<1>(pos->second)){
		std::getline(in, line);
		std::istringstream iss(line);
		iss >> id_word >> doc;
		std::map<int, int>::iterator it = doc_f.find(doc);
		if(it == doc_f.end()){
			doc_f.insert({doc, 1});
		} else {
			it->second++;
		}
	}
}

void sim(){
	std::map<int, std::vector<float>>::iterator final_tf_idf_docs_it;
	std::vector<float>::iterator it_q, it_d;
	float numerador = 0, den1 = 0, den2 = 0, result;
	for(final_tf_idf_docs_it = final_tf_idf_docs.begin(); final_tf_idf_docs_it != final_tf_idf_docs.end(); ++final_tf_idf_docs_it){
		numerador = 0;
		den1 = 0;
		den2 = 0;
		for(it_q = tf_idf_query.begin() , it_d = final_tf_idf_docs_it->second.begin(); it_q != tf_idf_query.end() && it_d != final_tf_idf_docs_it->second.end(); ++it_q, ++it_d){
			numerador += (*it_q) * (*it_d);
			den1 += pow((*it_q), 2);
			den2 += pow((*it_d),2); 
		}
		result = (-1)*(numerador / (sqrt(den1) * sqrt(den2)));
		ranking.insert(std::make_pair(result, final_tf_idf_docs_it->first));
	}
	std::set<std::pair<float, int>>::iterator rank_it;
	int para = 0;
	std::cout << "Resultados encontrados: " << ranking.size() << "\n";
	std::cout << "Primeiros resultados:" << "\n";
	for(rank_it = ranking.begin(); rank_it != ranking.end(); ++rank_it){
		std::unordered_map<int, std::string>::iterator id_url_it = id_url.find(rank_it->second);
		std::cout << "Similaridade: " << (-1)*(rank_it->first) << "\n" << "ID da página: " << rank_it->second << ", endereço da página:" << "\n" << id_url_it->second << "\n \n";
		para++;
		if(para == 5) break;
	}
}

std::string clean_query(std::string content){
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

int main(){
	std::setlocale(LC_CTYPE, "en_US.UTF-8");

	std::cout << "Carregando dados..." << "\n";
	auto start = std::chrono::high_resolution_clock::now();
	fill_terms_id();
	fill_dic();
	fill_id_url();
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
    std::cout << "Tempo de carregamento: " << ((float)duration.count())/1000 << " segundos." << "\n"; //guarda o tempo no arquivo
	std::string pesquisa, word;
	while(true){
		std::cout << "Digite sua consulta: ";
		std::getline(std::cin, pesquisa);
		pesquisa = clean_query(pesquisa);
		std::cout << "Convertendo consulta: " << pesquisa << "\n";
		start = std::chrono::high_resolution_clock::now();
		std::istringstream iss(pesquisa);
		while (iss >> word) { 
    	    std::map<std::string, int>::iterator it = query.find(word);
     	   if(it == query.end()){
       	 		query.insert({word, 1});
     	   	} else {
        		it->second++;
        	}
    	}
    	std::map<std::string, int>::iterator it;
    	std::map<int, int>::iterator doc_f_it;
    	std::map<int, std::vector<float>>::iterator tf_idf_docs_it;
    	float idf = 0;
    	float tf = 0;
		for(it = query.begin(); it != query.end(); ++it){
			tf = 1 + log2(it->second);
			idf = get_idf(it->first);
			tf_idf_query.push_back(tf*idf);
			get_f(it->first);
			for(doc_f_it = doc_f.begin(); doc_f_it != doc_f.end(); ++doc_f_it){
				tf_idf_docs_it = tf_idf_docs.find(doc_f_it->first);
				if(tf_idf_docs_it == tf_idf_docs.end()){
					std::vector<float> temp;
					temp.push_back((1 + log2(doc_f_it->second))*idf);
					tf_idf_docs.insert({doc_f_it->first, temp});
				} else{
					tf_idf_docs_it->second.push_back((1 + log2(doc_f_it->second))*idf);
				}
			}
			doc_f.clear();
   	 	}

    	std::vector<float>::iterator vect_it;
    	unsigned int num = query.size();

    	for(tf_idf_docs_it = tf_idf_docs.begin(); tf_idf_docs_it != tf_idf_docs.end(); ++tf_idf_docs_it){
    		if(tf_idf_docs_it->second.size() == num){
    			final_tf_idf_docs.insert({tf_idf_docs_it->first, tf_idf_docs_it->second});
    		}
    	}
    	sim();
    	stop = std::chrono::high_resolution_clock::now();
    	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
    	std::cout << "Tempo de consulta: " << ((float)duration.count())/1000 << " segundos." << "\n";
    	std::cout << "------------------------- // -------------------------" << "\n \n";
    	tf_idf_docs.clear();
    	final_tf_idf_docs.clear();
    	doc_f.clear();
    	ranking.clear();
    	query.clear();
    	tf_idf_query.clear();

    }
	return 0;
}