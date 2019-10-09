// TdxHqDemoCpp.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <winbase.h>
#include <iostream>
#include <vector>
using namespace std;


//�����ĵ�
//

//1.����API����TdxHqApi.dll�ļ��ĵ����������������º�����(�������麯����Ϊ�ͻ������������ѯ�����Ƿ���������)

//bool TdxL2Hq_GetDetailTransactionData(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//��ȡ��ʳɽ�
//bool TdxL2Hq_GetDetailOrderData(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//��ȡ���ί��
//bool TdxL2Hq_GetSecurityQuotes10 (byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);//��ȡʮ������
//bool TdxL2Hq_GetBuySellQueue(byte Market, char* Zqdm, char* Result, char* ErrInfo);//��ȡ��������


//bool  TdxL2Hq_Connect(char* IP, int Port, char* Result, char* ErrInfo);//����ȯ�����������
//void  TdxL2Hq_Disconnect();//�Ͽ�������
//bool  TdxL2Hq_GetSecurityCount(byte Market, short& Result, char* ErrInfo);//��ȡ�г�������֤ȯ����Ŀ
//bool  TdxL2Hq_GetSecurityList(byte Market, short Start, short& Count, char* Result, char* ErrInfo);//��ȡָ����Χ�����е�֤ȯ����
//bool  TdxL2Hq_GetSecurityBars(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//��ȡ��ƱK��
//bool  TdxL2Hq_GetIndexBars(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//��ȡָ��K��
//bool  TdxL2Hq_GetMinuteTimeData(byte Market,  char* Zqdm, char* Result, char* ErrInfo);//��ȡ��ʱͼ����
//bool  TdxL2Hq_GetHistoryMinuteTimeData(byte Market, char* Zqdm, int date, char* Result, char* ErrInfo);//��ȡ��ʷ��ʱͼ����
//bool  TdxL2Hq_GetTransactionData(byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//��ȡ��ʱ�ɽ�
//bool  TdxL2Hq_GetHistoryTransactionData(byte Market, char* Zqdm, short Start, short& Count, int date, char* Result, char* ErrInfo);//��ȡ��ʷ��ʱ�ɽ�
//bool  TdxL2Hq_GetSecurityQuotes(byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);//��ȡ�̿��嵵����
//bool  TdxL2Hq_GetCompanyInfoCategory(byte Market, char* Zqdm, char* Result, char* ErrInfo);//��ȡF10��Ϣ���
//bool  TdxL2Hq_GetCompanyInfoContent(byte Market, char* Zqdm, char* FileName, int Start, int Length, char* Result, char* ErrInfo);//��ȡF10��Ϣ����
//bool  TdxL2Hq_GetXDXRInfo(byte Market, char* Zqdm, char* Result, char* ErrInfo);//��ȡȨϢ����
//bool  TdxL2Hq_GetFinanceInfo(byte Market, char* Zqdm, char* Result, char* ErrInfo);//��ȡ������Ϣ



///����ӿ�ִ�к����ʧ�ܣ����ַ���ErrInfo�����˳�����Ϣ����˵����
///����ɹ������ַ���Result�����˽������,��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���
///���ص�Result������ݶ���\n,\t�ָ��������ַ����������ѯK�����ݣ����صĽ���ַ���������

///��ʱ��\t���̼�\t���̼�\t��߼�\t��ͼ�\t�ɽ���\t�ɽ���\n
///20150519\t4.644000\t4.732000\t4.747000\t4.576000\t146667487\t683638848.000000\n
///20150520\t4.756000\t4.850000\t4.960000\t4.756000\t353161092\t1722953216.000000��

///��ô�����֮��ͨ���ָ��ַ����� ���Իָ�Ϊ���м��еı����ʽ������


//2.APIʹ������Ϊ: Ӧ�ó����ȵ���TdxL2Hq_Connect����ͨ�������������,Ȼ��ſ��Ե��������ӿڻ�ȡ��������,Ӧ�ó���Ӧ���д��������������, �ӿ����̰߳�ȫ��
//������ߣ���������api������api�᷵���Ѿ����ߵĴ�����Ϣ��Ӧ�ó���Ӧ���ݴ˴�����Ϣ�������ӷ�������


//3.������������˵��

/// <summary>
/// ��ȡ��ʳɽ�ĳ����Χ�ڵ�����
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ��ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ��С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ,���2000</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetDetailTransactionDataDelegate) (byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);




/// <summary>
/// ��ȡ�������ί��ĳ����Χ�ڵ�����
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ��ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ��С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ,���2000</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetDetailOrderDataDelegate) (byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);

