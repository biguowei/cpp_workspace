// TdxHqDemoCpp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <winbase.h>
#include <iostream>
#include <vector>
using namespace std;


//开发文档
//

//1.行情API均是TdxHqApi.dll文件的导出函数，包括以下函数：(所有行情函数均为客户端主动请求查询，不是服务器推送)

//bool TdxL2Hq_GetDetailTransactionData(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//获取逐笔成交
//bool TdxL2Hq_GetDetailOrderData(byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);//获取逐笔委托
//bool TdxL2Hq_GetSecurityQuotes10 (byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);//获取十挡报价
//bool TdxL2Hq_GetBuySellQueue(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取买卖队列


//bool  TdxL2Hq_Connect(char* IP, int Port, char* Result, char* ErrInfo);//连接券商行情服务器
//void  TdxL2Hq_Disconnect();//断开服务器
//bool  TdxL2Hq_GetSecurityCount(byte Market, short& Result, char* ErrInfo);//获取市场内所有证券的数目
//bool  TdxL2Hq_GetSecurityList(byte Market, short Start, short& Count, char* Result, char* ErrInfo);//获取指定范围内所有的证券代码
//bool  TdxL2Hq_GetSecurityBars(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//获取股票K线
//bool  TdxL2Hq_GetIndexBars(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//获取指数K线
//bool  TdxL2Hq_GetMinuteTimeData(byte Market,  char* Zqdm, char* Result, char* ErrInfo);//获取分时图数据
//bool  TdxL2Hq_GetHistoryMinuteTimeData(byte Market, char* Zqdm, int date, char* Result, char* ErrInfo);//获取历史分时图数据
//bool  TdxL2Hq_GetTransactionData(byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//获取分时成交
//bool  TdxL2Hq_GetHistoryTransactionData(byte Market, char* Zqdm, short Start, short& Count, int date, char* Result, char* ErrInfo);//获取历史分时成交
//bool  TdxL2Hq_GetSecurityQuotes(byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);//获取盘口五档报价
//bool  TdxL2Hq_GetCompanyInfoCategory(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取F10信息类别
//bool  TdxL2Hq_GetCompanyInfoContent(byte Market, char* Zqdm, char* FileName, int Start, int Length, char* Result, char* ErrInfo);//获取F10信息内容
//bool  TdxL2Hq_GetXDXRInfo(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取权息数据
//bool  TdxL2Hq_GetFinanceInfo(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取财务信息



///行情接口执行后，如果失败，则字符串ErrInfo保存了出错信息中文说明；
///如果成功，则字符串Result保存了结果数据,形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。
///返回的Result结果数据都是\n,\t分隔的中文字符串，比如查询K线数据，返回的结果字符串就形如

///“时间\t开盘价\t收盘价\t最高价\t最低价\t成交量\t成交额\n
///20150519\t4.644000\t4.732000\t4.747000\t4.576000\t146667487\t683638848.000000\n
///20150520\t4.756000\t4.850000\t4.960000\t4.756000\t353161092\t1722953216.000000”

///查得此数据之后，通过分割字符串， 可以恢复为几行几列的表格形式的数据


//2.API使用流程为: 应用程序先调用TdxL2Hq_Connect连接通达信行情服务器,然后才可以调用其他接口获取行情数据,应用程序应自行处理网络断线问题, 接口是线程安全的
//如果断线，调用任意api函数后，api会返回已经断线的错误信息，应用程序应根据此错误信息重新连接服务器。


//3.各个函数功能说明

/// <summary>
/// 获取逐笔成交某个范围内的数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目,最大2000</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetDetailTransactionDataDelegate) (byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);




/// <summary>
/// 获取深圳逐笔委托某个范围内的数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目,最大2000</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetDetailOrderDataDelegate) (byte Market, char* Zqdm, int Start, short& Count, char* Result, char* ErrInfo);

/// <summary>
/// 批量获取多个证券的十档报价数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海, 第i个元素表示第i个证券的市场代码</param>
/// <param name="Zqdm">证券代码, Count个证券代码组成的数组</param>
/// <param name="Count">API执行前,表示用户要请求的证券数目,最大50, API执行后,保存了实际返回的数目</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityQuotes10Delegate) (byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);


