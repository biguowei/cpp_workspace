// TDFEasyDemo.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "stdafx.h"
#include <windows.h>

#include "TDFAPI.h"

#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <queue>
#include <cctype>
#include "ConfigParser.h"
#include "ConfigSettings.h"
#include "PathHelper.h"
#include <list>
#include <time.h>
#include "TDFAPIInner.h"
using namespace std;
#define MIN(x, y) ((x)>(y)?(y):(x))

class AutoLock
{
public:
    AutoLock()
    {
        InitializeCriticalSection(&m_cs);
    }
    void Lock()
    {
        EnterCriticalSection(&m_cs);
    }
    void UnLock()
    {
        LeaveCriticalSection(&m_cs);
    }
    ~AutoLock()
    {
        LeaveCriticalSection(&m_cs);
    }

private:
    CRITICAL_SECTION m_cs;
};
THANDLE g_hTDF = NULL;
THANDLE g_hSubWriteThread = NULL;
bool g_bCodeTable = false;

#if 0
#define TICK_BEGIN(Topic) \
    int nBefore##Topic##Tick_ = GetTickCount();

#define TICK_END(Topic) \
    int nAfter##Topic##Tick_ = GetTickCount(); \
    Print("Topic:%s time-sep:%d\n", #Topic, nAfter##Topic##Tick_ - nBefore##Topic##Tick_);


#define LAST_DURATION(Topic) \
    static int nLast##Topic##Time_ltzhou; \
    int nCur##Topic##Time_ltzhou = GetTickCount(); \
    nLast##Topic##Time_ltzhou = nLast##Topic##Time_ltzhou ? nLast##Topic##Time_ltzhou : nCur##Topic##Time_ltzhou; \
    char szBuf##Topic##Time_ltzhou[128];\
    sprintf(szBuf##Topic##Time_ltzhou, "Topic:%s time-sep:%d\n", #Topic, nCur##Topic##Time_ltzhou - nLast##Topic##Time_ltzhou); \
    Print(szBuf##Topic##Time_ltzhou);\
    nLast##Topic##Time_ltzhou = nCur##Topic##Time_ltzhou;
#endif


#define DISPLAY_TIME_GAP(nGap, nLastTime, const_expr, ...)                 \
        {                                                                  \
                                                      \
        __time64_t nCurTime = ::_time64(NULL);                          \
        if ((nCurTime - (nLastTime) > (nGap) )|| ((nLastTime)-nCurTime>(nGap)))                        \
            {                                                   \
            Print(const_expr, __VA_ARGS__);                     \
            nLastTime = nCurTime;                               \
            }                                                   \
        }

void RecvData(THANDLE hTdf, TDF_MSG* pMsgHead);

void RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg);
//�ο����ݻص��������ETF���ļ��ص�����
void RecvRefData(THANDLE hTdf, TDF_REFDATA_MSG* pMsgHead)
{
	if (!pMsgHead ||! hTdf)
	{
		return;
	}
	//ʧ�ܷ���
	if (0 != pMsgHead->errorCode)
		return;
	TDF_ETF_LIST_HEAD* pHead = (TDF_ETF_LIST_HEAD*)pMsgHead->pData;
	switch (pMsgHead->nDataType)
	{

	case MSG_REF_ETFL_LIST:
		{
			//��ӡETF�ļ�
			if (pHead->pData)
				printf("filename:%s,filelen:%d, filecontent:%s\n",pHead->szFileName,pHead->nFileLength,(char*)pHead->pData);
		}
		break;
	default:
		//assert(0);
		break;
	}
}

const char* GetErrStr(TDF_ERR nErr);
void LoadProxyTDFSettings(const ConfigSettings& configObj, TDF_PROXY_SETTING& proxy_settings);

int TranslateNsdTimeMinisecond(int nTime, int nMilliSecond)
{
	return (nTime/60/60*10000 + ((nTime/60)%60)*100 + nTime%60)*1000 + nMilliSecond;
}

#if 0
struct TimeInfo
{
    unsigned int nDate;
    unsigned int nTime;
};

inline void  GetCurrentTimeInfo(struct TimeInfo* timeInfo)
{
    //assert(timeInfo);
    SYSTEMTIME st;
    GetLocalTime(&st);
    timeInfo->nDate = st.wYear*10000 + st.wMonth*100 + st.wDay;
    timeInfo->nTime = st.wHour*10000000 + st.wMinute*100000 + st.wSecond*1000 + st.wMilliseconds;
}
#endif

#define MAKEYMD(y, m, d) ((y)*10000+(m)*100+(d))
#define MAKEHHMMSSsss(H, M, S, ms) ((H)*10000000 + (M)*100000 + (S)*1000 + (ms))

#define ELEM_COUNT(arr) (sizeof(arr)/sizeof(arr[0]))
#define SAFE_STR(str) ((str)?(str):"")
#define SAFE_CHAR(ch) ((ch) ? (ch) : ' ')
void LoadTDFSettings(const ConfigSettings& configObj,  TDF_OPEN_SETTING_EXT& settings);
void PushToListAndNotify(char* pBuf, std::list<char*>& listQueue, AutoLock& guard, HANDLE hEvent);
char* DeepCopy(const char*szIn);
bool InitDataFiles(const std::string& strDataDir);

void DumpScreenMarket(TDF_MARKET_DATA* pMarket, int nItems, int nTime);
void DumpScreenFuture(TDF_FUTURE_DATA* pFuture, int nItems);
void DumpScreenIndex(TDF_INDEX_DATA* pIndex, int nItems);
void DumpScreenTransaction(TDF_TRANSACTION* pTransaction, int nItems);
void DumpScreenOrder(TDF_ORDER* pOrder, int nItems);
void DumpScreenOrderQueue(TDF_ORDER_QUEUE* pOrderQueue, int nItems);
void DumpScreenBrokerQueue(TD_BrokerQueue* pBrokerQueue, int nItems);
void GetLastMarketData(THANDLE g_hTDF);
void GetLastIndexData(THANDLE g_hTDF);
void GetLastFutureData(THANDLE g_hTDF);
void GetLastSnapShot(THANDLE g_hTDF, const char* market);


char* g_pMarketBlock = NULL;
char* g_pFutureBlock = NULL;
char* g_pIndexBlock = NULL;
char* g_pTransactionBlock = NULL;
char* g_pOrderBlock = NULL;
char* g_pOrderQueueBlock = NULL;
char* g_pBBQStepBlock = NULL;
char* g_pBBQBidBlock = NULL;
char* g_pBrokerQueueBlock = NULL;

struct MarketInfo
{
    SYSTEMTIME st;
    unsigned int nServerTime;
    int nItems;
    TDF_MARKET_DATA rawData[1];
};

struct FutureInfo
{
    SYSTEMTIME st;
    unsigned int nServerTime;
    int nItems;
    TDF_FUTURE_DATA rawData[1];
};

struct OrderInfo
{
    SYSTEMTIME st;
    unsigned int nServerTime;
    int nItems;
    TDF_ORDER rawData[1];
};

struct OrderQueueInfo
{
    SYSTEMTIME st;
    unsigned int nServerTime;
    int nItems;
    TDF_ORDER_QUEUE rawData[1];
};

struct BrokerQueueInfo
{
	SYSTEMTIME st;
	unsigned int nServerTime;
	int nItems;
	TD_BrokerQueue rawData[1];
};

struct TransactionInfo
{
    SYSTEMTIME st;
    unsigned int nServerTime;
    int nItems;
    TDF_TRANSACTION rawData[1];
};

struct IndexInfo
{
    SYSTEMTIME st;
    unsigned int nServerTime;
    int nItems;
    TDF_INDEX_DATA rawData[1];
};
struct BBQTransactionInfo
{
	SYSTEMTIME st;
	unsigned int nServerTime;
	int nItems;
	TDF_BBQTRANSACTION_DATA rawData[1];
};
struct BBQBidInfo
{
	SYSTEMTIME st;
	unsigned int nServerTime;
	int nItems;
	TDF_BBQBID_DATA rawData[1];
};
struct DataBlockHead
{
    THANDLE hTdf;//which handle 
    TDF_MSG_ID nMsgId;
    unsigned int m_nTotalSize;

    unsigned int m_nOffset;//
};
struct OTCInfo
{
	SYSTEMTIME st;
	unsigned int nServerTime;
	int nItems;
	TDF_OTC_Option rawData[1];
};

template <class InfoType, class DataType> unsigned int GetInfoTotalSize(unsigned int nItemCount)
{
    //assert(nItemCount);
    unsigned int nLen = sizeof(DataBlockHead) + sizeof(InfoType) + (nItemCount-1)*sizeof(DataType) ;
    return nLen;
}


template <class InfoType, class DataType> unsigned int GetInfoSize(unsigned int nItemCount)
{
    //assert(nItemCount);
    unsigned int nLen = sizeof(InfoType) + (nItemCount-1)*sizeof(DataType) ;
    return nLen;
}

char* intarr2str(char* szBuf, int nBufLen, int arr[], int n)
{
    int nOffset = 0;
    for (int i=0; i<n; i++)
    {
        nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d ", arr[i]);
    }
    return szBuf;
}

char* int64arr2str(char* szBuf, int nBufLen, int64_t arr[], int n)
{
	int nOffset = 0;
	for (int i=0; i<n; i++)
	{
		nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%I64d ", arr[i]);
	}
	return szBuf;
}

char* shortarr2str(char* szBuf, int nBufLen, const short arr[], int n)
{
	int nOffset = 0;
	for (int i=0; i<n; i++)
	{
		nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d ", arr[i]);
	}
	return szBuf;
}

//��ԭʼ�ṹ�ľ���ͨ����ʹ��
char* shortarr2str(char* szBuf, int nBufLen, const short arr[], int n, bool ori)
{
	if (false == ori)
	{
		return shortarr2str(szBuf, nBufLen, arr, n);
	}
	int nOffset = 0;
	for (int i=0; i<n; i++)
	{
		nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d ", arr[i]&0x3fff);
	}
	return szBuf;
}

char* chararr2str(char* szBuf, int nBufLen, char arr[], int n)
{
    int nOffset = 0;
    for (int i=0; i<n; i++)
    {
        nOffset += _snprintf(szBuf+nOffset, nBufLen-nOffset, "%d(%c) ", arr[i], SAFE_CHAR(arr[i]));
    }
    return szBuf;
}


template <class InfoType> InfoType* InitDataBlockHead(DataBlockHead** ppHead, char* pBlockStart, unsigned int nBlockSize)
{
    //assert(nBlockSize >= sizeof(DataBlockHead));
    DataBlockHead* pHead = (DataBlockHead*)pBlockStart;
    pHead->m_nTotalSize = nBlockSize;
    pHead->m_nOffset = sizeof(DataBlockHead);
    
    if (ppHead)
    {
        *ppHead =  (DataBlockHead*)pBlockStart;
    }

    InfoType* pRet = (InfoType*)(pBlockStart + sizeof(DataBlockHead));
    return pRet;
}



AutoLock g_lockDataBlockList;
std::list<char*> g_listDataBlock;
HANDLE g_hEventDataArrived;


unsigned int g_nMaxBuf;
unsigned int g_nMaxWriteTimeGap;
int g_nQuitSubThread;
int g_nOutputDevice;
int g_nOutputCodeTable;

int g_nCommonLogGap;

std::ofstream g_fsLog;
std::ofstream g_fsMarket;
//std::ofstream g_fsMarket2;//no local time
std::ofstream g_fsFuture;
std::ofstream g_fsIndex;
std::ofstream g_fsOrder;
std::ofstream g_fsOrderQueue;
std::ofstream g_fsTransaction;
std::ofstream g_fsOption;
std::ofstream g_fsBBQTransaction;
std::ofstream g_fsBBQBid;
std::ofstream g_fsBrokerQueue;

DWORD __stdcall SubWriteThread(void* lParam);


void DumpMarketInfo(DataBlockHead* pHead)
{
	MarketInfo* pInfo = (MarketInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 4096;
	char szBuf[nMaxBufSize];
	char szBufSmaller1[160];
	char szBufSmaller2[160];
	char szBufSmaller3[160];
	char szBufSmaller4[160];
	char szBufSmaller5[64];
#if 0
	int nOffset = 0;

	char        szWindCode[32];         //600001.SH 
	char        szCode[32];             //ԭʼCode
	int         nActionDay;             //ҵ������(��Ȼ��)
	int         nTradingDay;            //������
	int			 nTime;					//ʱ��(HHMMSSmmm)
	int			 nStatus;				//״̬
	__int64 nPreClose;				//ǰ���̼�
	__int64 nOpen;					//���̼�
	__int64 nHigh;					//��߼�
	__int64 nLow;					//��ͼ�
	__int64 nMatch;				//���¼�
	__int64 nAskPrice[10];			//������
	__int64 nAskVol[10];			//������
	__int64 nBidPrice[10];			//�����
	__int64 nBidVol[10];			//������
	int nNumTrades;			//�ɽ�����
	__int64		 iVolume;				//�ɽ�����
	__int64		 iTurnover;				//�ɽ��ܽ��
	__int64		 nTotalBidVol;			//ί����������
	__int64		 nTotalAskVol;			//ί����������
	__int64 nWeightedAvgBidPrice;	//��Ȩƽ��ί��۸�
	__int64 nWeightedAvgAskPrice;  //��Ȩƽ��ί���۸�
	int			 nIOPV;					//IOPV��ֵ��ֵ
	int			 nYieldToMaturity;		//����������
	__int64 nHighLimited;			//��ͣ��
	__int64 nLowLimited;			//��ͣ��
	char		 chPrefix[4];			//֤ȯ��Ϣǰ׺
	int			 nSyl1;					//��ӯ��1
	int			 nSyl2;					//��ӯ��2
	int			 nSD2;					//����2���Ա���һ�ʣ�
	TDF_NoSSE_Bond TDF_Bond;			//��㽻������ծȯ�����Ϣ
	int			 nTradeFlag;			//���ױ�־,���庬��ο��ĵ�

	__int64 iAfterPrice;    //�̺�۸�(�ƴ�����ʹ�õ�)
	int nAfterVolume;		//�̺���(�ƴ�����ʹ�õ�)
	__int64 iAfterTurnover;	//�̺�ɽ����(�ƴ�����ʹ�õ�)
	int nAfterMatchItems;   //�̺�ɽ�����(�ƴ�����ʹ�õ�)

	const TDF_CODE_INFO *  pCodeInfo;   //������Ϣ�� TDF_Close�����������󣬴�ָ����Ч
	"����, ����ʱ��, ������ʱ��, ������ʱ��, ��ô���, ԭʼ����, ҵ������(��Ȼ��), ������,״̬,ǰ��,���̼�,��߼�,��ͼ�,���¼�,������,������,�����,������,�ɽ�����,�ɽ�����,�ɽ��ܽ��,ί����������,ί����������,��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�,IOPV��ֵ��ֵ,����������,��ͣ��,��ͣ��,֤ȯ��Ϣǰ׺,��ӯ��1,��ӯ��2,����2���Ա���һ�ʣ�,���ױ�־,�̺�۸�,�̺���,�̺�ɽ����,�̺�ɽ�����",
#endif

		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{

				TDF_MARKET_DATA* pTDFMarket = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize, "%d,%d,%d,%d,%s,%s,%d,%d,"
					"%d(%c),"    //status
					"%I64d,%I64d,%I64d,%I64d,%I64d,"        //preclose,open, high, low, last,
					"%s,%s,%s,%s,"
					"%d,%I64d,%I64d,%I64d,%I64d," 
					"%I64d,%I64d,%d,%d,"
					"%I64d,%I64d,%s,"
					"%d,%d,%d,%d"
					"%I64d,%d,%I64d,%d"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pInfo->rawData[i].nTime, 
					pTDFMarket->szWindCode, pTDFMarket->szCode, pTDFMarket->nActionDay, pTDFMarket->nTradingDay, 
					pTDFMarket->nStatus, SAFE_CHAR(pTDFMarket->nStatus),
					pTDFMarket->nPreClose, pTDFMarket->nOpen, pTDFMarket->nHigh ,pTDFMarket->nLow, pTDFMarket->nMatch,
					int64arr2str(szBufSmaller1, sizeof(szBufSmaller1), (int64_t*)pTDFMarket->nAskPrice, ELEM_COUNT(pTDFMarket->nAskPrice)),
					int64arr2str(szBufSmaller2, sizeof(szBufSmaller2), (int64_t*)pTDFMarket->nAskVol, ELEM_COUNT(pTDFMarket->nAskVol)),
					int64arr2str(szBufSmaller3, sizeof(szBufSmaller3), (int64_t*)pTDFMarket->nBidPrice, ELEM_COUNT(pTDFMarket->nBidPrice)),
					int64arr2str(szBufSmaller4, sizeof(szBufSmaller4), (int64_t*)pTDFMarket->nBidVol, ELEM_COUNT(pTDFMarket->nBidVol)),
					pTDFMarket->nNumTrades, pTDFMarket->iVolume, pTDFMarket->iTurnover, pTDFMarket->nTotalBidVol, pTDFMarket->nTotalAskVol, 
					pTDFMarket->nWeightedAvgBidPrice, pTDFMarket->nWeightedAvgAskPrice,pTDFMarket->nIOPV, pTDFMarket->nYieldToMaturity, 
					pTDFMarket->nHighLimited, pTDFMarket->nLowLimited, chararr2str(szBufSmaller5, sizeof(szBufSmaller5), pTDFMarket->chPrefix, ELEM_COUNT(pTDFMarket->chPrefix)),
					pTDFMarket->nSyl1, pTDFMarket->nSyl2, pTDFMarket->nSD2,pTDFMarket->nTradeFlag,
					pTDFMarket->iAfterPrice, pTDFMarket->nAfterVolume, pTDFMarket->iAfterTurnover, pTDFMarket->nAfterMatchItems
					);


				g_fsMarket<<szBuf<<std::endl;
			}

			pInfo =(MarketInfo*)((char*)pInfo + GetInfoSize<MarketInfo, TDF_MARKET_DATA>(pInfo->nItems));
		}
}

void DumpFutureInfo(DataBlockHead* pHead)
{
	FutureInfo* pInfo = (FutureInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 4096;
	char szBuf[nMaxBufSize];
	char szBufSmaller1[160];
	char szBufSmaller2[160];
	char szBufSmaller3[160];
	char szBufSmaller4[160];
	//char szBufSmaller5[64];
#if 0
	char        szWindCode[32];         //600001.SH 
	char        szCode[32];             //ԭʼCode
	int         nActionDay;             //ҵ������(��Ȼ��)
	int         nTradingDay;            //������
	int			 nTime;					//ʱ��(HHMMSSmmm)	
	int			 nStatus;				//״̬
	__int64		 iPreOpenInterest;		//��ֲ�
	__int64 nPreClose;				//�����̼�
	unsigned int nPreSettlePrice;		//�����
	__int64 nOpen;					//���̼�	
	__int64 nHigh;					//��߼�
	__int64 nLow;					//��ͼ�
	__int64 nMatch;				//���¼�
	__int64		 iVolume;				//�ɽ�����
	__int64		 iTurnover;				//�ɽ��ܽ��
	__int64		 iOpenInterest;			//�ֲ�����
	__int64 nClose;				//������
	unsigned int nSettlePrice;			//�����
	__int64 nHighLimited;			//��ͣ��
	__int64 nLowLimited;			//��ͣ��
	int			 nPreDelta;			    //����ʵ��
	int			 nCurrDelta;            //����ʵ��
	__int64 nAskPrice[5];			//������
	__int64 nAskVol[5];			//������
	__int64 nBidPrice[5];			//�����
	__int64 nBidVol[5];			//������
	//Add 20140605
	__int64	nAuctionPrice;		            //�������жϲο���
	int	nAuctionQty;		            //�������жϼ��Ͼ�������ƥ����	
	int nAvgPrice;                      //֣�����ڻ�����
	const TDF_CODE_INFO *  pCodeInfo;

	"����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��),������,״̬,��ֲ�,�����̼�,�����,���̼�,��߼�,��ͼ�,���¼�,�ɽ�����,�ɽ��ܽ��,�ֲ�����,������,�����,��ͣ��,��ͣ��,����ʵ��,����ʵ��,������,������,�����,������",
#endif

	while ((char*)pInfo < pEnd)
	{
		for (int i=0; i<pInfo->nItems; i++)
		{
			TDF_FUTURE_DATA* pTDFFuture = pInfo->rawData + i;

			if ((pTDFFuture->pCodeInfo->nType&0xf0)==0x90)
			{
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%s,%d,%d,"
					"%d(%c),"
					"%I64d,%I64d,%u,%I64d,%I64d,%I64d,%I64d,"
					"%I64d,%I64d,%I64d,"
					"%I64d,%u,%I64d,%I64d,"
					"%d,%d,"
					"%s,%s,%s,%s,"
					"%s,%s,%d(%c),%d",
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pTDFFuture->nTime, 
					pTDFFuture->szWindCode, pTDFFuture->szCode, pTDFFuture->nActionDay, pTDFFuture->nTradingDay, 
					pTDFFuture->nStatus, SAFE_CHAR(pTDFFuture->nStatus),
					pTDFFuture->iPreOpenInterest, pTDFFuture->nPreClose, pTDFFuture->nPreSettlePrice, pTDFFuture->nOpen, pTDFFuture->nHigh, pTDFFuture->nLow, pTDFFuture->nMatch, 
					pTDFFuture->iVolume, pTDFFuture->iTurnover, pTDFFuture->iOpenInterest, 
					pTDFFuture->nClose, pTDFFuture->nSettlePrice, pTDFFuture->nHighLimited, pTDFFuture->nLowLimited,
					pTDFFuture->nPreDelta, pTDFFuture->nCurrDelta,
					int64arr2str(szBufSmaller1, sizeof(szBufSmaller1), (int64_t*)pTDFFuture->nAskPrice, ELEM_COUNT(pTDFFuture->nAskPrice)),
					int64arr2str(szBufSmaller2, sizeof(szBufSmaller2), (int64_t*)pTDFFuture->nAskVol, ELEM_COUNT(pTDFFuture->nAskVol)),
					int64arr2str(szBufSmaller3, sizeof(szBufSmaller3), (int64_t*)pTDFFuture->nBidPrice, ELEM_COUNT(pTDFFuture->nBidPrice)),
					int64arr2str(szBufSmaller4, sizeof(szBufSmaller4), (int64_t*)pTDFFuture->nBidVol, ELEM_COUNT(pTDFFuture->nBidVol)),
					pTDFFuture->pCodeInfo->exCodeInfo.Option.chContractID, pTDFFuture->pCodeInfo->exCodeInfo.Option.szUnderlyingSecurityID, pTDFFuture->pCodeInfo->exCodeInfo.Option.chCallOrPut,SAFE_CHAR(pTDFFuture->pCodeInfo->exCodeInfo.Option.chCallOrPut),pTDFFuture->pCodeInfo->exCodeInfo.Option.nExerciseDate
					);
				g_fsOption << szBuf<<std::endl;
			}
			else
			{
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%s,%d,%d,"
					"%d(%c),"
					"%I64d,%I64d,%u,%I64d,%I64d,%I64d,%I64d,"
					"%I64d,%I64d,%I64d,"
					"%I64d,%u,%I64d,%I64d,"
					"%d,%d,"
					"%s,%s,%s,%s",
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pTDFFuture->nTime, 
					pTDFFuture->szWindCode, pTDFFuture->szCode, pTDFFuture->nActionDay, pTDFFuture->nTradingDay, 
					pTDFFuture->nStatus, SAFE_CHAR(pTDFFuture->nStatus),
					pTDFFuture->iPreOpenInterest, pTDFFuture->nPreClose, pTDFFuture->nPreSettlePrice, pTDFFuture->nOpen, pTDFFuture->nHigh, pTDFFuture->nLow, pTDFFuture->nMatch, 
					pTDFFuture->iVolume, pTDFFuture->iTurnover, pTDFFuture->iOpenInterest, 
					pTDFFuture->nClose, pTDFFuture->nSettlePrice, pTDFFuture->nHighLimited, pTDFFuture->nLowLimited,
					pTDFFuture->nPreDelta, pTDFFuture->nCurrDelta,
					int64arr2str(szBufSmaller1, sizeof(szBufSmaller1), (int64_t*)pTDFFuture->nAskPrice, ELEM_COUNT(pTDFFuture->nAskPrice)),
					int64arr2str(szBufSmaller2, sizeof(szBufSmaller2), (int64_t*)pTDFFuture->nAskVol, ELEM_COUNT(pTDFFuture->nAskVol)),
					int64arr2str(szBufSmaller3, sizeof(szBufSmaller3), (int64_t*)pTDFFuture->nBidPrice, ELEM_COUNT(pTDFFuture->nBidPrice)),
					int64arr2str(szBufSmaller4, sizeof(szBufSmaller4), (int64_t*)pTDFFuture->nBidVol, ELEM_COUNT(pTDFFuture->nBidVol)));
				g_fsFuture << szBuf<<std::endl;
			}

		}

		pInfo =(FutureInfo*)((char*)pInfo + GetInfoSize<FutureInfo, TDF_FUTURE_DATA>(pInfo->nItems));
	}
}

