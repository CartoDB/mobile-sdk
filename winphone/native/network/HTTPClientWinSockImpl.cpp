#include "HTTPClientWinSockImpl.h"
#include "components/Exceptions.h"
#include "utils/Log.h"

#include <cstddef>
#include <chrono>
#include <limits>
#include <regex>

#include <boost/algorithm/string.hpp>

#include <utf8.h>

#include <windows.h>
#include <wrl.h>
#include <msxml6.h>
#include <windows.storage.streams.h>

namespace {

    static std::wstring to_wstring(const std::string& str) {
        std::wstring wstr;
        utf8::utf8to16(str.begin(), str.end(), std::back_inserter(wstr));
        return wstr;
    }

    static std::string to_string(const std::wstring& wstr) {
        std::string str;
        utf8::utf16to8(wstr.begin(), wstr.end(), std::back_inserter(str));
        return str;
    }

    class HTTPPostStream : public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >,
        ISequentialStream>
    {
    public:
        HRESULT RuntimeClassInitialize(
            const void* pv,
            ULONG cb)
        {
            m_data = std::vector<unsigned char>(static_cast<const unsigned char*>(pv), static_cast<const unsigned char*>(pv) + cb);
            return S_OK;
        }

        virtual ~HTTPPostStream() {
        }

        STDMETHODIMP Read(
            __out_bcount_part(cb, *pcbRead) void *pv,
            ULONG cb,
            __out_opt  ULONG *pcbRead)
        {
            HRESULT hr = S_OK;

            for (*pcbRead = 0; *pcbRead < cb; ++*pcbRead, ++m_buffSeekIndex) {
                if (m_buffSeekIndex == m_data.size()) {
                    hr = S_FALSE;
                    break;
                }

                static_cast<unsigned char*>(pv)[*pcbRead] = m_data[m_buffSeekIndex];
            }
            return hr;
        }

        STDMETHODIMP Write(
            __in_bcount(cb)  const void *pv,
            ULONG cb,
            __out_opt  ULONG *pcbWritten)
        {
            return E_NOTIMPL;
        }

        STDMETHODIMP GetSize(_Out_ ULONGLONG *pullSize)
        {
            if (pullSize == NULL) {
                return E_INVALIDARG;
            }

            *pullSize = m_data.size();
            return S_OK;
        }

    private:
        std::size_t m_buffSeekIndex = 0;
        std::vector<unsigned char> m_data;
    };

    class HTTPCallbackProxy : public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags< Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >,
        IXMLHTTPRequest2Callback>
    {
    public:
        typedef std::function<bool(int, const std::map<std::string, std::string>&)> HeadersFn;
        typedef std::function<bool(const unsigned char*, size_t)> DataFn;
        typedef std::function<void(bool)> FinishFn;

        HRESULT RuntimeClassInitialize(
            HeadersFn headersFn,
            DataFn dataFn,
            FinishFn finishFn)
        {
            _headersFn = headersFn;
            _dataFn = dataFn;
            _finishFn = finishFn;
            return S_OK;
        }

        virtual ~HTTPCallbackProxy() {
        }

        STDMETHODIMP OnRedirect(
            IXMLHTTPRequest2 *pXHR,
            const WCHAR *pwszRedirectUrl)
        {
            return S_OK;
        }

        STDMETHODIMP OnHeadersAvailable(
            IXMLHTTPRequest2 *pXHR,
            DWORD dwStatus,
            const WCHAR *pwszStatus)
        {
            WCHAR* pwszHeaders = nullptr;

            HRESULT hr = pXHR->GetAllResponseHeaders(&pwszHeaders);
            if (FAILED(hr)) {
                _finishFn(false);
                return E_ABORT;
            }

            std::vector<std::string> headersVector;
            boost::split(headersVector, to_string(pwszHeaders), boost::is_any_of("\r\n"));
            std::map<std::string, std::string> headers;
            for (const std::string& header : headersVector) {
                std::string::size_type pos = header.find(':');
                if (pos != std::string::npos) {
                    std::string key = boost::trim_copy(header.substr(0, pos));
                    std::string value = boost::trim_copy(header.substr(pos + 1));
                    headers[key] = value;
                }
            }

            if (!_headersFn(dwStatus, headers)) {
                _finishFn(false);
                return E_ABORT;
            }
            return S_OK;
        }

        STDMETHODIMP OnDataAvailable(
            IXMLHTTPRequest2 *pXHR,
            ISequentialStream *pResponseStream)
        {
            std::vector<unsigned char> data;
            while (true) {
                unsigned char buf[1024];
                unsigned long bytesRead = 0;
                HRESULT hr = pResponseStream->Read(buf, sizeof(buf), &bytesRead);
                if (FAILED(hr)) {
                    _finishFn(false);
                    return E_ABORT;
                }

                if (!bytesRead) {
                    break;
                }

                data.insert(data.end(), &buf[0], &buf[bytesRead]);
            }

            if (!_dataFn(data.data(), data.size())) {
                _finishFn(false);
                return E_ABORT;
            }
            return S_OK;
        }

        STDMETHODIMP OnResponseReceived(
            IXMLHTTPRequest2 *pXHR,
            ISequentialStream *pResponseStream)
        {
            _finishFn(true);
            return S_OK;
        }

        STDMETHODIMP OnError(
            IXMLHTTPRequest2 *pXHR,
            HRESULT hrError)
        {
            _finishFn(false);
            return S_OK;
        }

    private:
        HeadersFn _headersFn;
        DataFn _dataFn;
        FinishFn _finishFn;
    };

}

