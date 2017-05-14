#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <string>
#include <queue>
#include <vector>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <cpr/cpr.h>
#include "deps/json.hpp"
using json = nlohmann::json;

typedef unsigned int uint;

struct Board{
    std::string name;
    std::queue<uint> threads;
};

class DvachMediaDownloader{
private:
    uint threads_counter = 0, threads_max = 10;
    std::condition_variable cv;
    std::mutex mtx;
    std::string path= "./downloads/", usercode_auth;
    std::queue<Board> boards;
    bool f_thread = false, f_board = false, f_webm_only = false, f_image_only = false, f_original_names = false;
public:
    DvachMediaDownloader(int argc, char *argv[]);
    void ShowHelp();
    void ParseList(std::string &download_list);
    void ThreadsToQueue(std::string &board);
    void DownloadThread(std::string &board, uint thread_id);
    void Download(std::string &filename, std::string &url);
};

#endif
