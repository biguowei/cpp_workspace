// TdxHqDemoCpp.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>
#include <winbase.h>
#include <iostream>

using namespace std;


//�����ĵ�
//

//1.��չ����API����TdxHqApi.dll�ļ��ĵ����������������º�����(�������麯����Ϊ�ͻ������������ѯ�����Ƿ���������)
//bool  TdxExHq_Connect(char* IP, int Port, char* Result, char* ErrInfo);//����ȯ�����������
//void  TdxExHq_Disconnect();//�Ͽ�������
//bool  TdxExHq_GetMarkets(char* Result, char* ErrInfo);//��ȡ�����г�����
//bool  TdxExHq_GetInstrumentCount(int& Result, char* ErrInfo);//��ȡ����Ʒ�ֵ���Ŀ
//bool  TdxExHq_GetInstrumentInfo(int Start, short Count, char* Result, char* ErrInfo);//��ȡ����Ʒ�ִ���
//bool  TdxExHq_GetInstrumentBars(byte Category, byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//��ȡָ��Ʒ�ֵ�K������
//bool  TdxExHq_GetMinuteTimeData(byte Market, char* Zqdm, char* Result, char* ErrInfo);//��ȡָ��Ʒ�ֵķ�ʱͼ����
//bool  TdxExHq_GetTransactionData(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//��ȡָ��Ʒ�ֵķ�ʱ�ɽ�����
//bool  TdxExHq_GetInstrumentQuote (byte Market, char* Zqdm, char* Result, char* ErrInfo);//��ȡָ��Ʒ�ֵ��̿ڱ���
//bool  TdxExHq_GetHistoryTransactionData(byte Market, char* Zqdm, int date, int Start, short& Count, char* Result, char* ErrInfo);//��ȡָ��Ʒ�ֵ���ʷ��ʱ�ɽ�����
//bool  TdxExHq_GetHistoryMinuteTimeData(byte Market, char* Zqdm, int date, char* Result, char* ErrInfo);//��ȡָ��Ʒ�ֵķ�ʱͼ����

///����ӿ�ִ�к����ʧ�ܣ����ַ���ErrInfo�����˳�����Ϣ����˵����
///����ɹ������ַ���Result�����˽������,��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���
///���ص�Result������ݶ���\n,\t�ָ��������ַ����������ѯK�����ݣ����صĽ���ַ���������

///��ʱ��\t���̼�\t���̼�\t��߼�\t��ͼ�\t�ɽ���\t�ɽ���\n
///20150519\t4.644000\t4.732000\t4.747000\t4.576000\t146667487\t683638848.000000\n
///20150520\t4.756000\t4.850000\t4.960000\t4.756000\t353161092\t1722953216.000000��

///��ô�����֮��ͨ���ָ��ַ����� ���Իָ�Ϊ���м��еı����ʽ������



//2.APIʹ������Ϊ: Ӧ�ó����ȵ���TdxExHq_Connect����ͨ�������������,Ȼ��ſ��Ե��������ӿڻ�ȡ��������,Ӧ�ó���Ӧ���д��������������, �ӿ����̰߳�ȫ��.
//������ߣ���������api������api�᷵���Ѿ����ߵĴ�����Ϣ��Ӧ�ó���Ӧ���ݴ˴�����Ϣ�������ӷ�������

//3.������������˵��
/// <summary>
///  ����ͨ������չ���������
/// </summary>
/// <param name="IP">������IP,����ȯ��ͨ���������¼���桰ͨѶ���á���ť�ڲ��</param>
/// <param name="Port">�������˿�</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall*  TdxExHq_ConnectDelegate)(char* IP, int Port, char* Result, char* ErrInfo);


/// <summary>
/// �Ͽ�ͬ������������
/// </summary>
typedef void(__stdcall*  TdxExHq_DisconnectDelegate)();


/// <summary>
///  ��ȡ��չ������֧�ֵĸ����г����г�����
/// </summary>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
typedef bool(__stdcall*  TdxExHq_GetMarketsDelegate)(char* Result, char* ErrInfo);




/// <summary>
///  ��ȡ�����ڻ���Լ������
/// </summary>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĺ�Լ������</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
typedef bool(__stdcall*  TdxExHq_GetInstrumentCountDelegate)(int& Result, char* ErrInfo);



