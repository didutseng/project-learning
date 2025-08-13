#include "../../include/Configuration.h"
#include "../../include/KeyRecommander/Dictionary.h"
#include "../../include/WebPageSearcher/WebPageQuery.h"
#include "../../include/SearchEngineServer/SearchEngineServer.h"
#include "../../include/LRUCache/CacheManager.h"
#include "../../include/TimerManager/TimerFd.h"

#include <thread>

using std::string;
using std::thread;

using namespace SearchEngine;

int main(int argc, char *argv[]) 
{
    map<string, string> configs = Configuration::getInstance()->getConfigs();
    //��ȡ����ģ����conf/server.conf�����ļ��е����ݣ�������ip��port�����߳���Ŀ
	//������д�С �ȵ� 

    int threadNum = stoi(configs["threadNum"]);
    // �̳߳��߳���
    int queSize   = stoi(configs["queSize"]);
    // ������д�С
    string ip     = configs["ip"];
    // ����IP
    size_t port   = stoi(configs["port"]);
    // �����˿�

    Dictionary::getInstance();
    //���ؼ����Ƽ����ֵ������ļ��е����ݼ��ص��ڴ��У�
	//�������ݽṹ����������������ļ��ֱ��ǣ�
	//�ʵ�⡢�����⡢��ݿ⣨idMap��
	
    WebPageQuery::getWebQueryPtr();
    //����ҳ�������ֶ�Ӧ�������ļ����ص��ڴ��У�
	//�����ݽṹ���������,�����ļ��ֱ��ǣ�
	//��ҳ�⡢��ҳƫ�ƿ⡢����������

    cout << "loading server ..." << endl;

    CacheManager *p = CacheManager::getInstance();
    //Ԥ��������Ŀ�ľ���Ϊ�˽����������CacheManager�е����ݳ�Ա���г�ʼ��

	//��ʱ���ඨʱִ�и��º���updateCache��updateCache2�����»���
	//���ݴ����߼���һ���ģ�����Ϊ����������ѯǰ��������������
    TimerFd tf1(2, 3, std::bind(&CacheManager::updateCache, p));
    thread th1(&TimerFd::start, &tf1);

    TimerFd tf2(3, 5, std::bind(&CacheManager::updateCache2, p));
    thread th2(&TimerFd::start, &tf2);

	//�������з������Ĵ��룬���������Ĳ�ѯ����
    SearchEngineServer seServer(threadNum, queSize, ip, port);
    seServer.start();
    return 0;
}
