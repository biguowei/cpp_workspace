// HqSpeed.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "hqApiCpp.h"

#include "LogTrace.h"

#include <windows.h>
#include <winbase.h>

#include <iostream>
#include <fstream>

#include <string>
#include <vector>

CLogTrace m_LogTrace;

//
//
//

class CStockItem
{
public:
    CStockItem()
        : m_nMarket(0),
          m_sStockCode("")
    {}

    CStockItem(byte nMarket, const std::string &sStockCode)
        : m_nMarket(nMarket),
          m_sStockCode(sStockCode)
    {}

    byte GetMarket()
    {
        return m_nMarket;
    }

    const char *GetStockCode() const
    {
        return m_sStockCode.c_str();
    }

public:
    byte m_nMarket;
    std::string m_sStockCode;
};

std::vector<CStockItem> vecStockList;

struct stHost
{
    const char *pszName;
    const char *pszHost;
    int nPort;
}
HostArray[] =
{
	{ "����֤ȯ��������", "119.147.212.81", 7709 },
	{ "��̩֤ȯ(�Ͼ�����)", "221.231.141.60", 7709 },
	{ "��̩֤ȯ(�Ϻ�����)", "101.227.73.20", 7709 },
	{ "��̩֤ȯ(�Ϻ����Ŷ�)", "101.227.77.254", 7709 },
	{ "��̩֤ȯ(���ڵ���)", "14.215.128.18", 7709 },
	{ "��̩֤ȯ(�人����)", "59.173.18.140", 7709 },
	{ "��̩֤ȯ(�����ͨ)", "60.28.23.80", 7709 },
	{ "��̩֤ȯ(������ͨ)", "218.60.29.136", 7709 },
	{ "��̩֤ȯ(�Ͼ���ͨ)", "122.192.35.44", 7709 },
	{ "��̩֤ȯ(�Ͼ���ͨ)", "122.192.35.44", 7709 },
};

//
//
//

void SplitStrToVector(std::vector<std::string> &sVec, const std::string &sStr, const std::string &sSep)
{
    std::string::size_type i = 0;
    std::string::size_type j = sStr.find(sSep);

    while (j != std::string::npos)
    {
        sVec.push_back(sStr.substr(i, j-i));

        i = j + sSep.length();
        j = sStr.find(sSep, j + sSep.length());
    }

    if (j == std::string::npos)
        sVec.push_back(sStr.substr(i, sStr.length()));
}

//
//
//

