#ifndef __KIO_HQAPI_H
#define __KIO_HQAPI_H

#include <windows.h>
#include <winbase.h>

#include <iostream>

using namespace std;

//
//开发文档
//

//1.行情API均是TdxHqApi.dll文件的导出函数，包括以下函数：(所有行情函数均为客户端主动请求查询，不是服务器推送)
//bool TdxHq_Connect(const char* IP, int Port, char* Result, char* ErrInfo);//连接券商行情服务器
//void TdxHq_Disconnect();//断开服务器
//bool TdxHq_GetSecurityCount(byte Market, short& Result, char* ErrInfo);//获取指定市场内的证券数目
//bool TdxHq_GetSecurityList(byte Market, short Start, short& Count, char* Result, char* ErrInfo);//获取市场内指定范围内的所有证券代码
//bool TdxHq_GetSecurityBars(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//获取股票K线
//bool TdxHq_GetIndexBars(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//获取指数K线
//bool TdxHq_GetMinuteTimeData(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取分时图数据
//bool TdxHq_GetHistoryMinuteTimeData(byte Market, char* Zqdm, int date, char* Result, char* ErrInfo);//获取历史分时图数据
//bool TdxHq_GetTransactionData(byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);//获取分时成交
//bool TdxHq_GetHistoryTransactionData(byte Market, char* Zqdm, short Start, short& Count, int date, char* Result, char* ErrInfo);//获取历史分时成交
//bool TdxHq_GetSecurityQuotes(byte Market[], char* Zqdm[], short& Count, char* Result, char* ErrInfo);//获取盘口五档报价
//bool TdxHq_GetCompanyInfoCategory(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取F10信息类别
//bool TdxHq_GetCompanyInfoContent(byte Market, char* Zqdm, char* FileName, int Start, int Length, char* Result, char* ErrInfo);//获取F10信息内容
//bool TdxHq_GetXDXRInfo(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取权息数据
//bool TdxHq_GetFinanceInfo(byte Market, char* Zqdm, char* Result, char* ErrInfo);//获取财务数据

///行情接口执行后，如果失败，则字符串ErrInfo保存了出错信息中文说明；
///如果成功，则字符串Result保存了结果数据,形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。
///返回的Result结果数据都是\n,\t分隔的中文字符串，比如查询K线数据，返回的结果字符串就形如

///“时间\t开盘价\t收盘价\t最高价\t最低价\t成交量\t成交额\n
///20150519\t4.644000\t4.732000\t4.747000\t4.576000\t146667487\t683638848.000000\n
///20150520\t4.756000\t4.850000\t4.960000\t4.756000\t353161092\t1722953216.000000”

///查得此数据之后，通过分割字符串， 可以恢复为几行几列的表格形式的数据



//2.API使用流程为: 应用程序先调用TdxHq_Connect连接通达信行情服务器,然后才可以调用其他接口获取行情数据,应用程序应自行处理网络断线问题, 接口是线程安全的
//如果断线，调用任意api函数后，api会返回已经断线的错误信息，应用程序应根据此错误信息重新连接服务器。


//3.各个函数功能说明
/// <summary>
///  连接通达信行情服务器
/// </summary>
/// <param name="IP">服务器IP,可在券商通达信软件登录界面“通讯设置”按钮内查得</param>
/// <param name="Port">服务器端口</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool (__stdcall*  TdxHq_ConnectDelegate)(const char* IP, int Port, char* Result, char* ErrInfo);

/// <summary>
/// 断开同服务器的连接
/// </summary>
typedef void(__stdcall* TdxHq_DisconnectDelegate)();

/// <summary>
/// 获取证券指定范围的的K线数据
/// </summary>
/// <param name="Category">K线种类, 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟  8->1分钟K线  9->日K线  10->季K线  11->年K线< / param>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围的开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围的大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目, 最大值800</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetSecurityBarsDelegate)(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);

/// <summary>
/// 获取市场内所有证券的数量
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的证券数量</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetSecurityCountDelegate)(byte Market, short& Result, char* ErrInfo);

