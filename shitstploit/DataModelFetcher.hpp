#include <basetsd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>


std::string appdata = std::getenv("LOCALAPPDATA");
std::filesystem::path rbx_logs = appdata + "\\Roblox\\logs";

std::string get_rbx_log() {
    std::string rbx_log;

    if (std::filesystem::is_directory(rbx_logs)) {
        for (std::filesystem::directory_entry file : std::filesystem::directory_iterator(rbx_logs)) {
            if (file.is_regular_file()) {
                std::string file_path = file.path().string();
                std::remove(file_path.c_str());

                if (file_path.find("_Player_") != std::string::npos) {
                    rbx_log = file_path;
                }
            }
        }
    }

    return rbx_log;
}


DWORD64 GetRenderView() {
    DWORD64 render_view = 0;
    std::ifstream file(get_rbx_log());

    if (file.fail()) {
        std::cout << "failed, error code: 104abf628";
        return render_view;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t match = line.find("view(");

        if (match != std::string::npos) {
            std::string address = line.substr(match + 5, 16);
            render_view = strtoll(address.c_str(), nullptr, 16);

            break;
        }
    }

    file.close();

    return render_view;
}