/// <summary>
/// ������ȡ���֤ȯ��ʮ����������
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�, ��i��Ԫ�ر�ʾ��i��֤ȯ���г�����</param>
/// <param name="Zqdm">֤ȯ����, Count��֤ȯ������ɵ�����</param>
/// <param name="Count">APIִ��ǰ,��ʾ�û�Ҫ�����֤ȯ��Ŀ,���50, APIִ�к�,������ʵ�ʷ��ص���Ŀ</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityQuotes10Delegate) (byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ������������
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetBuySellQueueDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);




/// <summary>
///  ����ͨ�������������
/// </summary>
/// <param name="IP">������IP,������ʾ���ڲ��</param>
/// <param name="Port">�������˿�</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool (__stdcall*  TdxL2Hq_ConnectDelegate)(char* IP, int Port, char* Result, char* ErrInfo);


/// <summary>
/// �Ͽ�ͬ������������
/// </summary>
typedef void(__stdcall* TdxL2Hq_DisconnectDelegate)();

/// <summary>
/// ��ȡ�г�������֤ȯ������
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��ص�֤ȯ����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityCountDelegate)(byte Market, short& Result, char* ErrInfo);


/// <summary>
/// ��ȡ�г���ĳ����Χ�ڵ�1000֧��Ʊ�Ĺ�Ʊ����
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Start">��Χ��ʼλ��,��һ����Ʊ��0, �ڶ�����1, ��������,λ����Ϣ����TdxL2Hq_GetSecurityCount���ص�֤ȯ����ȷ��</param>
/// <param name="Count">��Χ��С��APIִ�к�,������ʵ�ʷ��صĹ�Ʊ��Ŀ,</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��ص�֤ȯ������Ϣ,��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityListDelegate)(byte Market, short Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ֤ȯĳ����Χ�ڵ�K������
/// </summary>
/// <param name="Category">K������, 0->5����K��    1->15����K��    2->30����K��  3->1СʱK��    4->��K��  5->��K��  6->��K��  7->1����  8->1����K��  9->��K��  10->��K��  11->��K��< / param>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ��ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ��С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ, ���ֵ800</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityBarsDelegate)(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡָ��ĳ����Χ�ڵ�K������
/// </summary>
/// <param name="Category">K������, 0->5����K��    1->15����K��    2->30����K��  3->1СʱK��    4->��K��  5->��K��  6->��K��  7->1����  8->1����K��  9->��K��  10->��K��  11->��K��< / param>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ��ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ��С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ,���ֵ800</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool (__stdcall* TdxL2Hq_GetIndexBarsDelegate)(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);



/// <summary>
/// ��ȡ��ʱ����
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool (__stdcall* TdxL2Hq_GetMinuteTimeDataDelegate)(byte Market, char* Zqdm, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ��ʷ��ʱ����
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Date">����, ����2014��1��1��Ϊ����20140101</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetHistoryMinuteTimeDataDelegate)(byte Market, char* Zqdm, int Date, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ��ʱ�ɽ�ĳ����Χ�ڵ�����
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ��ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ��С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetTransactionDataDelegate) (byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ��ʷ��ʱ�ɽ�ĳ����Χ������
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ��ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ��СAPIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ</param>
/// <param name="Date">����, ����2014��1��1��Ϊ����20140101</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetHistoryTransactionDataDelegate) (byte Market, char* Zqdm, short Start, short& Count, int Date, char* Result, char* ErrInfo);

/// <summary>
/// ������ȡ���֤ȯ���嵵��������
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�, ��i��Ԫ�ر�ʾ��i��֤ȯ���г�����</param>
/// <param name="Zqdm">֤ȯ����, Count��֤ȯ������ɵ�����</param>
/// <param name="Count">APIִ��ǰ,��ʾ�û�Ҫ�����֤ȯ��Ŀ,���50(��ͬȯ�̿��ܲ�һ����������Ŀ��������ѯȯ�̻����), APIִ�к�,������ʵ�ʷ��ص���Ŀ</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityQuotesDelegate) (byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡF10���ϵķ���
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetCompanyInfoCategoryDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);




/// <summary>
/// ��ȡF10���ϵ�ĳһ���������
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="FileName">��Ŀ���ļ���, ��TdxL2Hq_GetCompanyInfoCategory������Ϣ�л�ȡ</param>
/// <param name="Start">��Ŀ�Ŀ�ʼλ��, ��TdxL2Hq_GetCompanyInfoCategory������Ϣ�л�ȡ</param>
/// <param name="Length">��Ŀ�ĳ���, ��TdxL2Hq_GetCompanyInfoCategory������Ϣ�л�ȡ</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����,����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetCompanyInfoContentDelegate) (byte Market, char* Zqdm, char* FileName, int Start, int Length, char* Result, char* ErrInfo);




