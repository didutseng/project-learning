#include "../include/WordSegmentation.h"

vector<string> SearchEngine::WordSegmentation::operator()(const string str)
{
    vector<string> words;
    jieba_.CutAll(str, words);  //完整段落
    return words;
}

void SearchEngine::WordSegmentation::CutSmall(const string& str, vector<string>& vec, size_t sz)
{
    jieba_.CutSmall(str, vec, sz);
}
