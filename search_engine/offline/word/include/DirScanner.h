//
// Created by Tseng on 25-8-4.
//

#ifndef DIRSCANNER_H
#define DIRSCANNER_H

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace SearchEngine
{
    class DirScanner {
    public:
        DirScanner();
        ~DirScanner();

        void operator()(const string& dirname);
        vector<string>& files();
        void traverse(const string& dirname);

    private:
        vector<string> files_;  //存储文件路径
    };

}



#endif //DIRSCANNER_H
