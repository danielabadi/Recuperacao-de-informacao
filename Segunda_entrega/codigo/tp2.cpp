#include <CkSpider.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <thread>
#include <queue>
#include <utility>
#include <set>
#include <algorithm>
#include <mutex>
#include <vector>

std::mutex long_term_scheduler_lock;
std::mutex used_urls_lock;
std::mutex domains_lock;
std::mutex num_pages_lock;
std::mutex sizes_lock;

std::priority_queue <std::pair<int, std::string>> long_term_scheduler; //fila principal
std::set <std::string> used_urls; //sites que ja foram coletados
std::set <std::string> domains; //domninios em processo de coleta
std::ofstream sizes { "sizes.txt" }; //arquivo para receber os tamanhos das URL's
int num_pages = 0; //variavel que armazena quantidade de paginas coletadas

std::string get_top(){ //funcao que retorna um link para a thread
    long_term_scheduler_lock.lock();
    while(long_term_scheduler.empty()){ //enqaunto a fila estiver vazia dorme 2s
        long_term_scheduler_lock.unlock();
        std::this_thread::sleep_for (std::chrono::seconds(2)); 
    }
    long_term_scheduler_lock.unlock();
	
    long_term_scheduler_lock.lock();
    std::string site = long_term_scheduler.top().second; //a string recebe o segundo dado do pair da fila
    long_term_scheduler.pop();
    long_term_scheduler_lock.unlock();
	
    return site;
}
  
int conta(std::string aux){ //conta numero de "." e "/"
    int size = std::count(aux.begin(), aux.end(),'.');
    size += std::count(aux.begin(), aux.end(),'/');
    size -= 2; // desconsidera as duas barras do "http://"
    return size*(-1); //retorna negativo para facilitar a codificacao
}

