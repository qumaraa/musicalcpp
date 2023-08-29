#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <filesystem>
#include <conio.h>
#include <thread>
#include <mmsystem.h>
#include <fstream>
#include <map>
#pragma comment(lib, "winmm.lib")

namespace fs = ::std::filesystem;


struct Config {
    bool show_system_time;
    int time_show_interval;
    std::string sound_path;
}cfg;

bool stringToBool(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);

    return (lowerStr == "true" || lowerStr == "1");
}

void PlaySound(const std::string& soundFile) {
    PlaySoundA(soundFile.c_str(), NULL, SND_FILENAME | SND_ASYNC);
}

void Time(bool isAllowed) {
    if (isAllowed) {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(cfg.time_show_interval));
            const auto now = std::chrono::system_clock::now();
            const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
            std::cout << "\nThe time: " << std::ctime(&t_c) << std::endl;
            //std::cout << "CONFIG: \n" << "show_system_time" << cfg.show_system_time << "\nset_interval" << cfg.time_show_interval << std::endl;
        }
    }
}

int main() {
    
    bool validDirectory = false; // Flag to track if a valid directory is entered

    std::ifstream cfgFile("config.txt");
    std::map<std::string, std::string> cfgData;

    if (cfgFile.is_open()) {
        std::string line;
        while (std::getline(cfgFile, line)) {
            if (line.empty() || line[0] == '#') {
                continue;
            }
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                cfgData[key] = value;
            }
        }
    }
    cfgFile.close();

    if (cfgData.count("show_system_time")) {
        cfg.show_system_time = stringToBool(cfgData["show_system_time"]);
    }
    if (cfg.show_system_time != false and cfgData.count("set_interval")) {
        cfg.time_show_interval = std::stoi(cfgData["set_interval"]);
    }
    if (cfgData.count("path")) {
        cfg.sound_path = cfgData["path"];
    }
    fs::path fakeDir(cfg.sound_path);
    fs::path itemsDir;
    
    if (fs::is_directory(fakeDir)) {
        itemsDir = fakeDir;
    }
    else if (!fs::is_directory(fakeDir) and !cfg.sound_path.empty()) {
        MessageBoxA(nullptr, "Directory not found or invalid. Please, check config file and enter a valid path or leave the 'path' line empty", "Error", MB_ICONERROR);
        exit(1);
    }
    

    if (fs::is_directory(itemsDir)) {
        validDirectory = true; // Set the flag to true if the entered path is a valid directory
    }
    if (cfg.sound_path.empty() and !validDirectory and itemsDir.empty()) {
        while (!validDirectory) {
            std::cout << "[format of sounds should be '.wav']" << std::endl;
            std::cout << "Enter path with sounds: ";
            std::cin >> itemsDir;

            if (fs::is_directory(itemsDir)) {
                validDirectory = true; // Set the flag to true if the entered path is a valid directory
            }
            else {
                std::cout << "Directory not found. Please enter a valid path." << std::endl;
            }
        }
    }

    long seconds = 0;
    std::vector<std::string> soundFiles;

    for (const auto& entry : fs::directory_iterator(itemsDir)) {
        if (entry.path().extension() == ".wav") {
            soundFiles.push_back(entry.path().filename().string());
        }
    }

    if (soundFiles.empty()) {
        std::cout << "No sound files found in the specified directory." << std::endl;
        return 1;
    }
    
    std::cout << "Available sound files:" << std::endl;
    for (const auto& soundFile : soundFiles) {
        std::cout << "- " << soundFile << std::endl;
    }

    std::cout << "Press Enter to start playing sounds..." << std::endl;
    while (_getch() != 13);

    std::thread time(Time,cfg.show_system_time);

    for (const auto& soundFile : soundFiles) {
        std::cout << "\nNow playing: " << soundFile << std::endl;
        PlaySound((itemsDir / soundFile).string());

        std::cout << "\nPress Enter to play the next sound..." << std::endl;

        while (_getch() != 13); // 13 - Enter
    }

    time.join();
    
    std::cout << std::endl; // Print a newline after the timer thread is stopped.
    return 0;
}

// on hotkeys show info about config