void DumpIndexInfo(DataBlockHead* pHead)
{
	IndexInfo* pInfo = (IndexInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 1024;
	char szBuf[nMaxBufSize];
	//char szBufSmaller5[64];
#if 0
	char        szWindCode[32];         //600001.SH 
	char        szCode[32];             //ԭʼCode
	int         nActionDay;             //ҵ������(��Ȼ��)
	int         nTradingDay;            //������
	int         nTime;			        //ʱ��(HHMMSSmmm)
	int			nStatus;				//״̬��20151223����
	__int64		    nOpenIndex;		        //����ָ��
	__int64 	    nHighIndex;		        //���ָ��
	__int64 	    nLowIndex;		        //���ָ��
	__int64 	    nLastIndex;		        //����ָ��
	__int64	    iTotalVolume;	        //���������Ӧָ���Ľ�������
	__int64	    iTurnover;		        //���������Ӧָ���ĳɽ����
	__int64		    nPreCloseIndex;	        //ǰ��ָ��

	"����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��),������,����ָ��,���ָ��,���ָ��,����ָ��,�ɽ�����,�ɽ��ܽ��,ǰ��ָ��",
#endif
		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{
				TDF_INDEX_DATA* pIndex = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%s,%d,%d,"
					"%d(%c),"
					"%I64d,%I64d,%I64d,%I64d,"
					"%I64d,%I64d,%I64d"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pIndex->nTime, 
					pIndex->szWindCode, pIndex->szCode, pIndex->nActionDay, pIndex->nTradingDay, 
					pIndex->nStatus,SAFE_CHAR(pIndex->nStatus),
					pIndex->nOpenIndex,pIndex->nHighIndex,pIndex->nLowIndex,pIndex->nLastIndex,
					pIndex->iTotalVolume, pIndex->iTurnover, pIndex->nPreCloseIndex);

				g_fsIndex << szBuf<<std::endl;
			}

			pInfo =(IndexInfo*)((char*)pInfo + GetInfoSize<IndexInfo, TDF_INDEX_DATA>(pInfo->nItems));
		}
}

void DumpTransactionInfo(DataBlockHead* pHead)
{
	TransactionInfo* pInfo = (TransactionInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 1024;
	char szBuf[nMaxBufSize];
	//char szBufSmaller5[64];
#if 0
	char    szWindCode[32];             //600001.SH 
	char    szCode[32];                 //ԭʼCode
	int     nActionDay;                 //��Ȼ��
	int 	nTime;		                //�ɽ�ʱ��(HHMMSSmmm)
	int 	nIndex;		                //�ɽ����
	__int64		nPrice;		                //�ɽ��۸�
	int 	nVolume;	                //�ɽ�����
	__int64		nTurnover;	                //�ɽ����
	int     nBSFlag;                    //��������(��'B', ����'S', ������' ')
	char    chOrderKind;                //�ɽ����
	char    chFunctionCode;             //�ɽ�����
	int	    nAskOrder;	                //������ί�����
	int	    nBidOrder;	                //����ί�����

	"����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��), �ɽ����,�ɽ��۸�,�ɽ�����,�ɽ����,��������,�ɽ����,�ɽ�����,����ί�����,��ί�����",
#endif
		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{
				TDF_TRANSACTION* pTransaction = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%s,%d,"
					"%d,%I64d,%d,%I64d,"
					"%d(%c)," //��������
					"%d(%c),%d(%c),"
					"%d,%d"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pTransaction->nTime, 
					pTransaction->szWindCode, pTransaction->szCode, pTransaction->nActionDay,
					pTransaction->nIndex,pTransaction->nPrice,pTransaction->nVolume,pTransaction->nTurnover,
					pTransaction->nBSFlag, SAFE_CHAR(pTransaction->nBSFlag),
					pTransaction->chOrderKind, SAFE_CHAR(pTransaction->chOrderKind),pTransaction->chFunctionCode,SAFE_CHAR(pTransaction->chFunctionCode),
					pTransaction->nAskOrder,pTransaction->nBidOrder);

				g_fsTransaction << szBuf<<std::endl;
			}

			pInfo =(TransactionInfo*)((char*)pInfo + GetInfoSize<TransactionInfo, TDF_TRANSACTION>(pInfo->nItems));
		}
}

void DumpBBQTransaction(DataBlockHead* pHead)
{
	BBQTransactionInfo* pInfo = (BBQTransactionInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 1024;
	char szBuf[nMaxBufSize];
	//char szBufSmaller5[64];
#if 0
	char        szWindCode[32];         //600001.SH 
	int         nActionDay;             //ҵ������(��Ȼ��)
	int			nTime;					//ʱ��(HHMMSSmmm)
	int         nDoneID;                //�ɽ����۱��
	__int64			nPrice;				    //�ɽ�������(%)��۸� *10000
	char        chPriceStatus;          //�����ʻ�۸��ʶ	1�������� 	2���۸�
	char        chStatus;               //�����ʻ�۸��ʶ	1�������� 	2���۸�
	char        chDirection;            //�ɽ�����	1��done	2��gvn		3��tkn		4������
	char        chSource;               //���ۻ���	1������ 2��ƽ����˳ 3���г� 4���������	5������
	char        chSpecialFlag;          //��ʶ�����Ƿ�����Ȩ����	0�ޱ��	1��ע�а�������		2��ע�а�����Ȩ

	"����,����ʱ��,������ʱ��,������ʱ��,��ô���,ҵ������(��Ȼ��), �ɽ����۱��,�ɽ��۸�/������,������/�۸��ʶ,ɾ��/�ɽ�,�ɽ�����,���ۻ���,��Ȩ/����",
#endif
		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{
				TDF_BBQTRANSACTION_DATA* pTransaction = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%d,"
					"%d,%I64d,"
					"%d(%c),%d(%c),"
					"%d(%c),"
					"%d(%c),%d(%c)"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pTransaction->nTime, 
					pTransaction->szWindCode, pTransaction->nActionDay,
					pTransaction->nDoneID,pTransaction->nPrice,pTransaction->chPriceStatus,SAFE_CHAR(pTransaction->chPriceStatus),
					pTransaction->chStatus,SAFE_CHAR(pTransaction->chStatus),
					pTransaction->chDirection, SAFE_CHAR(pTransaction->chDirection),
					pTransaction->chSource, SAFE_CHAR(pTransaction->chSource),pTransaction->chSpecialFlag,SAFE_CHAR(pTransaction->chSpecialFlag));

				g_fsBBQTransaction << szBuf<<std::endl;
			}
			pInfo =(BBQTransactionInfo*)((char*)pInfo + GetInfoSize<BBQTransactionInfo, TDF_BBQTRANSACTION_DATA>(pInfo->nItems));
		}
}
void DumpBBQBid(DataBlockHead* pHead)
{
	BBQBidInfo* pInfo = (BBQBidInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 1024;
	char szBuf[nMaxBufSize];
	//char szBufSmaller5[64];
#if 0
	char        szWindCode[32];         //600001.SH 
	int         nActionDay;             //ҵ������(��Ȼ��)
	int			nTime;					//ʱ��(HHMMSSmmm)
	char        chSource;               //���ۻ���

	__int64         nBidPrice;		       //���������ʻ�۸� x10000
	__int64         nBidVolume;		   //������ x10000
	char        chBidPriceStatus;      //�����ʻ�۸��ʶ	1�������� 	2���۸�
	char        chIsBid;               //�Ƿ�bid	0������bid����ͨ���ۣ�	1����bid�����򱨼ۣ�
	char        chBidSpecialFlag;      //��ʶ�����Ƿ�����Ȩ����	0�ޱ��	1��ע�а�������		2��ע�а�����Ȩ
	char        chBidStatus;           //�������ű���״̬	0����������	1����������

	__int64         nOfrPrice;		       //���������ʻ�۸� x10000
	__int64         nOfrVolume;		   //������	�������ļ��ܣ���2000+1000���ͺϲ�Ϊ3000�����ַ����к�- - ����Ϊ0. x10000
	char        chOfrPriceStatus;      //�����ʻ�۸��ʶ	1�������� 	2���۸�
	char        chIsOfr;               //�Ƿ�ofr	0������ofr����ͨ���ۣ�	  1����ofr�����򱨼ۣ�
	char        chOfrSpecialFlag;      //��ʶ�����Ƿ�����Ȩ����	0�ޱ��	1��ע�а�������		2��ע�а�����Ȩ
	char        chOfrStatus;           //�������ű���״̬	0����������	1����������
	int         nDoneID;                //�ɽ����۱��

	"����,����ʱ��,������ʱ��,����ʱ��,��ô���,������(��Ȼ��), ���ۻ���,����������/�۸�,������,����������/�۸��ʶ,�Ƿ�bid,��Ȩ/����,�������ű���״̬,����������/�۸�,������,����������/�۸��ʶ,�Ƿ�ofr,��Ȩ/����,�������ű���״̬,�ɽ����۱��",
#endif
		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{
				TDF_BBQBID_DATA* pBid = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%d,%d,"
					"%I64d,%I64d,%d(%c),%d(%c),%d(%c),%d(%c),"
					"%I64d,%I64d,%d(%c),%d(%c),%d(%c),%d(%c),"
					"%d"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pBid->nTime, 
					pBid->szWindCode, pBid->nActionDay,pBid->chSource,
					pBid->nBidPrice,pBid->nBidVolume,pBid->chBidPriceStatus,SAFE_CHAR(pBid->chBidPriceStatus),
					pBid->chIsBid,SAFE_CHAR(pBid->chIsBid),pBid->chBidSpecialFlag,SAFE_CHAR(pBid->chBidSpecialFlag),pBid->chBidStatus,SAFE_CHAR(pBid->chBidStatus),
					pBid->nOfrPrice,pBid->nOfrVolume,pBid->chOfrPriceStatus,SAFE_CHAR(pBid->chOfrPriceStatus),
					pBid->chIsOfr,SAFE_CHAR(pBid->chIsOfr),pBid->chOfrSpecialFlag,SAFE_CHAR(pBid->chOfrSpecialFlag),pBid->chOfrStatus,SAFE_CHAR(pBid->chOfrStatus), pBid->nDoneID);

				g_fsBBQBid << szBuf<<std::endl;
			}
			pInfo =(BBQBidInfo*)((char*)pInfo + GetInfoSize<BBQBidInfo, TDF_BBQBID_DATA>(pInfo->nItems));
		}
}

