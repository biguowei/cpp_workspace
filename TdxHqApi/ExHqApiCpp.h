// TdxHqDemoCpp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <winbase.h>
#include <iostream>

using namespace std;


//开发文档
//

//1.扩展行情API均是TdxHqApi.dll文件的导出函数，包括以下函数：(所有行情函数均为客户端主动请求查询，不是服务器推送)
//bool  TdxExHq_Connect(char* IP, int Port, char* Result, char* ErrInfo);//连接券商行情服务器
//void  TdxExHq_Disconnect();//断开服务器
//bool  TdxExHq_GetMarkets(char* Result, char* ErrInfo);//获取所有市场代码
//bool  TdxExHq_GetInstrumentCount(int& Result, char* ErrInfo);//获取所有品种的数目
//bool  TdxExHq_GetInstrumentInfo(int Start, short Count, char* Result, char* ErrInfo);//获取所有品种代码
//bool  TdxExHq_GetInstrumentBars(byte Category, byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//获取指定品种的K线数据
//bool  TdxExHq_GetMinuteTimeData(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取指定品种的分时图数据
//bool  TdxExHq_GetTransactionData(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//获取指定品种的分时成交数据
//bool  TdxExHq_GetInstrumentQuote (byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取指定品种的盘口报价
//bool  TdxExHq_GetHistoryTransactionData(byte Market, char* Zqdm, int date, int Start, short& Count, char* Result, char* ErrInfo);//获取指定品种的历史分时成交数据
//bool  TdxExHq_GetHistoryMinuteTimeData(byte Market, char* Zqdm, int date, char* Result, char* ErrInfo);//获取指定品种的分时图数据

///行情接口执行后，如果失败，则字符串ErrInfo保存了出错信息中文说明；
///如果成功，则字符串Result保存了结果数据,形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。
///返回的Result结果数据都是\n,\t分隔的中文字符串，比如查询K线数据，返回的结果字符串就形如

///“时间\t开盘价\t收盘价\t最高价\t最低价\t成交量\t成交额\n
///20150519\t4.644000\t4.732000\t4.747000\t4.576000\t146667487\t683638848.000000\n
///20150520\t4.756000\t4.850000\t4.960000\t4.756000\t353161092\t1722953216.000000”

///查得此数据之后，通过分割字符串， 可以恢复为几行几列的表格形式的数据



//2.API使用流程为: 应用程序先调用TdxExHq_Connect连接通达信行情服务器,然后才可以调用其他接口获取行情数据,应用程序应自行处理网络断线问题, 接口是线程安全的.
//如果断线，调用任意api函数后，api会返回已经断线的错误信息，应用程序应根据此错误信息重新连接服务器。

//3.各个函数功能说明
/// <summary>
///  连接通达信扩展行情服务器
/// </summary>
/// <param name="IP">服务器IP,可在券商通达信软件登录界面“通讯设置”按钮内查得</param>
/// <param name="Port">服务器端口</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall*  TdxExHq_ConnectDelegate)(char* IP, int Port, char* Result, char* ErrInfo);


/// <summary>
/// 断开同服务器的连接
/// </summary>
typedef void(__stdcall*  TdxExHq_DisconnectDelegate)();


/// <summary>
///  获取扩展行情中支持的各个市场的市场代码
/// </summary>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
typedef bool(__stdcall*  TdxExHq_GetMarketsDelegate)(char* Result, char* ErrInfo);




/// <summary>
///  获取所有期货合约的总数
/// </summary>
/// <param name="Result">此API执行返回后，Result内保存了返回的合约总数。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
typedef bool(__stdcall*  TdxExHq_GetInstrumentCountDelegate)(int& Result, char* ErrInfo);



/// <summary>
///  获取指定范围的期货合约的代码
/// </summary>
// <param name="Start">合约范围的开始位置, 由TdxExHq_GetInstrumentCount返回信息中确定</param>
/// <param name="Count">合约的数目, 由TdxExHq_GetInstrumentCount返回信息中获取</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据,出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
typedef bool(__stdcall*  TdxExHq_GetInstrumentInfoDelegate)(int Start, short Count, char* Result, char* ErrInfo);


