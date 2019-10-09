////////////////////////////////////////////////////////////////////////
//  LogTrace.cpp -- Interface for the CLogTrace class
//  A class to do debug logging

#ifndef __LOGTRACE_H__
#define __LOGTRACE_H__

class CLogTrace
{
// Construction/Destruction
public:
    CLogTrace();
    ~CLogTrace();

// Attributes
public:
    CString m_strAppName;
    LARGE_INTEGER litmp;
    LONGLONG QPart1,QPart2;
    double dfMinus, dfFreq, dfTim;

protected:
    BOOL m_bActive;
    CString m_strFileName;
    BOOL m_bTimeStamp;

// Operations
public:
    void WriteLine(LPCTSTR szLine);
    void WriteLine(LPCTSTR szFormat, LPCTSTR szAddInfo);
    void WriteLine(LPCTSTR szFormat, int nAddInfo);
    void ResetFile();
    void OnStartup(BOOL bActive, BOOL bTimeStamp);
    void SetFileName(LPCTSTR szFileName);

protected:

// Inlines
public:
    void EndCalculateTime();
    void StartCalculateTime();
    inline void SetActive(BOOL bSet)
    {
        m_bActive = bSet;
    }
    inline CString GetFileName()
    {
        return m_strFileName;
    }
};

#endif // __LOGTRACE_H__