void DumpBrokerQueue(DataBlockHead* pHead)
{
	BrokerQueueInfo* pInfo = (BrokerQueueInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 3072;
	char szBuf[nMaxBufSize];
	char szBufSmaller1[2500] = {0};
	char szBufSmaller2[2500] = {0};
#if 0

	//���Ͷ���(HK)/////////////////////////////////////
	struct TD_BrokerQueue
	{
		char  szWindCode[32]; //600001.SH 
		char  szCode[32];     //ԭʼCode
		int   nActionDay;     //��Ȼ��
		int   nAskTime;       // ����ʱ�䣨HHMMSSmmm��
		int   nBidTime;       // ����ʱ�䣨HHMMSSmmm��
		int   nAskBrokers;    // �������͸���
		int   nBidBrokers;    // �������͸���
		short sAskBroker[40]; // ����ǰ40����
		short sBidBroker[40]; // ����ǰ40����
		const TDF_CODE_INFO *  pCodeInfo;     //������Ϣ�� TDF_Close�����������󣬴�ָ����Ч
	};
	"����,����ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��),����ʱ��, ����ʱ��, �������͸���, �������͸���,����ǰ40����,����ǰ40����",

#endif
		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{
				TD_BrokerQueue* pBrokerQueue = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%s,%s,%d,"
					"%d,%d,%d,%d,"
					"%s,%s"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pBrokerQueue->szWindCode, pBrokerQueue->szCode, pBrokerQueue->nActionDay,
					pBrokerQueue->nAskTime, pBrokerQueue->nBidTime, pBrokerQueue->nAskBrokers, pBrokerQueue->nBidBrokers,
					shortarr2str(szBufSmaller1, sizeof(szBufSmaller1), pBrokerQueue->sAskBroker, pBrokerQueue->nAskBrokers),
					shortarr2str(szBufSmaller2, sizeof(szBufSmaller2), pBrokerQueue->sBidBroker, pBrokerQueue->nBidBrokers)
					);

				g_fsBrokerQueue << szBuf<<std::endl;
			}

			pInfo =(BrokerQueueInfo*)((char*)pInfo + GetInfoSize<BrokerQueueInfo, TD_BrokerQueue>(pInfo->nItems));
		}
}

void DumpOrder(DataBlockHead* pHead)
{
	OrderInfo* pInfo = (OrderInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 1024;
	char szBuf[nMaxBufSize];
	//char szBufSmaller5[64];
#if 0

	char    szWindCode[32]; //600001.SH 
	char    szCode[32];     //ԭʼCode
	int 	nActionDay;	    //ί������(YYMMDD)
	int 	nTime;			//ί��ʱ��(HHMMSSmmm)
	int 	nOrder;	        //ί�к�
	__int64		nPrice;			//ί�м۸�
	int 	nVolume;		//ί������
	char    chOrderKind;	//ί�����
	char    chFunctionCode;	//ί�д���('B','S','C')
	int     nBroker;		//�����̱���
	char    chStatus;		//ί��״̬
	char	chFlag;			//��־

	"����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��), ί�к�, ί�м۸�, ί������, ί�����, ί�д���, �����̱���, ί��״̬, ��־",
#endif
		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{
				TDF_ORDER* pOrder = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%s,%d,"
					"%d,%I64d,%d,%d(%c),%d(%c),"
					"%d,%d(%c),%d(%c)"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pOrder->nTime, 
					pOrder->szWindCode, pOrder->szCode, pOrder->nActionDay,
					pOrder->nOrder, pOrder->nPrice, pOrder->nVolume,
					pOrder->chOrderKind, SAFE_CHAR(pOrder->chOrderKind),
					pOrder->chFunctionCode, SAFE_CHAR(pOrder->chFunctionCode),
					pOrder->nBroker, pOrder->chStatus, SAFE_CHAR(pOrder->chStatus),pOrder->chFlag, SAFE_CHAR(pOrder->chFlag)
					);

				g_fsOrder << szBuf<<std::endl;
			}

			pInfo =(OrderInfo*)((char*)pInfo + GetInfoSize<OrderInfo, TDF_ORDER>(pInfo->nItems));
		}
}

void DumpOrderQueue(DataBlockHead* pHead)
{
	OrderQueueInfo* pInfo = (OrderQueueInfo*)((char*)pHead+sizeof(DataBlockHead));
	char* pEnd = (char*)pHead + pHead->m_nOffset;

	const int nMaxBufSize = 3072;
	char szBuf[nMaxBufSize];
	char szBufSmaller1[2500];
#if 0

	char    szWindCode[32]; //600001.SH 
	char    szCode[32];     //ԭʼCode
	int     nActionDay;     //��Ȼ��
	int 	nTime;			//ʱ��(HHMMSSmmm)
	int     nSide;			//��������('B':Bid 'A':Ask)
	__int64		nPrice;			//ί�м۸�
	int 	nOrders;		//��������
	int 	nABItems;	
	int 	nABVolume[200];	//������ϸ

	"����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��), ��������,�ɽ��۸�,��������,��ϸ����,������ϸ",

#endif
		while ((char*)pInfo < pEnd)
		{
			for (int i=0; i<pInfo->nItems; i++)
			{
				TDF_ORDER_QUEUE* pOrderQueue = pInfo->rawData + i;
				_snprintf(szBuf, nMaxBufSize,
					"%d,%d,%d,%d,%s,%s,%d,"
					"%d(%c),%I64d,%d,%d,%s"
					,
					MAKEYMD(pInfo->st.wYear, pInfo->st.wMonth, pInfo->st.wDay), 
					MAKEHHMMSSsss(pInfo->st.wHour, pInfo->st.wMinute, pInfo->st.wSecond, pInfo->st.wMilliseconds), 
					pInfo->nServerTime, 
					pOrderQueue->nTime, 
					pOrderQueue->szWindCode, pOrderQueue->szCode, pOrderQueue->nActionDay,
					pOrderQueue->nSide, SAFE_CHAR(pOrderQueue->nSide),
					pOrderQueue->nPrice, pOrderQueue->nOrders, pOrderQueue->nABItems, intarr2str(szBufSmaller1, sizeof(szBufSmaller1), pOrderQueue->nABVolume, pOrderQueue->nABItems)
					);

				g_fsOrderQueue << szBuf<<std::endl;
			}

			pInfo =(OrderQueueInfo*)((char*)pInfo + GetInfoSize<OrderQueueInfo, TDF_ORDER_QUEUE>(pInfo->nItems));
		}
}
#define  SIZEITEM(type) {#type, sizeof(type)}


BOOL WINAPI ConsoleHandler(DWORD msgType)
{
    if (msgType == CTRL_C_EVENT ||msgType == CTRL_CLOSE_EVENT)
    {
        if (g_hTDF)
        {
			
            g_nQuitSubThread = 1;
            SetEvent(g_hEventDataArrived);
            if (g_hSubWriteThread)
            {
                WaitForSingleObject(g_hSubWriteThread, INFINITE);
                CloseHandle(g_hSubWriteThread);
                g_hSubWriteThread = NULL;
            }
            TDF_Close(g_hTDF);
            g_hTDF = NULL;
            if (g_hEventDataArrived)
            {
                CloseHandle(g_hEventDataArrived);
                g_hEventDataArrived = NULL;
            }
        }
		if (g_pMarketBlock)
		{
			//DumpMarketInfo((DataBlockHead*)g_pMarketBlock);
			delete[] (char*)g_pMarketBlock;
			g_pMarketBlock = NULL;
		}

		if (g_pFutureBlock)
		{
			//DumpFutureInfo((DataBlockHead*)g_pFutureBlock);
			delete [](char*)g_pFutureBlock;
			g_pFutureBlock = NULL;
		}

		if (g_pIndexBlock)
		{
			//DumpIndexInfo((DataBlockHead*)g_pIndexBlock);
			delete [](char*)g_pIndexBlock;
			g_pIndexBlock = NULL;
		}

		if (g_pTransactionBlock)
		{
			//DumpTransactionInfo((DataBlockHead*)g_pTransactionBlock);
			delete [](char*)g_pTransactionBlock;
			g_pTransactionBlock = NULL;
		}

		if (g_pOrderBlock)
		{
			//DumpOrder((DataBlockHead*)g_pOrderBlock);
			delete [](char*)g_pOrderBlock;
			g_pOrderBlock = NULL;
		}

		if (g_pOrderQueueBlock)
		{
			//DumpOrderQueue((DataBlockHead*)g_pOrderQueueBlock);
			delete [](char*)g_pOrderQueueBlock;
			g_pOrderQueueBlock = NULL;
		}

        g_fsMarket.close();
		//g_fsMarket2.close();
        g_fsFuture.close();
        g_fsIndex.close();
        g_fsOrderQueue.close();
        g_fsOrder.close();
        g_fsTransaction.close();
        g_fsOption.close();
		g_fsBBQTransaction.close();
		g_fsBBQBid.close();
		g_fsBrokerQueue.close();

        g_fsLog.close();
        Print("Close complete!\n");
        exit(0);
        return TRUE;
    }
 
    return TRUE;
}