void Crawler(int id){
    std::ofstream file { "./links/"+std::to_string(id)+"links.txt" }; //arquivo links
    std::ofstream file2 { "./htmls/"+std::to_string(id)+"htmls.txt" }; //arquivo htmls
    std::ofstream times { "./times/"+std::to_string(id)+"times.txt" }; //aquivo de tempos
	
    bool condition_num_pages = true, cond; //variaveis de condicao dos whiles
	
    CkSpider spider;
    spider.put_Utf8(true); //forcando ter o padrao utf-8
    spider.AddMustMatchPattern("*.br*"); //forcando ter o padrao .br na url
    spider.AddAvoidPattern("*.pdf*"); //evitando pdfs
	spider.AddAvoidPattern("*.png*"); //evitando pngs
	spider.AddAvoidPattern("*.jpeg*"); //evitando jpegs
    spider.AddAvoidPattern("*.britishcouncil*"); //durante o processo percebi que essas duas
	spider.AddAvoidPattern("*.brandeisjudges*"); //paginas estavam entrando entre as brasileiras
    spider.put_MaxResponseSize(800000); //tamanho maximo de 800KB
    spider.put_ConnectTimeout(3); //tempo maximo de resposta, nem sempre respeitado
	
    std::string url, dominio;
    char *newUrl;
    std::vector<std::string> urls_doms_usados; //vector auxiliar para pegar novos links
    int size, n, pages_crawler = 0;

    while(condition_num_pages){ //a condicao so se torna falsa quando o limite de 100 mil paginas e atingido
        cond = true;
        while(cond){ //a condicao se torna falsa quando se consegue passar um link valido
            url = get_top();
            domains_lock.lock();
            try{
                if(domains.find(spider.getUrlDomain(&url[0])) != domains.end()){ //verificando se o dominio esta sendo usado
                    domains_lock.unlock();
                    urls_doms_usados.push_back(url); //colocando links no vector auxiliar
					
                } else{
                    domains_lock.unlock();
                    newUrl = &url[0];
                    dominio = spider.getUrlDomain(newUrl);
                    cond = false;
                }
            } catch(std::logic_error error){
                domains_lock.unlock();
                std::cout << "Erro" << '\n';
            }
        }
        size = urls_doms_usados.size();
        for(int i = 0; i < size; i++){
            long_term_scheduler_lock.lock();
            long_term_scheduler.push({conta(urls_doms_usados[i]), urls_doms_usados[i]}); //voltando links para fila
            long_term_scheduler_lock.unlock();
        }
        urls_doms_usados.clear();

        spider.Initialize(spider.getUrlDomain(newUrl));
        spider.AddUnspidered(newUrl);

        domains_lock.lock();
        domains.insert(dominio); //dominios sendo usados
        domains_lock.unlock();
		
        spider.CrawlNext();
		
        used_urls_lock.lock();
        used_urls.insert(spider.lastUrl()); //sites que ja foram crawleados
        used_urls_lock.unlock();
		
        n = spider.get_NumUnspidered(); //recebe o numero de paginas niveis 1 do site atual
		
        num_pages_lock.lock();
        num_pages++;
        num_pages_lock.unlock();
		
        pages_crawler++; //contador de paginas local da thread

        file << pages_crawler << ": { " << spider.lastUrl() << " } numero URLS lvl 1:"<< n << "\n";
        file2 << pages_crawler << ": § " << spider.lastHtml() << " §\n";

        for (int i = 0; i < n; i++) {
            bool success;
			
            auto start = std::chrono::high_resolution_clock::now(); //marcando os tempos
            success = spider.CrawlNext();
            auto stop = std::chrono::high_resolution_clock::now();
			
            used_urls_lock.lock();
            if ((success == true) & (used_urls.find(spider.lastUrl()) == used_urls.end())) { //verifica se link ja foi recolhido
                used_urls_lock.unlock();
				
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
                times << (int)duration.count() << "\n"; //guarda o tempo no arquivo
				
                num_pages_lock.lock();
                num_pages++;
                num_pages_lock.unlock();
				
                pages_crawler++;
				
                sizes_lock.lock();
				sizes << (conta(spider.lastUrl())*-1) << "\n"; //escreve o tamanho no arquivo de tamanhos
				sizes_lock.unlock();
				
                file << pages_crawler << ": { " << spider.lastUrl() << " }\n";
                file2 << pages_crawler << ": § " << spider.lastHtml() << " §\n";

                used_urls_lock.lock();
                used_urls.insert(spider.lastUrl()); //coloca a url no set das que ja foram usadas
                used_urls_lock.unlock();

                for(int i = 0; i < spider.get_NumOutboundLinks(); i++){ //loop para links que apontam para outros dominios
                    try{
                        std::string aux = spider.getOutboundLink(i);
						
                        long_term_scheduler_lock.lock();
                        long_term_scheduler.push({conta(aux), aux}); //adicionando os links na fila principal
                        long_term_scheduler_lock.unlock();
						
                    } catch(std::logic_error error){
                          std::cout << "Erro";
                    }
                }
                spider.ClearOutboundLinks();
            }
            else {
                used_urls_lock.unlock();
                if (spider.get_NumUnspidered() == 0) {
                    std::cout << "No more URLs to spider" << "\r\n";
                }
                else {
                    std::cout << spider.lastErrorText() << "\r\n";
                }
            }
            spider.SleepMs(100);
			
            num_pages_lock.lock();
            if(num_pages>=100000) condition_num_pages = false; //verifica a condicao para parar o while principal
            num_pages_lock.unlock();
        }

        domains_lock.lock();
        domains.erase(dominio); //tira o dominio do set de usados
        domains_lock.unlock();
		
		num_pages_lock.lock();
        std::cout << num_pages << '\n';
        num_pages_lock.unlock();

    }
    
    file.close();
    file2.close();
	times.close();
}

int main(int argc, char *argv[]){
	if(argc < 2){
        std::cout << "Argumentos insuficientes \n";
    }
	
    std::ifstream file;
    file.open(argv[1]);
    if(!file.is_open()){
        std::cout << "Nao foi possivel encontrar o arquivo\n";
        return 0;
    }
	
    std::string url;
    while(!file.eof()){
        file >> url;
        long_term_scheduler.push({conta(url), url});
    }
    file.close();
	
    std::thread threads[30];
    for(int i = 0; i < 30; i++){
        threads[i] = std::thread(Crawler, i);
    }
	
    for(int i = 0; i < 30; i++){
        threads[i].join();
    }
	
    std::cout << "Numero de paginas coletadas: " << num_pages << ". \n";

    return 0;
}
