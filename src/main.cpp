#include <filesystem>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
namespace fs = std::filesystem;

#include <ftxui/component/task.hpp>  // for Task, Closure, AnimationTask
#include <ftxui/dom/table.hpp>
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/event.hpp"
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "config.h"

using namespace ftxui;
std::vector<std::string> getFileList(std::string path);
float get_focus_y(int selected, int length);
std::string getParentDirectory(const std::string path);
std::string getCurrentFile(const std::string path);

int main(void) {
  // Get list of files
  std::string path = std::filesystem::current_path();
  std::vector<std::string> fileList = getFileList(path);
  int selected = 0;
  bool input_file_path_focused = false;

  // setup path selector
  auto input_option = InputOption();
  input_option.transform = [&](InputState state) {
    if (state.focused) {
      input_file_path_focused = true;
    } else {
      input_file_path_focused = false;
    }
    return state.element;
  };
  input_option.on_enter = [&] { fileList = getFileList(path); };

  Component input_file_path = Input(&path, "Enter Path", input_option);
  input_file_path |= CatchEvent([&](Event event) {
    if (ftxui::Event::Character('\n') == event) {
      fileList = getFileList(path);
      return true;
    }
    return false;
  });

  float focus_y = 0.0f;

  // populate file list
  auto menu_option = MenuOption().Vertical();
  menu_option.on_enter = [&] {
    auto selectedFile = fileList[selected];
    selected = 0;
    focus_y = 0;
    path = selectedFile == ".." ? getParentDirectory(path) : selectedFile;
    fileList = getFileList(path);
  };
  menu_option.on_change = [&] {
    focus_y = get_focus_y(selected, fileList.size());
  };
  // TODO: mark directories
  Component menu = Menu(&fileList, &selected, menu_option);

  // The component tree:
  auto component = Container::Vertical({
      input_file_path,
      menu,
  });

  auto slider_y = Slider("y", &focus_y, 0.f, 1.f, 0.01f);

  auto renderer = Renderer(component, [&] {
    return vbox({
        hbox(input_file_path->Render()),
        separator(),
        menu->Render() | focusPositionRelative(0.0f, focus_y) | frame | flex,
    });
  });

  renderer |= CatchEvent([&](Event event) {
    if (event == Event::Backspace && !input_file_path_focused) {
      // go up a directory
      path = getParentDirectory(path);
      fileList = getFileList(path);
      return true;
    }
    if (event == Event::Character('h') && !input_file_path_focused) {
      showHidden = !showHidden;
      fileList = getFileList(path);
      selected = 0;
      focus_y = 0;
      return true;
    }
    if (event == Event::Character('d') && !input_file_path_focused) {
      mixDirectoriesAndFiles = !mixDirectoriesAndFiles;
      fileList = getFileList(path);
      selected = 0;
      focus_y = 0;
      return true;
    }
    if (event == Event::Home && !input_file_path_focused) {
      selected = 0;
      focus_y = 0;
      return true;
    }
    if (event == Event::End && !input_file_path_focused) {
      selected = fileList.size();
      focus_y = 100;
      return true;
    }
    // if (event == Event::PageUp && !input_file_path_focused) {
    //   selected = selected * 0.25;
    //   focus_y = (selected / fileList.size()) * 100;
    //   path = std::to_string(selected);
    //   return true;
    // }
    // if (event == Event::PageDown && !input_file_path_focused) {
    //   selected = selected / 0.25;
    //   focus_y = (selected / fileList.size()) * 100;
    //   path = std::to_string(selected);
    //   return true;
    // }
    return false;
  });

  auto screen =
      ScreenInteractive::TerminalOutput().ScreenInteractive::Fullscreen();
  screen.Loop(renderer);
  // std::system("ls /");
  std::string exitPath = "(cd " + path + " && bash)";
  std::system(exitPath.c_str());
  return EXIT_SUCCESS;
}

std::vector<std::string> getFileList(std::string path) {
  std::vector<std::string> combinedList;
  std::vector<std::string> fileList;
  std::vector<std::string> dirList;
  try {
    for (const auto& entry : fs::directory_iterator(path)) {
      // TODO: skip hidden files
      if (!showHidden || (showHidden && getCurrentFile(entry.path().string())[0] != '.')) {
        // if mix just add to dirs
        if(mixDirectoriesAndFiles) {
          dirList.push_back(entry.path().string());
        } else {
          // check type
          if(entry.is_directory()){
            dirList.push_back(entry.path().string());
          } else {
            fileList.push_back(entry.path().string());
          }
        }
      }
    }
  } catch (...) {}
  // alphabetize
  std::sort(dirList.begin(), dirList.end(), [](std::string a, std::string b) { return a < b; });
  std::sort(fileList.begin(), fileList.end(), [](std::string a, std::string b) { return a < b; });
  // if empty add ..
  if (fileList.size() == 0 && dirList.size() == 0) fileList.push_back("..");
  combinedList = dirList;
  combinedList.insert(combinedList.end(), fileList.begin(), fileList.end());
  return combinedList;
}

float get_focus_y(int selected, int length) {
  return static_cast<float>(selected) / static_cast<float>(length);
}

std::string getParentDirectory(const std::string path) {
  std::stringstream ss(path);
  std::vector<std::string> tokens;
  std::string token;

  while (std::getline(ss, token, '/')) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }

  if (tokens.size() >= 2) {
    tokens.pop_back();
    return std::accumulate(
        tokens.begin(), tokens.end(), std::string(""),
        [](const std::string& a, const std::string& b) { return a + '/' + b; });
  } else {
    return "/";
  }
}

std::string getCurrentFile(const std::string path) {
    std::stringstream ss(path);
  std::vector<std::string> tokens;
  std::string token;

  while (std::getline(ss, token, '/')) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }

  if (tokens.size() > 0) {
    return tokens[tokens.size() - 1];
  }
  return path;
}