int main(char*argv[], int argc)
{

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    ConfigSettings cfgSettings;
    std::string strConfigFile = GetConfigDir() + GetBasicFileName() + ".ini";

    bool bLoadSettings = cfgSettings.LoadSettings(strConfigFile);

	//1��������������,�ɲ����ã�ֱ��ʹ��Ĭ��ֵ
    TDF_SetEnv(TDF_ENVIRON_HEART_BEAT_INTERVAL, cfgSettings.nHeartBeatGap);
    TDF_SetEnv(TDF_ENVIRON_MISSED_BEART_COUNT, cfgSettings.nMissedHeartBeatCount);
    TDF_SetEnv(TDF_ENVIRON_OPEN_TIME_OUT, cfgSettings.nOpenTimeOut);

	//���ݻص�ģʽѡ��Ĭ�Ϲ�Ʊ�����ָ������ֿ�ģʽ
	//���ú���յ�MSG_DATA_ORIGINAL�滻��ǰ��MSG_DATA_INDEX��MSG_DATA_MARKET��MSG_DATA_FUTURE��MSG_DATA_ORDERQUEUE��MSG_DATA_BBQBID��MSG_DATA_BROKERQUEUE
	// TDF_SetEnv((TDF_ENVIRON_SETTING)TDF_ENVIRON_ORIGI_MODE, 1);//1:�ص�����ԭʼ�ṹ(��������ص���һ���̶��ṹ),other: false
	
	//TDF_SetEnv(TDF_ENVIRON_HEART_BEAT_FLAG, 2);
	//TDF_SetEnv(TDF_ENVIRON_USE_PACK_OVER,1);
	
	//˫��ģʽѡ��Ĭ��Ϊ�ϲ�ģʽ(ѡ����·�нϿ�һ·���лص�)
	// TDF_SetEnv(TDF_ENVIRON_SOURCE_MODE,(unsigned int)TDF_SOURCE_SWITCH);//˫��������ʹ��
	// TDF_SetEnv(TDF_ENVIRON_SOURCE_MODE_VALUE,5000);//TDF_SOURCE_SWITCHģʽ�Ĳ���ֵ����ʾ5000������·û���ݸ��±��л�����·

	//log�ļ�������ʽ��Ĭ�ϴ�������������Ŀ¼�µ�log�ļ����£�����Ϊ�ڵ�ǰ����·����ֱ�Ӵ���
	//TDF_SetEnv(TDF_ENVIRON_OUT_LOG,1);//1����ǰĿ¼�´���log�������ڵ�ǰ·����log�ļ���ʱ������log�ļ�����

	//2����������Ϣ��Open����������
    if (bLoadSettings)
    {
        std::string strLogPath(cfgSettings.szDataDir, 0, sizeof(cfgSettings.szDataDir)-1);
        strLogPath += "��־�ļ�.txt";

        g_fsLog.open(strLogPath.c_str(), std::ios_base::out);
        if (!g_fsLog.is_open())
        {
            Print("����־�ļ�: %s ʧ�ܣ�һ��������ͬʱ���ж�ݣ�������ļ���,�� DataDir ����ָ��ͬĿ¼��\n��Enter��������\n", strLogPath.c_str());
            getchar();
            return 0;
        }
    }
    else
    {
        Print("��������ʧ�ܣ����������ļ�:%s���ڣ���ip,port,user,password�Ѿ����ã�\n��Enter��������\n", strConfigFile.c_str());
        getchar();
        return 0;
    }
    
    //assert(g_fsLog.is_open());

    if (!InitDataFiles(cfgSettings.szDataDir))
    {
        Print("�� Enter ���˳�����!\n");
        getchar();
        return 0;
    }
    
	TDF_OPEN_SETTING_EXT open_settings_ext = {0};
	int nValidServerCount = cfgSettings.ServerCount();
    LoadTDFSettings(cfgSettings, open_settings_ext);
	open_settings_ext.nServerNum = nValidServerCount;

    g_nMaxBuf = cfgSettings.nMaxMemBuf;
    g_nMaxWriteTimeGap = cfgSettings.nMaxWriteTimeGap;
    g_nOutputDevice = cfgSettings.nOutputDevice;
    g_nCommonLogGap = cfgSettings.nCommonLogGap;
    g_nOutputCodeTable = cfgSettings.bOutputCodeTable;

    TDF_ERR nErr = TDF_ERR_SUCCESS;

	//3��Open
    if (cfgSettings.bEnableProxy)
    {
        TDF_PROXY_SETTING proxy_settings = {(TDF_PROXY_TYPE)0};
        LoadProxyTDFSettings(cfgSettings, proxy_settings);
        g_hTDF = TDF_OpenProxyExt(&open_settings_ext, &proxy_settings, &nErr);
    }
    else
    {
		g_hTDF = TDF_OpenExt(&open_settings_ext , &nErr);
    }


    Print("TDF_Open returned:%s\n", GetErrStr(nErr));

	//4��Open��������
    while (nErr == TDF_ERR_NETWORK_ERROR)
    {
        Print("TDF_Open ����ʧ�ܣ�3��֮������\n");
        Sleep(3*1000);

        if (cfgSettings.bEnableProxy)
        {
            TDF_PROXY_SETTING proxy_settings = {(TDF_PROXY_TYPE)0};
            LoadProxyTDFSettings(cfgSettings, proxy_settings);
            g_hTDF = TDF_OpenProxyExt(&open_settings_ext, &proxy_settings, &nErr);
        }
        else
        {
            g_hTDF = TDF_OpenExt(&open_settings_ext, &nErr);
        }

        Print("TDF_Open returned:%s\n", GetErrStr(nErr));
    }

	//5��д�����̣߳��յ��������ڸ��̴߳���д�ļ�
    HANDLE hSubWriteThread = NULL;
    if (g_hTDF)
    {
        g_hEventDataArrived = CreateEvent(NULL, FALSE, FALSE, NULL);
        g_hSubWriteThread = CreateThread(NULL, 0, SubWriteThread, NULL, 0, 0);
    }

	/*while(true)
	{
		Sleep(5*1000);
		ConStatInfo consInfo;
		int nRet = TDF_GetConStatInfo(g_hTDF,0,&consInfo);
		if( nRet == 0)
		{
			printf("ilastConTime::%I64d,iTotalPacks:%I64d,iTotalTraffic:%I64d,nToalConTimes:%d\n",consInfo.ilastConTime,consInfo.iTotalPacks,consInfo.iTotalTraffic,consInfo.nToalConTimes);
		}else
		{
			printf("TDF_GetConStatInfo Error:%d, ConsTimes:%d\n",nRet,consInfo.nToalConTimes);
		}

		nRet = TDF_GetConStatInfo(g_hTDF,1,&consInfo);
		if( nRet == 0)
		{
			printf("---------ilastConTime::%I64d,iTotalPacks:%I64d,iTotalTraffic:%I64d,nToalConTimes:%d\n",consInfo.ilastConTime,consInfo.iTotalPacks,consInfo.iTotalTraffic,consInfo.nToalConTimes);

		}else
		{
			printf("---------TDF_GetConStatInfo Error:%d\n",nRet);
		}

	}*/
 	//Print("������Enter ��������!\n");
	//getchar();
	
#if 0//6���ο����ݻص�����
	TDF_OPEN_REFDATA_SETTING open_settings_ref = {0};
	open_settings_ref.pfnRefDataHandler = RecvRefData;
	HANDLE g_hTDF2 = TDF_Open_RefData(&open_settings_ref,&nErr);
	if(g_hTDF2)
		TDF_ReqETFList(g_hTDF2,1,"SH","518800",20161230,1);
#endif

#if 1
	Print("������q��������!\n");
	Print("\td ɾ��һ������\n");
	Print("\ts ���ö���\n");
	Print("\tc �������\n");
	Print("\ta ��Ӷ���\n");

	char szCmd[128];
	std::cin >> szCmd;

	auto split = [](char* str, const char* sc)->std::vector<std::string>{
		std::vector<std::string> ret;
		char* nextToken = nullptr;
		auto s = strtok_s(str, sc, &nextToken);
		while(s != NULL)
		{
			ret.push_back(s);
			s = strtok_s(nullptr, sc, &nextToken);
		}

		return ret;
	};

	auto tostr = [](std::vector<std::string> vec)->std::string{
		std::string ret;
		for (int i = 1; i < vec.size(); ++i)
		{
			ret += vec[i];
			ret += ";";
		}
		return ret;
	};
	//��ȡ�����DOS����:"o,SH-2-0"
	auto outCodeTable = [&](const char* szMarket){
		std::fstream f;
		char szFile[255];
		sprintf(szFile, "data\\outCD_%s.csv", szMarket);
		f.open(szFile, std::ios_base::out);

		f << "szMarket,szWindType,szWindCode,szCode,szENName,szCNName,nType,nRecord,nLotSize" << std::endl;
		TDF_CODE* pCodeTable; 
		unsigned int nItems;
		TDF_GetCodeTable(g_hTDF, szMarket, &pCodeTable, &nItems);

		std::for_each(pCodeTable, pCodeTable+nItems, [&](TDF_CODE& code){
			f << code.szMarket << ",";
			f << code.szWindType << ",";
			f << code.szWindCode << ",";
			f << code.szCode << ",";
			f << code.szENName << ",";
			f << code.szCNName << ",";
			f << code.nType << ",";
			f << code.nRecord << ",";
			f << code.nLotSize << std::endl;
		});

		f.close();
		TDF_FreeArr(pCodeTable);
	}; 
	//��ȡ��Ȩ�����DOS����:"g,10000672.SH,SH-1-1"
	auto getCodeInfo = [&](const char* szCode, const char* szMarket){
		TDF_OPTION_CODE code;
		if (TDF_ERR_SUCCESS == TDF_GetOptionCodeInfo(g_hTDF, szCode, &code, szMarket))
		{
			std::cout << "szMarket:" << code.basicCode.szMarket << std::endl;
			std::cout << "szWindType:" << code.basicCode.szWindType << std::endl;
			std::cout << "szWindCode:" << code.basicCode.szWindCode << std::endl;
			std::cout << "szCode:" << code.basicCode.szCode << std::endl;
			std::cout << "szENName:" << code.basicCode.szENName << std::endl;
			std::cout << "szCNName:" << code.basicCode.szCNName << std::endl;
			std::cout << "nType:" << code.basicCode.nType << std::endl;

			std::cout << "szContractID:" << code.szContractID << std::endl;
			std::cout << "szUnderlyingSecurityID:" << code.szUnderlyingSecurityID << std::endl;
			std::cout << "chCallOrPut:" << code.chCallOrPut           << std::endl;
			std::cout << "nExerciseDate:" << code.nExerciseDate << std::endl;
			std::cout << "chUnderlyingType:" << code.chUnderlyingType	 << std::endl;
			std::cout << "chOptionType:" << code.chOptionType << std::endl;
			std::cout << "chPriceLimitType:" << code.chPriceLimitType      <<std::endl;
			std::cout << "nContractMultiplierUnit:" << code.nContractMultiplierUnit << std::endl;
			std::cout << "nExercisePrice:" << code.nExercisePrice        << std::endl;
			std::cout << "nStartDate:" << code.nStartDate          << std::endl;
			std::cout << "nEndDate:" << code.nEndDate           << std::endl;
			std::cout << "nExpireDate:" << code.nExpireDate << std::endl;
		}
		else
		{
			std::cout << "��ȡ" << szCode << "������Ϣʧ��" << std::endl;
		}
	};
	auto cmds = split(szCmd, ",");
	while(cmds.size() > 0 && cmds[0] != "q")
	{
		if (cmds.size() > 1)
		{
			if (cmds[0] == "d")
			{
				for (int i = 1; i < cmds.size(); ++i)
				{
					TDF_SetSubscription(g_hTDF, cmds[i].c_str(), SUBSCRIPTION_DEL);
				}
			}
			else if (cmds[0] == "a")
			{
				for (int i = 1; i < cmds.size(); ++i)
				{
					TDF_SetSubscription(g_hTDF, cmds[i].c_str(), SUBSCRIPTION_ADD);
				}
			}
			else if (cmds[0] == "s")
			{
				TDF_SetSubscription(g_hTDF, tostr(cmds).c_str(), SUBSCRIPTION_SET);
			}
			else if (cmds[0] == "c")
			{
				TDF_SetSubscription(g_hTDF, "", SUBSCRIPTION_FULL);
			}
			else if (cmds[0] == "o")
			{
				for (int i = 1; i < cmds.size(); ++i)
					outCodeTable(cmds[i].c_str());
			}
			else if (cmds[0] == "g")
			{
				getCodeInfo(cmds[1].c_str(), cmds[2].c_str());
			}
		}
		else if(cmds.size() > 0 && cmds[0] == "c")
			TDF_SetSubscription(g_hTDF, "", SUBSCRIPTION_FULL);

		Print("������q��������!\n");
		std::cin >> szCmd;
		cmds = split(szCmd, ",");
	}
#endif
    if (g_hTDF)
    {
		
        g_nQuitSubThread = 1;
        SetEvent(g_hEventDataArrived);
        WaitForSingleObject(g_hSubWriteThread, INFINITE);
        CloseHandle(g_hSubWriteThread);
        g_hSubWriteThread = NULL;
        TDF_Close(g_hTDF);

        g_hTDF = NULL;
        CloseHandle(g_hEventDataArrived);
        g_hEventDataArrived = NULL;
    }
	if (g_pMarketBlock)
	{
		//DumpMarketInfo((DataBlockHead*)g_pMarketBlock);
		delete[] (char*)g_pMarketBlock;
		g_pMarketBlock = NULL;
	}

	if (g_pFutureBlock)
	{
		//DumpFutureInfo((DataBlockHead*)g_pFutureBlock);
		delete [](char*)g_pFutureBlock;
		g_pFutureBlock = NULL;
	}

	if (g_pIndexBlock)
	{
		//DumpIndexInfo((DataBlockHead*)g_pIndexBlock);
		delete [](char*)g_pIndexBlock;
		g_pIndexBlock = NULL;
	}

	if (g_pTransactionBlock)
	{
		//DumpTransactionInfo((DataBlockHead*)g_pTransactionBlock);
		delete [](char*)g_pTransactionBlock;
		g_pTransactionBlock = NULL;
	}

	if (g_pOrderBlock)
	{
		//DumpOrder((DataBlockHead*)g_pOrderBlock);
		delete [](char*)g_pOrderBlock;
		g_pOrderBlock = NULL;
	}

	if (g_pOrderQueueBlock)
	{
		//DumpOrderQueue((DataBlockHead*)g_pOrderQueueBlock);
		delete [](char*)g_pOrderQueueBlock;
		g_pOrderQueueBlock = NULL;
	}

    g_fsMarket.close();
	//g_fsMarket2.close();
    g_fsFuture.close();
    g_fsIndex.close();
    g_fsOrderQueue.close();
    g_fsOrder.close();
    g_fsTransaction.close();
    g_fsOption.close();
	g_fsBBQTransaction.close();
	g_fsBBQBid.close();
	g_fsBrokerQueue.close();

    Print("Close complete!\n");
    g_fsLog.close();
    return 0;
}


template<class InfoType, class DataType, TDF_MSG_ID nMsgId>
void DoRecv(THANDLE hTdf, unsigned int nItemCount, unsigned int nItemSize, __time64_t* nLastUpdateTime, char** ppDataBlock, void* pDataSource, unsigned int nServerTime)
{
    //assert(ppDataBlock);
    //assert(nItemSize == sizeof(DataType));
    DataBlockHead* pBlockHead = (DataBlockHead*)*ppDataBlock;
    unsigned int nAppendSize = GetInfoSize<InfoType, DataType>(nItemCount);

    if (!*ppDataBlock ||  nAppendSize + pBlockHead->m_nOffset > pBlockHead->m_nTotalSize)
    {
        if (*ppDataBlock)
        {
            PushToListAndNotify(*ppDataBlock, g_listDataBlock, g_lockDataBlockList, g_hEventDataArrived);
            *nLastUpdateTime = ::_time64(NULL);
        }
        unsigned int nSizeNeeded = GetInfoTotalSize<InfoType, DataType>(nItemCount);
        unsigned int nSizeAllocated = nSizeNeeded > g_nMaxBuf ? nSizeNeeded : g_nMaxBuf;
        *ppDataBlock = new char[nSizeAllocated];
        //Print("--------------new data block:0x%x--------------\n", *ppDataBlock);
        InitDataBlockHead<InfoType>(&pBlockHead, *ppDataBlock, nSizeAllocated);
        pBlockHead->hTdf = hTdf;
        pBlockHead->nMsgId = (TDF_MSG_ID)nMsgId;
    }

    InfoType* pInfo = (InfoType*)& (*ppDataBlock) [pBlockHead->m_nOffset];
    pInfo->nItems = nItemCount;
    pInfo->nServerTime = nServerTime;
    GetLocalTime(&pInfo->st);

    memcpy(pInfo->rawData, pDataSource, nItemSize*nItemCount);
    pBlockHead->m_nOffset += nAppendSize;

    if (pBlockHead->m_nOffset > pBlockHead->m_nTotalSize)
    {
        //assert(0);
    }
    else if (pBlockHead->m_nOffset == pBlockHead->m_nTotalSize || pBlockHead->m_nOffset > pBlockHead->m_nTotalSize-sizeof(InfoType))
    {
        PushToListAndNotify(*ppDataBlock, g_listDataBlock, g_lockDataBlockList, g_hEventDataArrived);
        *ppDataBlock = NULL;
        *nLastUpdateTime = ::_time64(NULL);
    }
    else
    {
        unsigned int nCurTick = ::_time64(NULL);
        if (nLastUpdateTime!=0 && nCurTick - *nLastUpdateTime> g_nMaxWriteTimeGap)
        {
            PushToListAndNotify(*ppDataBlock, g_listDataBlock, g_lockDataBlockList, g_hEventDataArrived);
            *ppDataBlock = NULL;
            *nLastUpdateTime = nCurTick;
        }
    }
}

#define GETRECORD(pBase, TYPE, nIndex) ((TYPE*)((char*)(pBase) + sizeof(TYPE)*(nIndex)))