/// <summary>
/// 获取买卖队列数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetBuySellQueueDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);




/// <summary>
///  连接通达信行情服务器
/// </summary>
/// <param name="IP">服务器IP,可在演示版内查得</param>
/// <param name="Port">服务器端口</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool (__stdcall*  TdxL2Hq_ConnectDelegate)(char* IP, int Port, char* Result, char* ErrInfo);


/// <summary>
/// 断开同服务器的连接
/// </summary>
typedef void(__stdcall* TdxL2Hq_DisconnectDelegate)();

/// <summary>
/// 获取市场内所有证券的数量
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的证券数量</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityCountDelegate)(byte Market, short& Result, char* ErrInfo);


/// <summary>
/// 获取市场内某个范围内的1000支股票的股票代码
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Start">范围开始位置,第一个股票是0, 第二个是1, 依此类推,位置信息依据TdxL2Hq_GetSecurityCount返回的证券总数确定</param>
/// <param name="Count">范围大小，API执行后,保存了实际返回的股票数目,</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的证券代码信息,形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityListDelegate)(byte Market, short Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// 获取证券某个范围内的K线数据
/// </summary>
/// <param name="Category">K线种类, 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟  8->1分钟K线  9->日K线  10->季K线  11->年K线< / param>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目, 最大值800</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityBarsDelegate)(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// 获取指数某个范围内的K线数据
/// </summary>
/// <param name="Category">K线种类, 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟  8->1分钟K线  9->日K线  10->季K线  11->年K线< / param>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目,最大值800</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool (__stdcall* TdxL2Hq_GetIndexBarsDelegate)(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);



/// <summary>
/// 获取分时数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool (__stdcall* TdxL2Hq_GetMinuteTimeDataDelegate)(byte Market, char* Zqdm, char* Result, char* ErrInfo);


/// <summary>
/// 获取历史分时数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Date">日期, 比如2014年1月1日为整数20140101</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetHistoryMinuteTimeDataDelegate)(byte Market, char* Zqdm, int Date, char* Result, char* ErrInfo);


/// <summary>
/// 获取分时成交某个范围内的数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetTransactionDataDelegate) (byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);


/// <summary>
/// 获取历史分时成交某个范围内数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围大小API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目</param>
/// <param name="Date">日期, 比如2014年1月1日为整数20140101</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetHistoryTransactionDataDelegate) (byte Market, char* Zqdm, short Start, short& Count, int Date, char* Result, char* ErrInfo);

/// <summary>
/// 批量获取多个证券的五档报价数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海, 第i个元素表示第i个证券的市场代码</param>
/// <param name="Zqdm">证券代码, Count个证券代码组成的数组</param>
/// <param name="Count">API执行前,表示用户要请求的证券数目,最大50(不同券商可能不一样，具体数目请自行咨询券商或测试), API执行后,保存了实际返回的数目</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetSecurityQuotesDelegate) (byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);


/// <summary>
/// 获取F10资料的分类
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetCompanyInfoCategoryDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);




/// <summary>
/// 获取F10资料的某一分类的内容
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="FileName">类目的文件名, 由TdxL2Hq_GetCompanyInfoCategory返回信息中获取</param>
/// <param name="Start">类目的开始位置, 由TdxL2Hq_GetCompanyInfoCategory返回信息中获取</param>
/// <param name="Length">类目的长度, 由TdxL2Hq_GetCompanyInfoCategory返回信息中获取</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据,出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetCompanyInfoContentDelegate) (byte Market, char* Zqdm, char* FileName, int Start, int Length, char* Result, char* ErrInfo);




/// <summary>
/// 获取除权除息信息
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据,出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetXDXRInfoDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);



/// <summary>
/// 获取财务信息
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据,出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxL2Hq_GetFinanceInfoDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);




