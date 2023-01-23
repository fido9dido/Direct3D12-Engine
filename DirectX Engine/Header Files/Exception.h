#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <windows.h>
#include <codecvt>
#include <locale>

inline bool IsFail(HRESULT hr)
{
	if (hr < 0)
	{
		return true;
	}
	return false;
}
class CException : std::exception
{
public:
	CException() = default;
	CException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

	//todo 
	const char* what() const throw() { return 0; }

	std::wstring ToString(void)const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

//microsoft
inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}


class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error(void) const { return m_hr; }
private:
	const HRESULT m_hr;
};


//mine 
inline HRESULT ThrowIfFailed(HRESULT hr)
{
	if (IsFail(hr))
	{
		throw HrException(hr);
	}
	
}



#endif