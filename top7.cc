#include <algorithm>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace std;

#define ERROR (-1)
#define VOTE 0
#define TOP 1
#define NEW 2
#define BLANK 3
#define TOPLENGTH 7

static bool isValidVote(unordered_set<uint32_t> &outOfVote,
                        unordered_set<uint32_t> &outOfTop, uint32_t max,
                        uint32_t value) {
  return !outOfVote.contains(value) && !outOfTop.contains(value) &&
         value <= max;
}

static bool cmp(const pair<uint32_t, uint32_t> &a,
                const pair<uint32_t, uint32_t> &b) {
  if (a.second == b.second) {
    return a.first < b.first;
  } else {
    return a.second > b.second;
  }
}

static vector<uint32_t> getTop7(unordered_map<uint32_t, uint32_t> &votes) {
  vector<pair<uint32_t, uint32_t>> topMap;
  for (auto &v : votes) {
    topMap.emplace_back(v);
    sort(topMap.begin(), topMap.end(), cmp);
    if (topMap.size() > TOPLENGTH) {
      topMap.erase(prev(topMap.end()));
    }
  }
  vector<uint32_t> top7;
  for (pair<uint32_t, uint32_t> t : topMap) {
    top7.push_back(t.first);
  }
  return top7;
}

static void addVotes(unordered_map<uint32_t, uint32_t> &votes,
                     unordered_map<uint32_t, uint32_t> &voted) {
  for (auto &i : voted) {
    votes[i.first]++;
  }
}

static unordered_map<uint32_t, int8_t> vectorToMap(vector<uint32_t> &v) {
  unordered_map<uint32_t, int8_t> map;
  for (size_t i = 0; i < v.size(); ++i) {
    map[v[i]] = (int8_t)(i + 1);
  }
  return map;
}

static void writeSong(uint32_t song, int8_t rank,
                      unordered_map<uint32_t, int8_t> &prev) {
  cout << song;
  if (prev[song] != 0) {
    cout << " " << prev[song] - rank << '\n';
    return;
  }
  cout << " -\n";
}

static void writeRank(vector<uint32_t> &actual, vector<uint32_t> &prev) {
  unordered_map<uint32_t, int8_t> prevMap = vectorToMap(prev);
  for (size_t i = 0; i < actual.size(); ++i) {
    writeSong(actual[i], (int8_t)(i + 1), prevMap);
  }
}

static void updatePointsAndOutOfVotes(vector<uint32_t> &actual,
                                      vector<uint32_t> &prev,
                                      unordered_map<uint32_t, uint32_t> &points,
                                      unordered_set<uint32_t> &OutOfVote) {
  for (size_t i = 0; i < actual.size(); ++i) {
    points[actual[i]] += TOPLENGTH - i;
  }
  unordered_map<uint32_t, int8_t> actualMap = vectorToMap(actual);
  for (auto &song : prev) {
    if (actualMap[song] == 0) {
      OutOfVote.insert(song);
    }
  }
}

static void updateOutOfTop(vector<uint32_t> &top,
                           unordered_set<uint32_t> &outOfVote,
                           unordered_set<uint32_t> &outOfTop,
                           unordered_map<uint32_t, uint32_t> &points) {
  unordered_map<uint32_t, int8_t> topMap = vectorToMap(top);
  vector<uint32_t> newOut;
  for (auto &song : outOfVote) {
    if (topMap[song] == 0) {
      newOut.push_back(song);
    }
  }
  for (uint32_t &i : newOut) {
    outOfVote.erase(i);
    outOfTop.insert(i);
    points.erase(i);
  }
}

int main() {
  string line;
  uint32_t lineNumber = 1;
  uint32_t max = 0;
  uint32_t value;
  regex regVote(R"((\s*[1-9][0-9]{0,7})(\s+([1-9][0-9]{0,7}))*\s*)");
  regex regTop("\\s*TOP\\s*");
  regex regNew(R"(\s*NEW\s*([1-9][0-9]{0,7})\s*)");
  regex regBlank("\\s*");
  istringstream stringValue;
  unordered_map<uint32_t, uint32_t> votes, points;
  vector<uint32_t> prevNew, prevTop;
  unordered_set<uint32_t> outOfVote, outOfTop;

  while (getline(cin, line)) {
    int lineOption = ERROR;
    stringValue.clear();

    if (regex_match(line, regVote)) {
      lineOption = VOTE;
      stringValue.str(line);
      unordered_map<uint32_t, uint32_t> voted;
      while (stringValue >> value) {
        if (!isValidVote(outOfVote, outOfTop, max, value)) {
          lineOption = ERROR;
          break;
        } else {
          voted[value]++;
          if (voted[value] > 1) {
            lineOption = ERROR;
            break;
          }
        }
      }
      if (lineOption == VOTE) {
        addVotes(votes, voted);
      }
      voted.clear();

    } else if (regex_match(line, regTop)) {
      lineOption = TOP;
      vector<uint32_t> actualTop = getTop7(points);
      writeRank(actualTop, prevTop);
      updateOutOfTop(actualTop, outOfVote, outOfTop, points);
      prevTop = actualTop;

    } else if (regex_match(line, regNew)) {
      stringValue.str(line);
      string readNew;
      stringValue >> readNew;
      stringValue >> value;

      if (value >= max) {
        lineOption = NEW;
        vector<uint32_t> actualNew = getTop7(votes);
        writeRank(actualNew, prevNew);
        max = value;
        updatePointsAndOutOfVotes(actualNew, prevNew, points, outOfVote);
        votes.clear();
        prevNew = actualNew;
      }
    } else if (regex_match(line, regBlank)) {
      lineOption = BLANK;
    }
    if (lineOption == ERROR) {
      cerr << "Error in line " << lineNumber << ": " << line << "\n";
    }

    lineNumber++;
  }
  return 0;
}