int _tmain(int argc, _TCHAR* argv[])
{
    //载入dll, dll要复制到debug和release目录下,必须采用多字节字符集编程设置,用户编程时需自己控制浮点数显示的小数位数与精度
    HMODULE TdxApiHMODULE = LoadLibrary("TdxHqApi.dll");

    //获取api函数
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



    //开始获取行情数据
    char* Result = new char[1024 * 1024];
    char* ErrInfo = new char[256];
    short Count = 10;

    ///所有券商L2服务器都一样，为以下地址:
    ///高级行情_上海电信1:61.152.107.173:7707
    ///高级行情_上海电信2:61.152.168.232:7715
    ///高级行情_上海电信3:61.152.168.227:7709
    ///高级行情_上海电信4:61.152.107.170:7707
    ///高级行情_深圳电信1:119.147.86.172:443
    ///高级行情_深圳电信2:113.105.73.81:7721
    ///高级行情_东莞电信1:113.105.142.138:7709
    ///高级行情_东莞电信2:113.105.142.139:7709
    ///高级行情_武汉电信1:59.175.238.41:443
    ///高级行情_武汉电信2:119.97.185.4:7709
    ///高级行情_武汉电信3:59.175.238.38:7707
    ///高级行情_济南联通:123.129.245.202:80
    ///高级行情_北京联通:61.135.142.90:443
    ///高级行情_上海联通:210.51.55.212:80
    ///高级行情_东莞联通1:58.253.96.198:7709
    ///高级行情_东莞联通2:58.253.96.200:7709

    bool bool1 =  TdxL2Hq_Connect("61.152.107.173", 7707,Result, ErrInfo);
    if (!bool1)
    {
        cout << ErrInfo << endl;//登录失败
        return;
    }

    cout << Result << endl;//登录成功


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



    //获取股票K线数据
    //bool1 = TdxL2Hq_GetSecurityBars(0, 0, "000001", 0, Count, Result, ErrInfo);//数据种类, 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟K线  8->1分钟K线  9->日K线  10->季K线  11->年K线
    //if (!bool1)
    //{
    //	cout << ErrInfo << endl;
    //	return 0;
    //}
    //cout << Result << endl;



    //获取指数K线数据
    bool1 = TdxL2Hq_GetIndexBars(4, 1, "000001", 0, Count, Result, ErrInfo);//数据种类, 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟K线     8->1分钟K线    9->日K线  10->季K线  11->年K线
    if (!bool1)
    {
        cout << ErrInfo << endl;
        return 0;
    }
    cout << Result << endl;





    //获取分时图数据
    /*bool1 = TdxL2Hq_GetMinuteTimeData(0, "000001",  Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/


    //获取历史分时图数据
    /*bool1 = TdxL2Hq_GetHistoryMinuteTimeData(0, "000001", 20140904, Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/


    //获取分笔图数据
    /*bool1 = TdxL2Hq_GetTransactionData(0, "000001", 0, Count, Result, ErrInfo);
    if (!bool1)
    {
    cout << ErrInfo << endl;
    return 0;
    }
    cout << Result << endl;*/



    //获取历史分笔图数据
    /*bool1 = TdxL2Hq_GetHistoryTransactionData(0, "000001", 0, Count, 20140904,  Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/




    //获取五档报价数据
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



    //获取F10数据的类别
    /*bool1 = TdxL2Hq_GetCompanyInfoCategory(0, "000001", Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/



    //获取F10数据的某类别的内容
    /*bool1 = TdxL2Hq_GetCompanyInfoContent(1, "600030", "600030.txt", 0, 16824, Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/


    //获取除权除息信息
    /*bool1 = TdxL2Hq_GetXDXRInfo(0, "000001", Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/





    //获取财务信息
    /*bool1 = TdxL2Hq_GetFinanceInfo(0, "000001", Result, ErrInfo);
    if (!bool1)
    {
    	cout << ErrInfo << endl;
    	return 0;
    }
    cout << Result << endl;*/






    TdxL2Hq_Disconnect();

    cout << "已经断开服务器"<<endl;


    FreeLibrary(TdxApiHMODULE);

    int a;
    cin >> a;

    return 0;
}

