/**
@project seeker
@author Tao Zhang
@since 2020/3/1
@version 0.0.1-SNAPSHOT 2020/5/13
*/
#pragma once
#include <chrono>
#include <regex>


namespace seeker {

using std::string;
using std::to_string;

class Time {
 public:
  static int64_t currentTime() {
    using namespace std::chrono;
    auto time_now = system_clock::now();
    auto durationIn = duration_cast<milliseconds>(time_now.time_since_epoch());
    return durationIn.count();
  };

  static int64_t microTime() {
    using namespace std::chrono;
    auto time_now = system_clock::now();
    auto durationIn = duration_cast<microseconds>(time_now.time_since_epoch());
    return durationIn.count();
  };

  static string parseTime(int64_t timestamp) {
      auto mTime = std::chrono::milliseconds(timestamp + (int64_t)8 * 60 * 60 * 1000);
      auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
      auto tt = std::chrono::system_clock::to_time_t(tp);
      std::tm* now = std::gmtime(&tt);
      string s = to_string(now->tm_year + 1900) + to_string(now->tm_mon + 1) + to_string(now->tm_mday) + " " + to_string(now->tm_hour) + ":" + to_string(now->tm_min) + ":" + to_string(now->tm_sec);
      return s;
  }
};

class String {
 public:
  static string toLower(const string& target) {
    string out{};
    for (auto c : target) {
      out += ::tolower(c);
    }
    return out;
  }

  static string toUpper(const string& target) {
    string out{};
    for (auto c : target) {
      out += ::toupper(c);
    }
    return out;
  }

  static string trim(string& s) {
    string rst{s};
    if (rst.empty()) {
      return rst;
    }
    rst.erase(0, rst.find_first_not_of(" "));
    rst.erase(s.find_last_not_of(" ") + 1);
    return rst;
  }

  static std::vector<string> split(const string& target, const string& sp) {
    std::vector<string> rst{};
    if (target.size() == 0) {
      return rst;
    }

    const auto spLen = sp.length();
    string::size_type pos = 0;
    auto f = target.find(sp, pos);

    while (f != string::npos) {
      auto r = target.substr(pos, f - pos);
      rst.emplace_back(r);
      pos = f + spLen;
      f = target.find(sp, pos);
    }
    rst.emplace_back(target.substr(pos, target.length()));
    return rst;
  }

  static string removeBlanks(const string& target) {
    static std::regex blankRe{R"(\s+)"};
    return std::regex_replace(target, blankRe, "");
  }

  static string removeLastEmptyLines(const string& target) {
    size_t len = target.length();
    size_t i = len - 1;
    for (; i > 0; i--) {
      if (target[i] == '\n') {
        continue;
      } else if (target[i] == '\r') {
        continue;
      } else {
        break;
      }
    }
    return target.substr(0, i + 1);
  }
};


class ByteArray {
 public:
  template <typename T>
  static void writeData(uint8_t* buf, T num) {
    size_t len(sizeof(T));
    for (size_t i = 0; i < len; ++i) {
      buf[i] = (uint8_t)(num >> (i * 8));
    }
  }

  template <typename T>
  static void readData(uint8_t* buf, T& num) {
    uint8_t len(sizeof(T));
    static uint8_t byMask(0xFF);
    num = 0;

    for (size_t i = 0; i < len; ++i) {
      num <<= 8;
      num |= (T)(buf[len - 1 - i] & byMask);
    }
  }
};


}  // namespace seeker