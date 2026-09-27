//
// simple sequential MapReduce.
//
// ./mrsequential ../mrapps/wc.so pg*.txt
//

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "mr/mr.h"
#include "mr/plugin.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "Usage: mrsequential xxx.so inputfiles...\n");
        return 1;
    }

    mr::Plugin p = mr::loadPlugin(argv[1]);

    //
    // read each input file,
    // pass it to Map,
    // accumulate the intermediate Map output.
    //
    std::vector<mr::KeyValue> intermediate;
    for (int i = 2; i < argc; i++) {
        std::string filename = argv[i];
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::fprintf(stderr, "cannot open %s\n", filename.c_str());
            return 1;
        }
        std::stringstream content;
        content << file.rdbuf();
        std::vector<mr::KeyValue> kva = p.map(filename, content.str());
        intermediate.insert(intermediate.end(), kva.begin(), kva.end());
    }

    //
    // a big difference from real MapReduce is that all the
    // intermediate data is in one place, intermediate[],
    // rather than being partitioned into NxM buckets.
    //

    std::sort(intermediate.begin(), intermediate.end(),
              [](const mr::KeyValue& a, const mr::KeyValue& b) { return a.key < b.key; });

    std::FILE* ofile = std::fopen("mr-out-0", "w");
    if (ofile == nullptr) {
        std::perror("mr-out-0");
        return 1;
    }

    //
    // call Reduce on each distinct key in intermediate[],
    // and print the result to mr-out-0.
    //
    size_t i = 0;
    while (i < intermediate.size()) {
        size_t j = i + 1;
        while (j < intermediate.size() && intermediate[j].key == intermediate[i].key) {
            j++;
        }
        std::vector<std::string> values;
        for (size_t k = i; k < j; k++) {
            values.push_back(intermediate[k].value);
        }
        std::string output = p.reduce(intermediate[i].key, values);

        // this is the correct format for each line of Reduce output.
        std::fprintf(ofile, "%s %s\n", intermediate[i].key.c_str(), output.c_str());

        i = j;
    }

    std::fclose(ofile);
    return 0;
}