int _tmain(int argc, _TCHAR* argv[])
{
    //
    //
    //

    m_LogTrace.m_strAppName = "L2Test";
    m_LogTrace.SetFileName("Log.txt");
    m_LogTrace.OnStartup(TRUE, TRUE);
    m_LogTrace.WriteLine("Started the application");


    //
    // load stock list
    //

    //CStringArray sArray;
    CString m_strFolderPath;
    string sLine;

    ::GetModuleFileName(GetModuleHandle(NULL), m_strFolderPath.GetBuffer(MAX_PATH), MAX_PATH);

    m_strFolderPath.ReleaseBuffer();
    m_strFolderPath = m_strFolderPath.Left(m_strFolderPath.ReverseFind('\\'));

    ifstream fp(m_strFolderPath + "\\��Ʊ�б�.txt");
    while (!fp.eof())
    {
        getline(fp, sLine);

        if (sLine.empty())
            break;

        std::vector<std::string> vecFields;

        SplitStrToVector(vecFields, sLine, ",");

        if (vecFields[0] == "1")
        {
            vecStockList.push_back(CStockItem(1, vecFields[1]));
        }
        else if (vecFields[0] == "2")
        {
            vecStockList.push_back(CStockItem(0, vecFields[1]));
        }
    }

    //
    // ѡ��ӿ�
    //

    int nAPI = -1;

    while (1)
    {
        cout << "\n�ӿ��б�:" << endl;

        cout << "\t0 -> TdxHqApi.dll\n";
        cout << "\t1 -> TradeX.dll\n";

        cout << "\n\t����ӿ����: ";

        cin >> nAPI;

        if (nAPI>=0 && nAPI<=1)
            break;

        cout << "****** ����! ������ѡ��ӿ�\n" << endl;
    }

    cout << endl;

    //
    // ѡ�������
    //

    int nHostNum = -1;

    while (1)
    {
        cout << "�������б�:" << endl;

        for (int i=0; i<sizeof(HostArray)/sizeof(HostArray[0]); i++)
        {
            printf("%3d : %+30s\t%s:%d\n", i + 1, HostArray[i].pszName, HostArray[i].pszHost, HostArray[i].nPort);
        }

        cout << endl;

        cout << "��������������:";
        cin >> nHostNum;

        if (nHostNum>0 && nHostNum<=sizeof(HostArray)/sizeof(HostArray[0]))
            break;

        cout << "����! ������ѡ�������\n" << endl;
    }

    nHostNum--;

    cout << "\t���������: " << HostArray[nHostNum].pszName;
    cout << endl;

    //
    // ѡ��ÿ�������Ʊ��
    //

    int nBatchNo = -1;
    int nBatch;  // max-batch = 80
    int nLoop; // = 40

    while (1)
    {
        cout << "\nѡ��ÿ�������Ʊ��:" << endl;

        printf("\t0 -> 10\n");
        printf("\t1 -> 25\n");
        printf("\t2 -> 30\n");
        printf("\t3 -> 50\n");
        printf("\t4 -> 75\n");

        cout << "\n\t�������: ";
        cin >> nBatchNo;

        if (nBatchNo>=0 && nBatchNo<=4)
            break;

        cout << "****** ����! ������ѡ��\n" << endl;
    }

    cout << endl;

    switch (nBatchNo)
    {
    case 0:
        nBatch = 10;
        nLoop = 300;
        break;

    case 1:
        nBatch = 25;
        nLoop = 120;
        break;

    case 2:
        nBatch = 30;
        nLoop = 100;
        break;

    case 3:
        nBatch = 50;
        nLoop = 60;
        break;

    case 4:
    default:
        nBatch = 75;
        nLoop = 40;
        break;
    }

    //
    // ���ؽӿ�
    //

    HMODULE TdxApiHMODULE;

    if (nAPI == 0)
    {
        cout << "\t�󶨽ӿ�: TdxHqApi.dll ..." << endl;
        TdxApiHMODULE = LoadLibrary("TdxHqApi.dll");
    }
    else if (nAPI == 1)
    {
        cout << "\t�󶨽ӿ�: TradeX.dll ..." << endl;
        TdxApiHMODULE = LoadLibrary("TradeX.dll");
    }

    //
    // ��ȡapi����
    //
    TdxHq_ConnectDelegate TdxHq_Connect = (TdxHq_ConnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Connect");
    TdxHq_DisconnectDelegate TdxHq_Disconnect = (TdxHq_DisconnectDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_Disconnect");
    TdxHq_GetSecurityBarsDelegate TdxHq_GetSecurityBars = (TdxHq_GetSecurityBarsDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetSecurityBars");
    TdxHq_GetIndexBarsDelegate TdxHq_GetIndexBars = (TdxHq_GetIndexBarsDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetIndexBars");
    TdxHq_GetMinuteTimeDataDelegate TdxHq_GetMinuteTimeData = (TdxHq_GetMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetMinuteTimeData");
    TdxHq_GetHistoryMinuteTimeDataDelegate TdxHq_GetHistoryMinuteTimeData = (TdxHq_GetHistoryMinuteTimeDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetHistoryMinuteTimeData");
    TdxHq_GetTransactionDataDelegate TdxHq_GetTransactionData = (TdxHq_GetTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetTransactionData");
    TdxHq_GetHistoryTransactionDataDelegate TdxHq_GetHistoryTransactionData = (TdxHq_GetHistoryTransactionDataDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetHistoryTransactionData");
    TdxHq_GetSecurityQuotesDelegate TdxHq_GetSecurityQuotes = (TdxHq_GetSecurityQuotesDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetSecurityQuotes");
    TdxHq_GetCompanyInfoCategoryDelegate TdxHq_GetCompanyInfoCategory = (TdxHq_GetCompanyInfoCategoryDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetCompanyInfoCategory");
    TdxHq_GetCompanyInfoContentDelegate TdxHq_GetCompanyInfoContent = (TdxHq_GetCompanyInfoContentDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetCompanyInfoContent");
    TdxHq_GetXDXRInfoDelegate TdxHq_GetXDXRInfo = (TdxHq_GetXDXRInfoDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetXDXRInfo");
    TdxHq_GetFinanceInfoDelegate TdxHq_GetFinanceInfo = (TdxHq_GetFinanceInfoDelegate)GetProcAddress(TdxApiHMODULE, "TdxHq_GetFinanceInfo");

    //
    // ��ʼ��ȡ��������
    //
    char* Result = new char[1024 * 1024];
    char* ErrInfo = new char[256];

    //short Count = 10;

    //
    // ���ӷ�����
    //
    std::string sHost = HostArray[nHostNum].pszHost;
    short nPort = HostArray[nHostNum].nPort;

    cout << "\n\t���ӷ����� " << sHost << ":" << nPort << " ...\n\n";

    bool bool1=TdxHq_Connect(sHost.c_str(), nPort, Result, ErrInfo);
    if (!bool1)
    {
        cout << ErrInfo << endl;
        getchar();
        return 0;
    }

    cout << Result << endl;
    cout << endl;

    //
    // ����
    //

    for (int k=0; k<10; k++)
    {
        m_LogTrace.StartCalculateTime();

        int nTotal = 0;

        byte nMarket[100];
        const char *vecZqdm[100];
        short nCount;

        for (int i =0; i<nLoop; i++)
        {
            // ������ȡ�嵵��������

            for (int j=0; j<nBatch; j++)
            {
                nMarket[j] = vecStockList[nTotal].GetMarket();
                vecZqdm[j] = vecStockList[nTotal].GetStockCode();

                nTotal++;

                //printf("%s\n", vecZqdm[j]);
            }

            nCount = nBatch;

            bool1 = TdxHq_GetSecurityQuotes(nMarket, vecZqdm, nCount, Result, ErrInfo);
            if (!bool1)
            {
                cout << ErrInfo << endl;
                return 0;
            }

            //cout << Result << endl;
            //cout << nCount << endl;
        }

        m_LogTrace.EndCalculateTime();

        printf("һ������%dֻ��Ʊ, ����TdxHq_GetSecurityQuotes ��ʱ%f ms\r\n",
               nBatch, m_LogTrace.dfTim/nLoop);
        printf("ÿ������%dֻ��Ʊ,����3000ֻ��Ʊ,һ����ʱ%f ms\r\n",
               nBatch, m_LogTrace.dfTim);
    }

    //
    //
    //

    TdxHq_Disconnect();

    cout << "\n\t�Ѿ��Ͽ�������, ��������˳� ..."<< endl;

    FreeLibrary(TdxApiHMODULE);

    fflush(stdin);

    getchar();

    return 0;
}