namespace carto {

    HTTPClient::WinSockImpl::WinSockImpl(bool log) :
        _log(log)
    {
    }

    bool HTTPClient::WinSockImpl::makeRequest(const HTTPClient::Request& request, HeadersFn headersFn, DataFn dataFn) const {
        MULTI_QI mqi = { 0 };

        mqi.hr = S_OK;
        mqi.pIID = &__uuidof(IXMLHTTPRequest2);

        HRESULT hr = CoCreateInstanceFromApp(CLSID_FreeThreadedXMLHTTP60,
            nullptr,
            CLSCTX_INPROC_SERVER,
            nullptr,
            1,
            &mqi);
        if (FAILED(hr) || FAILED(mqi.hr)) {
            throw NetworkException("Failed to create XMLHTTP object", request.url);
        }

        Microsoft::WRL::ComPtr<IXMLHTTPRequest2> httpRequest2;
        httpRequest2.Attach(static_cast<IXMLHTTPRequest2*>(mqi.pItf));

        std::shared_ptr<std::remove_pointer<::HANDLE>::type> resultEvent(::CreateEventExA(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE), &::CloseHandle);

        bool cancel = false;
        auto finishFn = [&](bool success) {
            cancel = !success;
            ::SetEvent(resultEvent.get());
        };
        
        Microsoft::WRL::ComPtr<HTTPCallbackProxy> callbackProxy;
        hr = Microsoft::WRL::MakeAndInitialize<HTTPCallbackProxy>(&callbackProxy, headersFn, dataFn, finishFn);
        if (FAILED(hr)) {
            throw NetworkException("Failed to initialize callback proxy", request.url);
        }

        for (auto it = request.headers.begin(); it != request.headers.end(); it++) {
            httpRequest2->SetRequestHeader(to_wstring(it->first).c_str(), to_wstring(it->second).c_str());
        }

        hr = httpRequest2->Open(to_wstring(request.method).c_str(),
            to_wstring(request.url).c_str(),
            callbackProxy.Get(),
            nullptr,
            nullptr,
            nullptr,
            nullptr);

        if (FAILED(hr)) {
            throw NetworkException("Failed to open HTTP connection", request.url);
        }

        if (!request.contentType.empty()) {
            Microsoft::WRL::ComPtr<HTTPPostStream> postStream;
            hr = Microsoft::WRL::MakeAndInitialize<HTTPPostStream>(&postStream, request.body.data(), request.body.size());
            if (FAILED(hr)) {
                throw NetworkException("Failed to initialize HTTP POST stream", request.url);
            }

            hr = httpRequest2->Send(postStream.Get(), request.body.size());
        }
        else {
            hr = httpRequest2->Send(nullptr, 0);
        }

        if (FAILED(hr)) {
            throw NetworkException("Failed to send HTTP request", request.url);
        }

        ::WaitForSingleObjectEx(resultEvent.get(), INFINITE, FALSE);

        return !cancel;
    }

}