void RecvData(THANDLE hTdf, TDF_MSG* pMsgHead)
{
   if (!pMsgHead->pData)
   {
       //assert(0);
       return ;
   }

   unsigned int nItemCount = pMsgHead->pAppHead->nItemCount;
   unsigned int nItemSize = pMsgHead->pAppHead->nItemSize;

   if (!nItemCount)
   {
       //assert(0);
       return ;
   }
   static __time64_t nLastLogTick = 0;
   switch(pMsgHead->nDataType)
   {
   case MSG_DATA_MARKET:
       {
           //assert(nItemSize == sizeof(TDF_MARKET_DATA));

           static __time64_t nLastUpdateTime = 0;

           if (g_nOutputDevice & OUTPUTTYPE_FILE)
           {
               DoRecv<MarketInfo, TDF_MARKET_DATA, MSG_DATA_MARKET>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pMarketBlock, pMsgHead->pData, pMsgHead->nServerTime);
           }

           if(g_nOutputDevice & OUTPUTTYPE_SCREN )
           {
               DumpScreenMarket((TDF_MARKET_DATA*)pMsgHead->pData, nItemCount, pMsgHead->nServerTime);
           }
           else
           {
               TDF_MARKET_DATA* pLastMarket = GETRECORD(pMsgHead->pData,TDF_MARKET_DATA, nItemCount-1);
               DISPLAY_TIME_GAP(g_nCommonLogGap, nLastLogTick, "���յ������¼:����(WindCode)��%s, ҵ������(ActionDay):%d, ʱ��(Time):%d, ���¼�(Match):%I64d���ɽ�����(Volume):%I64d, Type:%d \n", pLastMarket->szWindCode, pLastMarket->nActionDay, pLastMarket->nTime, pLastMarket->nMatch, pLastMarket->iVolume,pLastMarket->pCodeInfo->nType);

           }
       }
       break;
   case MSG_DATA_FUTURE:
       {
           static __time64_t nLastUpdateTime = 0;
           
		   int size = sizeof(TDF_FUTURE_DATA);

           //assert(nItemSize == sizeof(TDF_FUTURE_DATA));
           
           if (g_nOutputDevice & OUTPUTTYPE_FILE)
           {
               DoRecv<FutureInfo, TDF_FUTURE_DATA, MSG_DATA_FUTURE>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pFutureBlock, pMsgHead->pData, pMsgHead->nServerTime);
           }

           if (g_nOutputDevice & OUTPUTTYPE_SCREN)
           {
               DumpScreenFuture((TDF_FUTURE_DATA*)pMsgHead->pData, nItemCount);
           }
           else
           {
               TDF_FUTURE_DATA* pLastFuture = GETRECORD(pMsgHead->pData,TDF_FUTURE_DATA, nItemCount-1);
               DISPLAY_TIME_GAP(g_nCommonLogGap, nLastLogTick, "���յ��ڻ������¼:����(WindCode)��%s, ҵ������(ActionDay):%d, ʱ��(Time):%d, ���¼�(Match):%I64d���ֲ�����(OpenInterest):%I64d \n", pLastFuture->szWindCode, pLastFuture->nActionDay, pLastFuture->nTime, pLastFuture->nMatch, pLastFuture->iOpenInterest);
           }
       }
       break;

   case MSG_DATA_INDEX:
       {
           static __time64_t nLastUpdateTime = 0;

           if (g_nOutputDevice & OUTPUTTYPE_FILE)
           {
               DoRecv<IndexInfo, TDF_INDEX_DATA, MSG_DATA_INDEX>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pIndexBlock, pMsgHead->pData, pMsgHead->nServerTime);
           }
           
           if (g_nOutputDevice & OUTPUTTYPE_SCREN)
           {
               DumpScreenIndex((TDF_INDEX_DATA*)pMsgHead->pData, nItemCount);
           }
           else
           {
               TDF_INDEX_DATA* pLastIndex = GETRECORD(pMsgHead->pData,TDF_INDEX_DATA, nItemCount-1);
               DISPLAY_TIME_GAP(g_nCommonLogGap,nLastLogTick, "���յ�ָ����¼:����(WindCode)��%s, ҵ������(ActionDay):%d, ʱ��(Time):%d, ����ָ��(LastIndex):%I64d���ɽ�����(TotalVolume):%I64d \n", pLastIndex->szWindCode, pLastIndex->nActionDay, pLastIndex->nTime, pLastIndex->nLastIndex, pLastIndex->iTotalVolume);
           }

           
       }
       break;
   case MSG_DATA_TRANSACTION:
       {
           static __time64_t nLastUpdateTime;
		   TDF_TRANSACTION* pLastTransaction = (TDF_TRANSACTION*)pMsgHead->pData;

           if (g_nOutputDevice & OUTPUTTYPE_FILE)
           {
               DoRecv<TransactionInfo, TDF_TRANSACTION, MSG_DATA_TRANSACTION>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pTransactionBlock, pMsgHead->pData, pMsgHead->nServerTime);
           }

           if (g_nOutputDevice & OUTPUTTYPE_SCREN)
           {
               DumpScreenTransaction((TDF_TRANSACTION*)pMsgHead->pData, nItemCount);

           }
           else
           {
               TDF_TRANSACTION* pLastTransaction = GETRECORD(pMsgHead->pData,TDF_TRANSACTION, nItemCount-1);
               DISPLAY_TIME_GAP(g_nCommonLogGap, nLastLogTick, "���յ���ʳɽ���¼:����(WindCode)��%s, ҵ������(ActionDay):%d, ʱ��(Time):%d, �ɽ��۸�(Price):%I64d���ɽ�����(Volume):%d \n", pLastTransaction->szWindCode, pLastTransaction->nActionDay, pLastTransaction->nTime, pLastTransaction->nPrice, pLastTransaction->nVolume);
           }
       }
       break;
   case MSG_DATA_ORDERQUEUE:
       {

           static __time64_t nLastUpdateTime;

           if (g_nOutputDevice & OUTPUTTYPE_FILE)
           {
               DoRecv<OrderQueueInfo, TDF_ORDER_QUEUE, MSG_DATA_ORDERQUEUE>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pOrderQueueBlock, pMsgHead->pData, pMsgHead->nServerTime);
           }
           
           if (g_nOutputDevice & OUTPUTTYPE_SCREN)
           {
               DumpScreenOrderQueue((TDF_ORDER_QUEUE*)pMsgHead->pData, nItemCount);
           }
           else
           {
               TDF_ORDER_QUEUE* pLastOrderQueue = GETRECORD(pMsgHead->pData,TDF_ORDER_QUEUE, nItemCount-1);
               DISPLAY_TIME_GAP(g_nCommonLogGap, nLastLogTick, "���յ�ί�ж��м�¼:����(WindCode)��%s, ҵ������(ActionDay):%d, ʱ��(Time):%d, ί�м۸�(Price):%I64d����������(Orders):%d \n", pLastOrderQueue->szWindCode, pLastOrderQueue->nActionDay, pLastOrderQueue->nTime, pLastOrderQueue->nPrice, pLastOrderQueue->nOrders);
           }
       }
       break;
   case MSG_DATA_ORDER:
       {


		   TDF_ORDER* pData = (TDF_ORDER*)pMsgHead->pData;
		   if (nItemCount > 1)
		   {
			   TDF_ORDER data = pData[1];
		   }

           static __time64_t nLastUpdateTime = 0;

           if (g_nOutputDevice & OUTPUTTYPE_FILE)
           {
               DoRecv<OrderInfo, TDF_ORDER, MSG_DATA_ORDER>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pOrderBlock, pMsgHead->pData, pMsgHead->nServerTime);
           }
           
           if (g_nOutputDevice & OUTPUTTYPE_SCREN)
           {
               DumpScreenOrder((TDF_ORDER*)pMsgHead->pData, nItemCount);
           }
           else
           {
               TDF_ORDER* pLastOrder = GETRECORD(pMsgHead->pData,TDF_ORDER, nItemCount-1);
               DISPLAY_TIME_GAP(g_nCommonLogGap,nLastLogTick,  "���յ����ί�м�¼:����(WindCode)��%s, ҵ������(ActionDay):%d, ʱ��(Time):%d, ί�м۸�(Price):%I64d��ί������(Volume):%d \n", pLastOrder->szWindCode, pLastOrder->nActionDay, pLastOrder->nTime, pLastOrder->nPrice, pLastOrder->nVolume);
           }
           
       }
       break;
   case MSG_DATA_BBQTRANSACTION:
	   {
		   //assert(nItemSize == sizeof(TDF_BBQTRANSACTION_DATA));
		   static __time64_t nLastUpdateTime = 0;

		   if (g_nOutputDevice & OUTPUTTYPE_FILE)
		   {
			   DoRecv<BBQTransactionInfo, TDF_BBQTRANSACTION_DATA, MSG_DATA_BBQTRANSACTION>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pBBQStepBlock, pMsgHead->pData, pMsgHead->nServerTime);
		   }		  
		   for (int i=0; i<nItemCount; i++)
		   {
			   TDF_BBQTRANSACTION_DATA* pLastMarket = GETRECORD(pMsgHead->pData,TDF_BBQTRANSACTION_DATA, i);
			   printf( "MSG_DATA_BBQTRANSACTION: szWindCode:%s nActionDay:%d nTime:%d nDoneID:%d nPrice:%I64d  chPriceStatus:%c chStatus:%c chDirection:%c chSource:%c chSpecialFlag:%c\n", 
				   pLastMarket->szWindCode, pLastMarket->nActionDay, pLastMarket->nTime, pLastMarket->nDoneID, pLastMarket->nPrice,
				   pLastMarket->chPriceStatus, pLastMarket->chStatus,
				   pLastMarket->chDirection,pLastMarket->chSource,pLastMarket->chSpecialFlag);
		   }
	   }
	   break;
   case MSG_DATA_BBQBID:
	   {
		   //assert(nItemSize == sizeof(TDF_BBQBID_DATA));
		   static __time64_t nLastUpdateTime = 0;

		   if (g_nOutputDevice & OUTPUTTYPE_FILE)
		   {
			   DoRecv<BBQBidInfo, TDF_BBQBID_DATA, MSG_DATA_BBQBID>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pBBQBidBlock, pMsgHead->pData, pMsgHead->nServerTime);
		   }		 
		   for (int i=0; i<nItemCount; i++)
		   {
			   TDF_BBQBID_DATA* pLastMarket = GETRECORD(pMsgHead->pData,TDF_BBQBID_DATA, i);
			   printf( "MSG_DATA_BBQBID: %s %d %d,chSource(%d(%c)),nBidPrice(%I64d),nBidVolume(%I64d),chBidPriceStatus(%d(%c)),chIsBid(%d(%c)),chBidSpecialFlag(%d(%c)),chBidStatus(%d(%c))," 
				   "nOfrPrice(%I64d),nOfrVolume(%I64d),chOfrPriceStatus(%d(%c)),chIsOfr(%d(%c)),chOfrSpecialFlag(%d(%c)),chOfrStatus(%d(%c)),nDoneID\n\n", 
				   pLastMarket->szWindCode, pLastMarket->nActionDay, pLastMarket->nTime, pLastMarket->chSource,pLastMarket->chSource, 
				   pLastMarket->nBidPrice, pLastMarket->nBidVolume, pLastMarket->chBidPriceStatus, pLastMarket->chBidPriceStatus,
				   pLastMarket->chIsBid, pLastMarket->chIsBid, pLastMarket->chBidSpecialFlag, pLastMarket->chBidSpecialFlag, pLastMarket->chBidStatus,pLastMarket->chBidStatus,
				   pLastMarket->nOfrPrice,  pLastMarket->nOfrVolume, pLastMarket->chOfrPriceStatus, pLastMarket->chOfrPriceStatus,
				   pLastMarket->chIsOfr, pLastMarket->chIsOfr, pLastMarket->chOfrSpecialFlag, pLastMarket->chOfrSpecialFlag, pLastMarket->chOfrStatus, pLastMarket->chOfrStatus, pLastMarket->nDoneID);
		   }
	   }
	   break;
#if 0//�۹�ͨ����Ϣ
   case MSG_DATA_NON_MD:
	   {
		   TDF_NON_MD_DATA* pNonMDMsg = (TDF_NON_MD_DATA*)pMsgHead->pData;
		   int nItemSize = pNonMDMsg->nItemSize;
		   int nType = pNonMDMsg->nMsgType;
		   int nItems = pMsgHead->pAppHead->nItemCount;
		   char* pTemp = new char[nItemSize];
		   memcpy(pTemp, pNonMDMsg->pData, nItemSize);
 		   MsgDecoder msgDecoder;
 		   msgDecoder.Init(pTemp, nItemSize);
 		   static ofstream sse_amount("D:\\TD_SSE_HK_Amount2.txt");
 		   sse_amount << msgDecoder.GetInt64Field(0) <</*< "," << msgDecoder.GetInt32Field(1) << "," << msgDecoder.GetInt64Field(2) << "," << msgDecoder.GetInt64Field(3) << "," << msgDecoder.GetCharField(4) << */endl;
		   printf("volume:%I64d,date:%d,time:%d\n\n", msgDecoder.GetInt64Field(0),msgDecoder.GetInt32Field(1),msgDecoder.GetInt32Field(2));
		   delete[] pTemp;
	   }
	   break;
#endif
#if 0//ԭʼ�ṹ�汾�������û�������
   case  MSG_DATA_ORIGINAL:
	   {
		   TDF_ORIDATA_MSG * pOriMsg = (TDF_ORIDATA_MSG*)pMsgHead->pData;
		   TDMarketData_Packet* pMd = &pOriMsg->marketData;//�ýṹ�������������ֶ�,ΪTD�ں��ֶΣ����г��ʹ��������޹�
		   static int nCount = 0;
		   if (0 == nCount%100)
		   {
		      Print("%s,%d-%d:%d:%d,match=%I64d\n", pMd->chWindCode, pMd->nDate?pMd->nDate:pMd->nTradingDate, pMd->nTime/3600, pMd->nTime%3600/60, pMd->nTime%60, pMd->nMatch);
		    }
		     nCount++;
#if 1
		   if (pOriMsg->eMsgDataType == ORI_ORDERQUEUE)
		   {
			   //ί�ж��ж�Ӧ�ṹΪpMd->OrderQueue.Order��
			   //�۹�Ϊ�����̶���pMd->OrderQueue.Broker
			   //�۹ɾ����̶���
			   if (!strcmp(pOriMsg->marketKey,"HK-2-0"))
			   {
				   const TD_BrokerQueue_Packet* pBrokerQueue = &pMd->OrderQueue.Broker;
				   //�����̶���
				   if (pBrokerQueue->nAskBrokers > 0)
				   {
					   //'A'
					   const int nMaxBufSize = 3072;
					   char szBuf[nMaxBufSize];
					   char szBufSmaller1[2500];
					   _snprintf(szBuf, nMaxBufSize,
						   "%c,%d,%d,%d,%s"
						   ,
						   'A',
						   TranslateNsdTimeMinisecond(pBrokerQueue->nAskTime, 0),
						   pBrokerQueue->nAskBrokers, 
						   TranslateNsdTimeMinisecond(pMd->nTime,0),
						   shortarr2str(szBufSmaller1, sizeof(szBufSmaller1), pBrokerQueue->sAskBroker, pBrokerQueue->nAskBrokers > 40 ? 40 : pBrokerQueue->nAskBrokers, true)
						   );

				   }
				   if (pBrokerQueue->nBidBrokers > 0)
				   {
					   //'B'
					   const int nMaxBufSize = 3072;
					   char szBuf[nMaxBufSize];
					   char szBufSmaller1[2500];
					   _snprintf(szBuf, nMaxBufSize,
						   "%c,%d,%d,%d,%s"
						   ,
						   'B',
						   TranslateNsdTimeMinisecond(pBrokerQueue->nBidTime,0), 
						   pBrokerQueue->nBidBrokers, 
						   TranslateNsdTimeMinisecond(pMd->nTime,0),
						   shortarr2str(szBufSmaller1, sizeof(szBufSmaller1), pBrokerQueue->sBidBroker, pBrokerQueue->nBidBrokers, true)
						   );

				   }
			   }
			   else
			   {
				   //����ί�ж���
			   }
		   }
		   switch(pMd->nType&0xf0)
		   {
		   case ID_BASECLASS_INDEX:
			   //ָ��
			   break;
		   case ID_BASECLASS_FUTURES:
			   //�ڻ�
			   break;
		   case ID_BASECLASS_OPTIONS:
			   //��Ȩ
			   break;
		   default:
			   //����
			   break;
		   }
#endif
	   }
	break;
#endif
#if 0
   case MSG_DATA_OTC_OPTION:
	   {
		   static __time64_t nLastUpdateTime;

		   if (g_nOutputDevice & OUTPUTTYPE_FILE)
		   {
			   DoRecv<OTCInfo, TDF_OTC_Option, MSG_DATA_OTC_OPTION>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pOTCOptionBlock, pMsgHead->pData, pMsgHead->nServerTime);
		   }

		   if (g_nOutputDevice & OUTPUTTYPE_SCREN)
		   {
			   DumpScreenOtcOption((TDF_OTC_Option*)pMsgHead->pData, nItemCount);
		   }
		   else
		   {
			   TDF_OTC_Option* pLastOTCOption = GETRECORD(pMsgHead->pData,TDF_OTC_Option, nItemCount-1);
			   DISPLAY_TIME_GAP(g_nCommonLogGap, nLastLogTick, "���յ�������Ȩ��¼:���룺%s, ʱ��:%d, �ɽ��۸�:%d���ֲ���:%d \n", pLastOTCOption->szWindCode, pLastOTCOption->nTime, pLastOTCOption->iMatch, pLastOTCOption->nPosition);
		   }
	   }
	   break;
#endif
	   //�۹ɾ����̶���
   case MSG_DATA_BROKERQUEUE:
	   {
		   static __time64_t nLastUpdateTime;

		   if (g_nOutputDevice & OUTPUTTYPE_FILE)
		   {
			   DoRecv<BrokerQueueInfo, TD_BrokerQueue, MSG_DATA_BROKERQUEUE>(hTdf, nItemCount, nItemSize, &nLastUpdateTime, &g_pBrokerQueueBlock, pMsgHead->pData, pMsgHead->nServerTime);
		   }

		   if (g_nOutputDevice & OUTPUTTYPE_SCREN)
		   {
			   DumpScreenBrokerQueue((TD_BrokerQueue*)pMsgHead->pData, nItemCount);
		   }
		   else
		   {
			   TD_BrokerQueue* pLastBrokerQueue = GETRECORD(pMsgHead->pData,TD_BrokerQueue, nItemCount-1);
			   DISPLAY_TIME_GAP(g_nCommonLogGap, nLastLogTick, "���յ������̶��м�¼:���룺%s, ҵ������:%d, ����ʱ��:%d, ����ʱ��:%d���������͸���:%d, ���򾭼͸���:%d \n", pLastBrokerQueue->szWindCode, pLastBrokerQueue->nActionDay, pLastBrokerQueue->nAskTime, pLastBrokerQueue->nBidTime, pLastBrokerQueue->nAskBrokers, pLastBrokerQueue->nBidBrokers);
		   }
	   }
	   break;
   default:
       {
           //assert(0);
       }
       break;
   }
}

