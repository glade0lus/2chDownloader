# 2chDownloader  
## About  
Command-line tool for downloading images and videos from 2ch.hk  

## How to compile  
* Install CMake and Boost  
* Clone repository  
* Clone submodules: `git submodule update --init --recursive`  
* Compile: `cmake . && make`  

## How to use  
Run 2chDownloader without parameters or with `--help`  

#### Help page:  
USAGE:  
	2chDownloader <download_list> [options]  
	2chDownloader a,b/1,vg/1:2:3 --path=./a_b_vg/ --original-names  
  
DOWNLOAD_LIST:  
	board                   Download whole /board/  
	board/id                Download thread by id from /board/  
	board/id1:id2:id3       Download multiple threads from /board/  
  
OPTIONS:  
	--path=<path>           Set custom relative download path (default "./downloads/")  
	--threads=<num>         Set number of download threads (default 10)  
	--usercode-auth=<token> Set authcookie for access to hidden boards  
	--webm-only             Download only .webm videos  
	--image-only            Download only images  
	--original-names        Save files with names given by uploader, not by imageboard  
	--help                  Show this help  
	--version               Show version  

## Copyrights  
C++ Requests: https://github.com/whoshuu/cpr (MIT License)  
JSON for Modern C++: https://github.com/nlohmann/json (MIT License)  