/// <summary>
///  ��ȡָ����Χ���ڻ���Լ�Ĵ���
/// </summary>
// <param name="Start">��Լ��Χ�Ŀ�ʼλ��, ��TdxExHq_GetInstrumentCount������Ϣ��ȷ��</param>
/// <param name="Count">��Լ����Ŀ, ��TdxExHq_GetInstrumentCount������Ϣ�л�ȡ</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����,����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
typedef bool(__stdcall*  TdxExHq_GetInstrumentInfoDelegate)(int Start, short Count, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ��Լָ����Χ�ڵ�K������
/// </summary>
/// <param name="Category">K������, 0->5����K��    1->15����K��    2->30����K��  3->1СʱK��    4->��K��  5->��K��  6->��K��  7->1����  8->1����K��  9->��K��  10->��K��  11->��K��< / param>
/// <param name="Market">�г�����</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ�Ŀ�ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ�Ĵ�С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ, ���ֵ800</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall*  TdxExHq_GetInstrumentBarsDelegate)(byte Category, byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ��ʱ����
/// </summary>
/// <param name="Market">�г�����,</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall*  TdxExHq_GetMinuteTimeDataDelegate)(byte Market, char* Zqdm, char* Result, char* ErrInfo);



/// <summary>
/// ��ȡָ����Χ�ڵķ�ʱ�ɽ�����
/// </summary>
/// <param name="Market">�г�����</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ�Ŀ�ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ�Ĵ�С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall*  TdxExHq_GetTransactionDataDelegate)(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);



/// <summary>
/// ��ȡ��Լ���嵵��������
/// </summary>
/// <param name="Market">�г�����</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall*  TdxExHq_GetInstrumentQuoteDelegate) (byte Market, char* Zqdm,  char* Result, char* ErrInfo);


/// <summary>
/// ��ȡ��ʷ��ʱ�ɽ�ָ����Χ�ڵ�����
/// </summary>
/// <param name="Market">�г�����,</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Start">��Χ�Ŀ�ʼλ��,���һ��K��λ����0, ǰһ����1, ��������</param>
/// <param name="Count">��Χ�Ĵ�С��APIִ��ǰ,��ʾ�û�Ҫ�����K����Ŀ, APIִ�к�,������ʵ�ʷ��ص�K����Ŀ</param>
/// <param name="Date">����, ����2014��1��1��Ϊ����20140101</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall*  TdxExHq_GetHistoryTransactionDataDelegate)(byte Market, char* Zqdm, int date, int Start, short& Count, char* Result, char* ErrInfo);



/// <summary>
/// ��ȡ��ʷ��ʱ����
/// </summary>
/// <param name="Market">�г�����</param>
/// <param name="Zqdm">֤ȯ����</param>
/// <param name="Date">����, ����2014��1��1��Ϊ����20140101</param>
/// <param name="Result">��APIִ�з��غ�Result�ڱ����˷��صĲ�ѯ����, ��ʽΪ������ݣ�������֮��ͨ��\n�ַ��ָ������֮��ͨ��\t�ָ���һ��Ҫ����1024*1024�ֽڵĿռ䡣����ʱΪ���ַ�����</param>
/// <param name="ErrInfo">��APIִ�з��غ�������������˴�����Ϣ˵����һ��Ҫ����256�ֽڵĿռ䡣û����ʱΪ���ַ�����</param>
/// <returns>�ɹ�����true, ʧ�ܷ���false</returns>
typedef bool(__stdcall*  TdxExHq_GetHistoryMinuteTimeDataDelegate)(byte Market, char* Zqdm, int date, char* Result, char* ErrInfo);

int _tmain(int argc, _TCHAR* argv[])
{
    //����dll, dllҪ���Ƶ�debug��releaseĿ¼��,������ö��ֽ��ַ����������,�û����ʱ���Լ����Ƹ�������ʾ��С��λ���뾫��
    HMODULE TdxApiHMODULE = LoadLibrary("TdxHqApi.dll");

    //��ȡapi����
    TdxExHq_ConnectDelegate TdxExHq_Connect = (TdxExHq_ConnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_Connect");
    TdxExHq_DisconnectDelegate TdxExHq_Disconnect = (TdxExHq_DisconnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_Disconnect");
    TdxExHq_GetMarketsDelegate TdxExHq_GetMarkets = (TdxExHq_GetMarketsDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetMarkets");
    TdxExHq_GetInstrumentCountDelegate TdxExHq_GetInstrumentCount = (TdxExHq_GetInstrumentCountDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetInstrumentCount");
    TdxExHq_GetInstrumentInfoDelegate TdxExHq_GetInstrumentInfo = (TdxExHq_GetInstrumentInfoDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetInstrumentInfo");
    TdxExHq_GetInstrumentBarsDelegate TdxExHq_GetInstrumentBars = (TdxExHq_GetInstrumentBarsDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetInstrumentBars");
    TdxExHq_GetMinuteTimeDataDelegate TdxExHq_GetMinuteTimeData = (TdxExHq_GetMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetMinuteTimeData");
    TdxExHq_GetTransactionDataDelegate TdxExHq_GetTransactionData = (TdxExHq_GetTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetTransactionData");
    TdxExHq_GetInstrumentQuoteDelegate TdxExHq_GetInstrumentQuote = (TdxExHq_GetInstrumentQuoteDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetInstrumentQuote");
    TdxExHq_GetHistoryTransactionDataDelegate TdxExHq_GetHistoryTransactionData = (TdxExHq_GetHistoryTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetHistoryTransactionData");
    TdxExHq_GetHistoryMinuteTimeDataDelegate TdxExHq_GetHistoryMinuteTimeData = (TdxExHq_GetHistoryMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxExHq_GetHistoryMinuteTimeData");




    //��ʼ��ȡ��������
    char* Result = new char[1024 * 1024];
    char* ErrInfo = new char[256];
    short Count = 80;

    //���ӷ�����
    bool bool1=TdxExHq_Connect("111.111.111.111", 7727, Result, ErrInfo);
    cout << Result << endl;



    //bool1 = TdxExHq_GetMarkets(Result, ErrInfo);
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//����ʧ��
    //	return 0;
    //}
    //cout << Result << endl;




    //int InstrumentCount = 0;
    //bool1 = TdxExHq_GetInstrumentCount(InstrumentCount, ErrInfo);
    //cout << InstrumentCount << endl;




    //bool1 = TdxExHq_GetInstrumentInfo(0, 511, Result, ErrInfo);//Count���511
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//����ʧ��
    //	return 0;
    //}
    //cout << Result << endl;



    //Count = 50;
    //bool1 = TdxExHq_GetInstrumentBars(0, 30, "AGL3", 0, Count, Result, ErrInfo);// 0->5����K��    1->15����K��    2->30����K��  3->1СʱK��    4->��K��  5->��K��  6->��K��  7->1����  8->1����K��  9->��K��  10->��K��  11->��K��
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//����ʧ��
    //	return 0;
    //}
    //cout << Result << endl;

    //bool1 = TdxExHq_GetMinuteTimeData(47, "IF1409", Result, ErrInfo);
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//����ʧ��
    //	return 0;
    //}
    //cout << Result << endl;



    //bool1 = TdxExHq_GetTransactionData(47, "IF1409", 0, Count, Result, ErrInfo);
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//����ʧ��
    //	return 0;
    //}
    //cout << Result << endl;


    //��ȡ�嵵��������
    /*bool1 = TdxExHq_GetInstrumentQuote(47, "IF1409",  Result, ErrInfo);
    if (!bool1)
    {
    cout << ErrInfo << endl;
    return 0;
    }
    cout << Result << endl; */



    //bool1 = TdxExHq_GetHistoryTransactionData(47, "IF1409", 20140919, 0, Count, Result, ErrInfo);
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//����ʧ��
    //	return 0;
    //}
    //cout << Result << endl;




    bool1 = TdxExHq_GetHistoryMinuteTimeData(47, "IF1410", 20140827, Result, ErrInfo);
    if (!bool1)
    {
        cout << ErrInfo << endl;//����ʧ��
        return 0;
    }
    cout << Result << endl;





    TdxExHq_Disconnect();





    cout << "�Ѿ��Ͽ�������"<<endl;


    FreeLibrary(TdxApiHMODULE);

    int a;
    cin >> a;

    return 0;
}