/// <summary>
/// 获取合约指定范围内的K线数据
/// </summary>
/// <param name="Category">K线种类, 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟  8->1分钟K线  9->日K线  10->季K线  11->年K线< / param>
/// <param name="Market">市场代码</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围的开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围的大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目, 最大值800</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall*  TdxExHq_GetInstrumentBarsDelegate)(byte Category, byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// 获取分时数据
/// </summary>
/// <param name="Market">市场代码,</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall*  TdxExHq_GetMinuteTimeDataDelegate)(byte Market, char* Zqdm, char* Result, char* ErrInfo);



/// <summary>
/// 获取指定范围内的分时成交数据
/// </summary>
/// <param name="Market">市场代码</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围的开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围的大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall*  TdxExHq_GetTransactionDataDelegate)(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);



/// <summary>
/// 获取合约的五档报价数据
/// </summary>
/// <param name="Market">市场代码</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall*  TdxExHq_GetInstrumentQuoteDelegate) (byte Market, char* Zqdm,  char* Result, char* ErrInfo);


/// <summary>
/// 获取历史分时成交指定范围内的数据
/// </summary>
/// <param name="Market">市场代码,</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围的开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围的大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目</param>
/// <param name="Date">日期, 比如2014年1月1日为整数20140101</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall*  TdxExHq_GetHistoryTransactionDataDelegate)(byte Market, char* Zqdm, int date, int Start, short& Count, char* Result, char* ErrInfo);



/// <summary>
/// 获取历史分时数据
/// </summary>
/// <param name="Market">市场代码</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Date">日期, 比如2014年1月1日为整数20140101</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall*  TdxExHq_GetHistoryMinuteTimeDataDelegate)(byte Market, char* Zqdm, int date, char* Result, char* ErrInfo);

int _tmain(int argc, _TCHAR* argv[])
{
    //载入dll, dll要复制到debug和release目录下,必须采用多字节字符集编程设置,用户编程时需自己控制浮点数显示的小数位数与精度
    HMODULE TdxApiHMODULE = LoadLibrary("TdxHqApi.dll");

    //获取api函数
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




    //开始获取行情数据
    char* Result = new char[1024 * 1024];
    char* ErrInfo = new char[256];
    short Count = 80;

    //连接服务器
    bool bool1=TdxExHq_Connect("111.111.111.111", 7727, Result, ErrInfo);
    cout << Result << endl;



    //bool1 = TdxExHq_GetMarkets(Result, ErrInfo);
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//连接失败
    //	return 0;
    //}
    //cout << Result << endl;




    //int InstrumentCount = 0;
    //bool1 = TdxExHq_GetInstrumentCount(InstrumentCount, ErrInfo);
    //cout << InstrumentCount << endl;




    //bool1 = TdxExHq_GetInstrumentInfo(0, 511, Result, ErrInfo);//Count最大511
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//连接失败
    //	return 0;
    //}
    //cout << Result << endl;



    //Count = 50;
    //bool1 = TdxExHq_GetInstrumentBars(0, 30, "AGL3", 0, Count, Result, ErrInfo);// 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟  8->1分钟K线  9->日K线  10->季K线  11->年K线
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//连接失败
    //	return 0;
    //}
    //cout << Result << endl;

    //bool1 = TdxExHq_GetMinuteTimeData(47, "IF1409", Result, ErrInfo);
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//连接失败
    //	return 0;
    //}
    //cout << Result << endl;



    //bool1 = TdxExHq_GetTransactionData(47, "IF1409", 0, Count, Result, ErrInfo);
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;//连接失败
    //	return 0;
    //}
    //cout << Result << endl;


    //获取五档报价数据
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
    //	cout << ErrInfo << endl;//连接失败
    //	return 0;
    //}
    //cout << Result << endl;




    bool1 = TdxExHq_GetHistoryMinuteTimeData(47, "IF1410", 20140827, Result, ErrInfo);
    if (!bool1)
    {
        cout << ErrInfo << endl;//连接失败
        return 0;
    }
    cout << Result << endl;





    TdxExHq_Disconnect();





    cout << "已经断开服务器"<<endl;


    FreeLibrary(TdxApiHMODULE);

    int a;
    cin >> a;

    return 0;
}