/// <summary>
/// 获取市场内某个范围内的1000支股票的股票代码
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Start">范围开始位置,第一个股票是0, 第二个是1, 依此类推,位置信息依据TdxHq_GetSecurityCount返回的证券总数确定</param>
/// <param name="Count">范围的大小，API执行后,保存了实际返回的股票数目,</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的证券代码信息,形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetSecurityListDelegate)(byte Market, short Start, short& Count, char* Result, char* ErrInfo);

/// <summary>
/// 获取指数的指定范围内K线数据
/// </summary>
/// <param name="Category">K线种类, 0->5分钟K线    1->15分钟K线    2->30分钟K线  3->1小时K线    4->日K线  5->周K线  6->月K线  7->1分钟  8->1分钟K线  9->日K线  10->季K线  11->年K线< / param>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围的大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目,最大值800</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool (__stdcall* TdxHq_GetIndexBarsDelegate)(byte Category, byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);

/// <summary>
/// 获取分时数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool (__stdcall* TdxHq_GetMinuteTimeDataDelegate)(byte Market, char* Zqdm, char* Result, char* ErrInfo);

/// <summary>
/// 获取历史分时数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Date">日期, 比如2014年1月1日为整数20140101</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetHistoryMinuteTimeDataDelegate)(byte Market, char* Zqdm, int Date, char* Result, char* ErrInfo);

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
typedef bool(__stdcall* TdxHq_GetTransactionDataDelegate) (byte Market, char* Zqdm, short Start, short& Count, char* Result, char* ErrInfo);

/// <summary>
/// 获取历史分时成交某个范围内的数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Start">范围开始位置,最后一条K线位置是0, 前一条是1, 依此类推</param>
/// <param name="Count">范围大小，API执行前,表示用户要请求的K线数目, API执行后,保存了实际返回的K线数目</param>
/// <param name="Date">日期, 比如2014年1月1日为整数20140101</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetHistoryTransactionDataDelegate) (byte Market, char* Zqdm, short Start, short& Count, int Date, char* Result, char* ErrInfo);

/// <summary>
/// 批量获取多个证券的五档报价数据
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海, 第i个元素表示第i个证券的市场代码</param>
/// <param name="Zqdm">证券代码, Count个证券代码组成的数组</param>
/// <param name="Count">API执行前,表示用户要请求的证券数目,最大50(不同券商可能不一样,具体数目请自行咨询券商或测试), API执行后,保存了实际返回的数目</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetSecurityQuotesDelegate) (byte Market[], const char* Zqdm[], short& Count, char* Result, char* ErrInfo);

/// <summary>
/// 获取F10资料的分类
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据, 形式为表格数据，行数据之间通过\n字符分割，列数据之间通过\t分隔。一般要分配1024*1024字节的空间。出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetCompanyInfoCategoryDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);

/// <summary>
/// 获取F10资料的某一分类的内容
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="FileName">类目的文件名, 由TdxHq_GetCompanyInfoCategory返回信息中获取</param>
/// <param name="Start">类目的开始位置, 由TdxHq_GetCompanyInfoCategory返回信息中获取</param>
/// <param name="Length">类目的长度, 由TdxHq_GetCompanyInfoCategory返回信息中获取</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据,出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetCompanyInfoContentDelegate) (byte Market, char* Zqdm, char* FileName, int Start, int Length, char* Result, char* ErrInfo);

/// <summary>
/// 获取除权除息信息
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据,出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetXDXRInfoDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);

/// <summary>
/// 获取财务信息
/// </summary>
/// <param name="Market">市场代码,   0->深圳     1->上海</param>
/// <param name="Zqdm">证券代码</param>
/// <param name="Result">此API执行返回后，Result内保存了返回的查询数据,出错时为空字符串。</param>
/// <param name="ErrInfo">此API执行返回后，如果出错，保存了错误信息说明。一般要分配256字节的空间。没出错时为空字符串。</param>
/// <returns>成功返货true, 失败返回false</returns>
typedef bool(__stdcall* TdxHq_GetFinanceInfoDelegate) (byte Market, char* Zqdm, char* Result, char* ErrInfo);

#endif
