#include "main.hpp"

int main(int argc, char *argv[]) {
    DvachMediaDownloader DvachDownloader(argc, argv);
    return 0;
}

DvachMediaDownloader::DvachMediaDownloader(int argc, char *argv[]){
    if (argc <= 1)
        ShowHelp();
    if (strcmp(argv[1], "--version") == 0){
        std::cout << "2chDownloader version 0.9\n";
        exit(0);
    }
    if (strcmp(argv[1], "--help") == 0){
        ShowHelp();
    }else{
        std::string download_list(argv[1]);
        ParseList(download_list);
    }
    for (uint i = 2; i < argc; ++i){
        if (strncmp(argv[i], "--path=", 7) == 0){
            std::string argvi (argv[i]);
            std::size_t path_ptr = argvi.find_last_of("=");
            path = argvi.substr(path_ptr+1);
        }else if (strcmp(argv[i], "--webm-only") == 0){
            f_webm_only = true;
        }else if (strcmp(argv[i], "--image-only") == 0){
            f_image_only = true;
        }else if (strcmp(argv[i], "--original-names") == 0){
            f_original_names = true;
        }else if (strncmp(argv[i], "--threads=", 10) == 0){
            std::string argvi (argv[i]);
            std::size_t num_ptr = argvi.find_last_of("=");
            threads_max = std::stoul(argvi.substr(num_ptr+1));
        }else if (strncmp(argv[i], "--usercode-auth=", 16) == 0){
            std::string argvi (argv[i]);
            std::size_t token_ptr = argvi.find_last_of("=");
            usercode_auth = argvi.substr(token_ptr+1);
        }else{
            std::cout << "Incorrect option: " << argv[i] << std::endl;
            exit(1);
        }
    }
    while (!boards.empty()){
        if (boards.front().threads.empty()){
            ThreadsToQueue(boards.front().name);
        }else{
            while (!boards.front().threads.empty()){
                DownloadThread(boards.front().name, boards.front().threads.front());
                boards.front().threads.pop();
            }
        }
        boards.pop();
    }
}

void DvachMediaDownloader::ShowHelp(){
    std::cout    << "USAGE:\n"
                 << "\t2chDownloader <download_list> [options]\n"
                 << "\t2chDownloader a,b/1,vg/1:2:3 --path=./a_b_vg/ --original-names\n\n"
                 << "DOWNLOAD_LIST:\n"
    <<boost::format("\tboard %|25t|Download whole /board/\n")
    <<boost::format("\tboard/id %|25t|Download thread by id from /board/\n")
    <<boost::format("\tboard/id1:id2:id3 %|25t|Download multiple threads from /board/\n\n")
                 << "OPTIONS:\n"
    <<boost::format("\t--path=<path> %|25t|Set custom relative download path (default \"./downloads/\")\n")
    <<boost::format("\t--threads=<num> %|25t|Set number of download threads (default 10)\n")
    <<boost::format("\t--usercode-auth=<token> %|25t|Set authcookie for access to hidden boards\n")
    <<boost::format("\t--webm-only %|25t|Download only .webm videos\n")
    <<boost::format("\t--image-only %|25t|Download only images\n")
    <<boost::format("\t--original-names %|25t|Save files with names given by uploader, not by imageboard\n")
    <<boost::format("\t--help %|25t|Show this help\n")
    <<boost::format("\t--version %|25t|Show version\n");
    exit(0);
}

void DvachMediaDownloader::ParseList(std::string &download_list){
    std::vector<std::string> split_list;
    boost::split(split_list, download_list, boost::is_any_of(","));
    for (auto &entry : split_list){
        if (entry.empty())
            continue;
        Board board_entry;
        if (entry.find("/") == std::string::npos){
            board_entry.name = entry;
        }
        else{
            std::vector<std::string> _split;
            boost::split(_split, entry, boost::is_any_of("/"));
            board_entry.name = _split[0];
            if (entry.find(":") == std::string::npos)
                board_entry.threads.push(std::stoul(_split[1]));
            else{
                std::vector<std::string> threads;
                boost::split(threads, _split[1], boost::is_any_of(":"));
                for (auto &thread : threads)
                    board_entry.threads.push(std::stoul(thread));
            }
        }
        boards.push(board_entry);
    }
}

void DvachMediaDownloader::ThreadsToQueue(std::string &board){
    auto res = cpr::Get(cpr::Url{"https://2ch.hk/"+board+"/threads.json"},
                        cpr::Cookies{{"usercode_auth", "56f407dd40a9f644f0f1ddf12ebf0fc3"},
                                    {"z", ""}});    auto data = json::parse(res.text);
    data = data.at("threads");
    Board board_entry;
    board_entry.name = board;
    for (auto &thread : data){
        auto num = thread.at("num");
        std::string _num = num;
        uint __num = std::stoul(_num);
        board_entry.threads.push(__num);
    }
    boards.push(board_entry);
}

void DvachMediaDownloader::DownloadThread(std::string &board, uint thread_id){
    auto res = cpr::Get(cpr::Url{"https://2ch.hk/"+board+"/res/"+std::to_string(thread_id)+".json"},
                        cpr::Cookies{{"usercode_auth", "56f407dd40a9f644f0f1ddf12ebf0fc3"},
                                    {"z", ""}});
    auto data = json::parse(res.text);
    data = data.at("threads");
    data = data[0];
    data = data.at("posts");
    for (auto &post : data){
        try{
            auto files = post.at("files");
            for (auto &file : files){
                std::string path = file.at("path").get<std::string>();
                std::string filename;
                if (f_original_names)
                    filename = file.at("fullname").get<std::string>();
                else
                    filename = file.at("name").get<std::string>();
                std::size_t filetype_ptr = filename.find_last_of(".");
                std::string filetype = filename.substr(filetype_ptr+1);
                if (f_webm_only and filetype != "webm"){
                    continue;
                }else if (f_image_only and filetype == "webm")
                    continue;
                if (threads_counter <= threads_max){
                    std::thread t1(&DvachMediaDownloader::Download, this, std::ref(filename), std::ref(path));
                    t1.join();
                }else{
                    std::unique_lock<std::mutex> main_thread_lock(mtx);
                    cv.wait(main_thread_lock);
                }
            }
        }catch (std::out_of_range){
            continue;
        }
    }
}

void DvachMediaDownloader::Download(std::string &filename, std::string &url){
    threads_counter++;
    auto curpath = boost::filesystem::current_path();
    curpath /= path;
    fs::create_directories(curpath);
    std::ifstream f(curpath.string()+filename);
    if (f.good()){
        std::cout << "Skipping " << filename << "\n";
    }else{
        auto res = cpr::Get(cpr::Url{"https://2ch.hk/"+url},
                            cpr::Cookies{{"usercode_auth", "56f407dd40a9f644f0f1ddf12ebf0fc3"},
                                        {"z", ""}});
        if (res.text.find("<!DOCTYPE html>") != 0){
            std::cout << "Downloading " << url << " as " << filename << std::endl;
            std::ofstream file;
            file.open(curpath.string()+filename);
            file << res.text;
            file.close();
        }else
            std::cout << "Skipping " << url << " cause 404 returned" << std::endl;
    }
    cv.notify_one();
    threads_counter--;
}
