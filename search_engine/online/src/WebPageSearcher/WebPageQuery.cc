#include "../../include/WebPageSearcher/WebPageQuery.h"

#include <math.h>
#include <nlohmann/json.hpp>
#include <iterator>
#include <algorithm>
#include <sstream>

using std::istringstream;
using std::ifstream;
using std::inserter;
using std::set_intersection;
using std::make_pair;

namespace SearchEngine
{

WebPageQuery * WebPageQuery::_pQuery = nullptr;

WebPageQuery::WebPageQuery()
: _jieba()
, _sim(DICT_PATH, HMM_PATH, IDF_PATH ,STOP_WORD_PATH)
{
    loadLibary();
    //加载三个库的数据
}

WebPageQuery::~WebPageQuery()
{}

vector<string> WebPageQuery::doQuery(const string &str)
{
	//使用cppjieba对象_jieba对待查存的句子进行分词
	//并将分词后的结果存放在results中
    vector<string> results = _jieba(str);
    size_t judeg = 0;
	//遍历分词的结果容器，也就是vector容器
	//然后在倒排索引表中进行查询
    for(auto & word : results) 
	{
        auto it = _inverIndexTable.find(word);
        if(it == _inverIndexTable.end()) 
		{
            ++judeg;
            // 统计无效词数量
        }
    }

    if(judeg == results.size()) 
	{
        return vector<string>();
        // 全部无效则返回空
    }

	//获取待查询结果的权重值，并准备作为基准向量
    //获得查询词向量，来作为文档向量的计算
    //比如查询苹果手机,经过分词得到 results = ["苹果", "手机"]
    //结果为{"苹果"的归一化权重, "手机"的归一化权重}
    //查询词向量Base = [w₁, w₂, ..., wₙ]
    //wᵢ：第i个查询词的归一化TF-IDF权重​
    //得到的结果是基整个语料库得到的结果，表示了该词在整个库中的权重
    vector<double> Base = getQueryWordsWeightVector(results);

    //存放词在文档中的权重，比如得到分词结果“人工”，“只能”
    // 101: [0.5, 0.8]，在文档101中"人工"权重0.5，"智能"权重0.8
    //存放文档向量
    unordered_map<int, vector<double>> resultVec;

	//results是对待查询词分词后的结果，比如：“我爱武汉”
	//会分成“我” “爱” “武汉”
    //executeQuery(results, resultVec)返回词在文档中的权重，转化为文档向量
    // 101: [0.5, 0.8]，在文档101中"人工"权重0.5，"智能"权重0.8
    if(executeQuery(results, resultVec)) 
	{
        //resultVec 1 (y1 y2 y3 ) 2(y1 y2 y3) ..
        //文档向量Doc = [v₁, v₂, ..., vₙ]
        //vᵢ：文档中第i个查询词对应的原始TF-IDF权重​（未归一化）
        //余弦相似度 = wi*vi的总和/ （wi 平方的总和的模*vi 平方的模）
        double X = 0.0;
        for(double x : Base) 
		{
            X += x * x;
            //查询词权重的总和wi
        }
        X = sqrt(X);
        //查询词向量的模wi
        vector<std::pair<int, double>> web_sort;
		//使用余弦相似度进行排序，夹角越小优先级越高

        //遍历resultVec存的文档向量
        for(auto it = resultVec.begin(); it != resultVec.end(); ++it)
		{
            int docid = it->first;
            double XY = 0.0, Y = 0.0;
            for(size_t i = 0; i < it->second.size(); ++i) 
			{
                XY += 1.0 * it->second[i] * Base[i];
                //查询词向量wi*文档向量vi 的总和
                Y += 1.0 * it->second[i] * it->second[i];
                //文档向量的总和
            }
            Y = sqrt(Y);
            //文档向量的模
            double COS = 1.0 * XY /(X * Y);
            //计算余弦相似度
            web_sort.push_back(std::make_pair(docid, COS));
            //存入文章 id和预算相似度
        }
        //对文档进行排序
        sort(web_sort.begin(), web_sort.end(), ComPair);
       
        vector<int> web_fin; //存放排序之后的网页

        for(auto it : web_sort) 
		{
            web_fin.push_back(it.first);
            //将排序之后的文章id存放在web_fin中
        }
        
        size_t sz = web_fin.size();
        //控制结果的数量，避免太多的结果展示
        if(sz < 3)
		{
            //小于三篇就按照实际值即可
        } 
        else if(sz >= 3 && sz < 6 )
		{
            web_fin.resize(3);//大于三篇按照三篇算
        } 
        else if(sz >= 6 && sz < 9) 
		{
            web_fin.resize(6);//大于6篇按照6篇算
        } 
        else if (sz >= 9 && sz < 12) 
		{
            web_fin.resize(9);//大于9篇按照9篇算
        }
        else 
		{
            web_fin.resize(12);//最多只存12篇
        }
        vector<string> rs = create_Json(web_fin, results);
        
        return rs;
    }
    else
	{
        return vector<string>();
    }
}

//组装网页id  标题  链接 摘要,目的是为了显示发给用户
//结果格式化
vector<string> WebPageQuery::create_Json(vector<int> & docidVec, 
										 const vector<string> &queryWords) 
{
    vector<string> result;
    string Line = "<---------------------------------------------------->\n";
    size_t id = 0;
    for(auto & docid : docidVec) 
    {
        string res;
        string Summary = _pageLib[docid].summary(queryWords);
        string Title = _pageLib[docid].getTile();
        string Url = _pageLib[docid].getUrl();
        char buf[65536] = {0};
        sprintf(buf, "%s所查询网页:%ld\n\t标题:%s\n\t链接:%s\n\t摘要:%s。\n", 
                Line.c_str() ,++id, Title.c_str(), Url.c_str(), Summary.c_str());
        res += buf;
        result.push_back(res);
        memset(buf, 0, sizeof(buf));
    }
    return result;
}

//resultVec是一个传出参数，里面存放的是文章id与权重，其文章id是包含待查询词的
//文章id（简单来说，就是最终可以展示给用户的文章）
//文档筛选，筛选出含有某个词项的文档 id
bool WebPageQuery::executeQuery(const vector<string> & queryWords, 
								unordered_map<int, vector<double>> & resultVec) 
{
    vector<set<int>> existWed;
    bool flag = false;
	//将待查询短语分词后的每个词对应的文章id都存放在tmp
	//中，然后将所有查询词的文章id都存放在existWed中
    for(size_t i = 0; i < queryWords.size(); ++i) 
	{
        string word = queryWords[i];
        auto it = _inverIndexTable[word]; //set<pair<int, double>>
        set<int> tmp;
        for(auto it2 = it.begin(); it2 != it.end(); ++it2 )
		{
            int docid = it2->first;
            tmp.insert(docid);//存储文章id
            flag = true;
        }
        existWed.push_back(tmp);//将存储文章id的集合set存放在vector中
    }

	//如果flag为false，说明待查询词不在倒排索引表中，也就是待查询词不存在
	//从来没有在对应的文章中出现过(比如非常生僻的字或者这些字组成的短语)
    if(flag == false) 
	{
        return false;
    }
	//将待查询词分词后，将所有分词的结果取交集，得到文章id
	//就是分出的词都出现的文章
    set<int> SameWeb = existWed[0];
    for(size_t i = 0; i < existWed.size(); ++i) 
	{
        set<int> tmp;
        set_intersection(SameWeb.begin(), SameWeb.end(), 
                         existWed[i].begin(), existWed[i].end(),
                         inserter(tmp, tmp.end()));
        SameWeb.swap(tmp);
    }

    if(!SameWeb.size()) 
	{ 
		return false ;
	}

	//SameWeb是包含待查询词的set集合，该集合中包含所有待查询词的
	//文章id
    for(auto it = SameWeb.begin(); it != SameWeb.end(); ++it) 
	{
        int docid = *it;
		//it2的类型其实就是vector<double>,存放所有的单词权重
        auto & it2 = resultVec[docid];
        //resultVec 存放文章 id和权重
        for(string  word : queryWords) 
		{
			//倒排索引unordered_map<string, set<std::pair<int, double>>> 
			//_inverIndexTable中通过word进行查找
            auto it3 = _inverIndexTable[word];
            for(auto it4 = it3.begin(); it4 != it3.end(); ++it4)
			{
				//找到文章id，说明相似的文章，就将权重存起来
				//放在it2中存起来
                if(it4->first == docid)
				{
                    it2.push_back(it4->second);
                    break;
                }
            }
        }
    }
    return true;
}

//计算权重，对文章进行归一化处理
vector<double> WebPageQuery::getQueryWordsWeightVector(vector<string> &queryWords) 
{
    vector<double> results; // w' w'
    unordered_map<string, int> tf;
    // 词频统计表
    double N = _pageLib.size() + 1;
    for(string & word : queryWords) 
	{
        int & isExit = tf[word];
        if(isExit) 
		{
            ++isExit;
            // 已存在则计数+1
        }
        else
		{
			isExit = 1;
            // 不存在则初始化为1
        }
    }
    vector<double> tmp; // w1 w2 w3... 
    for(string & word : queryWords) 
	{
        auto it = _inverIndexTable[word];
		//DF表示某个词在所有文章中出现的次数,即：包含该词语的文档数量
        double DF = it.size();
		//IDF表示逆文档频率
        //log2(所有文档的数量除以（包含该词文档的数量+1）)
        double IDF = log(N/(DF + 1)) / log(2);
		//某个词在文章中出现的次数
        double TF = tf[word];
		//每个词的权重，w = TF * IDF;
        double w = 1.0 * TF * IDF;
		//存放每个词的权重到tmp中存起来
        tmp.push_back(w);
    }
    double AllW = 0.0;
    for(double w : tmp) 
	{
        AllW += w * w;
    }
    AllW = sqrt(AllW);
	//归一化处理,并将结果存放在vector<double> results
	//中，然后返回
    for(double w : tmp) 
	{
        double FinW = w / AllW;
        results.push_back(FinW);
    }
    return results;
}

void WebPageQuery::loadLibary() 
{
	//加载偏移库，存放在容器中
    ifstream ifsOff;
    ifsOff.open("../offline/page/data/offset.dat");
    cout << "loading offset.dat ..." << endl;

    string line;
    while(ifsOff >> line) 
	{
        int id = atoi(line.c_str());
        int begin;
        int end;
        ifsOff >> begin;
        ifsOff >> end;
        _offsetLib[id] = std::make_pair(begin, end);
    }

	//加载网页库，存放在容器中
    ifstream ifsPage;
    ifsPage.open("../offline/page/data/ripepage.dat");
    cout << "loading ripepage.dat ..." << endl;

    for(auto it = _offsetLib.begin(); it != _offsetLib.end(); ++it) 
	{
        int begin = it->second.first;
        const size_t size = it->second.second;
        char buf[65536] = {0};
        string line;
        ifsPage.seekg(begin);
        ifsPage.read(buf, size);
        string doc = buf;
        _pageLib.insert(std::make_pair(it->first, webPage(doc, &_jieba, &_sim)));
    }

	//加载倒排索引库，存放在容器中
    ifstream ifsIndex;
    ifsIndex.open("../offline/page/data/invertIndex.dat");
    cout << "loading invertIndex.dat ..." << endl;

    string line2;
    string word;
    while(getline(ifsIndex, line2)) 
	{
        istringstream iss(line2);
        iss >> word;
        int docid;
        while (iss >> docid)
        {
           double w;
           iss >> w;
           _inverIndexTable[word].insert(std::make_pair(docid, w));
        }    
    }
   
    ifsOff.close();
    ifsPage.close();
    ifsIndex.close();
}
};//end of namespace search_engine








