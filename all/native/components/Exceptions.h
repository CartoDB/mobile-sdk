/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_EXCEPTIONS_H_
#define _CARTO_EXCEPTIONS_H_

#include <stdexcept>

namespace carto {

    class ParseException : public std::runtime_error {
    public:
        explicit ParseException(const std::string& msg, const std::string& string, int position = -1) : runtime_error(msg), _string(string), _position(position) { }

        const std::string& getString() const {
            return _string;
        }

        int getPosition() const {
            return _position;
        }

    private:
        std::string _string;
        int _position;
    };      

    class GenerateException : public std::runtime_error {
    public:
        explicit GenerateException(const std::string& msg) : runtime_error(msg) { }
    };      

    class FileException : public std::runtime_error {
    public:
        explicit FileException(const std::string& msg, const std::string& fileName) : runtime_error(msg), _fileName(fileName) { }

        const std::string& getFileName() const {
            return _fileName;
        }

    private:
        std::string _fileName;
    };

    class NetworkException : public std::runtime_error {
    public:
        explicit NetworkException(const std::string& msg, const std::string& host) : runtime_error(msg), _host(host) { }

        const std::string& getHost() const {
            return _host;
        }

    private:
        std::string _host;
    };

    class GenericException : public std::runtime_error {
    public:
        explicit GenericException(const std::string& msg, const std::string& details = std::string()) : runtime_error(msg), _details(details) { }

        const std::string& getDetails() const {
            return _details;
        }

    private:
        std::string _details;
    };

}

#endif