/// <summary>
/// ��ȡ��Ȩ��Ϣ��Ϣ
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����,����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetXDXRInfoDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);



/// <summary>
/// ��ȡ������Ϣ
/// </summary>
/// <param name="Market">�г�����,   0->����     1->�Ϻ�</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����,����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall* TdxL2Hq_GetFinanceInfoDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);




int _tmain(int argc, _TCHAR* argv[])
{
    //����dll, dllҪ���Ƶ�debug��releaseĿ¼��,������ö��ֽ��ַ����������,�û����ʱ���Լ����Ƹ�������ʾ��С��λ���뾫��
    HMODULE TdxApiHMODULE = LoadLibrary("TdxHqApi.dll");

    //��ȡapi����
    TdxL2Hq_GetDetailTransactionDataDelegate TdxL2Hq_GetDetailTransactionData = (TdxL2Hq_GetDetailTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetDetailTransactionData");
    TdxL2Hq_GetDetailOrderDataDelegate TdxL2Hq_GetDetailOrderData = (TdxL2Hq_GetDetailOrderDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetDetailOrderData");
    TdxL2Hq_GetSecurityQuotes10Delegate TdxL2Hq_GetSecurityQuotes10 = (TdxL2Hq_GetSecurityQuotes10Delegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetSecurityQuotes10");
    TdxL2Hq_GetBuySellQueueDelegate TdxL2Hq_GetBuySellQueue = (TdxL2Hq_GetBuySellQueueDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetBuySellQueue");


    TdxL2Hq_ConnectDelegate TdxL2Hq_Connect = (TdxL2Hq_ConnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_Connect");
    TdxL2Hq_DisconnectDelegate TdxL2Hq_Disconnect = (TdxL2Hq_DisconnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_Disconnect");
    TdxL2Hq_GetSecurityBarsDelegate TdxL2Hq_GetSecurityBars = (TdxL2Hq_GetSecurityBarsDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetSecurityBars");
    TdxL2Hq_GetIndexBarsDelegate TdxL2Hq_GetIndexBars = (TdxL2Hq_GetIndexBarsDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetIndexBars");
    TdxL2Hq_GetMinuteTimeDataDelegate TdxL2Hq_GetMinuteTimeData = (TdxL2Hq_GetMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetMinuteTimeData");
    TdxL2Hq_GetHistoryMinuteTimeDataDelegate TdxL2Hq_GetHistoryMinuteTimeData = (TdxL2Hq_GetHistoryMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetHistoryMinuteTimeData");
    TdxL2Hq_GetTransactionDataDelegate TdxL2Hq_GetTransactionData = (TdxL2Hq_GetTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetTransactionData");
    TdxL2Hq_GetHistoryTransactionDataDelegate TdxL2Hq_GetHistoryTransactionData = (TdxL2Hq_GetHistoryTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetHistoryTransactionData");
    TdxL2Hq_GetSecurityQuotesDelegate TdxL2Hq_GetSecurityQuotes = (TdxL2Hq_GetSecurityQuotesDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetSecurityQuotes");
    TdxL2Hq_GetCompanyInfoCategoryDelegate TdxL2Hq_GetCompanyInfoCategory = (TdxL2Hq_GetCompanyInfoCategoryDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetCompanyInfoCategory");
    TdxL2Hq_GetCompanyInfoContentDelegate TdxL2Hq_GetCompanyInfoContent = (TdxL2Hq_GetCompanyInfoContentDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetCompanyInfoContent");
    TdxL2Hq_GetXDXRInfoDelegate TdxL2Hq_GetXDXRInfo = (TdxL2Hq_GetXDXRInfoDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetXDXRInfo");
    TdxL2Hq_GetFinanceInfoDelegate TdxL2Hq_GetFinanceInfo = (TdxL2Hq_GetFinanceInfoDelegate)GetProcAddress(TdxApiHMODULE, "TdxL2Hq_GetFinanceInfo");



    //��ʼ��ȡ��������
    char* Result = new char[1024 * 1024];
    char* ErrInfo = new char[256];
    short Count = 10;

    ///����ȯ��L2��������һ����Ϊ���µ�ַ:
    ///�߼�����_�Ϻ�����1:61.152.107.173:7707
    ///�߼�����_�Ϻ�����2:61.152.168.232:7715
    ///�߼�����_�Ϻ�����3:61.152.168.227:7709
    ///�߼�����_�Ϻ�����4:61.152.107.170:7707
    ///�߼�����_���ڵ���1:119.147.86.172:443
    ///�߼�����_���ڵ���2:113.105.73.81:7721
    ///�߼�����_��ݸ����1:113.105.142.138:7709
    ///�߼�����_��ݸ����2:113.105.142.139:7709
    ///�߼�����_�人����1:59.175.238.41:443
    ///�߼�����_�人����2:119.97.185.4:7709
    ///�߼�����_�人����3:59.175.238.38:7707
    ///�߼�����_������ͨ:123.129.245.202:80
    ///�߼�����_������ͨ:61.135.142.90:443
    ///�߼�����_�Ϻ���ͨ:210.51.55.212:80
    ///�߼�����_��ݸ��ͨ1:58.253.96.198:7709
    ///�߼�����_��ݸ��ͨ2:58.253.96.200:7709

    bool bool1 =  TdxL2Hq_Connect("61.152.107.173", 7707,Result, ErrInfo);
    if (!bool1)
    {
        cout << ErrInfo << endl;//��¼ʧ��
        return;
    }

    cout << Result << endl;//��¼�ɹ�


    bool1 = TdxL2Hq_GetBuySellQueue(1, "600030", Result, ErrInfo);
    cout << Result << endl;


    byte Market[] = {0,1};
    char* Zqdm[] = {"000001","600030"};
    short ZqdmCount = 2;
    bool1 = TdxL2Hq_GetSecurityQuotes10(Market, Zqdm, ZqdmCount, Result, ErrInfo);
    cout << Result << endl;


    bool1 = TdxL2Hq_GetDetailTransactionData(0, "000001", 0, Count, Result, ErrInfo);
    cout << Result << endl;

    bool1 = TdxL2Hq_GetDetailOrderData(0, "000001", 0, Count, Result, ErrInfo);
    cout << Result << endl;



    //��ȡ��ƱK������
    //bool1 = TdxL2Hq_GetSecurityBars(0, 0, "000001", 0, Count, Result, ErrInfo);//��������, 0->5����K��    1->15����K��    2->30����K��  3->1СʱK��    4->��K��  5->��K��  6->��K��  7->1����K��  8->1����K��  9->��K��  10->��K��  11->��K��
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;
    //	return 0;
    //}
    //cout << Result << endl;



    //��ȡָ��K������
    bool1 = TdxL2Hq_GetIndexBars(4, 1, "000001", 0, Count, Result, ErrInfo);//��������, 0->5����K��    1->15����K��    2->30����K��  3->1СʱK��    4->��K��  5->��K��  6->��K��  7->1����K��     8->1����K��    9->��K��  10->��K��  11->��K��
    if (!bool1)
    {
        cout << ErrInfo << endl;
        return 0;
    }
    cout << Result << endl;





    //��ȡ��ʱͼ����
    /*bool1 = TdxL2Hq_GetMinuteTimeData(0, "000001",  Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/


    //��ȡ��ʷ��ʱͼ����
    /*bool1 = TdxL2Hq_GetHistoryMinuteTimeData(0, "000001", 20140904, Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/


    //��ȡ�ֱ�ͼ����
    /*bool1 = TdxL2Hq_GetTransactionData(0, "000001", 0, Count, Result, ErrInfo);
    if (!bool1)
    {
    cout << ErrInfo << endl;
    return 0;
    }
    cout << Result << endl;*/



    //��ȡ��ʷ�ֱ�ͼ����
    /*bool1 = TdxL2Hq_GetHistoryTransactionData(0, "000001", 0, Count, 20140904,  Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/




    //��ȡ�嵵��������
    /*byte Market[] = {0,1};
    char* Zqdm[] = {"000001","600030"};
    short ZqdmCount = 2;
    bool1 = TdxL2Hq_GetSecurityQuotes(Market, Zqdm, ZqdmCount, Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl; */



    //��ȡF10���ݵ����
    /*bool1 = TdxL2Hq_GetCompanyInfoCategory(0, "000001", Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/



    //��ȡF10���ݵ�ĳ��������
    /*bool1 = TdxL2Hq_GetCompanyInfoContent(1, "600030", "600030.txt", 0, 16824, Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/


    //��ȡ��Ȩ��Ϣ��Ϣ
    /*bool1 = TdxL2Hq_GetXDXRInfo(0, "000001", Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/





    //��ȡ������Ϣ
    /*bool1 = TdxL2Hq_GetFinanceInfo(0, "000001", Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/






    TdxL2Hq_Disconnect();

    cout << "�Ѿ��Ͽ�������"<<endl;


    FreeLibrary(TdxApiHMODULE);

    int a;
    cin >> a;

    return 0;
}