void RecvSys(THANDLE hTdf, TDF_MSG* pSysMsg)
{
    if (!pSysMsg ||! hTdf)
    {
        return;
    }

    switch (pSysMsg->nDataType)
    {
	case MSG_SYS_PACK_OVER:
		{
			TDF_PACK_OVER* pPackOver = (TDF_PACK_OVER*)pSysMsg->pData;
			Print("TDF_PACK_OVER:num:%d,conID:%d\n",pPackOver->nDataNumber,pPackOver->nConID);
		}
		break;
    case MSG_SYS_DISCONNECT_NETWORK:
        {
            Print("����Ͽ�\n");
        }
        break;
    case MSG_SYS_CONNECT_RESULT:
        {
            TDF_CONNECT_RESULT* pConnResult = (TDF_CONNECT_RESULT*)pSysMsg->pData;
            if (pConnResult && pConnResult->nConnResult)
            {
                Print("���� %s:%s user:%s, password:%s �ɹ�!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);			
            }
            else
            {
                Print("���� %s:%s user:%s, password:%s ʧ��!\n", pConnResult->szIp, pConnResult->szPort, pConnResult->szUser, pConnResult->szPwd);
            }
        }
        break;
    case MSG_SYS_LOGIN_RESULT:
        {
            TDF_LOGIN_RESULT* pLoginResult = (TDF_LOGIN_RESULT*)pSysMsg->pData;
            if (pLoginResult && pLoginResult->nLoginResult)
            {
                Print("��½�ɹ���info:%s, nMarkets:%d\n", pLoginResult->szInfo, pLoginResult->nMarkets);
                for (int i=0; i<pLoginResult->nMarkets; i++)
                {
					Print("market:%s, dyn_date:%d\n", pLoginResult->szMarket[i], pLoginResult->nDynDate[i]);
                }
            }
            else
            {
                Print("��½ʧ�ܣ�ԭ��:%s\n", pLoginResult->szInfo);
            }
        }
        break;
    case MSG_SYS_CODETABLE_RESULT://�ڴ˿ɻ�ȡ�����
        {
			g_bCodeTable = true;
            TDF_CODE_RESULT* pCodeResult = (TDF_CODE_RESULT*)pSysMsg->pData;
            if (pCodeResult )
            {
                Print("���յ������info:%s, �г�����:%d\n", pCodeResult->szInfo, pCodeResult->nMarkets);
                for (int i=0; i<pCodeResult->nMarkets; i++)
                {
                    Print("�г�:%s, ���������:%d, ���������:%d\n", pCodeResult->szMarket[i], pCodeResult->nCodeCount[i], pCodeResult->nCodeDate[i]);
					//��ȡ����� 
                    if (g_nOutputCodeTable)
                    {
                        //��ȡ����� 
                        TDF_CODE* pCodeTable; 
                        unsigned int nItems;
                        TDF_GetCodeTable(hTdf, pCodeResult->szMarket[i], &pCodeTable, &nItems);
						printf("���������%d",nItems);
                        for (int i=0; i<nItems; i++)
                        {
                            TDF_CODE& code = pCodeTable[i];
                            Print("windcode:%s, code:%s, market:%s, name:%s, nType:0x%x\n",code.szWindCode, code.szCode, code.szMarket, code.szCNName, code.nType);
                        }

                        TDF_FreeArr(pCodeTable);
                    }
                }
            }
        }
        break;
    case MSG_SYS_QUOTATIONDATE_CHANGE://��ȡ��
		{
			TDF_QUOTATIONDATE_CHANGE* pChange = (TDF_QUOTATIONDATE_CHANGE*)pSysMsg->pData;
			if (pChange)
			{
				Print("�յ��������ڱ��֪ͨ�������Զ���������������%s, ԭ����:%d, �����ڣ�%d\n", pChange->szMarket, pChange->nOldDate, pChange->nNewDate);
			}
		}
        break;
    case MSG_SYS_MARKET_CLOSE://��ȡ��
        {
            TDF_MARKET_CLOSE* pCloseInfo = (TDF_MARKET_CLOSE*)pSysMsg->pData;
            if (pCloseInfo)
            {
                Print("������Ϣ:market:%s, time:%d, info:%s\n", pCloseInfo->szMarket, pCloseInfo->nTime, pCloseInfo->chInfo);
            }
        }
        break;
    case MSG_SYS_HEART_BEAT:
        {
            Print("�յ�������Ϣ\n");
        }
        break;
	case MSG_SYS_SINGLE_CODETABLE_RESULT:
		{
			TDF_SINGLE_CODE_RESULT* pCodeResult = (TDF_SINGLE_CODE_RESULT*)pSysMsg->pData;
			if (pCodeResult)
			{
				Print("�յ��г�%s�����,���������:%d, ���������:%d\n", pCodeResult->szMarket, pCodeResult->nCodeCount, pCodeResult->nCodeDate);
				//�յ�����Ϣ�󣬿ɻ�ȡpCodeResult->szMarket�Ĵ����.
			}
		}
		break;
    default:
        //assert(0);
        break;
    }
}

void LoadTDFSettings(const ConfigSettings& configObj,  TDF_OPEN_SETTING_EXT& settings)
{
	memset(&settings, 0, sizeof(settings));

	for (int i = 0; i < sizeof(settings.siServer)/sizeof(TDF_SERVER_INFO); ++i)
	{
		strncpy(settings.siServer[i].szIp, configObj.si[i].szIP, sizeof(settings.siServer[i].szIp)-1);
		_snprintf(settings.siServer[i].szPort, sizeof(settings.siServer[i].szPort)-1, "%d", configObj.si[i].nPort);
		strncpy(settings.siServer[i].szUser, configObj.si[i].szUser, sizeof(settings.siServer[i].szUser)-1);
		strncpy(settings.siServer[i].szPwd, configObj.si[i].szPwd, sizeof(settings.siServer[i].szPwd)-1);
	}

	//settings.nReconnectCount = configObj.nReconnectCount;
	//settings.nReconnectGap = configObj.nReconnGap;
	settings.pfnMsgHandler = RecvData;
	settings.pfnSysMsgNotify = RecvSys;

	settings.szMarkets = DeepCopy(configObj.szMarketList);
	settings.szResvMarkets = DeepCopy(configObj.szResvMarketList);
	settings.szSubScriptions = DeepCopy(configObj.szSubscriptions);

	settings.nTime = configObj.nTime;

	settings.nTypeFlags = configObj.nDataType;
}
/*
void LoadTDFSettings(const ConfigSettings& configObj,  TDF_OPEN_SETTING& settings)
{
    memset(&settings, 0, sizeof(settings));
    
    strncpy(settings.szIp, configObj.si[0].szIP, sizeof(settings.szIp)-1);
    _snprintf(settings.szPort, sizeof(settings.szPort)-1, "%d", configObj.si[0].nPort);
    strncpy(settings.szUser, configObj.si[0].szUser, sizeof(settings.szUser)-1);
    strncpy(settings.szPwd, configObj.si[0].szPwd, sizeof(settings.szPwd)-1);

    settings.nReconnectCount = configObj.nReconnectCount;
    settings.nReconnectGap = configObj.nReconnGap;

    settings.pfnMsgHandler = RecvData;
    settings.pfnSysMsgNotify = RecvSys;

    settings.nProtocol = 0;
    settings.szMarkets = DeepCopy(configObj.szMarketList);
    settings.szSubScriptions = DeepCopy(configObj.szSubscriptions);
    
    settings.nDate = configObj.nDate;
    settings.nTime = configObj.nTime;

    settings.nTypeFlags = configObj.nDataType;
}
*/
void LoadProxyTDFSettings(const ConfigSettings& configObj, TDF_PROXY_SETTING& proxy_settings)
{
    memset(&proxy_settings, 0, sizeof(proxy_settings));
    proxy_settings.nProxyType = configObj.nProxyType;
    strncpy(proxy_settings.szProxyHostIp, configObj.szProxyHostIp, sizeof(proxy_settings.szProxyHostIp)-1);
    _snprintf(proxy_settings.szProxyPort, sizeof(proxy_settings.szProxyPort)-1, "%d", configObj.nProxyHostPort);
    strncpy(proxy_settings.szProxyUser, configObj.szProxyUser, sizeof(proxy_settings.szProxyUser)-1);
    strncpy(proxy_settings.szProxyPwd, configObj.szProxyPassword, sizeof(proxy_settings.szProxyPwd)-1);
}
char* DeepCopy(const char*szIn)
{
    if (szIn)
    {
        int nLen = strlen(szIn);
        char* pRet = new char[nLen+1];
        pRet[nLen] = 0;
        strncpy(pRet, szIn, nLen+1);
        return pRet;
    }
    else
    {
        return NULL;
    }
}


const char* GetErrStr(TDF_ERR nErr)
{
    std::map<TDF_ERR, const char*> mapErrStr;
    mapErrStr.insert(std::make_pair(TDF_ERR_UNKOWN, "TDF_ERR_UNKOWN"));
    mapErrStr.insert(std::make_pair(TDF_ERR_INITIALIZE_FAILURE, "TDF_ERR_INITIALIZE_FAILURE"));
    mapErrStr.insert(std::make_pair(TDF_ERR_NETWORK_ERROR, "TDF_ERR_NETWORK_ERROR"));
    mapErrStr.insert(std::make_pair(TDF_ERR_INVALID_PARAMS, "TDF_ERR_INVALID_PARAMS"));
    mapErrStr.insert(std::make_pair(TDF_ERR_VERIFY_FAILURE, "TDF_ERR_VERIFY_FAILURE"));
    mapErrStr.insert(std::make_pair(TDF_ERR_NO_AUTHORIZED_MARKET, "TDF_ERR_NO_AUTHORIZED_MARKET"));
    mapErrStr.insert(std::make_pair(TDF_ERR_NO_CODE_TABLE, "TDF_ERR_NO_CODE_TABLE"));
    mapErrStr.insert(std::make_pair(TDF_ERR_SUCCESS, "TDF_ERR_SUCCESS"));
    if (mapErrStr.find(nErr)==mapErrStr.end())
    {
        return "TDF_ERR_UNKOWN";
    }
    else
    {
        return mapErrStr[nErr];
    }
}

DWORD __stdcall SubWriteThread(void* lParam)
{
    while (!g_nQuitSubThread)
    {
        WaitForSingleObject(g_hEventDataArrived, INFINITE);

        g_lockDataBlockList.Lock();
        std::list<char*> listTemp(g_listDataBlock.begin(), g_listDataBlock.end());
        if (g_listDataBlock.size())
        {
            g_listDataBlock.clear();
        }
        g_lockDataBlockList.UnLock();

        for (std::list<char*>::iterator it=listTemp.begin(); it!=listTemp.end(); it++)
        {
            DataBlockHead* pHead = (DataBlockHead*)*it;
            //assert(pHead);

			switch(pHead->nMsgId)
			{
			case MSG_DATA_INDEX:
				DumpIndexInfo(pHead);
				break;
			case MSG_DATA_MARKET:
				DumpMarketInfo(pHead);
				break;
			case MSG_DATA_FUTURE:
				DumpFutureInfo(pHead);
				break;
			case MSG_DATA_TRANSACTION:
				DumpTransactionInfo(pHead);
				break;
			case MSG_DATA_ORDERQUEUE:
				DumpOrderQueue(pHead);
				break;
			case MSG_DATA_ORDER:
				DumpOrder(pHead);
				break;
			case MSG_DATA_BBQTRANSACTION:
				DumpBBQTransaction(pHead);
				break;
			case MSG_DATA_BBQBID:
				DumpBBQBid(pHead);
				break;
			case MSG_DATA_BROKERQUEUE:
				DumpBrokerQueue(pHead);
				break;
			default:
				//assert(0);
				break;
			}
            delete[] (char*)pHead;
            //Print("-----delete data block:0x%x------\n", pHead);
            
        }
    }

    /*if (g_listDataBlock.size())
    {
        //assert(0);
    }*/

    if (g_pMarketBlock)
    {
        DumpMarketInfo((DataBlockHead*)g_pMarketBlock);
       // delete[] (char*)g_pMarketBlock;
       // g_pMarketBlock = NULL;
    }

    if (g_pFutureBlock)
    {
        DumpFutureInfo((DataBlockHead*)g_pFutureBlock);
       // delete [](char*)g_pFutureBlock;
        //g_pFutureBlock = NULL;
    }

    if (g_pIndexBlock)
    {
        DumpIndexInfo((DataBlockHead*)g_pIndexBlock);
        //delete [](char*)g_pIndexBlock;
        //g_pIndexBlock = NULL;
    }

    if (g_pTransactionBlock)
    {
        DumpTransactionInfo((DataBlockHead*)g_pTransactionBlock);
        //delete [](char*)g_pTransactionBlock;
       // g_pTransactionBlock = NULL;
    }

    if (g_pOrderBlock)
    {
        DumpOrder((DataBlockHead*)g_pOrderBlock);
        //delete [](char*)g_pOrderBlock;
       // g_pOrderBlock = NULL;
    }

    if (g_pOrderQueueBlock)
    {
        DumpOrderQueue((DataBlockHead*)g_pOrderQueueBlock);
        //delete [](char*)g_pOrderQueueBlock;
       // g_pOrderQueueBlock = NULL;
    }
	if(g_pBBQStepBlock)
	{
		DumpBBQTransaction((DataBlockHead*)g_pBBQStepBlock);
	}
	if(g_pBBQBidBlock)
	{
		DumpBBQBid((DataBlockHead*)g_pBBQBidBlock);
	}
	
	if(g_pBrokerQueueBlock)
	{
		DumpBrokerQueue((DataBlockHead*)g_pBrokerQueueBlock);
	}
	
	SetEvent(g_hSubWriteThread);
    return 0;
}


void PushToListAndNotify(char* pBuf, std::list<char*>& listQueue, AutoLock& guard, HANDLE hEvent)
{
    guard.Lock();
    listQueue.push_back(pBuf);
    guard.UnLock();
    SetEvent(hEvent);
}

bool OpenStream(std::ofstream& of, const std::string& strPath, const std::string& strLog)
{
    of.open(strPath.c_str(), std::ios_base::out);
    if (!of.is_open())
    {
        Print("%s: %s ʧ�ܣ�\n", strLog.c_str(), strPath.c_str());
        return false;
    }
    return true;
}


bool InitDataFiles(const std::string& strDataDir)
{
    std::string strMarketPath = strDataDir + "����(MarketData).csv";
	std::string strMarketPath2 = strDataDir + "����(MarketData)_no_local_time.csv";
    std::string strFuturePath = strDataDir + "�ڻ�����(Future).csv";
    std::string strFutureOptionPath = strDataDir + "��Ȩ(Option).csv";
    std::string strIndexPath = strDataDir + "ָ��(Index).csv";
    std::string strTransactionPath = strDataDir + "��ʳɽ�(Transaction).csv";
    std::string strOrderPath = strDataDir + "���ί��(Order).csv";
    std::string strOrderQueuePath = strDataDir + "ί�ж���(OrderQueue).csv";
    std::string strBBQTransactionPath = strDataDir + "BBQ�ɽ�.csv";
	std::string strBBQBidPath = strDataDir + "BBQ����.csv";
	std::string strBrokeQueue = strDataDir + "�����̶���(BrokeQueue).csv";
	std::string strOTCOption = strDataDir + "������Ȩ(OTCOption).csv";

    enum DATA_TYPE_OFFSET
    {
        OFFSET_MARKET=0,
		OFFSET_MARKET2,
        OFFSET_FUTURE,
        OFFSET_FUTURE_OPTION,
        OFFSET_INDEX,
        OFFSET_TRANSACTION,
        OFFSET_ORDER,
        OFFSET_ORDERQUEUE,
		OFFSET_BBQTRANSACTION,
		OFFSET_BBQBID,
		OFFSET_BROQUEUE,
		OFFSET_OTCOPTION,
    };
    const char* arrTitle[] = 
    {
		// ��������
		"����, ����ʱ��, ������ʱ��, ������ʱ��, ��ô���, ԭʼ����, ҵ������(��Ȼ��), ������,״̬,ǰ��,���̼�,��߼�,��ͼ�,���¼�,������,������,�����,������,�ɽ�����,�ɽ�����,�ɽ��ܽ��,ί����������,ί����������,��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�,IOPV��ֵ��ֵ,����������,��ͣ��,��ͣ��,֤ȯ��Ϣǰ׺,��ӯ��1,��ӯ��2,����2���Ա���һ�ʣ�,���ױ�־,�̺�۸�,�̺���,�̺�ɽ����,�̺�ɽ�����",
		// no local time����������
		"����, ������ʱ��, ��ô���, ԭʼ����, ҵ������(��Ȼ��), ������,״̬,ǰ��,���̼�,��߼�,��ͼ�,���¼�,������,������,�����,������,�ɽ�����,�ɽ�����,�ɽ��ܽ��,ί����������,ί����������,��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�,IOPV��ֵ��ֵ,����������,��ͣ��,��ͣ��,֤ȯ��Ϣǰ׺,��ӯ��1,��ӯ��2,����2���Ա���һ�ʣ�",
		// �ڻ�����
        "����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��),������,״̬,��ֲ�,�����̼�,�����,���̼�,��߼�,��ͼ�,���¼�,�ɽ�����,�ɽ��ܽ��,�ֲ�����,������,�����,��ͣ��,��ͣ��,����ʵ��,����ʵ��,������,������,�����,������",
		// ��Ȩ����
        "����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��),������,״̬,��ֲ�,�����̼�,�����,���̼�,��߼�,��ͼ�,���¼�,�ɽ�����,�ɽ��ܽ��,�ֲ�����,������,�����,��ͣ��,��ͣ��,����ʵ��,����ʵ��,������,������,�����,������,��Ȩ��Լ����,���֤ȯ����,�Ϲ��Ϲ�,��Ȩ��",
		// ָ��
        "����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��),������,״̬,����ָ��,���ָ��,���ָ��,����ָ��,�ɽ�����,�ɽ��ܽ��,ǰ��ָ��",
		// ��ʳɽ�
        "����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��), �ɽ����,�ɽ��۸�,�ɽ�����,�ɽ����,��������,�ɽ����,�ɽ�����,����ί�����,��ί�����",
		// ���ί��
        "����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��), ί�к�, ί�м۸�, ί������, ί�����, ί�д���,�����̱���,ί��״̬,��־",
		// ί�ж���
        "����,����ʱ��,������ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��), ��������,�ɽ��۸�,��������,��ϸ����,������ϸ",
		// BBQ���
		"����,����ʱ��,������ʱ��,������ʱ��,��ô���,ҵ������(��Ȼ��),�ɽ����۱��,�ɽ��۸�/������,������/�۸��ʶ,ɾ��/�ɽ�,�ɽ�����,���ۻ���,��Ȩ/����",
		// BBQbid
		"����,����ʱ��,������ʱ��,����ʱ��,��ô���,������(��Ȼ��), ���ۻ���,����������/�۸�,������,����������/�۸��ʶ,�Ƿ�bid,��Ȩ/����,�������ű���״̬,����������/�۸�,������,����������/�۸��ʶ,�Ƿ�ofr,��Ȩ/����,�������ű���״̬,�ɽ����۱��",
		// �����̶���
		"����,����ʱ��,������ʱ��,��ô���,ԭʼ����,ҵ������(��Ȼ��),����ʱ��, ����ʱ��, �������͸���, �������͸���,����ǰ40����,����ǰ40����",
		// OTCOP
		"��ô���, ԭʼ����, chOptionType , chCallOrPut,iExercisePrice,nTime,nVolume,iMatch,iPreSettlement,iSettlement,nPrePosition,nPosition,iAskPrice,iBidPrice,nAskVol,nBidVol",
    };

	const char* arrEnTitle[] = 
	{
		// marketdata
		"Date, LocalTime, ServerTime, TradeTime, WindCode, Code, ActionDay, TtradingDay,Status,PreClose,Open,High,Low,Match,AskPrice[10],AskVol[10],BidPrice[10],BidVol[10],NumTrades,Volume,Turnover,TotalBidVol,TotalAskVol,WeightedAvgBidPrice,WeightedAvgAskPrice,IOPV,YieldToMaturity,HighLimited,LowLimited,Prefix,Syl1,Syl2,SD2,TradeFlag,AfterPrice,AfterVolume,AfterTurnover,AfterMatchItems",
		// no local time for marketdata
		"Date, TradeTime, WindCode, Code, ActionDay, TtradingDay,Status,PreClose,Open,High,Low,Match,AskPrice[10],AskVol[10],BidPrice[10],BidVol[10],NumTrades,Volume,Turnover,TotalBidVol,TotalAskVol,WeightedAvgBidPrice,WeightedAvgAskPrice,IOPV,YieldToMaturity,HighLimited,LowLimited,Prefix,Syl1,Syl2,SD2",
		// future data
	"Date,LocalTime,ServerTime,TradeTime,WindCode,Code,ActionDay,TtradingDay,Status,PreOpenInterest,PreClose,PreSettlePrice,Open,High,Low,Match,Volume,Turnover,OpenInterest,Close,SettlePrice,HighLimited,LowLimited,PreDelta,CurrDelta,AskPrice[5],AskVol[5],BidPrice[5],BidVol[5]",
		// option data
	"Date,LocalTime,ServerTime,TradeTime,WindCode,Code,ActionDay,TtradingDay,Status,PreOpenInterest,PreClose,PreSettlePrice,Open,High,Low,Match,Volume,Turnover,OpenInterest,Close,SettlePrice,HighLimited,LowLimited,PreDelta,CurrDelta,AskPrice[5],AskVol[5],BidPrice[5],BidVol[5],ContractID,UnderlyingSecurityID,CallOrPut,ExerciseDate",
		// index data
		"Date,LocalTime,ServerTime,TradeTime,WindCode,Code,ActionDay,TtradingDay,Status,OpenIndex,HighIndex,LowIndex,LastIndex,Volume,Turnover,PreCloseIndex",
		// Transaction
		"Date,LocalTime,ServerTime,TradeTime,WindCode,Code,ActionDay, Index,Price,Volume,Turnover,BSFlag,OrderKind,FunctionCode,AskOrder,BidOrder",
		// Order
		"Date,LocalTime,ServerTime,TradeTime,WindCode,Code,ActionDay, Order, Price, Volume, OrderKind, FunctionCode,Broker,Status,Flag",
		// OrderQueue
		"Date,LocalTime,ServerTime,TradeTime,WindCode,Code,ActionDay, BSFlag,Price,Orders,ABItems,ABVolume[200]",
		// BBQTransaction
		"Date,LocalTime,ServerTime,TradeTime,WindCode,ActionDay,DoneID,Price,PriceStatus,Status,Direction,Source,SpecialFlag",
		// BBQBid
		"Date,LocalTime,ServerTime,Time,WindCode,ActionDay, Source,BidPrice,BidVolume,BidPriceStatus,IsBid,BidSpecialFlag,BidStatus,OfrPrice,OfrVolume,OfrPriceStatus,IsOfr,OfrSpecialFlag,OfrStatus,DoneID",
		// BrokerQueue
		"Date,LocalTime,ServerTime,WindCode,Code,ActionDay,AskTime, BidTime, AskBrokers, BidBrokers,AskBroker[40],BidBroker[40]",
		// OTCOPTION
		"WindCode, Code, OptionType , CallOrPut,ExercisePrice,Time,Volume,Match,PreSettlement,Settlement,PrePosition,Position,AskPrice,BidPrice,AskVol,BidVol",
	};

    if (!OpenStream(g_fsMarket, strMarketPath, "�����������ļ�"))
    {
        return false;
    }
    g_fsMarket<<arrTitle[OFFSET_MARKET]<<std::endl;
	g_fsMarket<<arrEnTitle[OFFSET_MARKET]<<std::endl;

// 	if (!OpenStream(g_fsMarket2, strMarketPath2, "�����������ļ�"))
// 	{
// 		return false;
// 	}
// 	g_fsMarket2<<arrTitle[OFFSET_MARKET2]<<std::endl;
// 	g_fsMarket2<<arrEnTitle[OFFSET_MARKET2]<<std::endl;

    if (!OpenStream(g_fsFuture, strFuturePath, "���ڻ����������ļ�"))
    {
        return false;
    }
    g_fsFuture<<arrTitle[OFFSET_FUTURE]<<std::endl;
	g_fsFuture<<arrEnTitle[OFFSET_FUTURE]<<std::endl;

    if (!OpenStream(g_fsOption, strFutureOptionPath, "����Ȩ�����ļ�"))
    {
        return false;
    }
    g_fsOption<<arrTitle[OFFSET_FUTURE_OPTION]<<std::endl;
	g_fsOption<<arrEnTitle[OFFSET_FUTURE_OPTION]<<std::endl;

    if (!OpenStream(g_fsIndex, strIndexPath, "��ָ�������ļ�"))
    {
        return false;
    }
    g_fsIndex<<arrTitle[OFFSET_INDEX]<<std::endl;
	g_fsIndex<<arrEnTitle[OFFSET_INDEX]<<std::endl;

    if (!OpenStream(g_fsTransaction, strTransactionPath, "����ʳɽ������ļ�"))
    {
        return false;
    }
    g_fsTransaction<<arrTitle[OFFSET_TRANSACTION]<<std::endl;
	g_fsTransaction<<arrEnTitle[OFFSET_TRANSACTION]<<std::endl;

    if (!OpenStream(g_fsOrder, strOrderPath, "�����ί�������ļ�"))
    {
        return false;
    }
    g_fsOrder<<arrTitle[OFFSET_ORDER]<<std::endl;
	g_fsOrder<<arrEnTitle[OFFSET_ORDER]<<std::endl;

    if (!OpenStream(g_fsOrderQueue, strOrderQueuePath, "��ί�ж��������ļ�"))
    {
        return false;
    }
    g_fsOrderQueue<<arrTitle[OFFSET_ORDERQUEUE]<<std::endl;
	g_fsOrderQueue<<arrEnTitle[OFFSET_ORDERQUEUE]<<std::endl;

	if (!OpenStream(g_fsBBQTransaction, strBBQTransactionPath, "��BBQ�ɽ������ļ�"))
	{
		return false;
	}
	g_fsBBQTransaction<<arrTitle[OFFSET_BBQTRANSACTION]<<std::endl;
	g_fsBBQTransaction<<arrEnTitle[OFFSET_BBQTRANSACTION]<<std::endl;

	if (!OpenStream(g_fsBBQBid, strBBQBidPath, "��BBQ���������ļ�"))
	{
		return false;
	}
	g_fsBBQBid<<arrTitle[OFFSET_BBQBID]<<std::endl;
	g_fsBBQBid<<arrEnTitle[OFFSET_BBQBID]<<std::endl;

	if (!OpenStream(g_fsBrokerQueue, strBrokeQueue, "�򿪾����̶��������ļ�"))
	{
		return false;
	}
	g_fsBrokerQueue<<arrTitle[OFFSET_BROQUEUE]<<std::endl;
	g_fsBrokerQueue<<arrEnTitle[OFFSET_BROQUEUE]<<std::endl;

    return true;
}


void GetLastMarketData(THANDLE g_hTDF)
{
	TDF_MARKET_DATA* pMarketData = NULL;
	int nItems = 0;
	//std::ofstream fsMarketData;
	//fsMarketData.open("�����������.csv", std::ios_base::out);

	int nRet = TDF_ERR_SUCCESS; 
	nRet = TDF_GetLastMarketData(g_hTDF, "SH", &pMarketData, &nItems);
	//fsMarketData << "������ʱ��, ��ô���, ԭʼ����, ҵ������(��Ȼ��), ������,״̬,ǰ��,���̼�,��߼�,��ͼ�,���¼�,������,������,�����,������,�ɽ�����,�ɽ�����,�ɽ��ܽ��,ί����������,ί����������,��Ȩƽ��ί��۸�,��Ȩƽ��ί���۸�,IOPV��ֵ��ֵ,����������,��ͣ��,��ͣ��,֤ȯ��Ϣǰ׺,��ӯ��1,��ӯ��2,����2���Ա���һ�ʣ�" << std::endl;
	if (nRet == TDF_ERR_SUCCESS && nItems>0)
	{
		Print("GetLastMarketData Success,nItems = %d\n", nItems);
		//����LastMarketData
		//DumpScreenMarket(pMarketData, nItems, 0);
	}
	else
	{
		Print("GetLastMarketData Failed,nItems = %d\n", nItems);
	}
	for (int i=0;i<nItems;i++)
	{
#if 0
		const int nMaxBufSize = 4096;
		char szBufSmaller1[160];
		char szBufSmaller2[160];
		char szBufSmaller3[160];
		char szBufSmaller4[160];
		char szBufSmaller5[64];
		char szBuf[nMaxBufSize];
		TDF_MARKET_DATA* pTDFMarket = &(pMarketData[i]);
		_snprintf(szBuf, nMaxBufSize, "%d,%s,%s,%d,%d,"
			"%d(%c),"    //status
			"%d,%d,%d,%d,%d,"        //preclose,open, high, low, last,
			"%s,%s,%s,%s,"
			"%d,%I64d,%I64d,%I64d,%I64d," 
			"%u,%u,%d,%d,"
			"%u,%u,%s,"
			"%d,%d,%d"
			, 
			pTDFMarket->nTime, 
			pTDFMarket->szWindCode, pTDFMarket->szCode, pTDFMarket->nActionDay, pTDFMarket->nTradingDay, 
			pTDFMarket->nStatus, SAFE_CHAR(pTDFMarket->nStatus),
			pTDFMarket->nPreClose, pTDFMarket->nOpen, pTDFMarket->nHigh ,pTDFMarket->nLow, pTDFMarket->nMatch,
			intarr2str(szBufSmaller1, sizeof(szBufSmaller1), (int*)pTDFMarket->nAskPrice, ELEM_COUNT(pTDFMarket->nAskPrice)),
			intarr2str(szBufSmaller2, sizeof(szBufSmaller2), (int*)pTDFMarket->nAskVol, ELEM_COUNT(pTDFMarket->nAskVol)),
			intarr2str(szBufSmaller3, sizeof(szBufSmaller3), (int*)pTDFMarket->nBidPrice, ELEM_COUNT(pTDFMarket->nBidPrice)),
			intarr2str(szBufSmaller4, sizeof(szBufSmaller4), (int*)pTDFMarket->nBidVol, ELEM_COUNT(pTDFMarket->nBidVol)),
			pTDFMarket->nNumTrades, pTDFMarket->iVolume, pTDFMarket->iTurnover, pTDFMarket->nTotalBidVol, pTDFMarket->nTotalAskVol, 
			pTDFMarket->nWeightedAvgBidPrice, pTDFMarket->nWeightedAvgAskPrice,pTDFMarket->nIOPV, pTDFMarket->nYieldToMaturity, 
			pTDFMarket->nHighLimited, pTDFMarket->nLowLimited, chararr2str(szBufSmaller5, sizeof(szBufSmaller5), pTDFMarket->chPrefix, ELEM_COUNT(pTDFMarket->chPrefix)),
			pTDFMarket->nSyl1, pTDFMarket->nSyl2, pTDFMarket->nSD2);
		fsMarketData << szBuf << std::endl;
#endif
		TDF_MARKET_DATA* pTDFMarket = &(pMarketData[i]);
		Print("Market Code:%s\n",pTDFMarket->szWindCode);
	}
	if (pMarketData)
	{
		delete[] pMarketData;
		pMarketData = NULL;
	}
	//fsMarketData.close();
}

void GetLastIndexData(THANDLE g_hTDF)
{
	TDF_INDEX_DATA* pIndexData = NULL;
	int nItems = 0;
	int nRet = TDF_ERR_SUCCESS; 

	nRet = TDF_GetLastIndexData(g_hTDF, "SH", &pIndexData, &nItems);
	if (nRet == TDF_ERR_SUCCESS && nItems>0)
	{
		Print("GetLastIndexData Success,nItems = %d\n", nItems);
		//����LastMarketData
	}
	else
	{
		Print("GetLastIndexData Failed,nItems = %d\n", nItems);
	}
	for (int i=0;i<nItems;i++)
	{
		TDF_INDEX_DATA* pTDFIndex = &(pIndexData[i]);
		Print("Index Code:%s\n",pTDFIndex->szWindCode);
	}
	if (pIndexData)
	{
		delete[] pIndexData;
		pIndexData = NULL;
	}
}

void GetLastFutureData(THANDLE g_hTDF)
{
	TDF_FUTURE_DATA* pFutureData = NULL;
	int nItems = 0;
	int nRet = TDF_ERR_SUCCESS; 
	nRet = TDF_GetLastFutureData(g_hTDF, "SH", &pFutureData, &nItems);
	if (nRet == TDF_ERR_SUCCESS && nItems>0 && pFutureData)
	{
		Print("GetLastFutureData Success,nItems = %d\n", nItems);
		//����LastMarketData
	}
	else
	{
		Print("GetLastFutureData Failed,nItems = %d\n", nItems);
	}
	for (int i=0;i<nItems;i++)
	{
		TDF_FUTURE_DATA* pTDFfuture = &(pFutureData[i]);
		Print("Future Code:%s\n", pTDFfuture->szWindCode);
	}
	if (pFutureData)
	{
		delete[] pFutureData;
		pFutureData = NULL;
	}
}

void GetLastSnapShot(THANDLE g_hTDF, const char* market)
{
#if 1
	TDF_CODE* pCode = NULL;
	unsigned int nItems = 0;
	int nRet = TDF_ERR_SUCCESS; 
	{
		TDMarketData_Packet* pMD = NULL;
		nRet = TDF_GetLastSnapShot(g_hTDF, market, (void**)&pMD, &nItems);
		if (nRet == TDF_ERR_SUCCESS)
		{
			Print("TDF_GetLastSnapShot Success,nItems = %d\n", nItems);
			//����LastMarketData
		}
		else
		{
			Print("TDF_GetLastSnapShot Failed,nItems = %d\n", nItems);
		}
		for (int i=0;i<nItems;i++)
		{
			//Print("Code:%s\n", pMD[i].chWindCode);
		}
		TDF_FreeArr(pMD);
	}
#endif
}


void DumpScreenMarket(TDF_MARKET_DATA* pMarket, int nItems, int nTime)
{
	Print("-------- Market, Count:%d --------\n", nItems);

	char szBuf1[512];
	char szBuf2[512];
	char szBuf3[512];
	char szBuf4[512];
	char szBufSmall[64];
	for (int i=0; i<nItems; i++)
	{
		const TDF_MARKET_DATA& marketData = pMarket[i];
		Print("��ô��� szWindCode: %s\n", marketData.szWindCode);
		Print("ԭʼ���� szCode: %s\n", marketData.szCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", marketData.nActionDay);
		Print("������ nTradingDay: %d\n", marketData.nTradingDay);
		Print("ʱ��(HHMMSSmmm) nTime: %d\n", marketData.nTime);
		Print("����ʱ��(HHMMSSmmm) nServerTime: %d\n", nTime);
		Print("״̬ nStatus: %d(%c)\n", marketData.nStatus, SAFE_CHAR(marketData.nStatus));
		Print("ǰ���̼� nPreClose: %I64d\n", marketData.nPreClose);
		Print("���̼� nOpen: %I64d\n", marketData.nOpen);
		Print("��߼� nHigh: %I64d\n", marketData.nHigh);
		Print("��ͼ� nLow: %I64d\n", marketData.nLow);
		Print("���¼� nMatch: %I64d\n", marketData.nMatch);
		Print("������ nAskPrice: %s \n", int64arr2str(szBuf1, sizeof(szBuf1), (int64_t*)marketData.nAskPrice, ELEM_COUNT(marketData.nAskPrice)));

		Print("������ nAskVol: %s \n", int64arr2str(szBuf2, sizeof(szBuf2), (int64_t*)marketData.nAskVol, ELEM_COUNT(marketData.nAskVol)));

		Print("����� nBidPrice: %s \n", int64arr2str(szBuf3, sizeof(szBuf3), (int64_t*)marketData.nBidPrice, ELEM_COUNT(marketData.nBidPrice)));

		Print("������ nBidVol: %s \n", int64arr2str(szBuf4, sizeof(szBuf4), (int64_t*)marketData.nBidVol, ELEM_COUNT(marketData.nBidVol)));

		Print("�ɽ����� nNumTrades: %d\n", marketData.nNumTrades);

		Print("�ɽ����� iVolume: %I64d\n", marketData.iVolume);
		Print("�ɽ��ܽ�� iTurnover: %I64d\n", marketData.iTurnover);
		Print("ί���������� nTotalBidVol: %I64d\n", marketData.nTotalBidVol);
		Print("ί���������� nTotalAskVol: %I64d\n", marketData.nTotalAskVol);

		Print("��Ȩƽ��ί��۸� nWeightedAvgBidPrice: %I64d\n", marketData.nWeightedAvgBidPrice);
		Print("��Ȩƽ��ί���۸� nWeightedAvgAskPrice: %I64d\n", marketData.nWeightedAvgAskPrice);

		Print("IOPV��ֵ��ֵ nIOPV: %d\n",  marketData.nIOPV);
		Print("���������� nYieldToMaturity: %d\n", marketData.nYieldToMaturity);
		Print("��ͣ�� nHighLimited: %I64d\n", marketData.nHighLimited);
		Print("��ͣ�� nLowLimited: %I64d\n", marketData.nLowLimited);
		Print("֤ȯ��Ϣǰ׺ chPrefix: %s\n", chararr2str(szBufSmall, sizeof(szBufSmall), (char*)marketData.chPrefix, ELEM_COUNT(marketData.chPrefix)));
		Print("��ӯ��1 nSyl1: %d\n", marketData.nSyl1);
		Print("��ӯ��2 nSyl2: %d\n", marketData.nSyl2);
		Print("����2���Ա���һ�ʣ� nSD2: %d\n", marketData.nSD2);
		if (nItems>1)
		{
			Print("\n");
		}
	}

	Print("\n");
}

void DumpScreenFuture(TDF_FUTURE_DATA* pFuture, int nItems)
{
	Print("-------- Future, Count:%d --------\n", nItems);
	char szBuf1[256];
	char szBuf2[256];
	char szBuf3[256];
	char szBuf4[256];

	for (int i=0; i<nItems; i++)
	{
		const TDF_FUTURE_DATA& futureData = pFuture[i];
		Print("��ô��� szWindCode: %s\n", futureData.szWindCode);

		if ((futureData.pCodeInfo->nType &0xf0 ) == 0x90 )
		{
			Print("��Ȩ��Լ���� szContractID:%s\n", futureData.pCodeInfo->exCodeInfo.Option.chContractID);
			Print("���֤ȯ���� szUnderlyingSecurityID:%s\n", futureData.pCodeInfo->exCodeInfo.Option.szUnderlyingSecurityID);
			Print("�Ϲ��Ϲ� chCallOrPut:%d(%c)\n", futureData.pCodeInfo->exCodeInfo.Option.chCallOrPut, SAFE_CHAR(futureData.pCodeInfo->exCodeInfo.Option.chCallOrPut));
			Print("��Ȩ�� nExerciseDate:%d\n", futureData.pCodeInfo->exCodeInfo.Option.nExerciseDate);
		}
		Print("ԭʼ���� szCode: %s\n", futureData.szCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", futureData.nActionDay);
		Print("������ nTradingDay: %d\n", futureData.nTradingDay);
		Print("ʱ��(HHMMSSmmm) nTime: %d\n", futureData.nTime);
		Print("״̬ nStatus: %d(%c)\n", futureData.nStatus, SAFE_CHAR(futureData.nStatus));

		Print("��ֲ� iPreOpenInterest: %I64d\n", futureData.iPreOpenInterest);
		Print("�����̼� nPreClose: %I64d\n", futureData.nPreClose);
		Print("����� nPreSettlePrice: %d\n", futureData.nPreSettlePrice);
		Print("���̼� nOpen: %I64d\n", futureData.nOpen);
		Print("��߼� nHigh: %I64d\n", futureData.nHigh);
		Print("��ͼ� nLow: %I64d\n", futureData.nLow);
		Print("���¼� nMatch: %I64d\n", futureData.nMatch);
		Print("�ɽ����� iVolume: %I64d\n", futureData.iVolume);
		Print("�ɽ��ܽ�� iTurnover: %I64d\n", futureData.iTurnover);
		Print("�ֲ����� iOpenInterest: %I64d\n", futureData.iOpenInterest);
		Print("������ nClose: %I64d\n", futureData.nClose);
		Print("����� nSettlePrice: %u\n", futureData.nSettlePrice);
		Print("��ͣ�� nHighLimited: %I64d\n", futureData.nHighLimited);
		Print("��ͣ�� nLowLimited: %I64d\n", futureData.nLowLimited);
		Print("����ʵ�� nPreDelta: %d\n", futureData.nPreDelta);
		Print("����ʵ�� nCurrDelta: %d\n", futureData.nCurrDelta);

		Print("������ nAskPrice: %s\n", int64arr2str(szBuf1, sizeof(szBuf1), (int64_t*)futureData.nAskPrice, ELEM_COUNT(futureData.nAskPrice)));
		Print("������ nAskVol: %s\n", int64arr2str(szBuf2, sizeof(szBuf2),(int64_t*)futureData.nAskVol, ELEM_COUNT(futureData.nAskVol)));
		Print("����� nBidPrice: %s\n", int64arr2str(szBuf3, sizeof(szBuf3),(int64_t*)futureData.nBidPrice, ELEM_COUNT(futureData.nBidPrice)));
		Print("������ nBidVol: %s\n", int64arr2str(szBuf4, sizeof(szBuf4),(int64_t*)futureData.nBidVol, ELEM_COUNT(futureData.nBidVol)));

		if (nItems>1)
		{
			Print("\n");
		}
	}

	Print("\n");
}

void DumpScreenIndex(TDF_INDEX_DATA* pIndex, int nItems)
{
	Print("-------- Index, Count:%d --------\n", nItems);

	for (int i=0; i<nItems; i++)
	{
		const TDF_INDEX_DATA& indexData = pIndex[i];
		Print("��ô��� szWindCode: %s\n", indexData.szWindCode);
		Print("ԭʼ���� szCode: %s\n", indexData.szCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", indexData.nActionDay);
		Print("������ nTradingDay: %d\n", indexData.nTradingDay);
		Print("ʱ��(HHMMSSmmm) nTime: %d\n", indexData.nTime);

		Print("����ָ�� nOpenIndex: %I64d\n", indexData.nOpenIndex);
		Print("���ָ�� nHighIndex: %I64d\n", indexData.nHighIndex);
		Print("���ָ�� nLowIndex: %I64d\n", indexData.nLowIndex);
		Print("����ָ�� nLastIndex: %I64d\n", indexData.nLastIndex);
		Print("�ɽ����� iTotalVolume: %I64d\n", indexData.iTotalVolume);
		Print("�ɽ��ܽ�� iTurnover: %I64d\n", indexData.iTurnover);
		Print("ǰ��ָ�� nPreCloseIndex: %I64d\n", indexData.nPreCloseIndex);

		if (nItems>1)
		{
			Print("\n");
		}
	}

	Print("\n");
}

void DumpScreenTransaction(TDF_TRANSACTION* pTransaction, int nItems)
{
	Print("-------- Transaction, Count:%d --------\n", nItems);

	for (int i=0; i<nItems; i++)
	{
		const TDF_TRANSACTION& transaction = pTransaction[i];
		Print("��ô��� szWindCode: %s\n", transaction.szWindCode);
		Print("ԭʼ���� szCode: %s\n", transaction.szCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", transaction.nActionDay);
		Print("ʱ��(HHMMSSmmm) nTime: %d\n", transaction.nTime);
		Print("�ɽ���� nIndex: %d\n", transaction.nIndex);
		Print("�ɽ��۸� nPrice: %I64d\n", transaction.nPrice);
		Print("�ɽ����� nVolume: %d\n", transaction.nVolume);
		Print("�ɽ���� nTurnover: %I64d\n", transaction.nTurnover);
		Print("�������� nBSFlag: %d(%c)\n", transaction.nBSFlag, SAFE_CHAR(transaction.nBSFlag));
		Print("�ɽ���� chOrderKind: %d(%c)\n", transaction.chOrderKind, SAFE_CHAR(transaction.chOrderKind));
		Print("�ɽ����� chFunctionCode: %d(%c)\n", transaction.chFunctionCode, SAFE_CHAR(transaction.chFunctionCode));
		Print("������ί����� nAskOrder: %d\n", transaction.nAskOrder);
		Print("����ί����� nBidOrder: %d\n", transaction.nBidOrder);

		if (nItems>1)
		{
			Print("\n");
		}
	}

	Print("\n");
}

void DumpScreenOrder(TDF_ORDER* pOrder, int nItems)
{
    Print("-------- Order, Count:%d --------\n", nItems);

    for (int i=0; i<nItems; i++)
    {
        const TDF_ORDER& order = pOrder[i];
        Print("��ô��� szWindCode: %s\n", order.szWindCode);
        Print("ԭʼ���� szCode: %s\n", order.szCode);
        Print("ҵ������(��Ȼ��) nActionDay: %d\n", order.nActionDay);
        Print("ʱ��(HHMMSSmmm) nTime: %d\n", order.nTime);
        Print("ί�к� nOrder: %d\n", order.nOrder);
        Print("ί�м۸� nPrice: %I64d\n", order.nPrice);
        Print("ί������ nVolume: %d\n", order.nVolume);
        Print("ί����� chOrderKind: %d(%c)\n", order.chOrderKind, SAFE_CHAR(order.chOrderKind));
		Print("ί�д��� chFunctionCode: %d(%c)\n", order.chFunctionCode, SAFE_CHAR(order.chFunctionCode));
		Print("�����̱��� nBroker: %d\n", order.nBroker);
		Print("ί��״̬ chStatus: %d(%c)\n", order.chStatus, SAFE_CHAR(order.chStatus));
		Print("��־ chFlag: %d(%c)\n", order.chFlag, SAFE_CHAR(order.chFlag));

        if (nItems>1)
        {
            Print("\n");
        }
    }

    Print("\n");
}

void DumpScreenBrokerQueue(TD_BrokerQueue* pBrokerQueue, int nItems)
{
	Print("-------- BrokerQueue, Count:%d --------\n", nItems);

	char szBuf[3200];
	for (int i=0; i<nItems; i++)
	{
		const TD_BrokerQueue& brokerQueue = pBrokerQueue[i];
		Print("��ô��� szWindCode: %s\n", brokerQueue.szWindCode);
		Print("ԭʼ���� szCode: %s\n", brokerQueue.szCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", brokerQueue.nActionDay);

		Print("����ʱ�� nAskTime: %d\n", brokerQueue.nAskTime);
		Print("����ʱ�� nBidTime: %d\n", brokerQueue.nBidTime);
		Print("�������͸��� nAskBrokers: %d\n", brokerQueue.nAskBrokers);
		Print("���򾭼͸��� nBidBrokers: %d\n", brokerQueue.nBidBrokers);

		Print("������ϸ sAskBroker: %s\n", shortarr2str(szBuf,sizeof(szBuf), (short*)brokerQueue.sAskBroker, brokerQueue.nAskBrokers));
		Print("������ϸ sBidBroker: %s\n", shortarr2str(szBuf,sizeof(szBuf), (short*)brokerQueue.sBidBroker, brokerQueue.nBidBrokers));

		if (nItems>1)
		{
			Print("\n");
		}
	}

	Print("\n");
}

void DumpScreenOrderQueue(TDF_ORDER_QUEUE* pOrderQueue, int nItems)
{
    Print("-------- Order, Count:%d --------\n", nItems);

    char szBuf[3200];
    for (int i=0; i<nItems; i++)
    {
        const TDF_ORDER_QUEUE& orderQueue = pOrderQueue[i];
        Print("��ô��� szWindCode: %s\n", orderQueue.szWindCode);
        Print("ԭʼ���� szCode: %s\n", orderQueue.szCode);
        Print("ҵ������(��Ȼ��) nActionDay: %d\n", orderQueue.nActionDay);
        Print("ʱ��(HHMMSSmmm) nTime: %d\n", orderQueue.nTime);

        Print("�������� nSide: %d(%c)\n", orderQueue.nSide, SAFE_CHAR(orderQueue.nSide));
        Print("ί�м۸� nPrice: %I64d\n", orderQueue.nPrice);
        Print("�������� nOrders: %d\n", orderQueue.nOrders);
        Print("��ϸ���� nOrder: %d\n", orderQueue.nABItems);

        Print("������ϸ nVolume: %s\n", intarr2str(szBuf,sizeof(szBuf), (int*)orderQueue.nABVolume, MIN(ELEM_COUNT(orderQueue.nABVolume),orderQueue.nABItems)));

        if (nItems>1)
        {
            Print("\n");
        }
    }

    Print("\n");
}

void DumpScreenBBQStep(TDF_BBQTRANSACTION_DATA* pBBQStep, int nItems)
{
	Print("-------- TDF_BBQTRANSACTION_DATA, Count:%d --------\n", nItems);

	for (int i=0; i<nItems; i++)
	{
		const TDF_BBQTRANSACTION_DATA& BBQStep = pBBQStep[i];
		Print("��ô��� szWindCode: %s\n", BBQStep.szWindCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", BBQStep.nActionDay);
		Print("ʱ��(HHMMSSmmm) nTime: %d\n", BBQStep.nTime);
		/*Print("ί�к� nOrder: %d\n", order.nOrder);
		Print("ί�м۸� nPrice: %dI64dn", order.nPrice);
		Print("ί������ nVolume: %d\n", order.nVolume);
		Print("ί����� chOrderKind: %d(%c)\n", order.chOrderKind, SAFE_CHAR(order.chOrderKind));
		Print("ί�д��� chFunctionCode: %d(%c)\n", order.chFunctionCode, SAFE_CHAR(order.chFunctionCode));*/

		if (nItems>1)
		{
			Print("\n");
		}
	}

	Print("\n");
}

void DumpScreenBBQBid(TDF_BBQBID_DATA* pBBQBid, int nItems)
{
	Print("-------- TDF_BBQBID_DATA, Count:%d --------\n", nItems);

	for (int i=0; i<nItems; i++)
	{
		const TDF_BBQBID_DATA& BBQBid = pBBQBid[i];
		Print("��ô��� szWindCode: %s\n", BBQBid.szWindCode);
		Print("ҵ������(��Ȼ��) nActionDay: %d\n", BBQBid.nActionDay);
		Print("ʱ��(HHMMSSmmm) nTime: %d\n", BBQBid.nTime);
		/*Print("ί�к� nOrder: %d\n", order.nOrder);
		Print("ί�м۸� nPrice: %I64d\n", order.nPrice);
		Print("ί������ nVolume: %d\n", order.nVolume);
		Print("ί����� chOrderKind: %d(%c)\n", order.chOrderKind, SAFE_CHAR(order.chOrderKind));
		Print("ί�д��� chFunctionCode: %d(%c)\n", order.chFunctionCode, SAFE_CHAR(order.chFunctionCode));*/

		if (nItems>1)
		{
			Print("\n");
		}
	}

	Print("\n");
}