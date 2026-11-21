#include <iostream>
#include <string>
#include <cstdio>
#include <cctype>
#include <array>
#include <vector>
#include <algorithm>

using namespace std;

static const string VERSION = "1.0.0-sushiui";

string exec(const string& cmd) {
    array<char, 256> buffer;
    string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);
    return result;
}

string pingCommand(const string& host) {
#ifdef _WIN32
    return "ping -n 1 " + host + " 2>&1";
#else
    return "ping -c 1 " + host + " 2>&1";
#endif
}

bool readLine(const string& prompt, string& out) {
    cout << prompt;
    if (!getline(cin, out)) return false;

    size_t first = out.find_first_not_of(" \t\r\n");
    size_t last = out.find_last_not_of(" \t\r\n");
    out = (first == string::npos) ? "" : out.substr(first, last - first + 1);
    return true;
}

bool validHost(const string& host) {
    if (host.empty() || host.size() > 253 || host[0] == '-') return false;

    for (unsigned char c : host) {
        if (!isalnum(c) && c != '.' && c != '-' && c != ':') return false;
    }

    return true;
}

bool isLiveMode(const string& input) {
    return input == "live";
}

bool hostResponded(const string& host) {
    string out = exec(pingCommand(host));

    return out.find("time=") != string::npos ||
           out.find("bytes from") != string::npos ||
           out.find("TTL=") != string::npos;
}

double safeToDouble(const string& t) {
    try {
        return stod(t);
    } catch (...) {
        return -1;
    }
}

string extractTime(const string& output) {
    size_t pos = output.find("time=");
    if (pos == string::npos) return "timeout";

    pos += 5;
    string ms;

    while (pos < output.size() &&
          (isdigit((unsigned char)output[pos]) || output[pos] == '.' || output[pos] == '<')) {

        if (output[pos] != '<')
            ms += output[pos];

        pos++;
    }

    if (ms.empty()) return "timeout";
    return ms;
}

void showStats(const vector<double>& times, int total, int failed) {
    if (times.empty()) {
        cout << "\nNo valid responses.\n";
        return;
    }

    double sum = 0;
    double mn = *min_element(times.begin(), times.end());
    double mx = *max_element(times.begin(), times.end());

    for (double t : times) sum += t;

    double avg = sum / times.size();
    double loss = (double)failed / total * 100.0;

    cout << "\n===== STATS =====\n";
    cout << "Min: " << mn << " ms\n";
    cout << "Max: " << mx << " ms\n";
    cout << "Avg: " << avg << " ms\n";
    cout << "Packet loss: " << loss << "%\n";
    cout << "=================\n";
}

void liveMode(const string& host) {
    cout << "\nLIVE MODE STARTED (CTRL+C to stop)\n\n";

    vector<double> times;
    int failed = 0;
    int count = 0;

    while (true) {
        string out = exec(pingCommand(host));
        string t = extractTime(out);

        count++;

        if (t == "timeout") {
            failed++;
            cout << "timeout\n";
        } else {
            double val = safeToDouble(t);

            if (val < 0) {
                failed++;
                cout << "timeout\n";
            } else {
                times.push_back(val);
                cout << val << " ms\n";
            }
        }

        if (count % 20 == 0) {
            showStats(times, count, failed);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            cout << "OhAPinger " << VERSION << "\n";
            return 0;
        }
    }

    while (true) {

        cout << "=====================================\n";
        cout << "        Welcome to OhAPinger!        \n";
        cout << "        Version " << VERSION << "\n";
        cout << " OSS project made by bangkkuser      \n";
        cout << " GitHub: rebangkkuser                \n";
        cout << " If you want a live ping, type \"live\" in the test quantity.\n";
        cout << "=====================================\n\n";

        string host;
        string inputTests;

        if (!readLine("Enter host/IP: ", host)) break;

        if (!validHost(host)) {
            cout << "Invalid host.\n\n";
            continue;
        }

        if (!readLine("Number of tests (or 'live'): ", inputTests)) break;

        if (isLiveMode(inputTests)) {
            if (!hostResponded(host)) {
                cout << "The host did not respond.\n\n";
                continue;
            }

            cout << "The host responded, continuing...\n";

            liveMode(host);
            continue;
        }

        int tests = 0;
        size_t parsed = 0;

        try {
            tests = stoi(inputTests, &parsed);
        } catch (...) {
            parsed = 0;
        }

        if (parsed != inputTests.size() || tests < 1) {
            cout << "Invalid input.\n\n";
            continue;
        }

        vector<double> times;
        int failed = 0;

        cout << "\nChecking host...\n";

        if (!hostResponded(host)) {
            cout << "The host did not respond.\n\n";
            continue;
        }

        cout << "The host responded, continuing the test.\n\n";

        for (int i = 1; i <= tests; i++) {

            string out = exec(pingCommand(host));
            string t = extractTime(out);

            if (t == "timeout") {
                cout << "Test " << i << ": timeout\n";
                failed++;
                continue;
            }

            double val = safeToDouble(t);

            if (val < 0) {
                cout << "Test " << i << ": timeout\n";
                failed++;
            } else {
                times.push_back(val);
                cout << "Test " << i << ": " << val << " ms\n";
            }
        }

        showStats(times, tests, failed);

        string again;

        if (!readLine("\nDo you want to start a new test? [y/n]: ", again) ||
            (again != "y" && again != "Y")) {
            cout << "Exiting OhAPinger...\n";
            break;
        }

        cout << "\n\n";
    }

    return 0;
}

