// HqSpeed.cpp : 定义控制台应用程序的入口点。
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
	{ "招商证券深圳行情", "119.147.212.81", 7709 },
	{ "华泰证券(南京电信)", "221.231.141.60", 7709 },
	{ "华泰证券(上海电信)", "101.227.73.20", 7709 },
	{ "华泰证券(上海电信二)", "101.227.77.254", 7709 },
	{ "华泰证券(深圳电信)", "14.215.128.18", 7709 },
	{ "华泰证券(武汉电信)", "59.173.18.140", 7709 },
	{ "华泰证券(天津联通)", "60.28.23.80", 7709 },
	{ "华泰证券(沈阳联通)", "218.60.29.136", 7709 },
	{ "华泰证券(南京联通)", "122.192.35.44", 7709 },
	{ "华泰证券(南京联通)", "122.192.35.44", 7709 },
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

    ifstream fp(m_strFolderPath + "\\股票列表.txt");
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
    // 选择接口
    //

    int nAPI = -1;

    while (1)
    {
        cout << "\n接口列表:" << endl;

        cout << "\t0 -> TdxHqApi.dll\n";
        cout << "\t1 -> TradeX.dll\n";

        cout << "\n\t请输接口序号: ";

        cin >> nAPI;

        if (nAPI>=0 && nAPI<=1)
            break;

        cout << "****** 错误! 请重新选择接口\n" << endl;
    }

    cout << endl;

    //
    // 选择服务器
    //

    int nHostNum = -1;

    while (1)
    {
        cout << "服务器列表:" << endl;

        for (int i=0; i<sizeof(HostArray)/sizeof(HostArray[0]); i++)
        {
            printf("%3d : %+30s\t%s:%d\n", i + 1, HostArray[i].pszName, HostArray[i].pszHost, HostArray[i].nPort);
        }

        cout << endl;

        cout << "请输入服务器序号:";
        cin >> nHostNum;

        if (nHostNum>0 && nHostNum<=sizeof(HostArray)/sizeof(HostArray[0]))
            break;

        cout << "错误! 请重新选择服务器\n" << endl;
    }

    nHostNum--;

    cout << "\t行情服务器: " << HostArray[nHostNum].pszName;
    cout << endl;

    //
    // 选择每次请求股票数
    //

    int nBatchNo = -1;
    int nBatch;  // max-batch = 80
    int nLoop; // = 40

    while (1)
    {
        cout << "\n选择每次请求股票数:" << endl;

        printf("\t0 -> 10\n");
        printf("\t1 -> 25\n");
        printf("\t2 -> 30\n");
        printf("\t3 -> 50\n");
        printf("\t4 -> 75\n");

        cout << "\n\t请输序号: ";
        cin >> nBatchNo;

        if (nBatchNo>=0 && nBatchNo<=4)
            break;

        cout << "****** 错误! 请重新选择\n" << endl;
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
    // 加载接口
    //

    HMODULE TdxApiHMODULE;

    if (nAPI == 0)
    {
        cout << "\t绑定接口: TdxHqApi.dll ..." << endl;
        TdxApiHMODULE = LoadLibrary("TdxHqApi.dll");
    }
    else if (nAPI == 1)
    {
        cout << "\t绑定接口: TradeX.dll ..." << endl;
        TdxApiHMODULE = LoadLibrary("TradeX.dll");
    }

    //
    // 获取api函数
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
    // 开始获取行情数据
    //
    char* Result = new char[1024 * 1024];
    char* ErrInfo = new char[256];

    //short Count = 10;

    //
    // 连接服务器
    //
    std::string sHost = HostArray[nHostNum].pszHost;
    short nPort = HostArray[nHostNum].nPort;

    cout << "\n\t连接服务器 " << sHost << ":" << nPort << " ...\n\n";

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
    // 测试
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
            // 批量获取五档报价数据

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

        printf("一次请求%d只股票, 函数TdxHq_GetSecurityQuotes 耗时%f ms\r\n",
               nBatch, m_LogTrace.dfTim/nLoop);
        printf("每次请求%d只股票,遍历3000只股票,一共耗时%f ms\r\n",
               nBatch, m_LogTrace.dfTim);
    }

    //
    //
    //

    TdxHq_Disconnect();

    cout << "\n\t已经断开服务器, 按任意键退出 ..."<< endl;

    FreeLibrary(TdxApiHMODULE);

    fflush(stdin);

    getchar();

    return 0;
}

